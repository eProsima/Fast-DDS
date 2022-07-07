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
 * @file ExternalLocatorsProcessor.cpp
 */

#include <rtps/network/ExternalLocatorsProcessor.hpp>

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

static void get_mask_and_cost(
        const Locator& locator,
        uint8_t& mask,
        uint8_t& cost)
{
    cost = 0;
    mask = 24;

    switch (locator.kind)
    {
        case LOCATOR_KIND_UDPv4:
        case LOCATOR_KIND_TCPv4:
            // TODO(MiguelCompany): We should take cost and mask from the routing table
            cost = 1;
            break;

        case LOCATOR_KIND_UDPv6:
        case LOCATOR_KIND_TCPv6:
            // TODO(MiguelCompany): We should take cost and mask from the routing table
            cost = 2;
            break;

        // SHM locators should always match exactly (full 128 bits of the address), and always have the lowest cost
        case LOCATOR_KIND_SHM:
            cost = 0;
            mask = 128;
            break;

        default:
            assert(false && "Unexpected locator kind");
            break;
    }
}

void set_listening_locators(
        ExternalLocators& external_locators,
        const LocatorList& listening_locators)
{
    // Remove all locators on externality index 0
    external_locators[0].clear();

    // Add all listening locators to externality index 0
    for (const auto& listening_locator : listening_locators)
    {
        // Get network mask and interface cost from system
        uint8_t mask = 24;
        uint8_t cost = 0;
        get_mask_and_cost(listening_locator, mask, cost);

        // Prepare LocatorWithMask to be added
        LocatorWithMask locator;
        static_cast<Locator&>(locator) = listening_locator;
        locator.mask(mask);

        // Add to externality index 0 with the corresponding cost
        external_locators[0][cost].push_back(locator);
    }
}

template<typename T>
static void perform_add_external_locators(
        T& announced_locators,
        const ExternalLocators& external_locators)
{
    for (const auto& externality_item : external_locators)
    {
        // Only add locators with externality greater than 0
        if (externality_item.first > 0)
        {
            for (const auto& cost_item : externality_item.second)
            {
                for (const auto& locator : cost_item.second)
                {
                    announced_locators.add_unicast_locator(locator);
                }
            }
        }
    }
}

void add_external_locators(
        ParticipantProxyData& data,
        const ExternalLocators& metatraffic_external_locators,
        const ExternalLocators& default_external_locators)
{
    perform_add_external_locators(data.metatraffic_locators, metatraffic_external_locators);
    perform_add_external_locators(data.default_locators, default_external_locators);
}

void add_external_locators(
        WriterProxyData& data,
        const ExternalLocators& external_locators)
{
    perform_add_external_locators(data, external_locators);
}

void add_external_locators(
        ReaderProxyData& data,
        const ExternalLocators& external_locators)
{
    perform_add_external_locators(data, external_locators);
}

static void filter_remote_locators(
        fastrtps::ResourceLimitedVector<Locator>& locators,
        const ExternalLocators& external_locators,
        bool ignore_non_matching)
{
}

void filter_remote_locators(
        ParticipantProxyData& data,
        const ExternalLocators& metatraffic_external_locators,
        const ExternalLocators& default_external_locators,
        bool ignore_non_matching)
{
    filter_remote_locators(data.metatraffic_locators.unicast, metatraffic_external_locators, ignore_non_matching);
    filter_remote_locators(data.default_locators.unicast, default_external_locators, ignore_non_matching);
}

void filter_remote_locators(
        LocatorSelectorEntry& locators,
        const ExternalLocators& external_locators,
        bool ignore_non_matching)
{
    filter_remote_locators(locators.unicast, external_locators, ignore_non_matching);
}

} // namespace ExternalLocatorsProcessor
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
