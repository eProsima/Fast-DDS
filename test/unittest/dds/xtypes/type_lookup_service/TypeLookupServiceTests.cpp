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
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/writer/StatefulWriter.h>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <fastdds/builtin/type_lookup_service/TypeLookupReplyListener.hpp>
#include <fastdds/builtin/type_lookup_service/TypeLookupRequestListener.hpp>
#include <fastdds/xtypes/type_representation/TypeObjectRegistry.hpp>
#include <rtps/network/NetworkFactory.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/RTPSDomainImpl.hpp>

using ::testing::Mock;
using ::testing::_;
using eprosima::fastrtps::rtps::RTPSDomain;
using namespace eprosima::fastdds::dds::builtin;
using namespace eprosima::fastdds::dds::xtypes;
using namespace eprosima::fastrtps::rtps;

namespace eprosima {

namespace fastrtps {
namespace rtps {
class RTPSDomain;

RTPSReader* RTPSDomain::reader_ = nullptr;
RTPSWriter* RTPSDomain::writer_ = nullptr;
RTPSParticipant* RTPSDomain::participant_ = nullptr;
} //namespace rtps
} //namespace fastrtps

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

    void onNewCacheChangeAdded(
            fastrtps::rtps::RTPSReader* reader_,
            const fastrtps::rtps::CacheChange_t* const change)
    {
        TypeLookupReplyListener::onNewCacheChangeAdded(reader_, change);
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

    void onNewCacheChangeAdded(
            fastrtps::rtps::RTPSReader* reader_,
            const fastrtps::rtps::CacheChange_t* const change)
    {
        // Call the private method indirectly through the public interface
        TypeLookupRequestListener::onNewCacheChangeAdded(reader_, change);
    }

    void process_requests()
    {
        TypeLookupRequestListener::process_requests();
    }

    std::queue<TypeLookup_Request>* get_requests_queue()
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
        RTPSParticipantImpl participant;
        tlm_ = new TypeLookupManager();
        tlm_->participant_ = &participant;

        reply_listener_ = new MockTypeLookupReplyListener(tlm_);
        request_listener_ = new MockTypeLookupRequestListener(tlm_);

        HistoryAttributes att;
        stateful_reader_.history_ = new ReaderHistory(att);
    }

    void TearDown() override
    {
        delete reply_listener_;
        delete request_listener_;
        delete tlm_;
    }

    TypeLookupManager* tlm_;
    MockTypeLookupReplyListener* reply_listener_;
    MockTypeLookupRequestListener* request_listener_;
    NetworkFactory network_factory_;
    ReaderProxyData reader_proxy_{0, 0};
    WriterProxyData writer_proxy_{0, 0};
    StatefulReader stateful_reader_;
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
            setup_expect_log_msg("RTPS_PROXY_DATA", "PID_TYPE_IDV1 not supported");

    CDRMessage_t msg(0);
    msg.init(typeid_msg_buffer, static_cast<uint32_t>(sizeof(typeid_msg_buffer)));
    msg.length = msg.max_size;

