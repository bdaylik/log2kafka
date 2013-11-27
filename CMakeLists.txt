#
# Copyright 2013 Produban
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
set(CMAKE_LEGACY_CYGWIN_WIN32 0)
cmake_minimum_required (VERSION 2.6)

project (log2kafka)

set (TARGET_VERSION_MAJOR 1)
set (TARGET_VERSION_MINOR 0)
set (TARGET_VERSION "${TARGET_VERSION_MAJOR}.${TARGET_VERSION_MINOR}")
set (${CMAKE_PROJECT_NAME}_VERSION "${TARGET_VERSION}")

if (WIN32 AND NOT CYGWIN AND NOT MSYS)
    add_definitions (/EHa)
    add_definitions (
        -DBOOST_REGEX_DYN_LINK
        -DBOOST_FILESYSTEM_DYN_LINK
        -DBOOST_SYSTEM_DYN_LINK
        -DBOOST_PROGRAM_OPTIONS_DYN_LINK
        -DBOOST_ALL_NO_LIB)
endif ()

# Compiler-specific C++11 activation.
#if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
#    execute_process(
#        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
#    if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
#        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
#    endif ()
#elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
#else ()
#    message(FATAL_ERROR "Your C++ compiler does not support C++11.")
#endif ()

# GCC specifics
if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} -Wall -std=c++0x")
    set (CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
    set (CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os -DNDEBUG")
    set (CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} -O4 -DNDEBUG")
    set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g")

# Visual Studio specifics
elseif (MSVC)
    # Warning Level 4 for Debug builds
    list(APPEND CMAKE_CXX_FLAGS_DEBUG " /W4")
    list(REMOVE_DUPLICATES CMAKE_CXX_FLAGS_DEBUG)

endif ()

set (CMAKE_PREFIX_PATH "/usr/local")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Find Boost

message ("\nLooking for Boost headers and libraries")
find_package (Boost 1.40 COMPONENTS filesystem system program_options REQUIRED)
add_definitions (${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
include_directories (${Boost_INCLUDE_DIRS})
message (STATUS "Include directories: ${Boost_INCLUDE_DIR}")
message (STATUS "Libraries: ${Boost_LIBRARIES}")


# Find ZLib

message ("\nLooking for ZLib codec headers and libraries")
find_package(ZLIB REQUIRED)
message (STATUS "Include directories: ${ZLIB_INCLUDE_DIRS}")
message (STATUS "Libraries: ${ZLIB_LIBRARIES}")

if (ZLIB_FOUND)
    set (ZLIB_PKG zlib)
    add_definitions (-DDEFLATE_CODEC)
    include_directories (${ZLIB_INCLUDE_DIRS})
    message (STATUS "** Enabled deflate codec **")

else ()
    set (ZLIB_PKG "")
    set (ZLIB_LIBRARIES "")
    message (STATUS "** Disabled deflate codec. zlib not found. **")

endif ()


# Find Snappy

message ("\nLooking for Snappy codec headers and libraries")
find_package(Snappy)

if (SNAPPY_FOUND AND ZLIB_FOUND)  # Snappy borrows crc32 from zlib
    set (SNAPPY_PKG libsnappy)
    add_definitions (-DSNAPPY_CODEC)
    include_directories (${SNAPPY_INCLUDE_DIRS})
    message (STATUS "** Enabled snappy codec **")

else ()
    set (SNAPPY_PKG "")
    set (SNAPPY_LIBRARIES "")
    message (STATUS "** Disabled snappy codec. libsnappy not found or zlib not found. **")

endif ()

# Find others

find_package (ApachePortableRuntime REQUIRED)
find_package (Log4CXX REQUIRED)
find_package (KafkaCPP REQUIRED)
find_package (AvroCPP REQUIRED)

include_directories (
    ${KAFKA_INCLUDE_DIRS}
    ${AVRO_INCLUDE_DIRS}
    ${APR_INCLUDE_DIRS}
    ${LOG4CXX_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
)

set (LIBS
    ${LIBS}
    ${AVRO_LIBRARIES}
    ${KAFKA_LIBRARIES}
    ${LOG4CXX_LIBRARIES}
    ${APR_LIBRARIES}
    ${Boost_LIBRARIES}
    ${SNAPPY_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

include (InstallRequiredSystemLibraries)


# Package settings

set (CPACK_SET_DESTDIR ON)
set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
#set (CPACK_PACKAGING_INSTALL_PREFIX "")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "${CMAKE_PROJECT_NAME} - Log to Kafka Producer")
set (CPACK_PACKAGE_VENDOR "Produban")
set (CPACK_PACKAGE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "0")
set (CPACK_PACKAGE_VERSION_PATCH "0")
set (CPACK_PACKAGE_INSTALL_DIRECTORY "/")

if (WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  set (CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  set (CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${CMAKE_PROJECT_NAME}.exe")
  set (CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
  #set (CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.${CMAKE_PROJECT_NAME}.org")
  #set (CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.${CMAKE_PROJECT_NAME}.com")
  #set (CPACK_NSIS_CONTACT "produban@${CMAKE_PROJECT_NAME}.com")
  #set (CPACK_NSIS_MODIFY_PATH ON)

else ()
  set (CPACK_STRIP_FILES "bin/${CMAKE_PROJECT_NAME}")
  set (CPACK_SOURCE_STRIP_FILES "")

endif ()

if (CYGWIN)
    set (CPACK_CYGWIN_PATCH_NUMBER "0")
endif ()

set (CPACK_PACKAGE_EXECUTABLES "${CMAKE_PROJECT_NAME}" "${CMAKE_PROJECT_NAME}")

include (CPack)

add_subdirectory (src)
