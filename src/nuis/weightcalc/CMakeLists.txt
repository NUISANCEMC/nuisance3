add_library(weightcalc SHARED WeightCalcFactory.cxx)

target_link_libraries(weightcalc PUBLIC nuis_options)

add_subdirectory(plugins)
if(NOT NUISANCE_USE_BOOSTDLL)
  target_link_libraries(weightcalc PUBLIC weightcalc_plugins)
endif()

install(TARGETS weightcalc DESTINATION lib)