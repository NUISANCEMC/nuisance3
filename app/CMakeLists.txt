add_executable(nuis-dump-events nuis-dump-events.cxx)

target_link_libraries(nuis-dump-events eventinput NuHepMC::CPPUtils)

install(TARGETS nuis-dump-events DESTINATION bin)