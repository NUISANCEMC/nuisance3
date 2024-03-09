add_subdirectory(plugins)

add_library(record SHARED RecordFactory.cxx Utility.cxx)

target_link_libraries(record PUBLIC nuis_options)

install(TARGETS record DESTINATION lib)
