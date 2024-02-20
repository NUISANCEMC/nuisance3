add_library(measurement SHARED HEPDataRecord.cxx  Projection.cxx Statistics.cxx)

target_link_libraries(measurement PUBLIC nuis_options)

install(TARGETS measurement DESTINATION lib)
