add_library(MEASUREMENT SHARED Bin.cxx   HEPDataLoader.cxx  Record.cxx  SimpleStatistics.cxx)

target_link_libraries(MEASUREMENT PUBLIC nuis_options)

install(TARGETS MEASUREMENT DESTINATION lib)

pybind11_add_module(pyMEASUREMENT SHARED pyMEASUREMENT.cxx)

target_link_libraries(pyMEASUREMENT PRIVATE pybind11::embed MEASUREMENT nuis_options)
set_target_properties(pyMEASUREMENT PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_include_directories(pyMEASUREMENT PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
  ${Python3_INCLUDE_DIRS})

install(TARGETS pyMEASUREMENT
    LIBRARY DESTINATION ${NUISANCE_PYTHONPATH}/)
