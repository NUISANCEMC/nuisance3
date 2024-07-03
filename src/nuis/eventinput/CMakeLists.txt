add_library(eventinput SHARED 
  IEventSourceIterator.cxx EventSourceFactory.cxx 
  INormalizedEventSource.cxx HepMC3EventSource.cxx
  IEventSourceWrapper.cxx)

target_link_libraries(eventinput PUBLIC nuis_options)

set(USE_BOOSTDLL FALSE)

add_subdirectory(plugins)
if(NOT USE_BOOSTDLL)
  target_link_libraries(eventinput PUBLIC ${eventinput_all_plugins})
else()
  target_compile_definitions(eventinput PRIVATE USE_BOOSTDLL)
endif()

install(TARGETS eventinput DESTINATION lib)