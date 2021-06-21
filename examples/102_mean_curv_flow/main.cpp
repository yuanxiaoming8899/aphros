// Created by Petr Karnakov on 28.12.2019
// Copyright 2019 ETH Zurich

#undef NDEBUG
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <string>

#include <debug/isnan.h>
#include <distr/distrbasic.h>
#include <dump/dumper.h>
#include <dump/vtk.h>
#include <func/init.h>
#include <func/init_u.h>
#include <linear/linear.h>
#include <parse/argparse.h>
#include <solver/curv.h>
#include <solver/reconst.h>
#include <solver/vofm.h>
#include <util/hydro.h>
#include <util/linear.h>
#include "func/init_bc.h"

using M = MeshCartesian<double, 2>;
using Scal = typename M::Scal;
using Vect = typename M::Vect;
using MIdx = typename M::MIdx;
using R = Reconst<Scal>;
constexpr Scal kClNone = Vofm<M>::kClNone;

// Computes flux proportional to curvature.
// Output:
// ff_flux: result
template <class M>
void CalcMeanCurvatureFlowFlux(
    FieldFace<Scal>& ff_flux, const GRange<size_t>& layers,
    const Multi<const FieldCell<Scal>*> fcu,
    const Multi<const FieldCell<Scal>*> fccl,
    const Multi<const FieldCell<Vect>*> fcn,
    const Multi<const FieldCell<Scal>*> fca,
    const Multi<const FieldCell<Scal>*> fck,
    const MapEmbed<BCondFluid<Vect>>& mebc, Scal gamma, bool divfree,
    Scal voidpenal, std::shared_ptr<linear::Solver<M>> linsolver, M& m) {
  (void)fcn;
  (void)fca;
  auto sem = m.GetSem();

  if (sem("calc")) {
    ff_flux.Reinit(m, 0);
    for (auto f : m.Faces()) {
      const IdxCell cm = m.GetCell(f, 0);
      const IdxCell cp = m.GetCell(f, 1);
      Scal um_sum = 0;
      Scal up_sum = 0;
      std::set<Scal> s;
      for (auto l : layers) {
        const Scal clm = (*fccl[l])[cm];
        const Scal clp = (*fccl[l])[cp];
        if (clm != kClNone) {
          s.insert(clm);
          um_sum += (*fcu[l])[cm];
        }
        if (clp != kClNone) {
          s.insert(clp);
          up_sum += (*fcu[l])[cp];
        }
      }
      um_sum = std::min(1., um_sum);
      up_sum = std::min(1., up_sum);
      for (auto cl : s) {
        Scal um = 0;
        Scal up = 0;
        Scal km = GetNan<Scal>();
        Scal kp = GetNan<Scal>();
        Vect nm(0);
        Vect np(0);
        for (auto l : layers) {
          if ((*fccl[l])[cm] == cl) {
            um = (*fcu[l])[cm];
            km = (*fck[l])[cm];
            nm = (*fcn[l])[cm];
          }
          if ((*fccl[l])[cp] == cl) {
            up = (*fcu[l])[cp];
            kp = (*fck[l])[cp];
            np = (*fcn[l])[cp];
          }
        }
        if (!IsNan(km) || !IsNan(kp)) {
          const Scal k = (std::abs(um - 0.5) < std::abs(up - 0.5) ? km : kp);
          const Vect n = (std::abs(um - 0.5) < std::abs(up - 0.5) ? nm : np);
          if (!IsNan(k) && !IsNan(n) && n.norm() > 0) {
            ff_flux[f] -= n.dot(m.GetSurface(f)) * (k * gamma / n.norm());
            if (std::abs(up_sum - um_sum) > 1e-2) {
              ff_flux[f] +=
                  n.dot(m.GetSurface(f)) * (voidpenal * gamma / n.norm());
            }
          }
        }
      }
    }
  }
  if (divfree && sem.Nested("proj")) {
    ProjectVolumeFlux(ff_flux, mebc, linsolver, m);
  }
  /*
  if (voidpenal && sem("void-penal")) {
    for (auto f : m.Faces()) {
      const IdxCell cm = m.GetCell(f, 0);
      const IdxCell cp = m.GetCell(f, 1);
      Scal um = 0;
      Scal up = 0;
      for (auto l : layers) {
        if ((*fccl[l])[cm] != kClNone) {
          um += (*fcu[l])[cm];
        }
        if ((*fccl[l])[cp] != kClNone) {
          up += (*fcu[l])[cp];
        }
      }
      um = std::min(1., um);
      up = std::min(1., up);
      ff_flux[f] += -(up - um) * voidpenal * m.GetArea(f);
    }
  }
  */
}

struct TrajEntry {
  Scal volume = 0;
  Vect center{Vect(0)};
  Scal k = 0; // average curvature
  Scal cells_k = 0; // cells with defined curvature
};

