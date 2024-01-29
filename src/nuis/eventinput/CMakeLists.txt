add_library(eventinput SHARED IEventSourceIterator.cxx)

target_link_libraries(eventinput PUBLIC NuHepMC::CPPUtils Boost::boost Boost::filesystem yaml-cpp)

add_subdirectory(plugins)

# install(TARGETS eventinput DESTINATION lib)