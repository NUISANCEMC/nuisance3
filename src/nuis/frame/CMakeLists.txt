add_library(frame SHARED FrameGen.cxx Frame.cxx)

target_link_libraries(frame PUBLIC nuis_options eventinput)

install(TARGETS frame DESTINATION lib)