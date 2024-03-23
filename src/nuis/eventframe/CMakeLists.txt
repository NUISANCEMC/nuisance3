add_library(eventframe SHARED EventFrameGen.cxx EventFrame.cxx)

target_link_libraries(eventframe PUBLIC nuis_options eventinput)

install(TARGETS eventframe DESTINATION lib)