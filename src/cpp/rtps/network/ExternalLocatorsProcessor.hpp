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
 * @file ExternalLocatorsProcessor.hpp
 */

#ifndef _RTPS_NETWORK_EXTERNALLOCATORSPROCESSOR_HPP_
#define _RTPS_NETWORK_EXTERNALLOCATORSPROCESSOR_HPP_

#include <fastdds/rtps/attributes/ExternalLocators.hpp>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ExternalLocatorsProcessor {

using eprosima::fastrtps::rtps::LocatorSelectorEntry;
using eprosima::fastrtps::rtps::ParticipantProxyData;
using eprosima::fastrtps::rtps::ReaderProxyData;
using eprosima::fastrtps::rtps::WriterProxyData;

/**
 * Sets the external locators entry for externality index 0 to a list of listening locators.
 *
 * @param [in,out]  external_locators   The external locators collection to be updated.
 * @param [in]      listening_locators  The list of listening locators to be set on externality index 0.
 */
void set_listening_locators(
        ExternalLocators& external_locators,
        const LocatorList& listening_locators);

/**
 * Adds external locators to the locators announced by a participant.
 *
 * @param [in,out]  data                           ParticipantProxyData of the local participant to be updated.
 * @param [in]      metatraffic_external_locators  The external locators collection with the external meta-traffic
 *                                                 locators to be announced.
 * @param [in]      default_external_locators      The external locators collection with the external default locators
 *                                                 to be announced.
 */
void add_external_locators(
        ParticipantProxyData& data,
        const ExternalLocators& metatraffic_external_locators,
        const ExternalLocators& default_external_locators);

/**
 * Adds external locators to the locators announced by a writer.
 *
 * @param [in,out]  data               WriterProxyData of the local writer to be updated.
 * @param [in]      external_locators  The external locators collection with the external locators to be announced.
 */
void add_external_locators(
        WriterProxyData& data,
        const ExternalLocators& external_locators);

/**
 * Adds external locators to the locators announced by a reader.
 *
 * @param [in,out]  data               ReaderProxyData of the local reader to be updated.
 * @param [in]      external_locators  The external locators collection with the external locators to be announced.
 */
void add_external_locators(
        ReaderProxyData& data,
        const ExternalLocators& external_locators);

/**
 * Adds external locators to a list of locators.
 *
 * @param [in,out]  list               LocatorList to be updated.
 * @param [in]      external_locators  The external locators collection with the external locators to be announced.
 */
void add_external_locators(
        LocatorList& list,
        const ExternalLocators& external_locators);

/**
 * Filters the locators of a remote participant according to the matching algorithm.
 *
 * @param [in,out]  data                           ParticipantProxyData of the remote participant to be updated.
 * @param [in]      metatraffic_external_locators  The external locators collection to use for filtering of the
 *                                                 meta-traffic locators.
 * @param [in]      default_external_locators      The external locators collection to use for filtering of the
 *                                                 default locators.
 * @param [in]      ignore_non_matching            Whether addresses not matching any of the input external locators
 *                                                 should be filtered out.
 */
void filter_remote_locators(
        ParticipantProxyData& data,
        const ExternalLocators& metatraffic_external_locators,
        const ExternalLocators& default_external_locators,
        bool ignore_non_matching);

/**
 * Filters the locators of a remote endpoint according to the matching algorithm.
 *
 * @param [in,out]  locators             LocatorSelectorEntry of the remote destination to be updated.
 * @param [in]      external_locators    The external locators collection to use for filtering of the locators.
 * @param [in]      ignore_non_matching  Whether addresses not matching any of the input external locators should be
 *                                       filtered out.
 */
void filter_remote_locators(
        LocatorSelectorEntry& locators,
        const ExternalLocators& external_locators,
        bool ignore_non_matching);

} // namespace ExternalLocatorsProcessor
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // _RTPS_NETWORK_EXTERNALLOCATORSPROCESSOR_HPP_
