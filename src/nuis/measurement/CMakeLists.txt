add_library(measurement SHARED Bin.cxx   HEPDataLoader.cxx  Record.cxx  SimpleStatistics.cxx)

target_link_libraries(measurement PUBLIC nuis_options)

install(TARGETS measurement DESTINATION lib)