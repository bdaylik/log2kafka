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
    InvalidBrokerException.cc
    InvalidMapperException.cc
    MapperMatchException.cc
    ProducerCreationException.cc
    Mapper.cc
    Serializer.cc
    ClientFacade.cc
)

configure_file (config.hh.in ${CMAKE_CURRENT_SOURCE_DIR}/config.hh)

add_executable (${CMAKE_PROJECT_NAME} ${MODULE_SOURCE_FILES})

target_link_libraries (${CMAKE_PROJECT_NAME} ${LIBS})

install (TARGETS ${CMAKE_PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

file (GLOB config-files "${CMAKE_CURRENT_SOURCE_DIR}/conf/*")

install(
    FILES
        ${config-files}
    DESTINATION
        /etc/${CMAKE_PROJECT_NAME}
)
