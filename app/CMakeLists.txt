add_executable(nuis-dump-events nuis-dump-events.cxx)
target_link_libraries(nuis-dump-events eventinput NuHepMC::CPPUtils)

add_executable(nuis-enu-histograms nuis-enu-histograms.cxx)
target_link_libraries(nuis-enu-histograms eventinput NuHepMC::CPPUtils)

add_executable(nuis-frame-test nuis-frame-test.cxx)
target_link_libraries(nuis-frame-test eventinput frame NuHepMC::CPPUtils)

install(TARGETS nuis-dump-events nuis-enu-histograms nuis-frame-test DESTINATION bin)