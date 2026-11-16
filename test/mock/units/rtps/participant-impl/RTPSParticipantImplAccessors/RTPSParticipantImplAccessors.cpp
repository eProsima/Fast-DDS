// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of the RTPSParticipantImpl attribute accessors, from
// src/cpp/rtps/participant/RTPSParticipantImpl.cpp, for unit tests compiling code that refers to a participant without
// creating one. They are defined as in the real source. No constructor or destructor is defined, so linking it does not
// bring the code of the participant members.

#include <rtps/participant/RTPSParticipantImpl.hpp>

#include <mutex>

namespace eprosima {
namespace fastdds {
namespace rtps {

const RTPSParticipantAttributes& RTPSParticipantImpl::get_attributes() const
{
    return m_att;
}

const RTPSParticipantConstantAttributes& RTPSParticipantImpl::get_const_attributes() const
{
    return m_const_att;
}

const RTPSParticipantMutableAttributes RTPSParticipantImpl::get_mutable_attributes() const
{
    std::lock_guard<std::mutex> _(mutex_);
    return RTPSParticipantMutableAttributes(m_att);
}

RTPSParticipantAttributes RTPSParticipantImpl::copy_attributes() const
{
    std::lock_guard<std::mutex> _(mutex_);
    return m_att;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
