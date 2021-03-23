// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file TypesBase.cpp
 */

#include <fastrtps/types/TypesBase.h>
#include <fastcdr/Cdr.h>

namespace eprosima {
namespace fastrtps {

using namespace rtps;

namespace types {

void MemberFlag::serialize(
        eprosima::fastcdr::Cdr& cdr) const
{
    //cdr << m_MemberFlag;
    uint16_t bits = static_cast<uint16_t>(m_MemberFlag.to_ulong());
    cdr << bits;
}

void MemberFlag::deserialize(
        eprosima::fastcdr::Cdr& cdr)
{
    //cdr >> (uint16_t)m_MemberFlag;
    uint16_t bits;
    cdr >> bits;
    m_MemberFlag = std::bitset<16>(bits);
}

size_t MemberFlag::getCdrSerializedSize(
        const MemberFlag&,
        size_t current_alignment)
{
    return 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
}

void TypeFlag::serialize(
        eprosima::fastcdr::Cdr& cdr) const
{
    //cdr << m_TypeFlag;
    uint16_t bits = static_cast<uint16_t>(m_TypeFlag.to_ulong());
    cdr << bits;
}

void TypeFlag::deserialize(
        eprosima::fastcdr::Cdr& cdr)
{
    //cdr >> (uint16_t)m_TypeFlag;
    uint16_t bits;
    cdr >> bits;
    m_TypeFlag = std::bitset<16>(bits);
}

size_t TypeFlag::getCdrSerializedSize(
        const TypeFlag&,
        size_t current_alignment)
{
    return 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
