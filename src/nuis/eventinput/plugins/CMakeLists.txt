set(eventinput_all_plugins)

set(plugin_libdir lib/plugins)
if(NOT USE_BOOSTDLL)
  set(plugin_libdir lib)
endif()

find_package(NEUT 5.8.0 QUIET)
if(NEUT_FOUND)
  CPMFindPackage(
    NAME nvconv
    GIT_TAG main
    GITHUB_REPOSITORY "neut-devel/neutvect-converter"
  )

  add_library(neutvect_eventinput_plugin SHARED neutvectEventSource.cxx)
  target_link_libraries(neutvect_eventinput_plugin PUBLIC nvconv nuis_options)
  target_compile_definitions(neutvect_eventinput_plugin PRIVATE USE_BOOSTDLL)

  set_target_properties(neutvect_eventinput_plugin PROPERTIES PREFIX "nuisplugin-eventinput-")
  set_target_properties(neutvect_eventinput_plugin PROPERTIES OUTPUT_NAME "neutvect")

  install(TARGETS neutvect_eventinput_plugin DESTINATION ${plugin_libdir})

  list(APPEND eventinput_all_plugins neutvect_eventinput_plugin)
  
endif()

if(BUILTIN_NuWro)
  CPMFindPackage(
    NAME nuwroconv
    GIT_TAG main
    GITHUB_REPOSITORY "NuHepMC/nuwro2hepmc3"
  )

  add_library(NuWroevent1_eventinput_plugin SHARED NuWroevent1EventSource.cxx)
  target_link_libraries(NuWroevent1_eventinput_plugin PUBLIC nuwroconv nuis_options)
  target_compile_definitions(NuWroevent1_eventinput_plugin PRIVATE USE_BOOSTDLL)

  set_target_properties(NuWroevent1_eventinput_plugin PROPERTIES PREFIX "nuisplugin-eventinput-")
  set_target_properties(NuWroevent1_eventinput_plugin PROPERTIES OUTPUT_NAME "NuWroevent1")

  install(TARGETS NuWroevent1_eventinput_plugin DESTINATION ${plugin_libdir})

  list(APPEND eventinput_all_plugins NuWroevent1_eventinput_plugin)
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
      GENIE_VERSION_STR="${GENIE_VERSION}"
      USE_BOOSTDLL)

    set_target_properties(GHEP3_eventinput_plugin PROPERTIES PREFIX "nuisplugin-eventinput-")
    set_target_properties(GHEP3_eventinput_plugin PROPERTIES OUTPUT_NAME "GHEP3")

    install(TARGETS GHEP3_eventinput_plugin DESTINATION ${plugin_libdir})

    list(APPEND eventinput_all_plugins GHEP3_eventinput_plugin)
  endif()

  add_library(NUISANCE2FlatTree_eventinput_plugin SHARED NUISANCE2FlatTreeEventSource.cxx)
  target_link_libraries(NUISANCE2FlatTree_eventinput_plugin PUBLIC NuHepMC::CPPUtils nuis_options ROOT::TreePlayer)
  target_compile_definitions(NUISANCE2FlatTree_eventinput_plugin PRIVATE USE_BOOSTDLL)

  set_target_properties(NUISANCE2FlatTree_eventinput_plugin PROPERTIES PREFIX "nuisplugin-eventinput-")
  set_target_properties(NUISANCE2FlatTree_eventinput_plugin PROPERTIES OUTPUT_NAME "NUISANCE2FlatTree")

  install(TARGETS NUISANCE2FlatTree_eventinput_plugin DESTINATION ${plugin_libdir})

  list(APPEND eventinput_all_plugins NUISANCE2FlatTree_eventinput_plugin)
endif()

set(eventinput_all_plugins ${eventinput_all_plugins} PARENT_SCOPE)