// Returns trajectory entries for components of selected colors.
// colors: colors to select
//
// Returns:
// array of trajectory entries for each color in `colors`
void CalcTraj(
    const Vofm<M>::Plic& plic, const Multi<const FieldCell<Scal>*>& fck,
    const std::vector<Scal>& colors, M& m,
    /*out*/
    std::vector<TrajEntry>& entries) {
  auto sem = m.GetSem();
  struct {
    std::map<Scal, TrajEntry> map;
  } * ctx(sem);
  auto& t = *ctx;
  if (sem()) {
    // Create empty entries for all colors
    for (auto l : plic.layers) {
      for (auto c : m.Cells()) {
        const auto cl = (*plic.vfccl[l])[c];
        if (cl != kClNone) {
          t.map[cl];
        }
      }
    }
    for (auto l : plic.layers) {
      for (auto c : m.CellsM()) {
        const auto cl = (*plic.vfccl[l])[c];
        if (cl != kClNone) {
          auto& entry = t.map.at(cl);
          entry.volume += (*plic.vfcu[l])[c] * c.volume();
          entry.center += (*plic.vfcu[l])[c] * c.volume() * c.center();
          if (!IsNan((*fck[l])[c])) {
            entry.k += (*fck[l])[c];
            entry.cells_k += 1;
          }
        }
      }
    }
    // Copy selected entries from map to array
    entries.resize(colors.size());
    for (size_t i = 0; i < colors.size(); ++i) {
      auto it = t.map.find(colors[i]);
      if (it != t.map.end()) {
        entries[i] = it->second;
      }
      m.Reduce(&entries[i].volume, Reduction::sum);
      for (auto d : M::dirs) {
        m.Reduce(&entries[i].center[d], Reduction::sum);
      }
      m.Reduce(&entries[i].k, Reduction::sum);
      m.Reduce(&entries[i].cells_k, Reduction::sum);
    }
  }
  if (sem()) {
    for (auto& entry : entries) {
      if (entry.volume > 0) {
        entry.center /= entry.volume;
        entry.k /= entry.cells_k;
      }
    }
  }
}

void WriteTrajHeader(std::ostream& out, size_t num_colors) {
  bool first = true;
  auto delim = [&]() {
    if (first) {
      first = false;
    } else {
      out << ' ';
    }
  };
  delim();
  out << "t";
  delim();
  out << "frame";
  for (auto field : {"x", "y", "volume", "k"}) {
    for (size_t i = 0; i < num_colors; ++i) {
      delim();
      out << util::Format("{:}_{:}", field, i);
    }
  }
  out << '\n';
}

void WriteTrajEntry(
    std::ostream& out, Scal time, size_t frame,
    const std::vector<TrajEntry>& entries) {
  bool first = true;
  auto delim = [&]() {
    if (first) {
      first = false;
    } else {
      out << ' ';
    }
  };
  delim();
  out << time;
  delim();
  out << frame;
  for (auto& entry : entries) {
    delim();
    out << entry.center[0];
  }
  for (auto& entry : entries) {
    delim();
    out << entry.center[1];
  }
  for (auto& entry : entries) {
    delim();
    out << entry.volume;
  }
  for (auto& entry : entries) {
    delim();
    out << entry.k;
  }
  out << '\n';
}

Scal GetTimeStep(M& m, Vars& var) {
  const Scal cflst = var.Double["cflst"];
  const Scal gamma = var.Double["gamma"];
  const Scal h3 = std::pow(m.GetCellSize()[0], 3);
  return cflst * std::sqrt(h3 / (4. * M_PI * std::abs(gamma)));
}

template <class M>
void SetZeroBoundaryFlux(FieldFace<Scal>& fcu, const M& m) {
  for (auto fb : m.Faces()) {
    size_t nci;
    if (m.IsBoundary(fb, nci)) {
      const IdxCell c = m.GetCell(fb, nci);
      for (auto q : m.Nci(c)) {
        const IdxFace f = m.GetFace(c, q);
        fcu[f] = 0;
      }
    }
  }
}

