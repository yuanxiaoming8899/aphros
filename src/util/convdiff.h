#pragma once

#include <memory>

#include "solver/cond.h"
#include "geom/map.h"
#include "solver/convdiff.h"
#include "solver/convdiffv.h"

using namespace solver;


// Converts vector conditions to scalar.
// mfv: vector velocity conditions
// d: direction, 0..2
template <class M>
MapFace<std::shared_ptr<solver::CondFace>> 
GetScalarCond(const MapFace<std::shared_ptr<solver::CondFace>>& mfv, 
              size_t d, const M& m) {
  using namespace solver;
  using Scal = typename M::Scal;
  using Vect = typename M::Vect;
  MapFace<std::shared_ptr<CondFace>> mfs;
  // Face conditions for each velocity component
  for (auto it : mfv) {
    IdxFace f = it.GetIdx();
    CondFace* cb = it.GetValue().get();
    if (auto p = dynamic_cast<CondFaceVal<Vect>*>(cb)) {
      mfs[f] = std::make_shared<CondFaceValComp<Vect>>(p, d);
    } else if (auto p = dynamic_cast<CondFaceGrad<Vect>*>(cb)) {
      mfs[f] = std::make_shared<CondFaceGradComp<Vect>>(p, d);
    } else if (auto p = dynamic_cast<CondFaceReflect*>(cb)) {
      auto nci = cb->GetNci();
      // XXX: adhoc for cartesian grid
      if (d == m.GetNormal(f).abs().argmax()) { 
        // normal, zero value
        mfs[f] = std::make_shared<CondFaceValFixed<Scal>>(0., nci);
      } else { 
        // tangential, zero gradient
        mfs[f] = std::make_shared<CondFaceGradFixed<Scal>>(0., nci);
      }
    } else {
      throw std::runtime_error("GetScalarCond: unknown face condition");
    }
  }
  return mfs;
}

enum class Conv {exp, imp};

template <class M>
struct GetConvDiff {
  using Scal = typename M::Scal;
  using Vect = typename M::Vect;
  using Par = typename ConvDiffScal<M>::Par;

  std::unique_ptr<ConvDiffVect<M>> operator()(
      Conv conv, M& m, const FieldCell<Vect>& fcvel,
      const MapFace<std::shared_ptr<CondFace>>& mfc,
      const MapCell<std::shared_ptr<CondCell>>& mcc,
      const FieldCell<Scal>* fcr, const FieldFace<Scal>* ffd,
      const FieldCell<Vect>* fcs, const FieldFace<Scal>* ffv,
      double t, double dt, const Par& par);
};

template <class ConvDiffPar, class FluidPar>
void SetConvDiffPar(ConvDiffPar& d, const FluidPar& p) {
  d.relax = p.vrelax;
  d.guessextra = p.guessextra;
  d.second = p.second;
  d.sc = p.convsc;
  d.df = p.convdf;
  d.linreport = p.linreport;
}

template <class ConvDiffPar, class FluidPar>
ConvDiffPar UpdateConvDiffPar(ConvDiffPar d, const FluidPar& p) {
  SetConvDiffPar(d, p);
  return d;
}
