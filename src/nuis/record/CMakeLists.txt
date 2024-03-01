add_library(record SHARED HEPDataRecordFactory.cxx Table.cxx Comparison.cxx)

target_link_libraries(record PUBLIC nuis_options)

install(TARGETS record DESTINATION lib)
