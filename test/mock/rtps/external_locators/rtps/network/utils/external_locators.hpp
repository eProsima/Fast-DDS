// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file external_locators.hpp
 */

#ifndef FASTDDS_RTPS_NETWORK_UTILS_EXTERNAL_LOCATORS_HPP
#define FASTDDS_RTPS_NETWORK_UTILS_EXTERNAL_LOCATORS_HPP

#include <fastdds/rtps/attributes/ExternalLocators.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace network {
namespace external_locators {

/**
 * Sets the external locators entry for externality index 0 to a list of listening locators.
 *
 * @param [in,out]  external_locators   The external locators collection to be updated.
 * @param [in]      listening_locators  The list of listening locators to set on externality index 0.
 */
void set_listening_locators(
        ExternalLocators& /*external_locators*/,
        const LocatorList& /*listening_locators*/)
{
}

/**
 * Add external locators to the locators announced by a participant.
 *
 * @param [in,out]  data                           ParticipantProxyData of the local participant to update.
 * @param [in]      metatraffic_external_locators  The external locators collection with the external meta-traffic
 *                                                 locators to announce.
 * @param [in]      default_external_locators      The external locators collection with the external default locators
 *                                                 to announce.
 */
void add_external_locators(
        ParticipantProxyData& /*data*/,
        const ExternalLocators& /*metatraffic_external_locators*/,
        const ExternalLocators& /*default_external_locators*/)
{
}

/**
 * Add external locators to the locators announced by a writer.
 *
 * @param [in,out]  data               WriterProxyData of the local writer to update.
 * @param [in]      external_locators  The external locators collection with the external locators to announce.
 */
void add_external_locators(
        WriterProxyData& /*data*/,
        const ExternalLocators& /*external_locators*/)
{
}

/**
 * Add external locators to the locators announced by a reader.
 *
 * @param [in,out]  data               ReaderProxyData of the local reader to update.
 * @param [in]      external_locators  The external locators collection with the external locators to announce.
 */
void add_external_locators(
        ReaderProxyData& /*data*/,
        const ExternalLocators& /*external_locators*/)
{
}

void add_external_locators(
        LocatorList&,
        const ExternalLocators&)
{
}

/**
 * Filter the locators of a remote participant according to the matching algorithm.
 *
 * @param [in,out]  data                           ParticipantProxyData of the remote participant to update.
 * @param [in]      metatraffic_external_locators  The external locators collection to use for filtering of the
 *                                                 meta-traffic locators.
 * @param [in]      default_external_locators      The external locators collection to use for filtering of the
 *                                                 default locators.
 * @param [in]      ignore_non_matching            Whether addresses not matching any of the input external locators
 *                                                 should be filtered out.
 */
void filter_remote_locators(
        ParticipantProxyData& /*data*/,
        const ExternalLocators& /*metatraffic_external_locators*/,
        const ExternalLocators& /*default_external_locators*/,
        bool /*ignore_non_matching*/)
{
}

/**
 * Filter the locators of a remote endpoint according to the matching algorithm.
 *
 * @param [in,out]  locators             LocatorSelectorEntry of the remote destination to update.
 * @param [in]      external_locators    The external locators collection to use for filtering of the locators.
 * @param [in]      ignore_non_matching  Whether addresses not matching any of the input external locators should be
 *                                       filtered out.
 */
void filter_remote_locators(
        LocatorSelectorEntry& /*locators*/,
        const ExternalLocators& /*external_locators*/,
        bool /*ignore_non_matching*/)
{
}

} // namespace external_locators
} // namespace network
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // FASTDDS_RTPS_NETWORK_UTILS_EXTERNAL_LOCATORS_HPP
