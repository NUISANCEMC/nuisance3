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

### GRUMPY LUKE SAYS: I think the RPATH stuff above should allow running from the build dir
# Hack for build tree allow running directly from build
# set(CMAKE_INCLUDE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/include)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Force position independent code as issues with some python libraries if not
### GRUMPY LUKE SAYS: This isn't the best way to do this, add it to an interface target and link that
###                   target to the python libs. We could add this to nuis_options for example. 
###                   Setting global cmake options like this will break stuff, especially 
###                   CPM-pulled-in dependencies.
# set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC")


######################### Dependencies

include(CPM)

### GRUMPY LUKE SAYS: Do we need this? #WorksForMe fully CPM without it
# Use a package lock
# CPMUsePackageLock(package-lock.cmake)

CPMFindPackage(
    NAME ProSelecta
    GITHUB_REPOSITORY NUISANCEMC/ProSelecta
    GIT_TAG main
)

CPMFindPackage(
    NAME fmt
    GIT_TAG 8.1.1
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL ON"
)

CPMFindPackage(
    NAME spdlog
    GIT_TAG v1.10.0
    GITHUB_REPOSITORY gabime/spdlog
    OPTIONS
        "SPDLOG_COMPILED_LIB ON"
        "SPDLOG_BUILD_SHARED ON"
        "SPDLOG_INSTALL ON"
)

find_package(Boost 1.70.0 COMPONENTS system filesystem REQUIRED)

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

CPMFindPackage(
  NAME Eigen3
  VERSION 3.4.0
  GITLAB_REPOSITORY "libeigen/eigen"
  GIT_TAG "3.4.0"
  OPTIONS "EIGEN_BUILD_DOC no" "BUILD_TESTING no"
)

# CPMFindPackage(
#   NAME cppzmq
#   GIT_TAG v4.8.1
#   GIT_REPOSITORY "https://github.com/zeromq/cppzmq"
# )

CPMFindPackage(
    NAME pybind11
    VERSION 2.9.2
    GITHUB_REPOSITORY pybind/pybind11
    GIT_TAG "v2.9.2"
)

add_library(nuis_options INTERFACE)
target_include_directories(nuis_options INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(nuis_options INTERFACE ProSelecta::Interpreter Boost::boost Boost::filesystem yaml-cpp::yaml-cpp spdlog::spdlog fmt::fmt Eigen3::Eigen)
target_compile_options(nuis_options INTERFACE -Wall -Werror -Wextra)

# PYTHON PATHS
set(NUISANCE_PYSITEARCH "${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")
set(NUISANCE_PYTHONPATH "python/${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/nuis/eventinput)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/nuis/measurement)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/nuis/weightcalc)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/app)

# BUILD SETUP
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/setup.NUISANCEV3.sh.in ${PROJECT_BINARY_DIR}/setup.NUISANCEV3.sh @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/setup.NUISANCEV3.sh DESTINATION bin)
