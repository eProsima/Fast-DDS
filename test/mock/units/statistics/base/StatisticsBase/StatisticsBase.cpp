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

// Link-seam implementation of StatisticsBase: replaces src/cpp/statistics/rtps/StatisticsBase.cpp, for unit tests not
// linking core. Only the conversions between the RTPS and the statistics types and the mask of enabled statistics
// writers of a participant are implemented, as in the real source.

#include <statistics/rtps/StatisticsBase.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {

detail::Locator_s to_statistics_type(
        fastdds::rtps::Locator_t locator)
{
    return *reinterpret_cast<detail::Locator_s*>(&locator);
}

fastdds::rtps::Locator_t to_fastdds_type(
        detail::Locator_s locator)
{
    return *reinterpret_cast<fastdds::rtps::Locator_t*>(&locator);
}

detail::GUID_s to_statistics_type(
        fastdds::rtps::GUID_t guid)
{
    return *reinterpret_cast<detail::GUID_s*>(&guid);
}

fastdds::rtps::GUID_t to_fastdds_type(
        detail::GUID_s guid)
{
    return *reinterpret_cast<fastdds::rtps::GUID_t*>(&guid);
}

detail::SampleIdentity_s to_statistics_type(
        fastdds::rtps::SampleIdentity sample_id)
{
    return *reinterpret_cast<detail::SampleIdentity_s*>(&sample_id);
}

void StatisticsParticipantImpl::set_enabled_statistics_writers_mask(
        uint32_t enabled_writers)
{
    enabled_writers_mask_.store(enabled_writers);
}

uint32_t StatisticsParticipantImpl::get_enabled_statistics_writers_mask()
{
    return enabled_writers_mask_.load();
}

void StatisticsParticipantImpl::on_entity_discovery(
        const GUID_t&,
        const fastdds::dds::ParameterPropertyList_t&)
{
}

} // namespace statistics
} // namespace fastdds
} // namespace eprosima
