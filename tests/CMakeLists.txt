add_executable(Binning_tests Binning_tests.cxx)
target_link_libraries(Binning_tests PRIVATE Catch2::Catch2WithMain histframe)
target_include_directories(Binning_tests PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}../>)

catch_discover_tests(Binning_tests)

add_executable(Projection_tests Projection_tests.cxx)
target_link_libraries(Projection_tests PRIVATE Catch2::Catch2WithMain histframe)
target_include_directories(Projection_tests PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}../>)

catch_discover_tests(Projection_tests)