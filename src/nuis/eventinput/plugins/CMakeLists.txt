find_package(NEUT 5.5.0 QUIET)
if(NEUT_FOUND)
  CPMFindPackage(
    NAME nvconv
    GIT_TAG main
    GITHUB_REPOSITORY "neut-devel/neutvect-converter"
  )

  add_library(neutvect_eventinput_plugin STATIC neutvectEventSource.cxx)
  target_link_libraries(neutvect_eventinput_plugin PUBLIC nvconv nuis_options)

  # Install the plugin into the plugin directory will a well-formed static plugin name that
  # the dynamic plugin loader can used to determine the symbol name to fetch from the main 
  # executable
  set_target_properties(neutvect_eventinput_plugin PROPERTIES PREFIX "nuisplugin-static-eventinput-")
  set_target_properties(neutvect_eventinput_plugin PROPERTIES OUTPUT_NAME "neutvect")

  install(TARGETS neutvect_eventinput_plugin DESTINATION lib/plugins)

  target_link_libraries(nuis_snowflakes INTERFACE neutvect_eventinput_plugin)

  # This stops the symbol stripper from thinking that we never used this symbol
  target_link_options(nuis_snowflakes INTERFACE -Wl,--undefined=MakeEventSource_neutvect)

endif()

CPMFindPackage(
  NAME nuwroconv
  GIT_TAG main
  GITHUB_REPOSITORY "NuHepMC/nuwro2hepmc3"
)

add_library(NuWroevent1_eventinput_plugin SHARED NuWroevent1EventSource.cxx)
target_link_libraries(NuWroevent1_eventinput_plugin PUBLIC nuwroconv nuis_options)

set_target_properties(NuWroevent1_eventinput_plugin PROPERTIES PREFIX "nuisplugin-eventinput-")
set_target_properties(NuWroevent1_eventinput_plugin PROPERTIES OUTPUT_NAME "NuWroevent1")

install(TARGETS NuWroevent1_eventinput_plugin DESTINATION lib/plugins)