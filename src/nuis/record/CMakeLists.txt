add_subdirectory(plugins)

add_library(record SHARED RecordFactory.cxx Utility.cxx IAnalysis.cxx)

target_link_libraries(record PUBLIC nuis_options)

if(NOT NUISANCE_USE_BOOSTDLL)
  target_link_libraries(record PUBLIC record_plugins)
endif()

install(TARGETS record DESTINATION lib)
