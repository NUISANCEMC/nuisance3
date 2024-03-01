add_library(eventinput SHARED 
  IEventSourceIterator.cxx EventSourceFactory.cxx 
  INormalizedEventSource.cxx HepMC3EventSource.cxx)

target_link_libraries(eventinput PUBLIC nuis_options)

add_subdirectory(plugins)

install(TARGETS eventinput DESTINATION lib)