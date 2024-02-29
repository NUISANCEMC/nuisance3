add_library(histframe SHARED Binning.cxx HistFrame.cxx Utility.cxx)

target_link_libraries(histframe PUBLIC nuis_options)

install(TARGETS histframe DESTINATION lib)