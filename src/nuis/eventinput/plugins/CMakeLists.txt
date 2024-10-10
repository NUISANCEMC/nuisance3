add_library(eventinput_plugins INTERFACE)

find_package(NEUT 5.6.4 QUIET)
if(NEUT_FOUND)
  CPMFindPackage(
    NAME nvconv
    GIT_TAG v1-RC1
    GITHUB_REPOSITORY "neut-devel/neutvect-converter"
  )

  add_library(neutvect_eventinput_plugin SHARED neutvectEventSource.cxx)
  target_link_libraries(neutvect_eventinput_plugin PUBLIC nvconv nuis_options)

  set_target_properties(neutvect_eventinput_plugin PROPERTIES PREFIX "nuisplugin-eventinput-")
  set_target_properties(neutvect_eventinput_plugin PROPERTIES OUTPUT_NAME "neutvect")
  target_compile_definitions(neutvect_eventinput_plugin INTERFACE NUIS_EVENTINPUT_neutvect_Enabled)

  install(TARGETS neutvect_eventinput_plugin DESTINATION ${NUISANCE_PLUGIN_LIBDIR})

  target_link_libraries(eventinput_plugins INTERFACE neutvect_eventinput_plugin)
  
endif()

if(BUILTIN_NuWro)
  CPMFindPackage(
    NAME nuwroconv
    GIT_TAG v1-RC1
    GITHUB_REPOSITORY "NuHepMC/nuwro2hepmc3"
  )

  add_library(NuWroevent1_eventinput_plugin SHARED NuWroevent1EventSource.cxx)
  target_link_libraries(NuWroevent1_eventinput_plugin PUBLIC nuwroconv nuis_options)

  set_target_properties(NuWroevent1_eventinput_plugin PROPERTIES PREFIX "nuisplugin-eventinput-")
  set_target_properties(NuWroevent1_eventinput_plugin PROPERTIES OUTPUT_NAME "NuWroevent1")
  target_compile_definitions(NuWroevent1_eventinput_plugin INTERFACE NUIS_EVENTINPUT_NuWroevent1_Enabled)

  install(TARGETS NuWroevent1_eventinput_plugin DESTINATION ${NUISANCE_PLUGIN_LIBDIR})

  target_link_libraries(eventinput_plugins INTERFACE NuWroevent1_eventinput_plugin)
endif()

CPMFindPackage(
      NAME CMakeModules
      GIT_TAG stable
      GITHUB_REPOSITORY NuHepMC/CMakeModules
      DOWNLOAD_ONLY
  )
include(${CMakeModules_SOURCE_DIR}/NuHepMCModules.cmake)

find_package(ROOT QUIET)
if(ROOT_FOUND)

  find_package(GENIE3 QUIET)
  if(GENIE3_FOUND)
    add_library(GHEP3_eventinput_plugin SHARED GHEP3EventSource.cxx)
    target_link_libraries(GHEP3_eventinput_plugin PUBLIC GENIE3::All NuHepMC::CPPUtils nuis_options 
      ROOT::Geom ROOT::Tree ROOT::MathMore)

    target_compile_definitions(GHEP3_eventinput_plugin PRIVATE 
      GENIE_VERSION_STR="${GENIE_VERSION}")

    set_target_properties(GHEP3_eventinput_plugin PROPERTIES PREFIX "nuisplugin-eventinput-")
    set_target_properties(GHEP3_eventinput_plugin PROPERTIES OUTPUT_NAME "GHEP3")
    target_compile_definitions(GHEP3_eventinput_plugin INTERFACE NUIS_EVENTINPUT_GHEP3_Enabled)

    install(TARGETS GHEP3_eventinput_plugin DESTINATION ${NUISANCE_PLUGIN_LIBDIR})

    target_link_libraries(eventinput_plugins INTERFACE GHEP3_eventinput_plugin)
  endif()

  add_library(NUISANCE2FlatTree_eventinput_plugin SHARED NUISANCE2FlatTreeEventSource.cxx)
  target_link_libraries(NUISANCE2FlatTree_eventinput_plugin PUBLIC NuHepMC::CPPUtils nuis_options ROOT::TreePlayer)

  set_target_properties(NUISANCE2FlatTree_eventinput_plugin PROPERTIES PREFIX "nuisplugin-eventinput-")
  set_target_properties(NUISANCE2FlatTree_eventinput_plugin PROPERTIES OUTPUT_NAME "NUISANCE2FlatTree")
  target_compile_definitions(NUISANCE2FlatTree_eventinput_plugin INTERFACE NUIS_EVENTINPUT_NUISANCE2FlatTree_Enabled)

  install(TARGETS NUISANCE2FlatTree_eventinput_plugin DESTINATION ${NUISANCE_PLUGIN_LIBDIR})

  target_link_libraries(eventinput_plugins INTERFACE NUISANCE2FlatTree_eventinput_plugin)
endif()
