# Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_definitions(
  -DBOOST_ASIO_STANDALONE
  -DASIO_STANDALONE
)

add_subdirectory(rtps/common)
add_subdirectory(rtps/reader)
add_subdirectory(rtps/writer)
add_subdirectory(rtps/history)
add_subdirectory(rtps/resources/timedevent)
add_subdirectory(rtps/network)
add_subdirectory(rtps/flowcontrol)
add_subdirectory(rtps/persistence)
add_subdirectory(dds/participant)
add_subdirectory(dynamic_types)
add_subdirectory(transport)
add_subdirectory(logging)
add_subdirectory(utils)
add_subdirectory(xmlparser)
add_subdirectory(xtypes)
if(SECURITY)
    add_subdirectory(security/authentication)
    add_subdirectory(security/cryptography)
    add_subdirectory(rtps/security)
endif()
