#pragma once

#include <vector>
#include <memory>
#include <limits>
#include <iomanip>
#include <set>

#include <march.h>

#include "vof.h"
#include "geom/mesh.h"
#include "dump/vtk.h"
#include "solver/reconst.h"
#include "debug/isnan.h"

namespace solver {

template <class M_>
struct UVof<M_>::Imp {
  using R = Reconst<Scal>;
  static constexpr size_t dim = M::dim;
  static constexpr Scal kClNone = -1;

  Imp() = default;
  ~Imp() = default;

  void DumpPoly(
      const GRange<size_t>& layers,
      const Multi<const FieldCell<Scal>*>& fcu,
      const Multi<const FieldCell<Scal>*>& fccl,
      const Multi<const FieldCell<Vect>*>& fcn,
      const Multi<const FieldCell<Scal>*>& fca,
      const Multi<const FieldCell<bool>*>& fci,
      std::string fn, Scal t, Scal th, bool bin, bool merge, M& m) {
    (void) th;
    auto sem = m.GetSem("dumppoly");
    if (sem("local")) {
      dl_.clear();
      dlc_.clear();
      dll_.clear();
      dlcl_.clear();
      auto h = m.GetCellSize();
      for (auto i : layers) {
        for (auto c : m.Cells()) {
          Scal u = (*fcu[i])[c];
          if (IsNan(u) || IsNan((*fcn[i])[c]) || IsNan((*fca[i])[c])) {
            continue;
          }
          if ((*fci[i])[c]) {
            dl_.push_back(R::GetCutPoly(
                  m.GetCenter(c), (*fcn[i])[c], (*fca[i])[c], h));
            dlc_.push_back(m.GetHash(c));
            dll_.push_back(i);
            dlcl_.push_back(fccl[i] ? (*fccl[i])[c] : 0);
          }
        }
      }
      using TV = typename M::template OpCatVT<Vect>;
      m.Reduce(std::make_shared<TV>(&dl_));
      using TS = typename M::template OpCatT<Scal>;
      m.Reduce(std::make_shared<TS>(&dlc_));
      m.Reduce(std::make_shared<TS>(&dll_));
      m.Reduce(std::make_shared<TS>(&dlcl_));
    }
    if (sem("write")) {
      if (m.IsRoot()) {
        std::cout << std::fixed << std::setprecision(8)
            << "dump" << " t=" << t << " to " << fn << std::endl;
        WriteVtkPoly<Vect>(fn, dl_, nullptr, 
            {&dlc_, &dll_, &dlcl_}, {"c", "l", "cl"},
            "Interface from PLIC", true,
            bin, merge);
      }
    }
  }

