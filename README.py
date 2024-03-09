#!/usr/bin/env python3

import re
import os

text = '''<!---DO NOT EDIT THIS FILE. Generated by [[GEN]]-->
# Aphros

<img src="[[IMAGES]]/foam.png" width=300 align="right">

Finite volume solver for incompressible multiphase flows with surface tension.

Key features:

- implementation in C++14
- scalability to thousands of compute nodes
- fluid solver based on SIMPLE or Bell-Colella-Glaz methods
- advection with PLIC volume-of-fluid
- particle method for curvature estimation accurate at low resolutions
[[demo]](https://cselab.github.io/aphros/curv.html)
[[ref:partstr]]
- Multi-VOF for scalable coalescence prevention
[[demo]](https://cselab.github.io/aphros/wasm/hydro.html)
[[ref:pasc20]]
[[ref:multivof]]

## [Gallery of interactive simulations](https://github.com/cselab/aphros/wiki/Aphros-Explorer)

 [<img src="[[IMAGES]]/aphros_tile9.jpg" width=120>](https://github.com/cselab/aphros/wiki/Aphros-Explorer) | [<img src="[[IMAGES]]/thumb/curv.jpg" height=120>](https://cselab.github.io/aphros/curv.html) | [<img src="[[IMAGES]]/thumb/hydro.jpg" height=120>](https://cselab.github.io/aphros/wasm/hydro.html) | [<img src="[[IMAGES]]/thumb/electrochem.jpg" height=120>](https://cselab.github.io/aphros/wasm/electrochem.html) | [<img src="[[IMAGES]]/thumb/parser.jpg" height=120>](https://cselab.github.io/aphros/wasm/parser.html?config=M4UwLgBAlgdpUQAwChSQCYHsCuAjANiBOhAIwB0ArKuBAG4gDGkdZEATBAMw2TBgAnWAHMIwCGBAAPMLzGCRYgPoBbCACJkECPlghSWnXs4ASYMk1p5QmKOB4IJhCdZm5_G6JB0AhvkcAFAEmJABUHACUEXJYeIQQoihWDMwJAj50UGAAno6JQA=)
:---:|:---:|:---:|:---:|:---:
[Gallery wiki](https://github.com/cselab/aphros/wiki/Aphros-Explorer) | [Curvature](https://cselab.github.io/aphros/curv.html) | [Multi-VOF](https://cselab.github.io/aphros/wasm/hydro.html) | [Electrochemistry](https://cselab.github.io/aphros/wasm/electrochem.html) | [Parser](https://cselab.github.io/aphros/wasm/parser.html?config=M4UwLgBAlgdpUQAwChSQCYHsCuAjANiBOhAIwB0ArKuBAG4gDGkdZEATBAMw2TBgAnWAHMIwCGBAAPMLzGCRYgPoBbCACJkECPlghSWnXs4ASYMk1p5QmKOB4IJhCdZm5_G6JB0AhvkcAFAEmJABUHACUEXJYeIQQoihWDMwJAj50UGAAno6JQA=)

### Documentation

[Online documentation](https://cselab.github.io/aphros/doc) and [PDF](https://cselab.github.io/aphros/aphros.pdf) generated by [doc/sphinx](doc/sphinx).

Default parameters are listed in [deploy/scripts/sim_base.conf](deploy/scripts/sim_base.conf).

### Requirements

C++14, CMake

Optional dependencies:
MPI,
parallel HDF5,
python3,
python3-numpy

Bundled optional dependencies:
[hypre](https://github.com/hypre-space/hypre),
[overlap](https://github.com/severinstrobl/overlap),
[fpzip](https://github.com/LLNL/fpzip)

### Clone and build

```
git clone https://github.com/cselab/aphros.git
```

First, follow [deploy/README.md](deploy/README.md) to
prepare environment and install dependencies. Then build with

```
cd src
make
```

### Code Ocean

The [Code Ocean](https://codeocean.com/) platform hosts the following compute capsule

* [Computing foaming flows across scales: from breaking waves to microfluidics](https://codeocean.com/capsule/7188369/tree/v1)

which builds Aphros in a Linux environment, runs a set of examples, and visualizes the results.

### Docker

Instead of building the code in your system, you can build a Docker
container and run a simulation example

```
docker build github.com/cselab/aphros --tag aphros
cd examples/202_coalescence/standalone
./conf
docker run -v `pwd`:`pwd` -w `pwd` aphros
```

### Minimal build without CMake

Build without dependencies and tests on Unix-like systems
(`APHROS_PREFIX` is the installation directory, with `USE_MPI=1`,
`USE_HDF=1`, `USE_OPENCL=1`, `USE_AVX=1` builds with MPI, parallel
HDF5 library, OpenCL, and AVX extensions):

```
cd src
../make/bootstrap
make -f Makefile_legacy install APHROS_PREFIX=$HOME/.local USE_MPI=0 USE_HDF=0 USE_OPENCL=0 USE_AVX=0
```

on Windows using Microsoft C++ toolset (NMAKE, LINK, and CL):

```
cd src
../make/bootstrap # Requires sh and awk.
nmake /f NMakefile
```

## Videos

Examples of simulations visualized using
[ParaView](https://www.paraview.org/) and [OSPRay](https://www.ospray.org/).
Links `[conf]` lead to the solver configuration.

|    |    |
:---:|:---:
[<img src="[[VIDTHUMB]]/coalescence.jpg" height=150>](https://www.youtube.com/watch?v=pRWGhGoQjyI) | [<img src="[[VIDTHUMB]]/taylor_green.jpg" height=150>](https://www.youtube.com/watch?v=lCf_T0C5Kmg)
Coalescence of bubbles [[conf]](examples/202_coalescence) [[ref:partstr]] | Taylor-Green vortex with bubbles [[ref:pasc19]] [[ref:datadriven]]
[<img src="[[VIDTHUMB]]/reactor.jpg" height=150>](https://www.youtube.com/watch?v=xEo51gqLdds) | [<img src="[[VIDTHUMB]]/reactor_full2.jpg" height=150>](https://www.youtube.com/watch?v=Rm-xDGpIEJA)
Bubble jump-off [[ref:ees]] | Electrochemical reactor [[conf]](examples/213_electrochem/reactor) [[ref:thesis]]
[<img src="[[VIDTHUMB]]/vortex_bubble.jpg" height=200>](https://www.youtube.com/watch?v=x9hk6pcicj0) | [<img src="[[VIDTHUMB]]/plunging_jet.jpg" height=200>](https://www.youtube.com/watch?v=9NPoiHHFkh0)
Bubble trapped by vortex ring [[ref:datadriven]] | Plunging jet [[ref:pasc19]]
[<img src="[[VIDTHUMB]]/rising_bubbles.jpg" height=150>](https://www.youtube.com/watch?v=WzOe0buD8uM) | [<img src="[[VIDTHUMB]]/foaming_waterfall.jpg" height=150>](https://www.youtube.com/watch?v=0Cj8pPYNJGY)
 Clustering of bubbles [[conf]](examples/205_multivof/clustering) [[ref:aps]] [[ref:cscs]] [[ref:multivof]] | Foaming waterfall [[conf]](examples/205_multivof/waterfall) [[ref:pasc20]] [[ref:multivof]]
[<img src="[[VIDTHUMB]]/bidisperse.jpg" height=150>](https://www.youtube.com/watch?v=2fm_JX9-Wbg) | [<img src="[[VIDTHUMB]]/crystal.jpg" height=45>](https://www.youtube.com/watch?v=8iPmOsXnXAM)
 Bidisperse foam [[conf]](examples/205_multivof/bidisperse) [[ref:multivof]] | Microfluidic crystals [[conf]](examples/205_multivof/crystal) [[ref:multivof]]
[<img src="[[VIDTHUMB]]/lammps_polymers.jpg" height=200>](https://www.youtube.com/watch?v=scz2YVKmDaQ) | [<img src="[[VIDTHUMB]]/korali_pipe.jpg" height=200>](https://www.youtube.com/watch?v=O5Dhnjrfe8A)
LAMMPS polymers in Taylor-Green vortex [[conf]](examples/212_polymers) | Bubble pipe optimization [[ref:korali]]
[<img src="[[VIDTHUMB]]/mesh_bubbles.jpg" height=150>](https://www.youtube.com/watch?v=rs7OhSixm5c) |
Bubbles through mesh |

|     |
|:---:|
|[<img src="[[VIDTHUMB]]/breaking_waves.jpg" height=300>](https://www.youtube.com/watch?v=iGdphpztCJQ)|
|APS Gallery of Fluid Motion 2019 award winner<br> Breaking waves: to foam or not to foam? [[ref:aps]]<br> Collaboration with Jean M. Favre at [CSCS](https://www.cscs.ch).|

## Developers

Aphros is developed by researchers at [ETH Zurich](https://www.cse-lab.ethz.ch) and [Harvard University](https://cse-lab.seas.harvard.edu/)

* [Petr Karnakov](https://pkarnakov.com)
* [Sergey Litvinov](https://www.cse-lab.ethz.ch/member/sergey-litvinov)

advised by

* [Prof. Petros Koumoutsakos](https://www.seas.harvard.edu/person/petros-koumoutsakos)

Other contributors are: Fabian Wermelinger (Cubism backend)

## Publications

[[item:ees]] Hashemi SMH, Karnakov P, Hadikhani P, Chinello E, Litvinov S, Moser C, Koumoutsakos P, Psaltis D.
  A versatile and membrane-less electrochemical reactor for the electrolysis of water and brine.
  _Energy & environmental science_. 2019
  [10.1039/C9EE00219G](https://doi.org/10.1039/C9EE00219G)
[[item:pasc19]] Karnakov P, Wermelinger F, Chatzimanolakis M, Litvinov S, Koumoutsakos P.
  A high performance computing framework for multiphase, turbulent flows on structured grids.
  _Proceedings of the platform for advanced scientific computing conference on – PASC ’19_. 2019
  [10.1145/3324989.3325727](https://doi.org/10.1145/3324989.3325727)
  [[pdf]]([[PDF]]/pasc2019.pdf)
[[item:icmf]] Karnakov P, Litvinov S, Koumoutsakos P.
  Coalescence and transport of bubbles and drops.
  _10th International Conference on Multiphase Flow (ICMF)_. 2019
  [[pdf]]([[PDF]]/icmf2019.pdf)
[[item:partstr]] Karnakov P, Litvinov S, and Koumoutsakos P.
  A hybrid particle volume-of-fluid method for curvature estimation in multiphase flows.
  _International journal of multiphase flow_. 2020
  [10.1016/j.ijmultiphaseflow.2020.103209](https://doi.org/10.1016/j.ijmultiphaseflow.2020.103209)
  [arXiv:1906.00314](https://arxiv.org/pdf/1906.00314)
[[item:datadriven]] Wan Z, Karnakov P, Koumoutsakos P, Sapsis T.
  Bubbles in Turbulent Flows: Data-driven, kinematic models with history terms.
  _International journal of multiphase flow_. 2020
  [10.1016/j.ijmultiphaseflow.2020.103286](https://doi.org/10.1016/j.ijmultiphaseflow.2020.103286)
  [arXiv:1910.02068](https://arxiv.org/pdf/1910.02068)
[[item:aps]] Karnakov P, Litvinov S, Favre JM, Koumoutsakos P.
  V0018: Breaking waves: to foam or not to foam?
  _Gallery of Fluid Motion Award_
  [video](https://doi.org/10.1103/APS.DFD.2019.GFM.V0018)
  [article](http://dx.doi.org/10.1103/physrevfluids.5.110503)
[[item:cscs]] Annual report 2019 of the Swiss National Supercomputing Centre (cover page)
  [[link]](https://www.cscs.ch/publications/annual-reports/cscs-annual-report-2019)
[[item:pasc20]] Karnakov P, Wermelinger F, Litvinov S, Koumoutsakos P.
  Aphros: High Performance Software for Multiphase Flows with Large Scale Bubble and Drop Clusters.
  _Proceedings of the platform for advanced scientific computing conference on – PASC ’20_. 2020
  [10.1145/3394277.3401856](https://doi.org/10.1145/3394277.3401856)
  [[pdf]]([[PDF]]/pasc2020.pdf)
[[item:thesis]] Karnakov P. The multilayer volume-of-fluid method for multiphase
  flows across scales: breaking waves, microfluidics, and membrane-less
  electrolyzers. _PhD thesis_. ETH Zurich. 2021
  [10.3929/ethz-b-000547518](https://doi.org/10.3929/ethz-b-000547518)
[[item:korali]] Martin SM, Wälchli D, Arampatzis G, Economides AE, Karnakov P, Koumoutsakos P.
  Korali: Efficient and scalable software framework for Bayesian uncertainty quantification and stochastic optimization.
  _Computer Methods in Applied Mechanics and Engineering_. 2021
  [10.1016/j.cma.2021.114264](https://doi.org/10.1016/j.cma.2021.114264)
[[item:multivof]] Karnakov P, Litvinov S, Koumoutsakos P.
  Computing foaming flows across scales: from breaking waves to microfluidics.
  _Science Advances_. 2022
  [10.1126/sciadv.abm0590](https://doi.org/10.1126/sciadv.abm0590)

## Citing

If you use Aphros in your work, please consider using the following
```
@article{aphros2022,
  author = {Petr Karnakov  and Sergey Litvinov  and Petros Koumoutsakos},
  title = {Computing foaming flows across scales: From breaking waves to microfluidics},
  journal = {Science Advances},
  volume = {8},
  number = {5},
  pages = {eabm0590},
  year = {2022},
  doi = {10.1126/sciadv.abm0590},
  URL = {https://www.science.org/doi/abs/10.1126/sciadv.abm0590},
  eprint = {https://www.science.org/doi/pdf/10.1126/sciadv.abm0590},
}
```


'''