void Run(M& m, Vars& var) {
  auto sem = m.GetSem();
  struct {
    std::unique_ptr<Vofm<M>> as; // advection solver
    Multi<FieldCell<Scal>> fcu0; // initial volume fraction
    Multi<FieldCell<Scal>> fccl0; // initial color
    FieldCell<Scal> fc_src; // volume source
    FieldEmbed<Scal> fe_flux; // volume flux
    FieldCell<Vect> fc_vel; // velocity
    Multi<FieldCell<Scal>> fck; // curvature
    FieldCell<Scal> fck_single; // curvature from single layer
    MapEmbed<BCondFluid<Vect>> mebc_fluid; // face conditions
    MapEmbed<BCondAdvection<Scal>> mebc_adv; // face conditions
    GRange<size_t> layers;
    typename PartStrMeshM<M>::Par psm_par;
    std::unique_ptr<PartStrMeshM<M>> psm;
    Scal dt = 0;
    size_t step = 0;
    size_t frame = 0;
    std::unique_ptr<Dumper> dumper;
    std::shared_ptr<linear::Solver<M>> linsolver;
    // `traj[frame][cl]` is trajectory entry of component with color `cl`
    std::vector<std::vector<TrajEntry>> traj;
    std::ofstream trajfile;
    bool dumptraj;
    size_t dumptraj_colors;
  } * ctx(sem);
  auto& layers = ctx->layers;
  auto& t = *ctx;

  auto& as = ctx->as;
  const Scal tmax = var.Double["tmax"];
  const Scal dtmax = var.Double["dtmax"];
  if (sem("init")) {
    t.dumper = std::make_unique<Dumper>(var, "dump_");
    t.fc_src.Reinit(m, 0);
    t.fe_flux.Reinit(m, 0);
    t.dumptraj = var.Int["dumptraj"];
    t.dumptraj_colors = var.Int["dumptraj_colors"];

    t.linsolver = ULinear<M>::MakeLinearSolver(var, "symm", m);

    m.flags.is_periodic[0] = var.Int["hypre_periodic_x"];
    m.flags.is_periodic[1] = var.Int["hypre_periodic_y"];
    m.flags.linreport = var.Int["linreport"];

    {
      auto p = InitBc(var, m, {}, {});
      ctx->mebc_fluid = std::get<0>(p);
      ctx->mebc_adv = std::get<1>(p);
    }

    typename Vofm<M>::Par parvof;
    parvof.sharpen = var.Int["sharpen"];
    parvof.sharpen_cfl = var.Double["sharpen_cfl"];
    parvof.extrapolate_boundaries = var.Int["extrapolate_boundaries"];
    parvof.layers = var.Int["vofm_layers"];
    parvof.clipth = var.Double["clipth"];
    parvof.filterth = var.Double["filterth"];
    parvof.dim = M::dim;
    //parvof.scheme = Vofm<M>::Par::Scheme::aulisa;
    layers = GRange<size_t>(parvof.layers);
    t.fcu0.resize(layers);
    t.fccl0.resize(layers);

    auto buf = ReadPrimList(var.String["list_path"], m.IsRoot());
    InitOverlappingComponents(buf, t.fcu0, t.fccl0, layers, m);
    as.reset(new Vofm<M>(
        m, m, t.fcu0, t.fccl0, ctx->mebc_adv, &t.fe_flux, &t.fc_src, 0, t.dt,
        parvof));
    auto mod = [&t](auto& fcu, auto& fccl, auto layers, auto&) {
      for (auto l : layers) {
        (*fcu[l]) = t.fcu0[l];
        (*fccl[l]) = t.fccl0[l];
      }
    };
    as->AddModifier(mod);

    if (t.dumptraj && m.IsRoot()) {
      t.trajfile.open("traj.dat");
      WriteTrajHeader(t.trajfile, t.dumptraj_colors);
    }

    t.fck.Reinit(layers, m, 0);
    t.psm_par.dump_fr = 1;
    t.psm_par.dim = M::dim;

    if (m.IsRoot()) {
      std::cout << util::Format("meancurvflow dt={:}\n", GetTimeStep(m, var));
    }
  }
  sem.LoopBegin();
  if (sem.Nested("start")) {
    as->StartStep();
  }
  if (sem.Nested("iter")) {
    as->MakeIteration();
  }
  if (sem.Nested("finish")) {
    as->FinishStep();
  }
  if (sem.Nested("post")) {
    as->PostStep();
  }
  if (sem.Nested()) {
    t.psm = UCurv<M>::CalcCurvPart(as->GetPlic(), t.psm_par, t.fck, m, m);
  }
  if (sem.Nested("flux")) {
    CalcMeanCurvatureFlowFlux(
        t.fe_flux.GetFieldFace(), layers, as->GetFieldM(), as->GetColor(),
        as->GetNormal(), as->GetAlpha(), t.fck, ctx->mebc_fluid,
        var.Double["gamma"], var.Int["divfree"], var.Double["voidpenal"],
        t.linsolver, m);
  }
  if (sem("dt")) {
    if (var.Int["anchored_boundaries"]) {
      auto mod = [&t](auto& fcu, auto& fccl, auto layers, auto& m) {
        for (auto fb : m.Faces()) {
          size_t nci;
          if (m.IsBoundary(fb, nci)) {
            const IdxCell c = m.GetCell(fb, nci);
            for (auto l : layers) {
              (*fcu[l])[c] = t.fcu0[l][c];
              (*fccl[l])[c] = t.fccl0[l][c];
            }
          }
        }
      };
      as->AddModifier(mod);
    }
    Scal maxv = 0;
    for (auto f : m.Faces()) {
      maxv = std::max(maxv, std::abs(t.fe_flux[f]));
    }
    const Scal cfl = var.Double["cfl"];
    t.dt = cfl * m.GetCellSize().prod() / maxv;
    t.dt = std::min(t.dt, GetTimeStep(m, var));
    m.Reduce(&t.dt, "min");
  }
  if (sem("mindt")) {
    t.dt = std::min(t.dt, dtmax);
    as->SetTimeStep(t.dt);
    if (m.IsRoot() && t.step % var.Int["report_every"] == 0) {
      std::cout << util::Format(
          "step={:} t={:} dt={:}\n", t.step, as->GetTime(), t.dt);
    }
  }
  const bool dump = t.dumper->Try(as->GetTime(), as->GetTimeStep());
  if (sem.Nested() && var.Int["dumppoly"] && dump) {
    as->DumpInterface(GetDumpName("s", ".vtk", t.frame));
  }
  if (sem.Nested() && var.Int["dumppolymarch"] && dump) {
    as->DumpInterfaceMarch(GetDumpName("sm", ".vtk", t.frame));
  }
  if (sem.Nested() && var.Int["dumppart"] && dump) {
    t.psm->DumpParticles(
        as->GetAlpha(), as->GetNormal(), t.frame, as->GetTime());
  }
  if (t.dumptraj) {
    if (sem() && dump) {
      t.traj.emplace_back();
    }
    if (sem.Nested() && dump) {
      const auto plic = as->GetPlic();
      std::vector<Scal> colors(t.dumptraj_colors);
      std::iota(colors.begin(), colors.end(), 0);
      CalcTraj(plic, t.fck, colors, m, t.traj.back());
    }
    if (sem() && dump) {
      if (m.IsRoot()) {
        WriteTrajEntry(t.trajfile, as->GetTime(), t.frame, t.traj.back());
        t.trajfile.flush();
      }
    }
  }
  if (sem("checkloop")) {
    if (var.Int["dumpfields"] && dump) {
      ++t.frame;

      // Compute velocity from flux
      t.fc_vel.Reinit(m, Vect(0));
      for (auto c : m.AllCells()) {
        for (auto q : m.Nci(c)) {
          const auto f = m.GetFace(c, q);
          t.fc_vel[c] += m.GetNormal(f) * (t.fe_flux[f] * 0.5 / m.GetArea(f));
        }
      }
      // Fill curvature from any layer
      t.fck_single.Reinit(m, GetNan<Scal>());
      const auto plic = as->GetPlic();
      for (auto c : m.Cells()) {
        for (auto l : layers) {
          if (!IsNan(t.fck[l][c])) {
            t.fck_single[c] = t.fck[l][c];
            break;
          }
        }
      }
      m.Dump(&t.fc_vel, 0, "vx");
      m.Dump(&t.fc_vel, 1, "vy");
      m.Dump(&t.fck_single, "k");
      for (auto l : layers) {
        m.Dump(as->GetPlic().vfcu[l], "u" + std::to_string(l));
        m.Dump(as->GetPlic().vfccl[l], "cl" + std::to_string(l));
      }
    }
    if (as->GetTime() >= tmax) {
      sem.LoopBreak();
    }
    ++t.step;
  }
  sem.LoopEnd();
}

