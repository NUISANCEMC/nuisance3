add_executable(Binning_tests Binning_tests.cxx)
target_link_libraries(Binning_tests PRIVATE Catch2::Catch2WithMain histframe)
target_include_directories(Binning_tests PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}../>)

catch_discover_tests(Binning_tests)

if(TARGET ROOT::Hist)
  add_executable(Binning_benchmarking Binning_benchmarking.cxx)
  target_link_libraries(Binning_benchmarking PRIVATE Catch2::Catch2WithMain histframe ROOT::Hist)
  target_include_directories(Binning_benchmarking PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}../>)
endif()

# add_executable(Projection_tests Projection_tests.cxx)
# target_link_libraries(Projection_tests PRIVATE Catch2::Catch2WithMain histframe)
# target_include_directories(Projection_tests PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}../>)

# catch_discover_tests(Projection_tests)

add_executable(EventFrame_tests EventFrame_tests.cxx)
target_link_libraries(EventFrame_tests PRIVATE Catch2::Catch2WithMain eventframe)
target_include_directories(EventFrame_tests PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}../>)

catch_discover_tests(EventFrame_tests)

add_executable(Response_tests Response_tests.cxx)
target_link_libraries(Response_tests PRIVATE Catch2::Catch2WithMain response)
target_include_directories(Response_tests PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}../>)

catch_discover_tests(Response_tests)