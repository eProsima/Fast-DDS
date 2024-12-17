# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set(fastdds_FOUND TRUE)

add_subdirectory(cpp/benchmark)
add_subdirectory(cpp/configuration)
add_subdirectory(cpp/content_filter)
add_subdirectory(cpp/custom_payload_pool)
add_subdirectory(cpp/delivery_mechanisms)
add_subdirectory(cpp/discovery_server)
add_subdirectory(cpp/flow_control)
add_subdirectory(cpp/hello_world)
add_subdirectory(cpp/request_reply)
add_subdirectory(cpp/rtps)
add_subdirectory(cpp/static_edp_discovery)
add_subdirectory(cpp/topic_instances)
add_subdirectory(cpp/xtypes)

if (SECURITY)
    add_subdirectory(cpp/security)
endif()
