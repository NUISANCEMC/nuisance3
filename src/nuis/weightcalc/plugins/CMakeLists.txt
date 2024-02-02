find_package(NEUT 5.5.0 QUIET)
if(NEUT_FOUND)
  add_library(NReWeight_weightcalc_plugin STATIC NReWeightCalc.cxx)
  target_link_libraries(NReWeight_weightcalc_plugin PUBLIC neutvect_eventinput_plugin NEUT::ReWeight nuis_options)

  # Install the plugin into the plugin directory will a well-formed static plugin name that
  # the dynamic plugin loader can used to determine the symbol name to fetch from the main 
  # executable
  set_target_properties(NReWeight_weightcalc_plugin PROPERTIES PREFIX "nuisplugin-static-weightcalc-")
  set_target_properties(NReWeight_weightcalc_plugin PROPERTIES OUTPUT_NAME "NReWeight")

  install(TARGETS NReWeight_weightcalc_plugin DESTINATION lib/plugins)

  target_link_libraries(nuis_snowflakes INTERFACE NReWeight_weightcalc_plugin)

  # This stops the symbol stripper from thinking that we never used this symbol
  target_link_options(nuis_snowflakes INTERFACE -Wl,--undefined=MakeWeightCalc_NReWeight)

endif()