$(WRK)/color/color.o: $(SRC)/color/color.c; mkdir -p $(WRK)/color && $(CC_RULE)
$(WRK)/distr/distrbasic.o: $(SRC)/distr/distrbasic.cpp; mkdir -p $(WRK)/distr && $(CXX_RULE)
$(WRK)/distr/distr.o: $(SRC)/distr/distr.cpp; mkdir -p $(WRK)/distr && $(CXX_RULE)
$(WRK)/distr/distrsolver.o: $(SRC)/distr/distrsolver.cpp; mkdir -p $(WRK)/distr && $(CXX_RULE)
$(WRK)/distr/local.o: $(SRC)/distr/local.cpp; mkdir -p $(WRK)/distr && $(CXX_RULE)
$(WRK)/distr/report.o: $(SRC)/distr/report.cpp; mkdir -p $(WRK)/distr && $(CXX_RULE)
$(WRK)/dump/dumper.o: $(SRC)/dump/dumper.cpp; mkdir -p $(WRK)/dump && $(CXX_RULE)
$(WRK)/dump/dump.o: $(SRC)/dump/dump.cpp; mkdir -p $(WRK)/dump && $(CXX_RULE)
$(WRK)/dump/hdf.o: $(SRC)/dump/hdf.cpp; mkdir -p $(WRK)/dump && $(CXX_RULE)
$(WRK)/dump/raw.o: $(SRC)/dump/raw.cpp; mkdir -p $(WRK)/dump && $(CXX_RULE)
$(WRK)/dump/xmf.o: $(SRC)/dump/xmf.cpp; mkdir -p $(WRK)/dump && $(CXX_RULE)
$(WRK)/func/init_contang.o: $(SRC)/func/init_contang.cpp; mkdir -p $(WRK)/func && $(CXX_RULE)
$(WRK)/func/init.o: $(SRC)/func/init.cpp; mkdir -p $(WRK)/func && $(CXX_RULE)
$(WRK)/func/init_vel.o: $(SRC)/func/init_vel.cpp; mkdir -p $(WRK)/func && $(CXX_RULE)
$(WRK)/func/primlist.o: $(SRC)/func/primlist.cpp; mkdir -p $(WRK)/func && $(CXX_RULE)
$(WRK)/geom/mesh.o: $(SRC)/geom/mesh.cpp; mkdir -p $(WRK)/geom && $(CXX_RULE)
$(WRK)/inside/bbox.o: $(SRC)/inside/bbox.c; mkdir -p $(WRK)/inside && $(CC_RULE)
$(WRK)/inside/err.o: $(SRC)/inside/err.c; mkdir -p $(WRK)/inside && $(CC_RULE)
$(WRK)/inside/main.o: $(SRC)/inside/main.c; mkdir -p $(WRK)/inside && $(CC_RULE)
$(WRK)/inside/memory.o: $(SRC)/inside/memory.c; mkdir -p $(WRK)/inside && $(CC_RULE)
$(WRK)/inside/off.o: $(SRC)/inside/off.c; mkdir -p $(WRK)/inside && $(CC_RULE)
$(WRK)/inside/ply.o: $(SRC)/inside/ply.c; mkdir -p $(WRK)/inside && $(CC_RULE)
$(WRK)/inside/predicate.o: $(SRC)/inside/predicate.c; mkdir -p $(WRK)/inside && $(CC_RULE)
$(WRK)/inside/stl.o: $(SRC)/inside/stl.c; mkdir -p $(WRK)/inside && $(CC_RULE)
$(WRK)/linear/linear.o: $(SRC)/linear/linear.cpp; mkdir -p $(WRK)/linear && $(CXX_RULE)
$(WRK)/march/main.o: $(SRC)/march/main.c; mkdir -p $(WRK)/march && $(CC_RULE)
$(WRK)/parse/argparse.o: $(SRC)/parse/argparse.cpp; mkdir -p $(WRK)/parse && $(CXX_RULE)
$(WRK)/parse/conf2py.o: $(SRC)/parse/conf2py.cpp; mkdir -p $(WRK)/parse && $(CXX_RULE)
$(WRK)/parse/parser.o: $(SRC)/parse/parser.cpp; mkdir -p $(WRK)/parse && $(CXX_RULE)
$(WRK)/parse/template.o: $(SRC)/parse/template.cpp; mkdir -p $(WRK)/parse && $(CXX_RULE)
$(WRK)/parse/vars.o: $(SRC)/parse/vars.cpp; mkdir -p $(WRK)/parse && $(CXX_RULE)
$(WRK)/solver/approx_eb.o: $(SRC)/solver/approx_eb.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/approx.o: $(SRC)/solver/approx.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/convdiffe.o: $(SRC)/solver/convdiffe.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/convdiffi.o: $(SRC)/solver/convdiffi.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/convdiffvg.o: $(SRC)/solver/convdiffvg.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/curv.o: $(SRC)/solver/curv.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/electro.o: $(SRC)/solver/electro.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/embed.o: $(SRC)/solver/embed.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/fluid_dummy.o: $(SRC)/solver/fluid_dummy.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/normal.o: $(SRC)/solver/normal.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/particles.o: $(SRC)/solver/particles.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/partstrmeshm.o: $(SRC)/solver/partstrmeshm.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/proj_eb.o: $(SRC)/solver/proj_eb.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/proj.o: $(SRC)/solver/proj.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/simple.o: $(SRC)/solver/simple.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/solver.o: $(SRC)/solver/solver.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/tracer.o: $(SRC)/solver/tracer.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/vofm.o: $(SRC)/solver/vofm.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/solver/vof.o: $(SRC)/solver/vof.cpp; mkdir -p $(WRK)/solver && $(CXX_RULE)
$(WRK)/util/convdiff.o: $(SRC)/util/convdiff.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/distr.o: $(SRC)/util/distr.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/events.o: $(SRC)/util/events.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/filesystem.o: $(SRC)/util/filesystem.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/fixed_allocator.o: $(SRC)/util/fixed_allocator.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/fluid.o: $(SRC)/util/fluid.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/format.o: $(SRC)/util/format.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/gitgen.o: $(SRC)/util/gitgen.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/git.o: $(SRC)/util/git.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/histogram.o: $(SRC)/util/histogram.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/hydro.o: $(SRC)/util/hydro.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/linear.o: $(SRC)/util/linear.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/mpi.o: $(SRC)/util/mpi.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/posthook_default.o: $(SRC)/util/posthook_default.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/subcomm_dummy.o: $(SRC)/util/subcomm_dummy.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/suspender.o: $(SRC)/util/suspender.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/sysinfo.o: $(SRC)/util/sysinfo.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/timer.o: $(SRC)/util/timer.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/util/vof.o: $(SRC)/util/vof.cpp; mkdir -p $(WRK)/util && $(CXX_RULE)
$(WRK)/young/main.o: $(SRC)/young/main.cpp; mkdir -p $(WRK)/young && $(CXX_RULE)
