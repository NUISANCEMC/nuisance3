add_library(histframe SHARED HistFrame.cxx BinnedValues.cxx utility.cxx)

target_link_libraries(histframe PUBLIC binning nuis_options)

install(TARGETS histframe DESTINATION lib)