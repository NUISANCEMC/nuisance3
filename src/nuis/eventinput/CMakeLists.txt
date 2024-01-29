add_library(eventinput SHARED IEventSourceIterator.cxx)

target_link_libraries(eventinput PUBLIC HepMC3::All Boost::boost Boost::filesystem yaml-cpp spdlog::spdlog)

add_subdirectory(plugins)

install(TARGETS eventinput DESTINATION lib)