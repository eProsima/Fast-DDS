// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file RPCHeadersImpl.cpp
 *
 */

#include <fastdds/dds/builtin/common/ReplyHeader.hpp>
#include <fastdds/dds/builtin/common/RequestHeader.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>

#include "RPCHeadersImplCdrAux.ipp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

size_t ReplyHeader::getCdrSerializedSize(
        const ReplyHeader& data,
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;
    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv1);
    return eprosima::fastcdr::calculate_serialized_size(calculator, data, current_alignment) - initial_alignment;
}

void ReplyHeader::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    eprosima::fastcdr::serialize(scdr, *this);
}

void ReplyHeader::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    eprosima::fastcdr::deserialize(dcdr, *this);
}

size_t RequestHeader::getCdrSerializedSize(
        const RequestHeader& data,
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;
    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv1);
    return eprosima::fastcdr::calculate_serialized_size(calculator, data, current_alignment) - initial_alignment;
}

void RequestHeader::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    eprosima::fastcdr::serialize(scdr, *this);
}

void RequestHeader::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    eprosima::fastcdr::deserialize(dcdr, *this);
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima
