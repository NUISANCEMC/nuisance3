find_package(ROOT REQUIRED)

pybind11_add_module(_pyNUISANCE SHARED 
  pyNUISANCE.cxx 
  pyYAML.cxx 
  pyEventInput.cxx 
  pyEventFrame.cxx 
  pyHistFrame.cxx 
  pyHistFrameFillers.cxx
  pyRecord.cxx 
  pyWeightCalc.cxx 
  pyNUISANCELog.cxx 
  pyBinning.cxx
  pyConvert.cxx
  pyResponse.cxx
  )

target_link_libraries(_pyNUISANCE 
  PRIVATE 
    eventinput 
    eventframe 
    histframe 
    weightcalc 
    record 
    convert
    response
    pybind11::module
    ROOT::Hist)

if(NUIS_ARROW_ENABLED)
  target_link_libraries(_pyNUISANCE 
    PRIVATE 
      arrow_python_shared)
endif()

target_include_directories(_pyNUISANCE PRIVATE
  ${Python3_INCLUDE_DIRS})

install(TARGETS _pyNUISANCE
    LIBRARY DESTINATION ${NUISANCE3_PYTHONPATH}/pyNUISANCE)

add_subdirectory(modules)