  // uu: volume fraction in nodes
  // xc: cell center
  // h: cell size
  // iso: isovalue for surface uu=iso
  void GetMarchTriangles(
      const std::array<Scal, 8>& uu, const std::array<Vect, 8>& nn,
      const Vect& xc, 
      const Vect& h, Scal iso,
      std::vector<std::vector<Vect>>& vv,
      std::vector<std::vector<Vect>>& vvn) {
    std::array<double, 8> uuz = uu;
    for (auto& u : uuz) {
      u -= iso;
    }
    int nt;
    const int ms = MARCH_NTRI;
    std::array<double, ms> tri;
    march_cube(uuz.data(), &nt, tri.data());
    std::array<int, ms> vc0;
    std::array<int, ms> vc1;
    std::array<double, ms> vw;
    march_cube_location(vc0.data(), vc1.data(), vw.data());
    assert(n * 3 * 3 <= tri.size());

    vv.resize(nt);
    {
      size_t i = 0;
      for (auto& v : vv) {
        v.resize(3);
        for (auto& x : v) {
          x[0] = tri[i++] - 0.5;
          x[1] = tri[i++] - 0.5;
          x[2] = tri[i++] - 0.5;
          x = xc + h * x;
        }
      }
    }
    vvn.resize(nt);
    {
      size_t i = 0;
      for (auto& vn : vvn) {
        vn.resize(3);
        for (auto& n : vn) {
          Scal w = vw[i];
          int c0 = vc0[i];
          int c1 = vc1[i];
          ++i;
          n = nn[c0] * (1 - w) + nn[c1] * w;
        }
      }
    }
  }
  // Interpolates from cells to nodes.
  // stencil half-width
  template <int sw, int sn=sw*2+1, int snn=sw*2>
  std::array<Scal, snn*snn*snn> ToNodes(const std::array<Scal, sn*sn*sn>& uu) {
    std::array<Scal, snn*snn*snn> uun;
    size_t i = 0;
    for (int z = 0; z < snn; ++z) {
      for (int y = 0; y < snn; ++y) {
        for (int x = 0; x < snn; ++x) {
          auto u = [&uu,x,y,z](int dx, int dy, int dz) {
            return uu[(z+dz)*sn*sn + (y+dy)*sn + (x+dx)];
          };
          uun[i++] = (1. / 8.) * (
              u(0,0,0) + u(1,0,0) + u(0,1,0) + u(1,1,0) +
              u(0,0,1) + u(1,0,1) + u(0,1,1) + u(1,1,1));
        }
      }
    }
    return uun;
  }
  // Interpolates from cells to nodes.
  // stencil half-width
  template <int sw, int sn=sw*2+1, int snn=sw*2>
  std::array<Vect, snn*snn*snn> GradientNodes(const std::array<Scal, sn*sn*sn>& uu) {
    std::array<Vect, snn*snn*snn> gg;
    size_t i = 0;
    for (int z = 0; z < snn; ++z) {
      for (int y = 0; y < snn; ++y) {
        for (int x = 0; x < snn; ++x) {
          auto u = [&uu,x,y,z](int dx, int dy, int dz) {
            return uu[(z+dz)*sn*sn + (y+dy)*sn + (x+dx)];
          };
          auto& g = gg[i++];
          g[0] = ((u(1,0,0)+ u(1,1,0)+ u(1,0,1)+ u(1,1,1)) -
                   (u(0,0,0)+ u(0,1,0)+ u(0,0,1)+ u(0,1,1))) * 0.25;
          g[1] = ((u(0,1,0)+ u(1,1,0)+ u(0,1,1)+ u(1,1,1)) -
                   (u(0,0,0)+ u(1,0,0)+ u(0,0,1)+ u(1,0,1))) * 0.25;
          g[2] = ((u(0,0,1)+ u(1,0,1)+ u(0,1,1)+ u(1,1,1)) -
                   (u(0,0,0)+ u(1,0,0)+ u(0,1,0)+ u(1,1,0))) * 0.25;
        }
      }
    }
    return gg;
  }
    // Returns values over stencil centered at cell c with color cl.
    // Values for neighbors without color cl are filled with 0.
    // sw: stencil half-width
    template <size_t sw>
    struct GetStencilPure {
      static constexpr size_t sn = sw * 2 + 1;
      std::array<Scal, sn*sn*sn> operator()(
          const FieldCell<Scal>& fc, IdxCell c, const M& m) {
        using MIdx = typename M::MIdx;
        auto& bc = m.GetIndexCells();
        GBlock<IdxCell, M::dim> bo(MIdx(-sw), MIdx(sn));
        MIdx w = bc.GetMIdx(c);
        std::array<typename M::Scal, sn*sn*sn> uu;
        size_t k = 0;
        for (MIdx wo : bo) {
          IdxCell cn = bc.GetIdx(w + wo);
          uu[k++] = fc[cn];
        }
        return uu;
      }
    };
  // bcfill: if >=0. add triangles from 
  // fcus [a]: sum of volume fractions, add triangles from SuCells if not null
  void DumpPolyMarch(
      const GRange<size_t>& layers,
      const Multi<const FieldCell<Scal>*>& fcu,
      const Multi<const FieldCell<Scal>*>& fccl,
      const Multi<const FieldCell<Vect>*>& fcn,
      const Multi<const FieldCell<Scal>*>& fca,
      const Multi<const FieldCell<bool>*>& fci,
      std::string fn, Scal t, Scal th, bool bin, bool merge, Scal iso, 
      const FieldCell<Scal>* fcus, M& m) {
    (void) fcn;
    (void) fca;
    (void) fci;
    (void) th;
    auto sem = m.GetSem("dumppolymarch");
    if (sem("local")) {
      dl_.clear();
      dln_.clear();
      dlc_.clear();
      dll_.clear();
      dlcl_.clear();
      // visited cells without color
      std::set<std::pair<size_t, Scal>> done;
      auto h = m.GetCellSize();
      FieldCell<bool> in(m, false); // inner cell
      for (auto c : m.Cells()) {
        in[c] = true;
      }
      for (auto i : layers) {
        for (auto c : m.Cells()) {
          Scal cl = (*fccl[i])[c];
          if (cl != kClNone) {
            const size_t sw = 1; // stencil halfwidth
            const size_t sn = sw * 2 + 1;
            auto& bc = m.GetIndexCells();
            using MIdx = typename M::MIdx;
            GBlock<IdxCell, dim> bo(MIdx(-sw), MIdx(sn));
            MIdx w = bc.GetMIdx(c);
            for (MIdx wo : bo) {
              IdxCell cn = bc.GetIdx(w + wo);
              if (!in[cn]) continue;
              bool q = false;
              for (auto j : layers) {
                if ((*fccl[j])[cn] == cl) {
                  q = true;
                  break;
                }
              }
              // Add triangles from c or neighhbor without color
              if (c == cn || !q) {
                auto e = std::make_pair(size_t(cn), cl);
                if (!q && done.count(e)) {
                  continue;
                }
                done.insert(e);
                auto uu = GetStencil<M, 1>{}(layers, fcu, fccl, cn, cl, m);
                auto uun = ToNodes<1>(uu);
                auto nn = GradientNodes<1>(uu);
                for (auto& n : nn) {
                  n = -n;
                }
                std::vector<std::vector<Vect>> vv, vvn;
                GetMarchTriangles(uun, nn, m.GetCenter(cn), h, iso, vv, vvn);
                for (size_t j = 0; j < vv.size(); ++j) {
                  dl_.push_back(vv[j]);
                  dln_.push_back(vvn[j]);
                  dlc_.push_back(m.GetHash(cn));
                  dll_.push_back(i);
                  dlcl_.push_back(cl);
                }
              }
            }
          }
        }
      }
      if (fcus) {
        // Append triangles on boundaries
        auto& bc = m.GetIndexCells();
        using MIdx = typename M::MIdx;
        for (auto c : m.SuCells()) {
          MIdx w = bc.GetMIdx(c);
          if (!(MIdx(1) <= w && w < m.GetGlobalSize() - MIdx(1))) {
            auto uu = GetStencilPure<1>{}(*fcus, c, m);
            auto uun = ToNodes<1>(uu);
            auto nn = GradientNodes<1>(uu);
            for (auto& n : nn) { n = -n; }
            std::vector<std::vector<Vect>> vv, vvn;
            GetMarchTriangles(uun, nn, m.GetCenter(c), h, iso, vv, vvn);
            for (size_t j = 0; j < vv.size(); ++j) {
              dl_.push_back(vv[j]);
              dln_.push_back(vvn[j]);
              dlc_.push_back(m.GetHash(c));
              dll_.push_back(-1);
              dlcl_.push_back(-1);
            }
          }
        }
      }
      using TV = typename M::template OpCatVT<Vect>;
      m.Reduce(std::make_shared<TV>(&dl_));
      m.Reduce(std::make_shared<TV>(&dln_));
      using TS = typename M::template OpCatT<Scal>;
      m.Reduce(std::make_shared<TS>(&dlc_));
      m.Reduce(std::make_shared<TS>(&dll_));
      m.Reduce(std::make_shared<TS>(&dlcl_));
    }
    if (sem("write")) {
      if (m.IsRoot()) {
        std::cout << std::fixed << std::setprecision(8)
            << "dump" << " t=" << t << " to " << fn << std::endl;
        WriteVtkPoly<Vect>(fn, dl_, &dln_,
            {&dlc_, &dll_, &dlcl_}, {"c", "l", "cl"}, 
            "Interface from marching cubes", true, bin, merge);
      }
    }
  }

