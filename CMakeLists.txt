################################################################################
# MIT License

# Copyright (c) 2022 NUISANCE/hist

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

project(NUISANCE_eventinput)

set (NUISANCE_eventinput_VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

#Changes default install path to be a subdirectory of the build dir.
#Can set build dir at configure time with -DCMAKE_INSTALL_PREFIX=/install/path
if(CMAKE_INSTALL_PREFIX STREQUAL "" OR CMAKE_INSTALL_PREFIX STREQUAL
  "/usr/local")
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}")
elseif(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
elseif(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
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

# Hack for build tree allow running directly from build
set(CMAKE_INCLUDE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/include)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Force position independent code as issues with some python libraries if not 
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC")


######################### Dependencies

include(CPM)

# Use a package lock
CPMUsePackageLock(package-lock.cmake)

CPMFindPackage(
    NAME fmt
    GIT_TAG 8.1.1
    GITHUB_REPOSITORY fmtlib/fmt
)

CPMFindPackage(
    NAME spdlog
    GIT_TAG v1.10.0
    GITHUB_REPOSITORY gabime/spdlog
    OPTIONS
        "SPDLOG_COMPILED_LIB ON"
        "SPDLOG_FMT_EXTERNAL ON"
        "SPDLOG_BUILD_SHARED ON"
)

find_package(Boost 1.70.0 COMPONENTS system filesystem REQUIRED)

CPMFindPackage(
    NAME yaml-cpp
    VERSION 0.6.3
    GITHUB_REPOSITORY "jbeder/yaml-cpp"
    GIT_TAG "yaml-cpp-0.6.3"
)

CPMAddPackage(
  NAME Eigen
  VERSION 3.2.8
  URL https://gitlab.com/libeigen/eigen/-/archive/3.2.8/eigen-3.2.8.tar.gz
  # Eigen's CMakelists are not intended for library use
  DOWNLOAD_ONLY YES 
)

if(Eigen_ADDED)
  add_library(Eigen INTERFACE IMPORTED)
  target_include_directories(Eigen INTERFACE ${Eigen_SOURCE_DIR})
endif()

# CPMFindPackage(
#   NAME cppzmq
#   GIT_TAG v4.8.1
#   GIT_REPOSITORY "https://github.com/zeromq/cppzmq"
# )

CPMAddPackage(
  NAME NuHepMC_CPPUtils
  GIT_TAG main
  GIT_REPOSITORY "https://github.com/NuHepMC/cpputils.git"
  OPTIONS "BUILTIN_HEPMC3 ON"
          "NuHepMC_BUILD_STATIC_LIBS OFF"
)

CPMFindPackage(
    NAME pybind11
    VERSION 2.9.2
    GITHUB_REPOSITORY pybind/pybind11
    GIT_TAG v2.9.2
)

find_package(ROOT 6.10)

find_package(ProSelecta REQUIRED)


add_library(nuis_options INTERFACE)
target_include_directories(nuis_options INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(nuis_options INTERFACE HepMC3::All Boost::boost Boost::filesystem yaml-cpp spdlog::spdlog PROSELECTA::IO Eigen)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/nuis/eventinput)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/nuis/measurement)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/app)

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/setup.nuis-eventinput.sh.in
  "${PROJECT_BINARY_DIR}/setup.nuis-eventinput.sh" @ONLY)
install(PROGRAMS
  "${PROJECT_BINARY_DIR}/setup.nuis-eventinput.sh" DESTINATION ${CMAKE_INSTALL_PREFIX})


  
# PYTHON PATHS
set(NUISANCE_PYSITEARCH "${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")
set(NUISANCE_PYTHONPATH "python/${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
 
# BUILD SETUP
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/setup.NUISANCEV3.sh.in ${PROJECT_BINARY_DIR}/setup.NUISANCEV3.sh @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/setup.NUISANCEV3.sh DESTINATION bin)
