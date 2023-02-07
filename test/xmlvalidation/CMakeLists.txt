# Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

###############################################################################
# XML validation
###############################################################################
message(STATUS "Configuring CLI xml validation...")

# Copy the schema to the build directory
configure_file(${PROJECT_SOURCE_DIR}/resources/xsd/fastRTPS_profiles.xsd
    ${CMAKE_CURRENT_BINARY_DIR}/fastRTPS_profiles.xsd COPYONLY)

# Copy the neccessary files over to the build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/profileAll.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileAll.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/profileDataReader.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileDataReader.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/profileDataWriter.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileDataWriter.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/profileDomainParticipant.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileDomainParticipant.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/profileTransportDescriptor.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileTransportDescriptor.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/profileLog.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileLog.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/profileTypes.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/profileTypes.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/XMLTesterExample.xml
    ${CMAKE_CURRENT_BINARY_DIR}/xmldocuments/XMLTesterExample.xml COPYONLY)


# Check XML profile validation against schema
add_test(NAME xml.validate COMMAND fastdds xml validate xmldocuments -x fastRTPS_profiles.xsd)
