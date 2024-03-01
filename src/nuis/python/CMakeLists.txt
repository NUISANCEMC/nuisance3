pybind11_add_module(pyNUISANCE SHARED pyNUISANCE.cxx)

target_link_libraries(pyNUISANCE PRIVATE eventinput frame histframe weightcalc_plugins nuis_options NuHepMC::CPPUtils pybind11::module)

target_include_directories(pyNUISANCE PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
  ${Python3_INCLUDE_DIRS})

install(TARGETS pyNUISANCE
    LIBRARY DESTINATION ${NUISANCE_PYTHONPATH})