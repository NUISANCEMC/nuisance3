add_library(eventframe SHARED EventFrameGen.cxx EventFrame.cxx column_types.cxx)

target_link_libraries(eventframe PUBLIC nuis_options eventinput)

install(TARGETS eventframe DESTINATION lib)