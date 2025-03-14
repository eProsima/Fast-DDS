// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <fastdds/builtin/type_lookup_service/TypeLookupReplyListener.hpp>
#include <fastdds/builtin/type_lookup_service/TypeLookupRequestListener.hpp>
#include <fastdds/xtypes/type_representation/TypeObjectRegistry.hpp>

#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/writer/StatefulWriter.hpp>
#include <rtps/RTPSDomainImpl.hpp>

using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::_;
using namespace eprosima::fastdds::dds::builtin;
using namespace eprosima::fastdds::dds::xtypes;
using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace dds {

octet typeid_msg_buffer[] =
{
    // Encapsulation
    0x00, 0x03, 0x00, 0x00,
    // Endpoint GUID
    0x5a, 0x00, 0x10, 0x00,
    0xc0, 0xa8, 0x01, 0x3a, 0x00, 0x00, 0x41, 0xa4, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x42,
    // Topic name
    0x05, 0x00, 0x10, 0x00,
    0x0c, 0x00, 0x00, 0x00, 0x72, 0x74, 0x69, 0x2f, 0x64, 0x69, 0x73, 0x74, 0x6c, 0x6f, 0x67, 0x00,
    // Type name
    0x07, 0x00, 0x20, 0x00,
    0x19, 0x00, 0x00, 0x00, 0x63, 0x6f, 0x6d, 0x3a, 0x3a, 0x72, 0x74, 0x69, 0x3a, 0x3a, 0x64, 0x6c,
    0x3a, 0x3a, 0x4c, 0x6f, 0x67, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x00, 0x00, 0x00, 0x00,
    // TypeID
    0x69, 0x00, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // Sentinel
    0x01, 0x00, 0x00, 0x00
};

octet typeobject_msg_buffer[] =
{
    // Encapsulation
    0x00, 0x03, 0x00, 0x00,
    // Endpoint GUID
    0x5a, 0x00, 0x10, 0x00,
    0xc0, 0xa8, 0x01, 0x3a, 0x00, 0x00, 0x41, 0xa4, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x42,
    // Topic name
    0x05, 0x00, 0x10, 0x00,
    0x0c, 0x00, 0x00, 0x00, 0x72, 0x74, 0x69, 0x2f, 0x64, 0x69, 0x73, 0x74, 0x6c, 0x6f, 0x67, 0x00,
    // Type name
    0x07, 0x00, 0x20, 0x00,
    0x19, 0x00, 0x00, 0x00, 0x63, 0x6f, 0x6d, 0x3a, 0x3a, 0x72, 0x74, 0x69, 0x3a, 0x3a, 0x64, 0x6c,
    0x3a, 0x3a, 0x4c, 0x6f, 0x67, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x00, 0x00, 0x00, 0x00,
    // Type object
    0x72, 0x00, 0xfc, 0x04,
    0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd0, 0x04, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x00, 0x18, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x28, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x52, 0x54, 0x49, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x00, 0x18, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x04, 0x04, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x44, 0x4c, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x00, 0x18, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x64, 0x00, 0x00, 0x00,
    0xe8, 0x02, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x41, 0x44, 0x4d, 0x49, 0x4e, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x16, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00,
    0x09, 0x00, 0x00, 0x00, 0xd0, 0x01, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x24, 0x00, 0x00, 0x00, 0x02, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8c, 0x51, 0x83, 0x23,
    0x55, 0x8c, 0x53, 0x3a, 0x10, 0x00, 0x00, 0x00, 0x43, 0x6f, 0x6d, 0x6d, 0x61, 0x6e, 0x64, 0x52,
    0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x65, 0x00, 0x00, 0x00,
    0x70, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x2c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xfe, 0x29, 0x56, 0xb1, 0x97, 0x58, 0xdf, 0x3f, 0x0d, 0x00, 0x00, 0x00,
    0x68, 0x6f, 0x73, 0x74, 0x41, 0x6e, 0x64, 0x41, 0x70, 0x70, 0x49, 0x64, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x7f, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xfe, 0x29, 0x56, 0xb1, 0x97, 0x58, 0xdf, 0x3f, 0x17, 0x00, 0x00, 0x00, 0x6f, 0x72, 0x69, 0x67,
    0x69, 0x6e, 0x61, 0x74, 0x6f, 0x72, 0x48, 0x6f, 0x73, 0x74, 0x41, 0x6e, 0x64, 0x41, 0x70, 0x70,
    0x49, 0x64, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00,
    0x0b, 0x00, 0x00, 0x00, 0x69, 0x6e, 0x76, 0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x00, 0x00,
    0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x7f, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xd9, 0xdc, 0x15, 0x0b, 0x91, 0x99, 0x13, 0x0e, 0x0e, 0x00, 0x00, 0x00, 0x63, 0x6f, 0x6d, 0x6d,
    0x61, 0x6e, 0x64, 0x52, 0x65, 0x73, 0x75, 0x6c, 0x74, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00,
    0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xdc, 0x5c, 0x98,
    0xa5, 0x08, 0x32, 0x91, 0x08, 0x00, 0x00, 0x00, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x00,
    0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
    0x0e, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x05, 0x00, 0x00, 0x00, 0xd8, 0x00, 0x00, 0x00,
    0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xd9, 0xdc, 0x15, 0x0b, 0x91, 0x99, 0x13, 0x0e, 0x0e, 0x00, 0x00, 0x00,
    0x43, 0x6f, 0x6d, 0x6d, 0x61, 0x6e, 0x64, 0x52, 0x65, 0x73, 0x75, 0x6c, 0x74, 0x00, 0x00, 0x00,
    0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x7f, 0x08, 0x00, 0x64, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
    0x01, 0x7f, 0x08, 0x00, 0x65, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x52, 0x54, 0x49, 0x5f, 0x44, 0x4c, 0x5f, 0x43,
    0x4f, 0x4d, 0x4d, 0x41, 0x4e, 0x44, 0x5f, 0x52, 0x45, 0x53, 0x55, 0x4c, 0x54, 0x5f, 0x4f, 0x4b,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x52, 0x54, 0x49, 0x5f,
    0x44, 0x4c, 0x5f, 0x43, 0x4f, 0x4d, 0x4d, 0x41, 0x4e, 0x44, 0x5f, 0x52, 0x45, 0x53, 0x55, 0x4c,
    0x54, 0x5f, 0x4e, 0x4f, 0x54, 0x5f, 0x53, 0x55, 0x50, 0x50, 0x4f, 0x52, 0x54, 0x45, 0x44, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x52, 0x54, 0x49, 0x5f, 0x44, 0x4c, 0x5f, 0x43,
    0x4f, 0x4d, 0x4d, 0x41, 0x4e, 0x44, 0x5f, 0x52, 0x45, 0x53, 0x55, 0x4c, 0x54, 0x5f, 0x45, 0x52,
    0x52, 0x4f, 0x52, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x00, 0x16, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x09, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x29, 0x56, 0xb1, 0x97, 0x58, 0xdf, 0x3f,
    0x0d, 0x00, 0x00, 0x00, 0x48, 0x6f, 0x73, 0x74, 0x41, 0x6e, 0x64, 0x41, 0x70, 0x70, 0x49, 0x64,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x64, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x65, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00, 0x0d, 0x00, 0x00, 0x00,
    0x72, 0x74, 0x70, 0x73, 0x5f, 0x68, 0x6f, 0x73, 0x74, 0x5f, 0x69, 0x64, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x7f, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00, 0x0c, 0x00, 0x00, 0x00,
    0x72, 0x74, 0x70, 0x73, 0x5f, 0x61, 0x70, 0x70, 0x5f, 0x69, 0x64, 0x00, 0x01, 0x7f, 0x08, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00,
    0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x00, 0x13, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00,
    0x74, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xdc, 0x5c, 0x98, 0xa5, 0x08, 0x32, 0x91,
    0x16, 0x00, 0x00, 0x00, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x5f, 0x32, 0x30, 0x34, 0x38, 0x5f,
    0x63, 0x68, 0x61, 0x72, 0x61, 0x63, 0x74, 0x65, 0x72, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00,
    0x64, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x65, 0x00, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0xc8, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x08, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x8c, 0x51, 0x83, 0x23, 0x55, 0x8c, 0x53, 0x3a, 0x02, 0x7f, 0x00, 0x00,
    // Sentinel
    0x01, 0x00, 0x00, 0x00
};

inline std::vector<uint8_t> create_continuation_point(
        size_t value)
{
    std::vector<uint8_t> continuation_point(32, 0);

    for (size_t value_i = 0; value_i < value; value_i++)
    {
        for (size_t i = continuation_point.size() - 1; i != SIZE_MAX; --i)
        {
            if (continuation_point[i] < 255)
            {
                ++continuation_point[i];
                // Break after successful increment
                break;
            }
            else
            {
                continuation_point[i] = 0;
            }
        }
    }
    return continuation_point;
}

SampleIdentity valid_sampleidentity()
{
    GUID_t guid;
    guid.guidPrefix({1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
    SequenceNumber_t seqn;
    seqn.high(1);
    seqn.low(1);

    SampleIdentity id;
    id.writer_guid(guid);
    id.sequence_number(seqn);

    return id;
}

class MockLogConsumer : public LogConsumer
{
public:

    virtual void Consume(
            const Log::Entry& entry)
    {
        std::unique_lock<std::mutex> guard(mMutex);
        mEntriesConsumed.push_back(entry);
        cv_.notify_all();
    }

    const std::vector<Log::Entry> ConsumedEntries() const
    {
        std::unique_lock<std::mutex> guard(mMutex);
        return mEntriesConsumed;
    }

    void clear_entries()
    {
        std::unique_lock<std::mutex> guard(mMutex);
        mEntriesConsumed.clear();
        cv_.notify_all();
    }

    size_t wait_for_entries(
            uint32_t amount,
            int max_wait)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        cv_.wait_for(lock, std::chrono::seconds(max_wait), [this, amount]() -> bool
                {
                    return mEntriesConsumed.size() >= amount;
                });
        return mEntriesConsumed.size();
    }

private:

    std::vector<Log::Entry> mEntriesConsumed;
    mutable std::mutex mMutex;
    std::condition_variable cv_;
};

class MockTypeLookupReplyListener : public TypeLookupReplyListener
{
public:

    MockTypeLookupReplyListener(
            TypeLookupManager* manager)
        : TypeLookupReplyListener(manager)
    {
    }

    void on_new_cache_change_added(
            fastdds::rtps::RTPSReader* reader_,
            const fastdds::rtps::CacheChange_t* const change)
    {
        TypeLookupReplyListener::on_new_cache_change_added(reader_, change);
    }

    void process_reply()
    {
        TypeLookupReplyListener::process_reply();
    }

    std::queue<ReplyWithServerGUID>* get_replies_queue()
    {
        return &replies_queue_;
    }

    void notify()
    {
        replies_processor_cv_.notify_all();
    }

    void set_processing(
            bool status)
    {
        processing_ = status;
    }

};

class MockTypeLookupRequestListener : public TypeLookupRequestListener
{
public:

    MockTypeLookupRequestListener(
            TypeLookupManager* manager)
        : TypeLookupRequestListener(manager)
    {
    }

    void on_new_cache_change_added(
            fastdds::rtps::RTPSReader* reader_,
            const fastdds::rtps::CacheChange_t* const change)
    {
        // Call the private method indirectly through the public interface
        TypeLookupRequestListener::on_new_cache_change_added(reader_, change);
    }

    void process_requests()
    {
        TypeLookupRequestListener::process_requests();
    }

    std::queue<std::pair<TypeLookup_Request, rtps::VendorId_t>>* get_requests_queue()
    {
        return &requests_queue_;
    }

    void notify()
    {
        request_processor_cv_.notify_all();
    }

    void set_processing(
            bool status)
    {
        processing_ = status;
    }

};

class TypeLookupServiceTests : public ::testing::Test
{
protected:

    void SetUp() override
    {
        tlm_ = new TypeLookupManager();
        tlm_->participant_ = &participant_;

        reply_listener_ = new MockTypeLookupReplyListener(tlm_);
        request_listener_ = new MockTypeLookupRequestListener(tlm_);

        HistoryAttributes att;
        stateful_reader_.history_ = new ReaderHistory(att);
    }

    void TearDown() override
    {
        delete stateful_reader_.history_;
        delete reply_listener_;
        delete request_listener_;
        delete tlm_;
    }

    TypeLookupManager* tlm_;
    MockTypeLookupReplyListener* reply_listener_;
    MockTypeLookupRequestListener* request_listener_;
    RTPSParticipantAttributes participant_attr_;
    NetworkFactory network_factory_ {participant_attr_};
    ReaderProxyData reader_proxy_{0, 0};
    WriterProxyData writer_proxy_{0, 0};
    NiceMock<RTPSParticipantImpl> participant_;
    NiceMock<StatefulReader> stateful_reader_;
    TypeObjectRegistry& registry {RTPSDomainImpl::get_instance()->type_object_registry_observer()};
};

MockLogConsumer* setup_expect_log_msg(
        std::string category,
        std::string msg)
{
    Log::Reset();
    Log::SetVerbosity(Log::Info);
    Log::SetCategoryFilter(std::regex(category));
    Log::SetErrorStringFilter(std::regex(msg));
    MockLogConsumer* log_consumer = new MockLogConsumer();
    std::unique_ptr<MockLogConsumer> log_consumer_unique_ptr(log_consumer);
    Log::RegisterConsumer(std::move(log_consumer_unique_ptr));
    return log_consumer;
}

TEST_F(TypeLookupServiceTests, ReaderProxyData_PID_TYPE_IDV1)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("RTPS_PROXY_DATA",
                    "Reception of TypeIdentifiers is not supported. They will be ignored.");

    CDRMessage_t msg(0);
    msg.init(typeid_msg_buffer, static_cast<uint32_t>(sizeof(typeid_msg_buffer)));
    msg.length = msg.max_size;

    ASSERT_TRUE(reader_proxy_.read_from_cdr_message(&msg, c_VendorId_eProsima));

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, WriterProxyData_PID_TYPE_IDV1)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("RTPS_PROXY_DATA",
                    "Reception of TypeIdentifiers is not supported. They will be ignored.");

    CDRMessage_t msg(0);
    msg.init(typeid_msg_buffer, static_cast<uint32_t>(sizeof(typeid_msg_buffer)));
    msg.length = msg.max_size;

    ASSERT_TRUE(writer_proxy_.read_from_cdr_message(&msg, c_VendorId_eProsima));

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, ReaderProxyData_PID_TYPE_OBJECTV1)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("RTPS_PROXY_DATA", "Reception of TypeObjects is not supported. They will be ignored.");

    CDRMessage_t msg(0);
    msg.init(typeobject_msg_buffer, static_cast<uint32_t>(sizeof(typeobject_msg_buffer)));
    msg.length = msg.max_size;

    ASSERT_TRUE(reader_proxy_.read_from_cdr_message(&msg, c_VendorId_eProsima));

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, WriterProxyData_PID_TYPE_OBJECTV1)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("RTPS_PROXY_DATA", "Reception of TypeObjects is not supported. They will be ignored.");

    CDRMessage_t msg(0);
    msg.init(typeobject_msg_buffer, static_cast<uint32_t>(sizeof(typeobject_msg_buffer)));
    msg.length = msg.max_size;

    ASSERT_TRUE(writer_proxy_.read_from_cdr_message(&msg, c_VendorId_eProsima));

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_wrong_EntityId)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TL_REPLY_READER", "Received data from a bad endpoint.");

    CacheChange_t* change {new CacheChange_t()};
    change->writerGUID.entityId = 0x111111C3;

    EXPECT_CALL(*stateful_reader_.history_, remove_change_mock(change)).Times(1).
            WillOnce(::testing::Return(true));

    reply_listener_->on_new_cache_change_added(&stateful_reader_, change);

    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_malformed_Reply)
{
    CacheChange_t* change {new CacheChange_t()};
    change->writerGUID.entityId = c_EntityId_TypeLookup_reply_writer;

    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastdds::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Reply&>(_)))
            .WillOnce(testing::Return(false));
    EXPECT_CALL(*stateful_reader_.history_, remove_change_mock(change)).Times(1).
            WillOnce(::testing::Return(true));

    reply_listener_->on_new_cache_change_added(&stateful_reader_, change);

    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_REMOTE_EX_UNSUPPORTED)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER",
                    "Received reply with exception code: " +
                    std::to_string(static_cast<int>(rpc::RemoteExceptionCode_t::REMOTE_EX_UNSUPPORTED)));


    CacheChange_t change;
    change.writerGUID.entityId = c_EntityId_TypeLookup_reply_writer;
    stateful_reader_.get_history()->add_change(&change);

    TypeLookup_Reply expectedReply;
    expectedReply.header().remoteEx(rpc::RemoteExceptionCode_t::REMOTE_EX_UNSUPPORTED);
    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastdds::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Reply&>(_)))
            .WillOnce(testing::DoAll(testing::SetArgReferee<1>(expectedReply), testing::Return(true)));

    reply_listener_->on_new_cache_change_added(&stateful_reader_, &change);

    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_REMOTE_EX_INVALID_ARGUMENT)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER",
                    "Received reply with exception code: " +
                    std::to_string(static_cast<int>(rpc::RemoteExceptionCode_t::REMOTE_EX_INVALID_ARGUMENT)));


    CacheChange_t change;
    change.writerGUID.entityId = c_EntityId_TypeLookup_reply_writer;
    stateful_reader_.get_history()->add_change(&change);

    TypeLookup_Reply expectedReply;
    expectedReply.header().remoteEx(rpc::RemoteExceptionCode_t::REMOTE_EX_INVALID_ARGUMENT);
    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastdds::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Reply&>(_)))
            .WillOnce(testing::DoAll(testing::SetArgReferee<1>(expectedReply), testing::Return(true)));

    reply_listener_->on_new_cache_change_added(&stateful_reader_, &change);

    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_REMOTE_EX_OUT_OF_RESOURCES)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER",
                    "Received reply with exception code: " +
                    std::to_string(static_cast<int>(rpc::RemoteExceptionCode_t::REMOTE_EX_OUT_OF_RESOURCES)));


    CacheChange_t change;
    change.writerGUID.entityId = c_EntityId_TypeLookup_reply_writer;
    stateful_reader_.get_history()->add_change(&change);

    TypeLookup_Reply expectedReply;
    expectedReply.header().remoteEx(rpc::RemoteExceptionCode_t::REMOTE_EX_OUT_OF_RESOURCES);
    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastdds::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Reply&>(_)))
            .WillOnce(testing::DoAll(testing::SetArgReferee<1>(expectedReply), testing::Return(true)));

    reply_listener_->on_new_cache_change_added(&stateful_reader_, &change);
    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_REMOTE_EX_UNKNOWN_OPERATION)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER",
                    "Received reply with exception code: " +
                    std::to_string(static_cast<int>(rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_OPERATION)));


    CacheChange_t change;
    change.writerGUID.entityId = c_EntityId_TypeLookup_reply_writer;
    stateful_reader_.get_history()->add_change(&change);

    TypeLookup_Reply expectedReply;
    expectedReply.header().remoteEx(rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_OPERATION);
    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastdds::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Reply&>(_)))
            .WillOnce(testing::DoAll(testing::SetArgReferee<1>(expectedReply), testing::Return(true)));

    reply_listener_->on_new_cache_change_added(&stateful_reader_, &change);
    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_REMOTE_EX_UNKNOWN_EXCEPTION)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER",
                    "Received reply with exception code: " +
                    std::to_string(static_cast<int>(rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_EXCEPTION)));


    CacheChange_t change;
    change.writerGUID.entityId = c_EntityId_TypeLookup_reply_writer;
    stateful_reader_.get_history()->add_change(&change);

    TypeLookup_Reply expectedReply;
    expectedReply.header().remoteEx(rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_EXCEPTION);
    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastdds::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Reply&>(_)))
            .WillOnce(testing::DoAll(testing::SetArgReferee<1>(expectedReply), testing::Return(true)));

    reply_listener_->on_new_cache_change_added(&stateful_reader_, &change);
    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_wrong_replyid)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER",
                    "Received unknown reply operation type in type lookup service.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Reply reply;
    reply.header().relatedRequestId(valid_sampleidentity());
    reply.return_value()._default();
    ReplyWithServerGUID reply_with_guid;
    reply_with_guid.reply = reply;

    reply_listener_->get_replies_queue()->push(reply_with_guid);
    reply_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_getTypes_empty)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER", "Received reply with no types.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Reply reply;
    reply.header().relatedRequestId(valid_sampleidentity());
    reply.return_value().getType({});
    reply.return_value().getType().result({});
    ReplyWithServerGUID reply_with_guid;
    reply_with_guid.reply = reply;

    EXPECT_CALL(*tlm_, remove_async_get_type_request(_)).WillOnce(testing::Return(true));

    reply_listener_->get_replies_queue()->push(reply_with_guid);
    reply_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_getTypes_registry_empty_type_error)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER", "Error registering remote type.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Reply reply;
    reply.header().relatedRequestId(valid_sampleidentity());
    reply.return_value().getType({});
    TypeIdentifierTypeObjectPair pair;
    pair.type_identifier({});
    pair.type_object({});
    TypeLookup_getTypes_Out out;
    out.types().push_back(pair);
    reply.return_value().getType().result(out);
    ReplyWithServerGUID reply_with_guid;
    reply_with_guid.reply = reply;
    TypeIdentifierPair type_ids;
    type_ids.type_identifier1(pair.type_identifier());

    EXPECT_CALL(registry,
            register_type_object(_, ::testing::Eq(type_ids), false)).Times(1).WillOnce(
        ::testing::Return(RETCODE_PRECONDITION_NOT_MET));
    EXPECT_CALL(*tlm_, remove_async_get_type_request(_)).WillOnce(testing::Return(true));

    reply_listener_->get_replies_queue()->push(reply_with_guid);
    reply_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}


TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_getTypes_registry_inconsistent_type_error)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER", "Error registering remote type.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Reply reply;
    reply.header().relatedRequestId(valid_sampleidentity());
    reply.return_value().getType({});

    TypeIdentifierTypeObjectPair pair;
    TypeIdentifier id;
    id._d(TK_CHAR8);
    TypeObject obj;
    obj.complete({});
    obj.complete().sequence_type({});
    pair.type_identifier(id);
    pair.type_object(obj);

    TypeLookup_getTypes_Out out;
    out.types().push_back(pair);
    reply.return_value().getType().result(out);
    ReplyWithServerGUID reply_with_guid;
    reply_with_guid.reply = reply;

    TypeIdentifierPair type_ids;
    type_ids.type_identifier1(pair.type_identifier());

    EXPECT_CALL(registry,
            register_type_object(::testing::Eq(obj), ::testing::Eq(type_ids), false)).Times(1).WillOnce(
        ::testing::Return(RETCODE_PRECONDITION_NOT_MET));
    EXPECT_CALL(*tlm_, remove_async_get_type_request(_)).WillOnce(testing::Return(true));

    reply_listener_->get_replies_queue()->push(reply_with_guid);
    reply_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_getDependencies_continuation_point_fail)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER",
                    "Failed to send get_type_dependencies request");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Reply reply;
    reply.header().relatedRequestId(valid_sampleidentity());
    reply.return_value().getTypeDependencies({});
    TypeIdentifierTypeObjectPair pair;
    TypeLookup_getTypeDependencies_Out out;
    out.continuation_point(create_continuation_point(10));
    reply.return_value().getTypeDependencies().result(out);
    ReplyWithServerGUID reply_with_guid;
    reply_with_guid.reply = reply;

    EXPECT_CALL(*tlm_,
            get_type_dependencies(testing::_, testing::_, testing::_)).WillOnce(testing::Return(
                INVALID_SAMPLE_IDENTITY));
    EXPECT_CALL(*tlm_, get_types(_, _)).WillOnce(testing::Return(INVALID_SAMPLE_IDENTITY));
    EXPECT_CALL(*tlm_, remove_async_get_type_request(_)).WillOnce(testing::Return(true));

    reply_listener_->get_replies_queue()->push(reply_with_guid);
    reply_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_getDependencies_get_types_fail)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER", "Failed to send get_types request");

    // EXPECT_CALL(*tlm_, get_types(testing::_, testing::_))
    //         .WillOnce(testing::Return(INVALID_SAMPLE_IDENTITY));

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Reply reply;
    reply.header().relatedRequestId(valid_sampleidentity());
    reply.return_value().getTypeDependencies({});
    TypeIdentifierTypeObjectPair pair;
    TypeLookup_getTypeDependencies_Out out;
    reply.return_value().getTypeDependencies().result(out);
    ReplyWithServerGUID reply_with_guid;
    reply_with_guid.reply = reply;

    EXPECT_CALL(*tlm_, get_types(_, _)).WillOnce(testing::Return(INVALID_SAMPLE_IDENTITY));
    EXPECT_CALL(*tlm_, remove_async_get_type_request(_)).WillOnce(testing::Return(true));

    reply_listener_->get_replies_queue()->push(reply_with_guid);
    reply_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_wrong_EntityId)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TL_REQUEST_READER", "Received data from a bad endpoint.");

    CacheChange_t* change {new CacheChange_t()};
    change->writerGUID.entityId = 0x111111C3;

    EXPECT_CALL(*stateful_reader_.history_, remove_change_mock(change)).Times(1).
            WillOnce(::testing::Return(true));

    request_listener_->on_new_cache_change_added(&stateful_reader_, change);

    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_malformed_Request)
{
    CacheChange_t* change {new CacheChange_t()};
    change->writerGUID.entityId = c_EntityId_TypeLookup_request_writer;

    EXPECT_CALL(*stateful_reader_.history_, remove_change_mock(change)).Times(1).
            WillOnce(::testing::Return(true));

    EXPECT_CALL(*tlm_, receive(::testing::Ref(*change), testing::Matcher<TypeLookup_Request&>(_)))
            .WillOnce(testing::Return(false));

    request_listener_->on_new_cache_change_added(&stateful_reader_, change);

    EXPECT_EQ(request_listener_->get_requests_queue()->size(), 0);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_wrong_requestid)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REQUEST_LISTENER",
                    "Received unknown request in type lookup service.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Request request;
    request.data()._default();

    EXPECT_CALL(*tlm_, send(testing::Matcher<TypeLookup_Reply&>(_))).WillOnce(testing::Return(true));

    request_listener_->get_requests_queue()->push({request, c_VendorId_eProsima});
    request_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_getTypes_empty)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REQUEST_LISTENER",
                    "Received request with no type identifiers.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Request request;
    request.data().getTypes({});

    request_listener_->get_requests_queue()->push({request, c_VendorId_eProsima});
    request_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_getTypes_registry_empty_type_error)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REQUEST_LISTENER",
                    "Requested TypeIdentifier is not a direct hash.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Request request;
    TypeLookup_getTypes_In in;
    TypeIdentifierSeq id_seq;
    TypeIdentifier id;
    id_seq.push_back(id);
    in.type_ids(id_seq);
    in.type_ids().push_back(id);
    request.data().getTypes(in);

    EXPECT_CALL(registry,
            get_type_object(::testing::Eq(id), _)).Times(1).WillOnce(
        ::testing::Return(RETCODE_PRECONDITION_NOT_MET));

    EXPECT_CALL(*tlm_, send(testing::Matcher<TypeLookup_Reply&>(_))).WillOnce(testing::Return(true));

    request_listener_->get_requests_queue()->push({request, c_VendorId_eProsima});
    request_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_getTypes_registry_unknown_type_error)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REQUEST_LISTENER",
                    "Requested TypeIdentifier is not found in the registry.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Request request;
    TypeLookup_getTypes_In in;
    TypeIdentifierSeq id_seq;
    TypeIdentifier id;
    id._d(TK_CHAR8);

    id_seq.push_back(id);
    in.type_ids(id_seq);
    in.type_ids().push_back(id);
    request.data().getTypes(in);

    EXPECT_CALL(registry,
            get_type_object(::testing::Eq(id), _)).Times(1).WillOnce(
        ::testing::Return(RETCODE_NO_DATA));

    EXPECT_CALL(*tlm_, send(testing::Matcher<TypeLookup_Reply&>(_))).WillOnce(testing::Return(true));

    request_listener_->get_requests_queue()->push({request, c_VendorId_eProsima});
    request_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_getDependencies_empty)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REQUEST_LISTENER", "Type dependencies request is empty.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Request request;
    request.data().getTypeDependencies({});

    request_listener_->get_requests_queue()->push({request, c_VendorId_eProsima});
    request_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_getDependencies_registry_uknown_type_error)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REQUEST_LISTENER",
                    "Requested TypeIdentifier is not found in the registry.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Request request;

    TypeLookup_getTypeDependencies_In in;
    TypeIdentifierSeq id_seq;
    TypeIdentifier id;
    id._d(TK_CHAR8);
    id_seq.push_back(id);
    in.type_ids(id_seq);
    in.type_ids().push_back(id);
    request.data().getTypeDependencies(in);

    EXPECT_CALL(registry,
            get_type_dependencies(::testing::Eq(in.type_ids()), _)).Times(1).WillOnce(
        ::testing::Return(RETCODE_NO_DATA));

    EXPECT_CALL(*tlm_, send(testing::Matcher<TypeLookup_Reply&>(_))).WillOnce(testing::Return(true));

    request_listener_->get_requests_queue()->push({request, c_VendorId_eProsima});
    request_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
