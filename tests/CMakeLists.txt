add_executable(histframe_tests histframe_tests.cxx)
target_link_libraries(histframe_tests PRIVATE Catch2::Catch2WithMain histframe)
target_include_directories(histframe_tests PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}../>)

catch_discover_tests(histframe_tests)