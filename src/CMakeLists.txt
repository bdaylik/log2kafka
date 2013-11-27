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

set (MODULE_SOURCE_FILES
    ${CMAKE_PROJECT_NAME}.cc
    Constants.cc
    Mapper.cc
    MapperMatchException.cc
    ClientProxy.cc
    Serializer.cc
)

configure_file (${CMAKE_PROJECT_NAME}.hh.in ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.hh)

add_executable (${CMAKE_PROJECT_NAME} ${MODULE_SOURCE_FILES})

target_link_libraries (${CMAKE_PROJECT_NAME} ${LIBS})

install (TARGETS ${CMAKE_PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

file (GLOB schemas "${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.json")

install(
    FILES
        log4cxx.properties
        ${schemas}
    DESTINATION
        /etc/${CMAKE_PROJECT_NAME}
)

file (GLOB avro-lib "${AVRO_LIBRARY_DIR}/${AVRO_LIBRARY_NAME}.*")
file (GLOB kafka-lib "${KAFKA_LIBRARY_DIR}/${KAFKA_LIBRARY_NAME}.*")
file (GLOB log4cxx-lib "${LOG4CXX_LIBRARY_DIR}/${LOG4CXX_LIBRARY_NAME}.*")
file (GLOB snappy-lib "${SNAPPY_LIBRARY_DIR}/${SNAPPY_LIBRARY_NAME}.*")

install(
    FILES
        ${avro-lib}
        ${kafka-lib}
        ${log4cxx-lib}
        ${snappy-lib}
    DESTINATION
        ${CMAKE_PREFIX_PATH}/lib
)                      

install(
    DIRECTORY
        ${AVRO_INCLUDE_DIRS}
        ${KAFKA_INCLUDE_DIRS}
        ${LOG4CXX_INCLUDE_DIRS}
        ${SNAPPY_INCLUDE_DIRS}
    DESTINATION
        ${CMAKE_PREFIX_PATH}/include
)