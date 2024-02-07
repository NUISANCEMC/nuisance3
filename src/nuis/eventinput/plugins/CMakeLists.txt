find_package(NEUT 5.8.0 QUIET)
if(NEUT_FOUND)
  CPMFindPackage(
    NAME nvconv
    GIT_TAG main
    GITHUB_REPOSITORY "neut-devel/neutvect-converter"
  )

  add_library(neutvect_eventinput_plugin SHARED neutvectEventSource.cxx)
  target_link_libraries(neutvect_eventinput_plugin PUBLIC nvconv nuis_options)

  set_target_properties(neutvect_eventinput_plugin PROPERTIES PREFIX "nuisplugin-eventinput-")
  set_target_properties(neutvect_eventinput_plugin PROPERTIES OUTPUT_NAME "neutvect")

  install(TARGETS neutvect_eventinput_plugin DESTINATION lib/plugins)
  
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

CPMFindPackage(
      NAME CMakeModules
      GIT_TAG stable
      GITHUB_REPOSITORY NuHepMC/CMakeModules
      DOWNLOAD_ONLY
  )
include(${CMakeModules_SOURCE_DIR}/NuHepMCModules.cmake)

find_package(GENIE3 QUIET)
if(GENIE3_FOUND)

add_library(GHEP3_eventinput_plugin SHARED GHEP3EventSource.cxx)
target_link_libraries(GHEP3_eventinput_plugin PUBLIC GENIE3::All NuHepMC::CPPUtils nuis_options ROOT::Geom)

target_compile_definitions(GHEP3_eventinput_plugin PRIVATE 
  GENIE_VERSION_STR="${GENIE_VERSION}")

set_target_properties(GHEP3_eventinput_plugin PROPERTIES PREFIX "nuisplugin-eventinput-")
set_target_properties(GHEP3_eventinput_plugin PROPERTIES OUTPUT_NAME "GHEP3")

install(TARGETS GHEP3_eventinput_plugin DESTINATION lib/plugins)

endif()