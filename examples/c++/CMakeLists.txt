add_executable(nuis-example-dumpevents nuis-example-dumpevents.cxx)
target_link_libraries(nuis-example-dumpevents eventinput)
install(TARGETS nuis-example-dumpevents DESTINATION bin)

add_executable(nuis-example-binning nuis-example-binning.cxx)
target_link_libraries(nuis-example-binning eventinput histframe)
install(TARGETS nuis-example-binning DESTINATION bin)

# add_executable(nuis-enu-histograms nuis-enu-histograms.cxx)
# target_link_libraries(nuis-enu-histograms eventinput NuHepMC::CPPUtils)

# add_executable(nuis-frame-test nuis-frame-test.cxx)
# target_link_libraries(nuis-frame-test eventinput frame NuHepMC::CPPUtils)


# find_package(ROOT 6 REQUIRED)
# add_executable(nuis-histframe-test nuis-histframe-test.cxx)
# target_link_libraries(nuis-histframe-test eventinput histframe record NuHepMC::CPPUtils ROOT::Hist ROOT::Gui)

# add_executable(nuis-record-test nuis-record-test.cxx)
# target_link_libraries(nuis-record-test eventinput record histframe NuHepMC::CPPUtils ROOT::Hist ROOT::Gui)


# install(TARGETS nuis-dump-events nuis-enu-histograms nuis-frame-test nuis-histframe-test nuis-record-test DESTINATION bin)
