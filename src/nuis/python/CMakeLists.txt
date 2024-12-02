find_package(ROOT REQUIRED)

set(SOURCES 
  pyNUISANCE.cxx 
  pyYAML.cxx 
  pyEventInput.cxx 
  pyEventFrame.cxx 
  pyHistFrame.cxx 
  pyHistFrameFillers.cxx
  pyWeightCalc.cxx 
  pyNUISANCELog.cxx 
  pyBinning.cxx
  pyConvert.cxx
  pyResponse.cxx)

  
if(NUISANCE_USE_HEPDATA)
  LIST(APPEND SOURCES pyRecord.cxx)
endif()

pybind11_add_module(_pyNUISANCE SHARED ${SOURCES})

target_link_libraries(_pyNUISANCE 
  PRIVATE 
    eventinput 
    eventframe 
    histframe 
    weightcalc 
    convert
    response)

if(NUISANCE_USE_HEPDATA)
  target_link_libraries(_pyNUISANCE 
    PRIVATE 
      record)
  target_compile_definitions(_pyNUISANCE PRIVATE NUISANCE_USE_HEPDATA)
endif()

if(NUIS_ARROW_ENABLED)
  target_link_libraries(_pyNUISANCE 
    PRIVATE 
      arrow_python_shared)
endif()

target_link_libraries(_pyNUISANCE 
  PRIVATE 
    pybind11::module
    ROOT::Hist)

target_include_directories(_pyNUISANCE PRIVATE
  ${Python3_INCLUDE_DIRS})

install(TARGETS _pyNUISANCE
    LIBRARY DESTINATION ${NUISANCE3_PYTHONPATH}/pyNUISANCE)

add_subdirectory(modules)
