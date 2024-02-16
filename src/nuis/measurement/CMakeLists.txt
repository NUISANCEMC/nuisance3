add_library(measurement SHARED HEPDataLoader.cxx  Projection.cxx  SimpleStatistics.cxx)

target_link_libraries(measurement PUBLIC nuis_options)

install(TARGETS measurement DESTINATION lib)
