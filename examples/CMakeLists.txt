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

set(fastrtps_FOUND TRUE)
add_subdirectory(C++/DeadlineQoSExample)
add_subdirectory(C++/OwnershipStrengthQoSExample)
add_subdirectory(C++/DynamicHelloWorldExample)
add_subdirectory(C++/HelloWorldExample)
#add_subdirectory(C++/HelloWorldExampleTCP)
add_subdirectory(C++/Filtering)
add_subdirectory(C++/UserDefinedTransportExample)
add_subdirectory(C++/FlowControlExample)
add_subdirectory(C++/ClientServerTest)
add_subdirectory(C++/RTPSTest_as_socket)
add_subdirectory(C++/RTPSTest_registered)
add_subdirectory(C++/RTPSTest_persistent)
add_subdirectory(C++/HistoryKind)
add_subdirectory(C++/Keys)
add_subdirectory(C++/LateJoiners)
add_subdirectory(C++/SampleConfig_Controller)
add_subdirectory(C++/SampleConfig_Events)
add_subdirectory(C++/SampleConfig_Multimedia)
add_subdirectory(C++/Configurability)
add_subdirectory(C++/StaticHelloWorldExample)
add_subdirectory(C++/XMLProfiles)
add_subdirectory(C++/Benchmark)

if(SECURITY)
    add_subdirectory(C++/SecureHelloWorldExample)
endif()
