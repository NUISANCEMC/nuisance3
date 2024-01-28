add_library(eventinput INTERFACE)

target_link_libraries(eventinput INTERFACE NuHepMC::CPPUtils Boost::boost Boost::filesystem yaml-cpp)

add_subdirectory(plugins)

# install(TARGETS eventinput DESTINATION lib)