  void Recolor(const GRange<size_t>& layers,
      const Multi<const FieldCell<Scal>*>& fcu,
      const Multi<FieldCell<Scal>*>& fccl,
      Scal clfixed, Vect clfixed_x, Scal coalth,
      const MapFace<std::shared_ptr<CondFace>>& mfc,
      bool bcc_reflect, bool verb, M& m) {
    constexpr Scal kClNone = -1;
    auto sem = m.GetSem("recolor");
    if (sem("reduce")) {
      fcclt_.resize(layers.size());
      // nearest block nearest to clfixed_x
      if (clfixed >= 0) {
        IdxCell c = m.FindNearestCell(clfixed_x);
        cldist_.first = m.GetCenter(c).dist(clfixed_x);
        cldist_.second = m.GetId();
        m.Reduce(std::make_shared<typename M::OpMinloc>(&cldist_));
      } else {
        cldist_.second = -1;
      }
    }
    if (sem("init")) {
      fcclt_.InitAll(FieldCell<Scal>(m, kClNone));
      // initial unique color
      Scal q = m.GetId() * m.GetInBlockCells().size() * layers.size();
      for (auto i : layers) {
        for (auto c : m.Cells()) {
          if ((*fccl[i])[c] != kClNone) {
            fcclt_[i][c] = (q += 1);
          }
        }
        if (cldist_.second == m.GetId()) {
          IdxCell c = m.FindNearestCell(clfixed_x);
          if ((*fccl[i])[c] != kClNone) {
            fcclt_[i][c] = clfixed;
          }
        }
        m.Comm(&fcclt_[i]);
      }

      // detect overlap
      for (auto i : layers) {
        for (auto c : m.Cells()) {
          if ((*fccl[i])[c] != kClNone) {
            for (auto j : layers) {
              if (j != i && (*fccl[j])[c] != kClNone) {
                if ((*fcu[i])[c] + (*fcu[j])[c] > coalth) {
                  Scal cl = std::min(fcclt_[i][c], fcclt_[j][c]);
                  fcclt_[i][c] = cl;
                  fcclt_[j][c] = cl;
                }
              }
            }
          }
        }
      }
    }
    sem.LoopBegin();
    if (sem("min")) {
      const int sw = 1; // stencil halfwidth
      using MIdx = typename M::MIdx;
      auto& bc = m.GetIndexCells();
      GBlock<IdxCell, dim> bo(MIdx(-sw), MIdx(sw * 2 + 1));
      size_t tries = 0;
      size_t cells = 0;
      while (true) {
        bool chg = false;
        for (auto i : layers) {
          for (auto c : m.Cells()) {
            if ((*fccl[i])[c] != kClNone) {
              MIdx w = bc.GetMIdx(c);
              // update color with minimum over neighbours
              for (MIdx wo : bo) {
                IdxCell cn = bc.GetIdx(w + wo);
                for (auto j : layers) {
                  if ((*fccl[j])[cn] == (*fccl[i])[c]) {
                    if (fcclt_[j][cn] < fcclt_[i][c]) {
                      chg = true;
                      ++cells;
                      fcclt_[i][c] = fcclt_[j][cn];
                    }
                  }
                }
              }
            }
          }
        }
        if (!chg) {
          break;
        }
        ++tries;
      }
      for (auto i : layers) {
        m.Comm(&fcclt_[i]);
      }
      recolor_tries_ = tries;
      m.Reduce(&recolor_tries_, "max");
    }
    if (sem("check")) {
      if (verb && m.IsRoot()) {
        std::cerr << "recolor:"
          << " max tries: " << recolor_tries_ 
          << std::endl;
      }
      if (!recolor_tries_) {
        sem.LoopBreak();
      }
    }
    sem.LoopEnd();
    if (sem("free")) {
      for (auto i : layers) {
        for (auto c : m.AllCells()) {
          (*fccl[i])[c] = fcclt_[i][c];
        }
        fcclt_[i].Free();
      }
    }
    if (bcc_reflect && sem("reflect")) {
      for (auto i : layers) {
        BcReflect(*fccl[i], mfc, kClNone, false, m);
      }
    }
  }

