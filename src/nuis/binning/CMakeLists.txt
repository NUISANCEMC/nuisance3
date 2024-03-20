add_library(binning SHARED Binning.cxx BinningFactories.cxx SingleExtent.cxx utility.cxx)

target_link_libraries(binning PUBLIC nuis_options)

install(TARGETS binning DESTINATION lib)