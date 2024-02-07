find_package(NEUT 5.8.0 QUIET)
if(NEUT_FOUND)
  add_library(NReWeight_weightcalc_plugin SHARED NReWeightCalc.cxx)
  target_link_libraries(NReWeight_weightcalc_plugin PUBLIC neutvect_eventinput_plugin NEUT::ReWeight nuis_options)

  set_target_properties(NReWeight_weightcalc_plugin PROPERTIES PREFIX "nuisplugin-weightcalc-")
  set_target_properties(NReWeight_weightcalc_plugin PROPERTIES OUTPUT_NAME "NReWeight")

  install(TARGETS NReWeight_weightcalc_plugin DESTINATION lib/plugins)
endif()