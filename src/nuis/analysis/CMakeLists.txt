#add_library(MEASUREMENT SHARED Bin.cxx   HEPDataLoader.cxx  Record.cxx  SimpleStatistics.cxx)
#target_link_libraries(MEASUREMENT PUBLIC nuis_options)
#install(TARGETS MEASUREMENT DESTINATION lib)

pybind11_add_module(pyNuisAnalysis SHARED pyAnalysis.cxx)

target_link_libraries(pyNuisAnalysis PRIVATE pybind11::embed nuis_options)
set_target_properties(pyNuisAnalysis PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_include_directories(pyNuisAnalysis PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
  ${Python3_INCLUDE_DIRS})

install(TARGETS pyNuisAnalysis
    LIBRARY DESTINATION ${NUISANCE_PYTHONPATH}/)
