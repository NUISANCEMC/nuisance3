
find_package(NEUT 5.5.0 QUIET)
if(NEUT_FOUND)
  CPMFindPackage(
    NAME nvconv
    GIT_TAG main
    GITHUB_REPOSITORY "neut-devel/neutvect-converter"
  )

  add_library(neutvectEventSource SHARED neutvectEventSource.cxx)
  target_link_libraries(neutvectEventSource PUBLIC nvconv Boost::boost yaml-cpp)

  install(TARGETS neutvectEventSource DESTINATION lib)

endif()
