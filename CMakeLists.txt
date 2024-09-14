################################################################################
# MIT License

# Copyright (c) 2024 NUISANCE

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
################################################################################

cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

#Use the compilers found in the path
find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

set (NUISANCE_VERSION 3.0.1)

project(NUISANCE3 VERSION ${NUISANCE_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

#Changes default install path to be a subdirectory of the build dir.
#Can set build dir at configure time with -DCMAKE_INSTALL_PREFIX=/install/path
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/"
           CACHE PATH "default install path" FORCE)
endif()

option(BUILTIN_NuWro "Whether to use build our own version of NuWro" ON)
option(NUISANCE_ENABLE_TESTS "Whether to enable test suite" OFF)
option(NUISANCE_ENABLE_SANITIZERS "Whether to enable ASAN LSAN and UBSAN" OFF)
option(NUISANCE_ENABLE_GCOV "Whether to enable GCOV" OFF)
option(NUISANCE_ENABLE_GPERFTOOLS "Whether to enable GPERFTOOLS" OFF)
option(NUISANCE_LEGACY_INTERFACE "Whether to try and link in NUISANCE2" OFF)
option(NUISANCE_USE_BOOSTDLL "Whether to use plugins as BOOST DLL plugins or just compile them in" OFF)
option(NUISANCE_USE_ARROW "Whether to use Apache Arrow and pyArrow" OFF)

if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  if(NUISANCE_ENABLE_TESTS)
    set(CMAKE_BUILD_TYPE Debug)
  else()
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
  endif()
endif()

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)

#### RPATH handling #####

set(CMAKE_MACOSX_RPATH 1)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
ENDIF("${isSystemDir}" STREQUAL "-1")

######################### Dependencies

include(CPM)

CPMFindPackage(
  NAME Eigen3
  VERSION 3.4.0
  GITLAB_REPOSITORY "libeigen/eigen"
  GIT_TAG "3.4.0"
  OPTIONS "EIGEN_BUILD_DOC no" "BUILD_TESTING no"
)

# try and find Protobuf as HepMC3 is broke
find_package(Protobuf 2.6 QUIET)
find_package(HepMC3 3.3 REQUIRED)
CPMFindPackage(
  NAME NuHepMC_CPPUtils
  GIT_TAG main
  GIT_REPOSITORY "https://github.com/NuHepMC/cpputils.git"
)

CPMFindPackage(
    NAME ProSelecta
    GITHUB_REPOSITORY NUISANCEMC/ProSelecta
    GIT_TAG main
)

CPMAddPackage(
    NAME fmt
    VERSION 8.1.1
    GIT_TAG 8.1.1
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL ON"
)

CPMAddPackage(
    NAME spdlog
    VERSION 1.10.0
    GIT_TAG v1.10.0
    GITHUB_REPOSITORY gabime/spdlog
    OPTIONS
        "SPDLOG_COMPILED_LIB ON"
        "SPDLOG_BUILD_SHARED ON"
        "SPDLOG_INSTALL ON"
)

CPMFindPackage(
    NAME yaml-cpp
    VERSION 0.8.0
    GITHUB_REPOSITORY "jbeder/yaml-cpp"
    GIT_TAG "0.8.0"
    OPTIONS "YAML_CPP_INSTALL ON"
            "YAML_CPP_BUILD_TESTS OFF"
            "YAML_CPP_BUILD_CONTRIB OFF"
            "YAML_BUILD_SHARED_LIBS ON"
)


find_package(Python3 REQUIRED COMPONENTS Development Interpreter)

