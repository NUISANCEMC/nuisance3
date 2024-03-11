add_library(convert SHARED json.cxx misc.cxx yaml.cxx)

target_link_libraries(convert PUBLIC binning histframe)

install(TARGETS convert DESTINATION lib)