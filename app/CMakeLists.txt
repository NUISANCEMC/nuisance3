add_executable(nuis-tohepmc3 nuis-tohepmc3.cxx)
target_link_libraries(nuis-tohepmc3 eventinput)
install(TARGETS nuis-tohepmc3 DESTINATION bin)
