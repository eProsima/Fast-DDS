# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

# Workaround for GCC 13 false positive with GoogleTest ON_CALL/EXPECT_CALL matchers, as in the unit tests.
# GCC 13 incorrectly reports uninitialized variables when using matchers like _. This is fixed in GCC 14.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
   CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0 AND
   CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14.0)
    add_compile_options(-Wno-error=uninitialized)
    add_compile_options(-Wno-error=maybe-uninitialized)
endif()

# The link-seam mocks define every method of the class they replace without using its private fields, which Clang
# reports as unused when all of them are defined in the same source file.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-unused-private-field)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/add_mock_library.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/target_link_units.cmake)

# Helpers shared by the mocks, e.g. mock/MockCompanion.hpp. Linked as public by every mock library.
add_library(fastdds-mock-common INTERFACE)
target_include_directories(fastdds-mock-common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/common)
target_link_libraries(fastdds-mock-common INTERFACE GTest::gmock)

# Link-seam mocks, under units/<unit path>/<Name>/ after the unit they replace (whole or partially).
add_subdirectory(units/core/BuiltinProtocols)
add_subdirectory(units/core/SecurityPluginFactory)
add_subdirectory(units/dds/condition/ConditionNotifier)
add_subdirectory(units/dds/condition/WaitSetImpl)
add_subdirectory(units/dds/data-writer/DataWriter)
add_subdirectory(units/dds/entities/Publisher)
add_subdirectory(units/dds/participant-factory/DomainParticipantFactory)
add_subdirectory(units/dds/type-lookup-manager/TypeLookupManager)
add_subdirectory(units/dds/type-support/TypeSupport)
add_subdirectory(units/dds/writer-history/DataWriterHistory)
add_subdirectory(units/log/Log)
add_subdirectory(units/rtps/data-sharing-pool/DataSharingPayloadPool)
add_subdirectory(units/rtps/domain/RTPSDomainImpl)
add_subdirectory(units/rtps/edp/EDP)
add_subdirectory(units/rtps/gap-builder/RTPSGapBuilder)
add_subdirectory(units/rtps/message-group/RTPSMessageGroup)
add_subdirectory(units/rtps/message-receiver/MessageReceiver)
add_subdirectory(units/rtps/network-factory/NetworkFactory)
add_subdirectory(units/rtps/participant-impl/RTPSParticipantImpl)
add_subdirectory(units/rtps/participant-impl/RTPSParticipantImplAccessors)
add_subdirectory(units/rtps/participant/RTPSParticipant)
add_subdirectory(units/rtps/pdp/PDP)
add_subdirectory(units/rtps/reader-base/BaseReader)
add_subdirectory(units/rtps/reader-history/ReaderHistory)
add_subdirectory(units/rtps/reader-locator/ReaderLocator)
add_subdirectory(units/rtps/security-manager/SecurityManager)
add_subdirectory(units/rtps/stateful-reader/StatefulReader)
add_subdirectory(units/rtps/stateful-writer/StatefulWriter)
add_subdirectory(units/rtps/stateless-reader/StatelessReader)
add_subdirectory(units/rtps/stateless-writer/StatelessWriter)
add_subdirectory(units/rtps/timed-event/TimedEvent)
add_subdirectory(units/rtps/wlp/WLP)
add_subdirectory(units/rtps/writer-base/BaseWriter)
add_subdirectory(units/rtps/writer-history/WriterHistory)
if(FASTDDS_STATISTICS)
    add_subdirectory(units/statistics/base/StatisticsBase)
    add_subdirectory(units/statistics/monitor-service/MonitorService)
endif()
add_subdirectory(units/utils/shared-memory/SharedMemSegment)
add_subdirectory(units/xtypes/TypeObjectRegistry)
add_subdirectory(units/xtypes/TypeObjectUtils)
add_subdirectory(units/xmlparser/endpoint-parser/XMLEndpointParser)
add_subdirectory(units/xmlparser/profile-manager/XMLProfileManager)
