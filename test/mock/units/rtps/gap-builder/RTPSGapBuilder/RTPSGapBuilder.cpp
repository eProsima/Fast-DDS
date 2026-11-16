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

// Link-seam implementation of RTPSGapBuilder: replaces src/cpp/rtps/messages/RTPSGapBuilder.cpp and forwards every
// method to the RTPSGapBuilderMock companion of the instance.

#include <rtps/messages/RTPSGapBuilder.hpp>

#include <mock/rtps/messages/RTPSGapBuilderMock.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<RTPSGapBuilder>;

RTPSGapBuilder::RTPSGapBuilder(
        RTPSMessageGroup& group)
    : group_(group)
{
    Companion::attach(this);
}

RTPSGapBuilder::RTPSGapBuilder(
        RTPSMessageGroup& group,
        const GUID_t& reader_guid)
    : with_specific_destination_(true)
    , group_(group)
    , reader_guid_(reader_guid)
{
    Companion::attach(this);
}

RTPSGapBuilder::~RTPSGapBuilder()
{
    Companion::detach(this);
}

bool RTPSGapBuilder::add(
        const SequenceNumber_t& gap_sequence)
{
    return Companion::get(this).add(gap_sequence);
}

bool RTPSGapBuilder::flush()
{
    return Companion::get(this).flush();
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
