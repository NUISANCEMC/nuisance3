add_library(eventinput SHARED 
  IEventSourceIterator.cxx EventSourceFactory.cxx 
  NormalizedEventSource.cxx HepMC3EventSource.cxx
  IEventSourceWrapper.cxx IEventSource.cxx)

target_link_libraries(eventinput PUBLIC nuis_options)

add_subdirectory(plugins)
if(NOT NUISANCE_USE_BOOSTDLL)
  target_link_libraries(eventinput PUBLIC eventinput_plugins)
endif()

install(TARGETS eventinput DESTINATION lib)