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

include(${PROJECT_SOURCE_DIR}/cmake/dev/compile_example.cmake)
compile_example(DeadlineQoSExample C++/DeadlineQoSExample)
compile_example(OwnershipStrengthQoSExample C++/OwnershipStrengthQoSExample)
compile_example(HelloWorldExample C++/HelloWorldExample)
compile_example(FilteringExample C++/Filtering)
compile_example(UserDefinedTransportExample C++/UserDefinedTransportExample)
compile_example(FlowControlExample C++/FlowControlExample)
compile_example(ClientServerTest C++/ClientServerTest)
compile_example(RTPSTest_as_socket C++/RTPSTest_as_socket)
compile_example(RTPSTest_registered C++/RTPSTest_registered)
compile_example(HistoryKind C++/HistoryKind)
compile_example(Keys C++/Keys)
compile_example(LateJoiners C++/LateJoiners)
compile_example(SampleConfig_Controller C++/SampleConfig_Controller)
compile_example(SampleConfig_Events C++/SampleConfig_Events)
compile_example(SampleConfig_Multimedia C++/SampleConfig_Multimedia)
compile_example(Configurability C++/Configurability)

if(SECURITY)
    compile_example(SecureHelloWorldExample C++/SecureHelloWorldExample)
endif()
