add_library(eventinput EventSourceFactory.cxx)

target_link_libraries(eventinput PUBLIC NuHepMC::CPPUtils Boost::boost yaml-cpp)

add_subdirectory(plugins)

install(TARGETS eventinput DESTINATION lib)