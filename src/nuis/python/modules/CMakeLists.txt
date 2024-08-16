CPMFindPackage(
      NAME CMakeModules
      GIT_TAG stable
      GITHUB_REPOSITORY NuHepMC/CMakeModules
      DOWNLOAD_ONLY
  )
include(${CMakeModules_SOURCE_DIR}/NuHepMCModules.cmake)

find_package(GENIE3 QUIET)
if(GENIE3_FOUND AND GENIEReWeight_ENABLED)
  pybind11_add_module(pyNUISANCEGENIE SHARED 
    pyNUISANCEGENIE.cxx 
    )

  target_link_libraries(pyNUISANCEGENIE 
    PRIVATE 
      GENIE3::All)

  target_include_directories(_pyNUISANCE PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
    ${Python3_INCLUDE_DIRS})

  install(TARGETS pyNUISANCEGENIE
      LIBRARY DESTINATION ${NUISANCE3_PYTHONPATH})
endif()

install(FILES
    __init__.py
    matplotlib_helpers.py
    root_helpers.py
  DESTINATION ${NUISANCE3_PYTHONPATH})

