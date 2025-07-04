add_library(weightcalc_plugins INTERFACE)

find_package(NEUT 5.6.4 QUIET)
if(NEUT_FOUND)
  add_library(NReWeight_weightcalc_plugin SHARED NReWeightCalc.cxx)
  target_link_libraries(NReWeight_weightcalc_plugin PUBLIC neutvect_eventinput_plugin NEUT::ReWeight nuis_options)

  set_target_properties(NReWeight_weightcalc_plugin PROPERTIES PREFIX "nuisplugin-weightcalc-")
  set_target_properties(NReWeight_weightcalc_plugin PROPERTIES OUTPUT_NAME "NReWeight")

  target_compile_definitions(NReWeight_weightcalc_plugin INTERFACE NUIS_WEIGHT_CALC_NReWeight_Enabled)

  install(TARGETS NReWeight_weightcalc_plugin DESTINATION ${NUISANCE_PLUGIN_LIBDIR})

  target_link_libraries(weightcalc_plugins INTERFACE NReWeight_weightcalc_plugin)
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
  add_library(GENIEReWeight_weightcalc_plugin SHARED GENIEReWeightCalc.cxx)
  target_link_libraries(GENIEReWeight_weightcalc_plugin PUBLIC GHEP3_eventinput_plugin)

  set_target_properties(GENIEReWeight_weightcalc_plugin PROPERTIES PREFIX "nuisplugin-weightcalc-")
  set_target_properties(GENIEReWeight_weightcalc_plugin PROPERTIES OUTPUT_NAME "GENIEReWeight")

  target_compile_definitions(GENIEReWeight_weightcalc_plugin INTERFACE NUIS_WEIGHT_CALC_GENIEReWeight_Enabled)

  install(TARGETS GENIEReWeight_weightcalc_plugin DESTINATION ${NUISANCE_PLUGIN_LIBDIR})

  target_link_libraries(weightcalc_plugins INTERFACE GENIEReWeight_weightcalc_plugin)
endif()

if(TARGET Prob3plusplus::Prob3plusplus)
  add_library(Prob3plusplus_weightcalc_plugin SHARED Prob3plusplusWeightCalc.cxx)
  target_link_libraries(Prob3plusplus_weightcalc_plugin PUBLIC eventinput Prob3plusplus::Prob3plusplus nuis_options)

  set_target_properties(Prob3plusplus_weightcalc_plugin PROPERTIES PREFIX "nuisplugin-weightcalc-")
  set_target_properties(Prob3plusplus_weightcalc_plugin PROPERTIES OUTPUT_NAME "Prob3plusplus")

  target_compile_definitions(Prob3plusplus_weightcalc_plugin INTERFACE NUIS_WEIGHT_CALC_Prob3plusplus_Enabled)

  install(TARGETS Prob3plusplus_weightcalc_plugin DESTINATION ${NUISANCE_PLUGIN_LIBDIR})

  target_link_libraries(weightcalc_plugins INTERFACE Prob3plusplus_weightcalc_plugin)
endif()

find_package(T2KReWeight QUIET)
if(FALSE AND T2KReWeight_FOUND)
  add_library(T2KReWeight_weightcalc_plugin SHARED T2KReWeightCalc.cxx)
  target_link_libraries(T2KReWeight_weightcalc_plugin PUBLIC neutvect_eventinput_plugin T2KReWeight::All nuis_options)

  set_target_properties(T2KReWeight_weightcalc_plugin PROPERTIES PREFIX "nuisplugin-weightcalc-")
  set_target_properties(T2KReWeight_weightcalc_plugin PROPERTIES OUTPUT_NAME "T2KReWeight")

  target_compile_definitions(T2KReWeight_weightcalc_plugin INTERFACE NUIS_WEIGHT_CALC_T2KReWeight_Enabled)
  
  install(TARGETS T2KReWeight_weightcalc_plugin DESTINATION ${NUISANCE_PLUGIN_LIBDIR})

  target_link_libraries(weightcalc_plugins INTERFACE T2KReWeight_weightcalc_plugin)
endif()

find_package(nusystematics QUIET)
if(nusystematics_FOUND)
  add_library(nusystematics_weightcalc_plugin SHARED nusystematicsWeightCalc.cxx)
  target_link_libraries(nusystematics_weightcalc_plugin PUBLIC GHEP3_eventinput_plugin nusyst::all nuis_options ROOT::Hist)

  set_target_properties(nusystematics_weightcalc_plugin PROPERTIES PREFIX "nuisplugin-weightcalc-")
  set_target_properties(nusystematics_weightcalc_plugin PROPERTIES OUTPUT_NAME "nusystematics")

  target_compile_definitions(nusystematics_weightcalc_plugin INTERFACE NUIS_WEIGHT_CALC_nusystematics_Enabled)
  
  install(TARGETS nusystematics_weightcalc_plugin DESTINATION ${NUISANCE_PLUGIN_LIBDIR})

  target_link_libraries(weightcalc_plugins INTERFACE nusystematics_weightcalc_plugin)
endif()

install(TARGETS weightcalc_plugins DESTINATION ${NUISANCE_PLUGIN_LIBDIR})