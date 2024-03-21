pybind11_add_module(pyNUISANCE SHARED 
  pyNUISANCE.cxx 
  pyYAML.cxx 
  pyEventInput.cxx 
  pyEventFrame.cxx 
  pyHistFrame.cxx 
  pyRecord.cxx 
  pyWeightCalc.cxx 
  pyNUISANCELog.cxx 
  pyBinning.cxx
  pyConvert.cxx
  )

target_link_libraries(pyNUISANCE 
  PRIVATE 
    eventinput 
    eventframe 
    histframe 
    weightcalc 
    weightcalc_plugins 
    record 
    convert
    NuHepMC::CPPUtils 
    pybind11::module)

target_include_directories(pyNUISANCE PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
  ${Python3_INCLUDE_DIRS})

install(TARGETS pyNUISANCE
    LIBRARY DESTINATION ${NUISANCE_PYTHONPATH})