m_refs = list(re.finditer("\[\[ref:[^]]*\]\]", text))
m_items = list(re.finditer("\[\[item:[^]]*\]\]", text))
refs = [m_ref.group(0) for m_ref in m_refs]

gen = text
gen = gen.replace('[[GEN]]', os.path.basename(__file__))
gen = gen.replace('[[VIDEOS]]', "https://cselab.github.io/aphros/videos")
gen = gen.replace('[[VIDTHUMB]]', "https://cselab.github.io/aphros/videos/preview/thumb")
gen = gen.replace('[[IMAGES]]', "https://cselab.github.io/aphros/images")
gen = gen.replace('[[PDF]]', "https://cselab.github.io/aphros/pdf")
found_refs = set()
for i, m_item in enumerate(m_items):
    item = m_item.group(0)
    name = re.match("\[\[item:([^]]*)\]\]", item).group(1)
    start = m_item.start(0)
    end = m_items[i + 1].start(0) if i + 1 < len(m_items) else len(text)
    m_url = re.search("\((http[^)]*)\)", text[start:end])
    ref = "[[ref:{}]]".format(name)
    found_refs.add(ref)

    gen = gen.replace(item, "{:}.".format(i + 1))

    if m_url:
        url = m_url.group(1)
        gen = gen.replace(ref, "[[{:}]]({})".format(i + 1, url))
    else:
        if ref in refs:
            print("Warning: no URL found for '{}'".format(item))
        gen = gen.replace(ref, "[{:}]".format(i + 1))

unknown_refs = set(refs) - found_refs
if len(unknown_refs):
    for ref in unknown_refs:
        print("Warning: item not found for '{}'".format(ref))

with open("README.md", 'w') as f:
    f.write(gen)
