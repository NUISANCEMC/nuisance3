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
    weightcalc_plugins 
    record 
    convert
    NuHepMC::CPPUtils 
    pybind11::module
    ROOT::Hist)

target_include_directories(_pyNUISANCE PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
  ${Python3_INCLUDE_DIRS})

install(TARGETS _pyNUISANCE
    LIBRARY DESTINATION ${NUISANCE_PYTHONPATH})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/nuis/python/modules/ DESTINATION ${NUISANCE_PYTHONPATH})

CPMFindPackage(
    NAME nuisancehepdata
    GITHUB_REPOSITORY NUISANCEMC/HEPData
    GIT_TAG main
    DOWNLOAD_ONLY TRUE
)

install(FILES ${nuisancehepdata_SOURCE_DIR}/HepDataRefResolver.py DESTINATION ${NUISANCE_PYTHONPATH}/)