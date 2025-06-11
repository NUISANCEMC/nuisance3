add_subdirectory(plugins)

add_library(record SHARED RecordFactory.cxx Utility.cxx IAnalysis.cxx)

target_link_libraries(record PUBLIC nuis_options)
#no idea why we need this
target_compile_definitions(record PUBLIC -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS)

if(NOT NUISANCE_USE_BOOSTDLL)
  target_link_libraries(record PUBLIC record_plugins)
endif()

install(TARGETS record DESTINATION lib)
