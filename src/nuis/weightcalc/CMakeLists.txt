add_subdirectory(plugins)

add_library(weightcalc SHARED WeightCalcFactory.cxx)

target_link_libraries(weightcalc PUBLIC nuis_options)

install(TARGETS weightcalc DESTINATION lib)