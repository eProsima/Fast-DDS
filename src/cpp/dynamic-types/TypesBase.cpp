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

namespace eprosima{
namespace fastrtps{

using namespace rtps;

namespace types{

const ReturnCode_t ReturnCode_t::RETCODE_OK = {0};
const ReturnCode_t ReturnCode_t::RETCODE_ERROR = {1};
const ReturnCode_t ReturnCode_t::RETCODE_UNSUPPORTED = {2};
const ReturnCode_t ReturnCode_t::RETCODE_BAD_PARAMETER = {3};
const ReturnCode_t ReturnCode_t::RETCODE_PRECONDITION_NOT_MET = {4};
const ReturnCode_t ReturnCode_t::RETCODE_OUT_OF_RESOURCES = {5};
const ReturnCode_t ReturnCode_t::RETCODE_NOT_ENABLED = {6};
const ReturnCode_t ReturnCode_t::RETCODE_IMMUTABLE_POLICY = {7};
const ReturnCode_t ReturnCode_t::RETCODE_INCONSISTENT_POLICY = {8};
const ReturnCode_t ReturnCode_t::RETCODE_ALREADY_DELETED = {9};
const ReturnCode_t ReturnCode_t::RETCODE_TIMEOUT = {10};
const ReturnCode_t ReturnCode_t::RETCODE_NO_DATA = {11};
const ReturnCode_t ReturnCode_t::RETCODE_ILLEGAL_OPERATION = {12};
#if HAVE_SECURITY
const ReturnCode_t ReturnCode_t::RETCODE_NOT_ALLOWED_BY_SECURITY = {13};
#endif // HAVE_SECURITY


void MemberFlag::serialize(eprosima::fastcdr::Cdr &cdr) const
{
    //cdr << m_MemberFlag;
    uint16_t bits = static_cast<uint16_t>(m_MemberFlag.to_ulong());
    cdr << bits;
}

void MemberFlag::deserialize(eprosima::fastcdr::Cdr &cdr)
{
    //cdr >> (uint16_t)m_MemberFlag;
    uint16_t bits;
    cdr >> bits;
    m_MemberFlag = std::bitset<16>(bits);
}

size_t MemberFlag::getCdrSerializedSize(const MemberFlag&, size_t current_alignment)
{
    return 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
}

void TypeFlag::serialize(eprosima::fastcdr::Cdr &cdr) const
{
    //cdr << m_TypeFlag;
    uint16_t bits = static_cast<uint16_t>(m_TypeFlag.to_ulong());
    cdr << bits;
}

void TypeFlag::deserialize(eprosima::fastcdr::Cdr &cdr)
{
    //cdr >> (uint16_t)m_TypeFlag;
    uint16_t bits;
    cdr >> bits;
    m_TypeFlag = std::bitset<16>(bits);
}

size_t TypeFlag::getCdrSerializedSize(const TypeFlag&, size_t current_alignment)
{
    return 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