# PYTHON PATHS
set(NUISANCE3_PYVMAJMIN "${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
set(NUISANCE3_PYTHONPATH "python/${NUISANCE3_PYVMAJMIN}")

CPMFindPackage(
    NAME pybind11
    VERSION 2.9.2
    GITHUB_REPOSITORY pybind/pybind11
    GIT_TAG "v2.9.2"
)

if(NUISANCE_USE_ARROW)
  find_package(Arrow QUIET)
  find_package(ArrowPython QUIET HINTS ${Arrow_DIR})
  set(NUIS_ARROW_ENABLED FALSE)
  if(Arrow_FOUND AND ArrowPython_FOUND)
    set(NUIS_ARROW_ENABLED TRUE)
  endif()
endif()

CPMFindPackage(
    NAME Prob3plusplus
    VERSION 3.10.4
    GITHUB_REPOSITORY rogerwendell/Prob3plusplus
    GIT_TAG main
)

if(NOT TARGET Prob3plusplus::Prob3plusplus AND TARGET Prob3plusplus)
  add_library(Prob3plusplus::Prob3plusplus ALIAS Prob3plusplus)
endif()
add_library(nuis_options INTERFACE)
target_include_directories(nuis_options INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(nuis_options INTERFACE ProSelecta::Interpreter NuHepMC::CPPUtils 
  yaml-cpp::yaml-cpp spdlog::spdlog fmt::fmt Eigen3::Eigen OpenMP::OpenMP_CXX)
target_compile_options(nuis_options INTERFACE -Wall -Werror -Wextra)
target_link_options(nuis_options INTERFACE)

if(NUISANCE_ENABLE_SANITIZERS)
  target_compile_options(nuis_options INTERFACE -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined)
  target_link_options(nuis_options INTERFACE -fsanitize=address -fsanitize=leak -fsanitize=undefined)
endif()

if(NUISANCE_ENABLE_GCOV)
  target_compile_options(nuis_options INTERFACE -fprofile-arcs -ftest-coverage)
  target_link_options(nuis_options INTERFACE -fprofile-arcs -ftest-coverage)
endif()

if(NUISANCE_ENABLE_GPERFTOOLS)
  target_link_options(nuis_options INTERFACE -ltcmalloc_and_profiler)
endif()

if(NUIS_ARROW_ENABLED)
  target_compile_definitions(nuis_options INTERFACE NUIS_ARROW_ENABLED)
  target_link_libraries(nuis_options INTERFACE arrow_shared)
endif()

if(CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo OR CMAKE_BUILD_TYPE STREQUAL Release)
  target_compile_definitions(nuis_options INTERFACE NUIS_NDEBUG)
endif()

if(NOT DEFINED NUISANCE_LOG_LEVEL)
  if(CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo OR CMAKE_BUILD_TYPE STREQUAL Release)
    SET(NUISANCE_LOG_LEVEL warn)
  else()
    SET(NUISANCE_LOG_LEVEL trace)
  endif()
endif()
if(NUISANCE_LOG_LEVEL STREQUAL trace)
  target_compile_definitions(nuis_options INTERFACE NUIS_ACTIVE_LEVEL=NUIS_LEVEL_TRACE)
elseif(NUISANCE_LOG_LEVEL STREQUAL debug)
  target_compile_definitions(nuis_options INTERFACE NUIS_ACTIVE_LEVEL=NUIS_LEVEL_DEBUG)
elseif(NUISANCE_LOG_LEVEL STREQUAL info)
  target_compile_definitions(nuis_options INTERFACE NUIS_ACTIVE_LEVEL=NUIS_LEVEL_INFO)
elseif(NUISANCE_LOG_LEVEL STREQUAL warn)
  target_compile_definitions(nuis_options INTERFACE NUIS_ACTIVE_LEVEL=NUIS_LEVEL_WARN)
elseif(NUISANCE_LOG_LEVEL STREQUAL error)
  target_compile_definitions(nuis_options INTERFACE NUIS_ACTIVE_LEVEL=NUIS_LEVEL_ERROR)
elseif(NUISANCE_LOG_LEVEL STREQUAL critical)
  target_compile_definitions(nuis_options INTERFACE NUIS_ACTIVE_LEVEL=NUIS_LEVEL_CRITICAL)
else()
  message(FATAL_ERROR "Invalid NUISANCE_LOG_LEVEL set: ${NUISANCE_LOG_LEVEL}. Should be one of: trace, debug, info, warn, error, critical.")
endif()
target_compile_definitions(nuis_options INTERFACE NUISANCE_LOG_ENABLED)

find_package(Boost 1.70.0 COMPONENTS system filesystem QUIET)
if(Boost_FOUND)
  target_link_libraries(nuis_options INTERFACE Boost::boost Boost::filesystem)
else()
  set(BOOST_DEPS metaparse config mpl predef preprocessor static_assert type_traits core utility assert io throw_exception)

  foreach(bd ${BOOST_DEPS})
    CPMAddPackage(
      NAME boost_${bd}
      VERSION 1.85.0
      GITHUB_REPOSITORY "boostorg/${bd}"
      GIT_TAG "boost-1.85.0"
      OPTIONS "BUILD_TESTING OFF"
    )
  endforeach()
  
  target_link_libraries(nuis_options INTERFACE boost_metaparse)

endif()

if(NUISANCE_USE_BOOSTDLL)
  if(Boost_FOUND)
    target_compile_definitions(nuis_options INTERFACE NUISANCE_USE_BOOSTDLL)
    set(NUISANCE_PLUGIN_LIBDIR lib/plugin)
  else()
    find_package(Boost 1.70.0 COMPONENTS system filesystem REQUIRED)
  endif()
else()
  set(NUISANCE_PLUGIN_LIBDIR lib)
endif()

find_package(OpenMP QUIET)
if(OpenMP_FOUND)
  target_link_libraries(nuis_options INTERFACE OpenMP::OpenMP_CXX)
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/nuis)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/examples/c++)

# BUILD SETUP
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/setup.NUISANCE3.sh.in 
  ${PROJECT_BINARY_DIR}/setup.NUISANCE3.sh @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/setup.NUISANCE3.sh DESTINATION bin)

if(NUISANCE_ENABLE_TESTS)

  find_package(Catch2)
  if(NOT Catch2_FOUND)
    CPMAddPackage("gh:catchorg/Catch2@3.5.2")
    LIST(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
  endif()

  include(CTest)
  include(Catch)

  add_subdirectory(tests)
endif()

add_subdirectory(scripts/cli)