  std::vector<std::vector<Vect>> dl_; // dump poly
  std::vector<std::vector<Vect>> dln_; // dump poly normals
  std::vector<Scal> dlc_; // dump poly cell
  std::vector<Scal> dll_; // dump poly layer
  std::vector<Scal> dlcl_; // dump poly color

  Scal recolor_tries_;
  std::pair<typename M::Scal, int> cldist_; // color,mesh_id
  Multi<FieldCell<Scal>> fcclt_;  // tmp color
};

template <class M_>
UVof<M_>::UVof() : imp(new Imp()) {}

template <class M_>
UVof<M_>::~UVof() = default;

template <class M_>
void UVof<M_>::DumpPoly(
    const GRange<size_t>& layers,
    const Multi<const FieldCell<Scal>*>& fcu,
    const Multi<const FieldCell<Scal>*>& fccl,
    const Multi<const FieldCell<Vect>*>& fcn,
    const Multi<const FieldCell<Scal>*>& fca,
    const Multi<const FieldCell<bool>*>& fci,
    std::string fn, Scal t, Scal th, bool bin, bool merge, M& m) {
  imp->DumpPoly(layers, fcu, fccl, fcn, fca, fci, fn, t, th, bin, merge, m);
}

template <class M_>
void UVof<M_>::DumpPoly(
    const FieldCell<Scal>& fcu, const FieldCell<Vect>& fcn,
    const FieldCell<Scal>& fca, const FieldCell<bool>& fci,
    std::string fn, Scal t, Scal th, bool bin, bool merge, M& m) {
  GRange<size_t> layers(0, 1);
  const FieldCell<Scal>* fccl(nullptr);
  imp->DumpPoly(layers, &fcu, fccl, &fcn, &fca, &fci,
                fn, t, th, bin, merge, m);
}

template <class M_>
void UVof<M_>::DumpPolyMarch(
    const GRange<size_t>& layers,
    const Multi<const FieldCell<Scal>*>& fcu,
    const Multi<const FieldCell<Scal>*>& fccl,
    const Multi<const FieldCell<Vect>*>& fcn,
    const Multi<const FieldCell<Scal>*>& fca,
    const Multi<const FieldCell<bool>*>& fci,
    std::string fn, Scal t, Scal th, bool bin, bool merge, Scal iso, 
    const FieldCell<Scal>* fcus, M& m) {
  imp->DumpPolyMarch(
      layers, fcu, fccl, fcn, fca, fci, fn, t, th, bin, merge, iso, fcus, m);
}

template <class M_>
void UVof<M_>::Recolor(const GRange<size_t>& layers,
    const Multi<const FieldCell<Scal>*>& fcu,
    const Multi<FieldCell<Scal>*>& fccl,
    Scal clfixed, Vect clfixed_x, Scal coalth,
    const MapFace<std::shared_ptr<CondFace>>& mfcu,
    bool bcc_reflect, bool verb, M& m) {
  imp->Recolor(layers, fcu, fccl, clfixed, clfixed_x, coalth, mfcu, 
               bcc_reflect, verb, m);
}


template <class M>
constexpr typename M::Scal UVof<M>::Imp::kClNone;

} // namespace solver
