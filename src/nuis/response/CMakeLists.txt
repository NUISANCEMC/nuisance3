add_library(response SHARED FramedResponse.cxx)

target_link_libraries(response PUBLIC binning eventframe nuis_options)

install(TARGETS response DESTINATION lib)