    ASSERT_TRUE(reader_proxy_.readFromCDRMessage(&msg, network_factory_, false, true));

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, WriterProxyData_PID_TYPE_IDV1)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("RTPS_PROXY_DATA", "PID_TYPE_IDV1 not supported");

    CDRMessage_t msg(0);
    msg.init(typeid_msg_buffer, static_cast<uint32_t>(sizeof(typeid_msg_buffer)));
    msg.length = msg.max_size;

    ASSERT_TRUE(writer_proxy_.readFromCDRMessage(&msg, network_factory_, false, true));

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, ReaderProxyData_PID_TYPE_OBJECTV1)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("RTPS_PROXY_DATA", "PID_TYPE_OBJECTV1 not supported");

    CDRMessage_t msg(0);
    msg.init(typeobject_msg_buffer, static_cast<uint32_t>(sizeof(typeobject_msg_buffer)));
    msg.length = msg.max_size;

    ASSERT_TRUE(reader_proxy_.readFromCDRMessage(&msg, network_factory_, false, true));

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, WriterProxyData_PID_TYPE_OBJECTV1)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("RTPS_PROXY_DATA", "PID_TYPE_OBJECTV1 not supported");

    CDRMessage_t msg(0);
    msg.init(typeobject_msg_buffer, static_cast<uint32_t>(sizeof(typeobject_msg_buffer)));
    msg.length = msg.max_size;

    ASSERT_TRUE(writer_proxy_.readFromCDRMessage(&msg, network_factory_, false, true));

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_wrong_EntityId)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TL_REPLY_READER", "Received data from a bad endpoint.");

    CacheChange_t* change = new CacheChange_t();
    change->writerGUID.entityId = 0x111111C3;
    stateful_reader_.getHistory()->add_change(change);

    reply_listener_->onNewCacheChangeAdded(&stateful_reader_, change);

    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_malformed_Reply)
{
    CacheChange_t* change = new CacheChange_t();
    change->writerGUID.entityId = c_EntityId_TypeLookup_reply_writer;
    stateful_reader_.getHistory()->add_change(change);

    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastrtps::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Reply&>(_)))
            .WillOnce(testing::Return(false));

    reply_listener_->onNewCacheChangeAdded(&stateful_reader_, change);

    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_REMOTE_EX_UNSUPPORTED)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER",
                    "Received reply with exception code: " +
                    static_cast<int>(rpc::RemoteExceptionCode_t::REMOTE_EX_UNSUPPORTED));


    CacheChange_t* change = new CacheChange_t();
    change->writerGUID.entityId = c_EntityId_TypeLookup_reply_writer;
    stateful_reader_.getHistory()->add_change(change);

    TypeLookup_Reply expectedReply;
    expectedReply.header().remoteEx(rpc::RemoteExceptionCode_t::REMOTE_EX_UNSUPPORTED);
    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastrtps::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Reply&>(_)))
            .WillOnce(testing::DoAll(testing::SetArgReferee<1>(expectedReply), testing::Return(true)));

    reply_listener_->onNewCacheChangeAdded(&stateful_reader_, change);

    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_REMOTE_EX_INVALID_ARGUMENT)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER",
                    "Received reply with exception code: " +
                    static_cast<int>(rpc::RemoteExceptionCode_t::REMOTE_EX_INVALID_ARGUMENT));


    CacheChange_t* change = new CacheChange_t();
    change->writerGUID.entityId = c_EntityId_TypeLookup_reply_writer;
    stateful_reader_.getHistory()->add_change(change);

    TypeLookup_Reply expectedReply;
    expectedReply.header().remoteEx(rpc::RemoteExceptionCode_t::REMOTE_EX_INVALID_ARGUMENT);
    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastrtps::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Reply&>(_)))
            .WillOnce(testing::DoAll(testing::SetArgReferee<1>(expectedReply), testing::Return(true)));

    reply_listener_->onNewCacheChangeAdded(&stateful_reader_, change);

    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_REMOTE_EX_OUT_OF_RESOURCES)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER",
                    "Received reply with exception code: " +
                    static_cast<int>(rpc::RemoteExceptionCode_t::REMOTE_EX_OUT_OF_RESOURCES));


    CacheChange_t* change = new CacheChange_t();
    change->writerGUID.entityId = c_EntityId_TypeLookup_reply_writer;
    stateful_reader_.getHistory()->add_change(change);

    TypeLookup_Reply expectedReply;
    expectedReply.header().remoteEx(rpc::RemoteExceptionCode_t::REMOTE_EX_OUT_OF_RESOURCES);
    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastrtps::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Reply&>(_)))
            .WillOnce(testing::DoAll(testing::SetArgReferee<1>(expectedReply), testing::Return(true)));

    reply_listener_->onNewCacheChangeAdded(&stateful_reader_, change);
    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_REMOTE_EX_UNKNOWN_OPERATION)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER",
                    "Received reply with exception code: " +
                    static_cast<int>(rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_OPERATION));


    CacheChange_t* change = new CacheChange_t();
    change->writerGUID.entityId = c_EntityId_TypeLookup_reply_writer;
    stateful_reader_.getHistory()->add_change(change);

    TypeLookup_Reply expectedReply;
    expectedReply.header().remoteEx(rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_OPERATION);
    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastrtps::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Reply&>(_)))
            .WillOnce(testing::DoAll(testing::SetArgReferee<1>(expectedReply), testing::Return(true)));

    reply_listener_->onNewCacheChangeAdded(&stateful_reader_, change);
    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_REMOTE_EX_UNKNOWN_EXCEPTION)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER",
                    "Received reply with exception code: " +
                    static_cast<int>(rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_EXCEPTION));


    CacheChange_t* change = new CacheChange_t();
    change->writerGUID.entityId = c_EntityId_TypeLookup_reply_writer;
    stateful_reader_.getHistory()->add_change(change);

    TypeLookup_Reply expectedReply;
    expectedReply.header().remoteEx(rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_EXCEPTION);
    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastrtps::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Reply&>(_)))
            .WillOnce(testing::DoAll(testing::SetArgReferee<1>(expectedReply), testing::Return(true)));

    reply_listener_->onNewCacheChangeAdded(&stateful_reader_, change);
    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_wrong_replyid)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER", "Received uknown reply type.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Reply reply;
    reply.return_value()._d() = 44444444U;
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
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER", "Error registering type.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Reply reply;
    reply.header().relatedRequestId(valid_sampleidentity());
    reply.return_value()._d() = TypeLookup_getTypes_HashId;
    ReplyWithServerGUID reply_with_guid;
    reply_with_guid.reply = reply;

    reply_listener_->get_replies_queue()->push(reply_with_guid);
    reply_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_getTypes_registry_empty_type_error)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER", "Error registering type.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Reply reply;
    reply.header().relatedRequestId(valid_sampleidentity());
    reply.return_value()._d() = TypeLookup_getTypes_HashId;
    TypeIdentifierTypeObjectPair pair;
    TypeLookup_getTypes_Out out;
    out.types().push_back(pair);
    reply.return_value().getType().result(out);
    ReplyWithServerGUID reply_with_guid;
    reply_with_guid.reply = reply;

    reply_listener_->get_replies_queue()->push(reply_with_guid);
    reply_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}


