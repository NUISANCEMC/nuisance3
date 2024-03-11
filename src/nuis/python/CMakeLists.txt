pybind11_add_module(pyNUISANCE SHARED pyNUISANCE.cxx pyYAML.cxx pyEventInput.cxx pyFrame.cxx pyHistFrame.cxx pyRecord.cxx pyWeightCalc.cxx pyNUISANCELog.cxx pyConvert.cxx)

target_link_libraries(pyNUISANCE PRIVATE eventinput frame histframe weightcalc weightcalc_plugins record NuHepMC::CPPUtils pybind11::module)

target_include_directories(pyNUISANCE PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
  ${Python3_INCLUDE_DIRS})

install(TARGETS pyNUISANCE
    LIBRARY DESTINATION ${NUISANCE_PYTHONPATH})