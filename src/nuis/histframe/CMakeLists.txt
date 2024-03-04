add_library(histframe SHARED Binning.cxx BinningUtility.cxx HistFrame.cxx HistProjector.cxx Utility.cxx)

target_link_libraries(histframe PUBLIC nuis_options)

install(TARGETS histframe DESTINATION lib)