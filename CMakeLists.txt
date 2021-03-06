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

#
# Options
#

option (USE_LOG4CXX "Use apache log4cxx library" OFF)
option (KAFKA_LINK_STATIC "For static linking of kafka library" OFF)
option (AVRO_LINK_STATIC "For static linking of avro library" OFF)
option (BUILD_DOC "Create and install the API documentation (requires Doxygen)" OFF)

#
# Global configuration
#

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

set (CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/local")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

#
# Find Threads package
#
if (NOT WIN32 AND NOT CYGWIN AND NOT MSYS)
  message ("\nLooking for thread headers and libraries")
  find_package (Threads)
  
  if (Threads_FOUND)
    set (LIBS
	${LIBS}
	${CMAKE_THREAD_LIBS_INIT}
    ) 
  endif ()
endif ()


#
# Find Boost
#

message ("\nLooking for Boost headers and libraries")
find_package (Boost 1.40 COMPONENTS filesystem system program_options REQUIRED)

add_definitions (${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
include_directories (${Boost_INCLUDE_DIRS})

message (STATUS "Include directories: ${Boost_INCLUDE_DIR}")
message (STATUS "Libraries: ${Boost_LIBRARIES}")

#
# Find ZLib
#

message ("\nLooking for ZLib codec headers and libraries")
find_package(ZLIB REQUIRED)

message (STATUS "Include directories: ${ZLIB_INCLUDE_DIRS}")
message (STATUS "Libraries: ${ZLIB_LIBRARIES}")

if (ZLIB_FOUND)
    set (ZLIB_PKG zlib)
    add_definitions (-DDEFLATE_CODEC)
endif ()

# 
# Find Logging 
#

if (USE_LOG4CXX)
    add_definitions (-D_LOG2KAFKA_USE_LOG4CXX_)
    find_package (ApachePortableRuntime REQUIRED)
    find_package (Log4CXX REQUIRED)    
    include_directories (${APR_INCLUDE_DIRS} ${LOG4CXX_INCLUDE_DIRS})
endif ()

# Find Snappy

# message ("\nLooking for Snappy codec headers and libraries")
# find_package(Snappy)

# if (SNAPPY_FOUND AND ZLIB_FOUND)  # Snappy borrows crc32 from zlib
    # set (SNAPPY_PKG libsnappy)
    # add_definitions (-DSNAPPY_CODEC)
    # include_directories (${SNAPPY_INCLUDE_DIRS})
    # message (STATUS "** Enabled snappy codec **")

# else ()
    # set (SNAPPY_PKG "")
    # set (SNAPPY_LIBRARIES "")
    # message (STATUS "** Disabled snappy codec. libsnappy not found or zlib not found. **")

# endif ()


# 
# Find Kafka
#

find_package (KafkaC REQUIRED)

if (KAFKA_LINK_STATIC)
    message (STATUS "Forced static linking of kafka library")
    add_library (kafka STATIC IMPORTED)
else ()
    include_directories (${KAFKA_INCLUDE_DIRS})
    add_library (kafka UNKNOWN IMPORTED)
endif ()

set_property (TARGET kafka PROPERTY IMPORTED_LOCATION ${KAFKA_LIBRARIES})


# 
# Find Avro
#

find_package (AvroCPP REQUIRED)

if (AVRO_LINK_STATIC)
    message (STATUS "Forced static linking of avro library")
    add_library (avro STATIC IMPORTED)
else ()
    include_directories (${AVRO_INCLUDE_DIRS})
    add_library (avro UNKNOWN IMPORTED)
endif ()

set_property (TARGET avro PROPERTY IMPORTED_LOCATION ${AVRO_LIBRARIES})



include_directories (
    ${ZLIB_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
)

set (LIBS
    ${LIBS}
    kafka
    avro
    ${LOG4CXX_LIBRARIES}
    ${APR_LIBRARIES}
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

include (InstallRequiredSystemLibraries)
add_subdirectory (src)

# file (GLOB log4cxx-lib "${LOG4CXX_LIBRARY_DIR}/${LOG4CXX_LIBRARY_NAME}.*")

# install(
    # FILES
        # ${log4cxx-lib}
    # DESTINATION
        # ${CMAKE_PREFIX_PATH}/lib
# )                      

# install(
    # DIRECTORY
        # ${LOG4CXX_INCLUDE_DIRS}
    # DESTINATION
        # ${CMAKE_PREFIX_PATH}/include
# )

#
# Documentation settings
#

if (BUILD_DOC)

    message ("\nConfigure documentation generation")
 
    find_package(Doxygen)
  
    if (NOT DOXYGEN_FOUND)
        message (FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif ()
 
    set (DOXYFILE_IN          ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in    )
    set (DOXYFILE             ${PROJECT_BINARY_DIR}/Doxyfile             )
    set (DOXY_HTML_INDEX_FILE ${CMAKE_CURRENT_BINARY_DIR}/doc/html/index.html)
    set (DOXY_OUTPUT_ROOT     ${CMAKE_CURRENT_BINARY_DIR}                ) # Pasted into Doxyfile.in
    set (DOXY_INPUT           ${PROJECT_SOURCE_DIR}/src                  ) # Pasted into Doxyfile.in
    set (DOXY_EXTRA_FILES     ${CMAKE_CURRENT_SOURCE_DIR}/mainpage.dox   ) # Pasted into Doxyfile.in
 
    configure_file (${DOXYFILE_IN} ${DOXYFILE} @ONLY)
 
    add_custom_command (OUTPUT ${DOXY_HTML_INDEX_FILE}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
        # The following should be ${DOXYFILE} only but it
        # will break the dependency.
        # The optimal solution would be creating a 
        # custom_command for ${DOXYFILE} generation
        # but I still have to figure out how...
        MAIN_DEPENDENCY ${DOXYFILE} ${DOXYFILE_IN}
        DEPENDS ${CMAKE_PROJECT_NAME} # ${DOXY_EXTRA_FILES}
        COMMENT "Generating HTML documentation")
 
    add_custom_target (doc ALL DEPENDS ${DOXY_HTML_INDEX_FILE})
 
    install (
        DIRECTORY 
            ${CMAKE_CURRENT_BINARY_DIR}/doc/html 
        DESTINATION 
            share/doc/${CMAKE_PROJECT_NAME}-${TARGET_VERSION}
    )
endif ()

#
# Package settings
#

message ("\nConfigure packaging")

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
set (CPACK_PACKAGE_EXECUTABLES "${CMAKE_PROJECT_NAME}" "${CMAKE_PROJECT_NAME}")
#set (CPACK_STRIP_FILES "bin/${CMAKE_PROJECT_NAME}")
#set (CPACK_SOURCE_STRIP_FILES "")

if (CYGWIN)
    set (CPACK_CYGWIN_PATCH_NUMBER "0")
endif ()

include (CPack)