TEST_F(TypeLookupServiceTests, TypeLookupReplyListener_getTypes_registry_inconsistent_type_error)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REPLY_LISTENER", "Error registering type.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Reply reply;
    reply.header().relatedRequestId(valid_sampleidentity());
    reply.return_value()._d() = TypeLookup_getTypes_HashId;

    TypeIdentifierTypeObjectPair pair;
    TypeIdentifier id;
    id._d() = TK_STRUCTURE;
    TypeObject obj;
    obj.complete()._d() = TK_SEQUENCE;
    pair.type_identifier(id);
    pair.type_object(obj);

    TypeLookup_getTypes_Out out;
    out.types().push_back(pair);
    reply.return_value().getType().result(out);
    ReplyWithServerGUID reply_with_guid;
    reply_with_guid.reply = reply;

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

    // EXPECT_CALL(*tlm_, get_type_dependencies(testing::_, testing::_, testing::_))
    //         .WillOnce(testing::Return(INVALID_SAMPLE_IDENTITY));

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Reply reply;
    reply.header().relatedRequestId(valid_sampleidentity());
    reply.return_value()._d() = TypeLookup_getDependencies_HashId;
    TypeIdentifierTypeObjectPair pair;
    TypeLookup_getTypeDependencies_Out out;
    out.continuation_point(create_continuation_point(10));
    reply.return_value().getTypeDependencies().result(out);
    ReplyWithServerGUID reply_with_guid;
    reply_with_guid.reply = reply;

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
    reply.return_value()._d() = TypeLookup_getDependencies_HashId;
    TypeIdentifierTypeObjectPair pair;
    TypeLookup_getTypeDependencies_Out out;
    reply.return_value().getTypeDependencies().result(out);
    ReplyWithServerGUID reply_with_guid;
    reply_with_guid.reply = reply;

    reply_listener_->get_replies_queue()->push(reply_with_guid);
    reply_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_wrong_EntityId)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TL_REQUEST_READER", "Received data from a bad endpoint.");

    CacheChange_t* change = new CacheChange_t();
    change->writerGUID.entityId = 0x111111C3;
    stateful_reader_.getHistory()->add_change(change);

    request_listener_->onNewCacheChangeAdded(&stateful_reader_, change);

    EXPECT_EQ(reply_listener_->get_replies_queue()->size(), 0);

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_malformed_Request)
{
    CacheChange_t* change = new CacheChange_t();
    change->writerGUID.entityId = c_EntityId_TypeLookup_request_writer;
    stateful_reader_.getHistory()->add_change(change);

    EXPECT_CALL(*tlm_, receive(
                testing::Matcher<fastrtps::rtps::CacheChange_t&>(_),
                testing::Matcher<TypeLookup_Request&>(_)))
            .WillOnce(testing::Return(false));

    request_listener_->onNewCacheChangeAdded(&stateful_reader_, change);

    EXPECT_EQ(request_listener_->get_requests_queue()->size(), 0);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_wrong_requestid)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REQUEST_LISTENER", "Received unknown request type.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Request request;
    request.data()._d() = 44444444U;

    request_listener_->get_requests_queue()->push(request);
    request_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_getTypes_empty)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REQUEST_LISTENER", "Error getting type.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Request request;
    request.data()._d() = TypeLookup_getTypes_HashId;

    request_listener_->get_requests_queue()->push(request);
    request_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_getTypes_registry_empty_type_error)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REQUEST_LISTENER", "Error getting type.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Request request;
    request.data()._d() = TypeLookup_getTypes_HashId;
    TypeLookup_getTypes_In in;
    TypeIdentifierSeq id_seq;
    TypeIdentifier id;
    id_seq.push_back(id);
    in.type_ids(id_seq);
    in.type_ids().push_back(id);
    request.data().getTypes(in);

    request_listener_->get_requests_queue()->push(request);
    request_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_getTypes_registry_unknown_type_error)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REQUEST_LISTENER", "Error getting type.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Request request;
    request.data()._d() = TypeLookup_getTypes_HashId;
    TypeLookup_getTypes_In in;
    TypeIdentifierSeq id_seq;
    TypeIdentifier id;
    id._d() = TK_STRUCTURE;

    id_seq.push_back(id);
    in.type_ids(id_seq);
    in.type_ids().push_back(id);
    request.data().getTypes(in);

    request_listener_->get_requests_queue()->push(request);
    request_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_getDependencies_empty)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REQUEST_LISTENER", "Error getting type dependencies.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Request request;
    request.data()._d() = TypeLookup_getDependencies_HashId;

    request_listener_->get_requests_queue()->push(request);
    request_listener_->notify();

    Log::Flush();
    EXPECT_EQ(log_consumer->wait_for_entries(1, 1), 1);
}

TEST_F(TypeLookupServiceTests, TypeLookupRequestListener_getDependencies_registry_uknown_type_error)
{
    MockLogConsumer* log_consumer =
            setup_expect_log_msg("TYPELOOKUP_SERVICE_REQUEST_LISTENER", "Error getting type dependencies.");

    TypeIdentfierWithSize tidws;
    tlm_->async_get_type_requests_.emplace(valid_sampleidentity(), tidws);

    TypeLookup_Request request;
    request.data()._d() = TypeLookup_getDependencies_HashId;

    TypeLookup_getTypeDependencies_In in;
    TypeIdentifierSeq id_seq;
    TypeIdentifier id;
    id._d() = TK_STRUCTURE;
    id_seq.push_back(id);
    in.type_ids(id_seq);
    in.type_ids().push_back(id);
    request.data().getTypeDependencies(in);

    request_listener_->get_requests_queue()->push(request);
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