int main(int argc, const char** argv) {
  ArgumentParser parser("Constrained mean curvature flow");
  parser.AddVariable<std::string>("config", "std.conf")
      .Help("Path to config file");
  parser.AddVariable<int>("--nx", 16).Help("Mesh size in x-direction");
  parser.AddVariable<std::string>("--extra", "")
      .Help("Extra configuration (commands 'set ... ')");
  parser.AddVariable<int>("--ny", 0).Help(
      "Mesh size in y-direction. Defaults to NX");
  parser.AddVariable<int>("--bs", 16).Help("Block size in all directions");
  const auto args = parser.ParseArgs(argc, argv);
  if (const int* p = args.Int.Find("EXIT")) {
    return *p;
  }

  const auto confpath = args.String["config"];
  std::ifstream fconf(confpath);

  if (!fconf.good()) {
    throw std::runtime_error("Can't open config '" + confpath + "'");
  }

  std::stringstream conf;
  conf << fconf.rdbuf();

  const int nx = args.Int["nx"];
  const int ny = args.Int["ny"];
  const MIdx mesh_size(nx, ny ? ny : nx);
  const MIdx block_size(args.Int["bs"], args.Int["bs"]);

  MpiWrapper mpi(&argc, &argv);
  Subdomains<MIdx> sub(mesh_size, block_size, mpi.GetCommSize());
  conf << sub.GetConfig() << '\n';
  conf << args.String["extra"] << '\n';

  return RunMpiBasicString<M>(mpi, Run, conf.str());
}
