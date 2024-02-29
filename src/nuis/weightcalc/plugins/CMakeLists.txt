find_package(NEUT 5.8.0 QUIET)
if(NEUT_FOUND)
  add_library(NReWeight_weightcalc_plugin SHARED NReWeightCalc.cxx)
  target_link_libraries(NReWeight_weightcalc_plugin PUBLIC neutvect_eventinput_plugin NEUT::ReWeight nuis_options)

  set_target_properties(NReWeight_weightcalc_plugin PROPERTIES PREFIX "nuisplugin-weightcalc-")
  set_target_properties(NReWeight_weightcalc_plugin PROPERTIES OUTPUT_NAME "NReWeight")

  install(TARGETS NReWeight_weightcalc_plugin DESTINATION lib/plugins)
endif()

CPMFindPackage(
      NAME CMakeModules
      GIT_TAG stable
      GITHUB_REPOSITORY NuHepMC/CMakeModules
      DOWNLOAD_ONLY
  )
include(${CMakeModules_SOURCE_DIR}/NuHepMCModules.cmake)

find_package(GENIE3 QUIET)
if(GENIE3_FOUND AND GENIEReWeight_ENABLED)

add_library(GENIEReWeight_eventinput_plugin SHARED GENIEReWeightCalc.cxx)
target_link_libraries(GENIEReWeight_eventinput_plugin PUBLIC GHEP3_eventinput_plugin)

set_target_properties(GENIEReWeight_eventinput_plugin PROPERTIES PREFIX "nuisplugin-weightcalc-")
set_target_properties(GENIEReWeight_eventinput_plugin PROPERTIES OUTPUT_NAME "GENIEReWeight")

install(TARGETS GENIEReWeight_eventinput_plugin DESTINATION lib/plugins)

endif()

CPMFindPackage(
    NAME Prob3plusplus
    VERSION 3.10.4
    GITHUB_REPOSITORY rogerwendell/Prob3plusplus
    GIT_TAG main
)

add_library(Prob3plusplus_eventinput_plugin SHARED Prob3plusplusWeightCalc.cxx)
target_link_libraries(Prob3plusplus_eventinput_plugin PUBLIC eventinput Prob3plusplus::Prob3plusplus nuis_options)

set_target_properties(Prob3plusplus_eventinput_plugin PROPERTIES PREFIX "nuisplugin-weightcalc-")
set_target_properties(Prob3plusplus_eventinput_plugin PROPERTIES OUTPUT_NAME "Prob3plusplus")

install(TARGETS Prob3plusplus_eventinput_plugin DESTINATION lib/plugins)
