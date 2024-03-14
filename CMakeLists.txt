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
    VERSION 8.1.1
    GIT_TAG 8.1.1
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL ON"
)

CPMFindPackage(
    NAME spdlog
    VERSION 1.10.0
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

find_package(Arrow REQUIRED)
set(NUIS_ARROW_ENABLED FALSE)
if(Arrow_FOUND)
  set(NUIS_ARROW_ENABLED TRUE)
endif()

# PYTHON PATHS
set(NUISANCE_PYSITEARCH "${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")
set(NUISANCE_PYTHONPATH "python/${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")

add_library(nuis_options INTERFACE)
target_include_directories(nuis_options INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(nuis_options INTERFACE ProSelecta::Interpreter Boost::boost Boost::filesystem yaml-cpp::yaml-cpp spdlog::spdlog fmt::fmt Eigen3::Eigen)
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
  if(NOT TARGET arrow_shared)
    message(FATAL_ERROR "NUIS_ARROW_ENABLED: ${NUIS_ARROW_ENABLED}, Arrow_FOUND: ${Arrow_FOUND}. AWDUJJWDQIOJDIOJ")
  endif()
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

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/nuis)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/examples/c++)


# Sorry all this below is a mess, I will tidy.... PS

if(NOT CMAKE_PROPERTY_LIST)
    execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)
    
    # Convert command output into a CMake list
    string(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
    string(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
    list(REMOVE_DUPLICATES CMAKE_PROPERTY_LIST)
endif()

function(print_target_properties target)
    if(NOT TARGET ${target})
      message(STATUS "There is no target named '${target}'")
      return()
    endif()

    foreach(property ${CMAKE_PROPERTY_LIST})
        string(REPLACE "<CONFIG>" "DEBUG" property ${property})

        get_property(was_set TARGET ${target} PROPERTY ${property} SET)
        if(was_set)
            get_target_property(value ${target} ${property})
            message("${target} ${property} = ${value}")
        endif()
    endforeach()
    message(STATUS "GOT LIB FOR TARGET '${target}'")
endfunction()

function(clear_global_properties)
    set(GLOBAL_COMPILE_OPTIONS "" CACHE INTERNAL "Global compile options")
    set(GLOBAL_INCLUDE_DIRS "" CACHE INTERNAL "Global include directories")
    set(GLOBAL_COMPILE_DEFS "" CACHE INTERNAL "Global compile definitions")
endfunction()

# Function to recursively collect properties
function(collect_target_properties target)
    get_target_property(link_libs ${target} INTERFACE_LINK_LIBRARIES)
    if(link_libs)
        foreach(lib ${link_libs})
            if(TARGET ${lib})
                collect_target_properties(${lib})
            endif()
        endforeach()
    endif()

    #message("LINK LIBS ${link_libs} ${target}")
    #print_target_properties(${target})
    # Collect properties
    #get_target_property(location ${target} LOCATION)
    get_target_property(include_dirs ${target} INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(compile_defs ${target} INTERFACE_COMPILE_DEFINITIONS)

    #message("${target} ${link_libs} ${location} ${include_dirs} ${compile_defs}")
    #message("GLOBALS ${GLOBAL_LOCATION} ")
    #message("GLOBALS ${GLOBAL_INCLUDE_DIRS} ")

    # Append properties to global properties
    if(location)
      set(GLOBAL_LOCATION "${GLOBAL_LOCATION};${location}" CACHE INTERNAL "Global compile definitions")
    endif()

    if(include_dirs)
      #message("ADDING INCLUDE PATH ${include_dirs} ${GLOBAL_INCLUDE_DIRS}")
      set(GLOBAL_INCLUDE_DIRS "${GLOBAL_INCLUDE_DIRS};${include_dirs}" CACHE INTERNAL "Global compile definitions")
    endif()

    if(compile_defs)
      #message("ADDING COMPILE DEF ${compile_defs} ${GLOBAL_COMPILE_DEFS}")
      set(GLOBAL_COMPILE_DEFS "${GLOBAL_COMPILE_DEFS};${compile_defs}" CACHE INTERNAL "Global compile definitions")
    endif()

  #message("LOCALCOMPILEDEF ${GLOBAL_COMPILE_DEFS}")
  #message("LOCALCOMPILEDIR ${GLOBAL_INCLUDE_DIRS}")
  #message("LOCALCOMPILELOC ${GLOBAL_LOCATION}")

endfunction()


# Call the function with your target
#print_target_properties(nuis_options)
clear_global_properties()
collect_target_properties(nuis_options)

string(REPLACE ";" " -D" GLOBAL_COMPILE_DEFS "${GLOBAL_COMPILE_DEFS}")
string(REPLACE ";" ":" GLOBAL_INCLUDE_DIRS "${GLOBAL_INCLUDE_DIRS}")
string(REPLACE ";" " -L" GLOBAL_LOCATION "${GLOBAL_LOCATION}")


string(REPLACE "$" "" GLOBAL_INCLUDE_DIRS "${GLOBAL_INCLUDE_DIRS}")
string(REPLACE "<" "" GLOBAL_INCLUDE_DIRS "${GLOBAL_INCLUDE_DIRS}")
string(REPLACE "BUILD_INTERFACE:" "" GLOBAL_INCLUDE_DIRS "${GLOBAL_INCLUDE_DIRS}")
string(REPLACE ">" "" GLOBAL_INCLUDE_DIRS "${GLOBAL_INCLUDE_DIRS}")
string(REPLACE "$" "" GLOBAL_INCLUDE_DIRS "${GLOBAL_INCLUDE_DIRS}")

string(REPLACE "-D$<$<NOT $<BOOL ON>> YAML_CPP_STATIC_DEFINE>" "" GLOBAL_COMPILE_DEFS "${GLOBAL_COMPILE_DEFS}")

# message("COMPILEDEF ${GLOBAL_COMPILE_DEFS}")
# message("COMPILEDIR ${GLOBAL_INCLUDE_DIRS}")
# message("COMPILELOC ${GLOBAL_LOCATION}")

set(NUISANCE_COMPILE_DEFS "${GLOBAL_COMPILE_DEFS}")
set(NUISANCE_INCLUDE_DIRS "${GLOBAL_INCLUDE_DIRS}")
set(NUISANCE_LOCATION "${GLOBAL_LOCATION}")

# BUILD SETUP
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/setup.NUISANCEV3.sh.in ${PROJECT_BINARY_DIR}/setup.NUISANCEV3.sh @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/setup.NUISANCEV3.sh DESTINATION bin)

if(NUISANCE_ENABLE_TESTS)

  CPMAddPackage("gh:catchorg/Catch2@3.5.2")

  LIST(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

  include(CTest)
  include(Catch)

  add_subdirectory(tests)
endif()

