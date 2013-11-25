#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
# Tries to find Avro headers and libraries.
#
# Usage of this module as follows:
#
#  find_package(Avro)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  AVRO_ROOT_DIR  Set this variable to the root installation of
#                 Avro if the module has problems finding
#                 the proper installation path.
#
# Variables defined by this module:
#
#  Avro_FOUND              System has Avro libs/headers
#  AVRO_LIBRARIES          The Avro libraries
#  AVRO_INCLUDE_DIR        The location of Avro headers

find_package(PkgConfig)
pkg_check_modules(PC_AVRO avro-c)
set(AVRO_DEFINITIONS ${PC_AVRO_CFLAGS_OTHER})
		  
find_path(AVRO_INCLUDE_DIR avro.h
	HINTS ${AVRO_ROOT_DIR}/include ${PC_AVRO_INCLUDEDIR} ${PC_AVRO_INCLUDE_DIRS}
	PATH_SUFFIXES avro
)

find_library(AVRO_LIBRARY 
	NAMES avro
	HINTS ${AVRO_ROOT_DIR}/lib ${PC_AVRO_LIBDIR} ${PC_AVRO_LIBRARY_DIRS} 
)
			 
set(AVRO_LIBRARIES ${AVRO_LIBRARY})
set(AVRO_INCLUDE_DIRS ${AVRO_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set Avro_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Avro 
	DEFAULT_MSG
    AVRO_LIBRARY 
	AVRO_INCLUDE_DIR
)

mark_as_advanced(AVRO_INCLUDE_DIR AVRO_LIBRARY)
