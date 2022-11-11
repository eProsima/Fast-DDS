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
#include <fastrtps/utils/IPLocator.h>

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

template<>
void perform_add_external_locators<LocatorList>(
        LocatorList& locators,
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
                    locators.push_back(locator);
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

void add_external_locators(
        LocatorList& list,
        const ExternalLocators& external_locators)
{
    perform_add_external_locators(list, external_locators);
}

/**
 * Checks if the first significant bits of two locator addresses match.
 *
 * @param [in] addr1     First address to check.
 * @param [in] addr2     Second address to check.
 * @param [in] num_bits  Number of bits to take into account.
 *
 * @return true when the first @c num_bits of both addresses are equal, false otherwise.
 */
static bool address_matches(
        const uint8_t* addr1,
        const uint8_t* addr2,
        uint64_t num_bits)
{
    uint64_t full_bytes = num_bits / 8;
    if ((0 == full_bytes) || std::equal(addr1, addr1 + full_bytes, addr2))
    {
        uint64_t rem_bits = num_bits % 8;
        if (rem_bits == 0)
        {
            return true;
        }

        uint8_t mask = 0xFF << (8 - rem_bits);
        return (addr1[full_bytes] & mask) == (addr2[full_bytes] & mask);
    }

    return false;
}

/**
 * Checks if a remote locator matches with a local locator.
 *
 * @param [in] local_locator     LocatorWithMask with the matching configuration.
 * @param [in] remote_locator    Locator being checked.
 *
 * @return true when the locators match, false otherwise.
 */
static bool match_with_mask(
        const LocatorWithMask& local_locator,
        const Locator& remote_locator)
{
    if (local_locator.kind == remote_locator.kind)
    {
        switch (local_locator.kind)
        {
            case LOCATOR_KIND_UDPv4:
            case LOCATOR_KIND_TCPv4:
                assert(32 >= local_locator.mask());
                return address_matches(remote_locator.address + 12, local_locator.address + 12, local_locator.mask());

            case LOCATOR_KIND_UDPv6:
            case LOCATOR_KIND_TCPv6:
            case LOCATOR_KIND_SHM:
                assert(128 >= local_locator.mask());
                return address_matches(remote_locator.address, local_locator.address, local_locator.mask());
        }
    }

    return false;
}

static constexpr uint64_t heuristic_value(
        uint64_t externality,
        uint64_t cost)
{
    return ((255ull - externality) << 16) | (cost << 8);
}

static uint64_t heuristic(
        const Locator& remote_locator,
        const ExternalLocators& external_locators,
        bool ignore_non_matching)
{
    if (LOCATOR_KIND_SHM == remote_locator.kind)
    {
        return heuristic_value(0, 0);
    }

    if (fastrtps::rtps::IPLocator::isLocal(remote_locator))
    {
        return heuristic_value(0, 1);
    }

    for (const auto& externality : external_locators)
    {
        for (const auto& cost : externality.second)
        {
            for (const LocatorWithMask& local_locator : cost.second)
            {
                if (match_with_mask(local_locator, remote_locator))
                {
                    return heuristic_value(externality.first, cost.first);
                }
            }
        }
    }

    return ignore_non_matching ? std::numeric_limits<uint64_t>::max() : 0;
}

static void filter_remote_locators(
        fastrtps::ResourceLimitedVector<Locator>& locators,
        const ExternalLocators& external_locators,
        bool ignore_non_matching)
{
    auto compare_locators = [external_locators, ignore_non_matching](const Locator& lhs, const Locator& rhs) -> bool
            {
                return heuristic(lhs, external_locators, ignore_non_matching) <
                       heuristic(rhs, external_locators, ignore_non_matching);
            };

    /* This will sort the received locators according to the following criteria:
     * 1. Non-matching locators when not ignored. Heuristic value: 0
     * 2. Matching locators. Heuristic value: ((255ull - externality) << 16) | (cost << 8)
     * 3. Non-matching locators when ignored. Heuristic value: max_uint64_t
     *
     * The heuristic has been chosen so non-matching locators will never give a value that will be given to a matching
     * locator. Matching locators will be sorted first by highest externality, then by lowest cost.
     */
    std::sort(locators.begin(), locators.end(), compare_locators);

    /* Remove non-matching locators if requested to.
     * This is done by removing all locators at the end with an heuristic value of max_uint64_t.
     */
    if (ignore_non_matching)
    {
        while (!locators.empty())
        {
            uint64_t h = heuristic(locators.back(), external_locators, ignore_non_matching);
            if (std::numeric_limits<uint64_t>::max() != h)
            {
                break;
            }
            locators.pop_back();
        }
    }

    // Check what locators to keep
    auto it = locators.begin();

    // Keep non-matching locators with an heuristic value of 0.
    if (!ignore_non_matching)
    {
        while (it != locators.end() && (0 == heuristic(*it, external_locators, ignore_non_matching)))
        {
            ++it;
        }
    }

    // Traverse external_locators in heuristic order, checking if certain heuristic value should be ignored
    for (const auto& externality : external_locators)
    {
        for (const auto& cost : externality.second)
        {
            // Check if the locators on this heuristic value should be ignored
            uint64_t entry_heuristic = heuristic_value(externality.first, cost.first);
            auto end_it = it;
            size_t num_exactly_matched = 0;
            while (end_it != locators.end() &&
                    (entry_heuristic == heuristic(*end_it, external_locators, ignore_non_matching)))
            {
                for (const LocatorWithMask& local_locator : cost.second)
                {
                    if (std::equal(end_it->address, end_it->address + 16, local_locator.address))
                    {
                        ++num_exactly_matched;
                        break;
                    }
                }
                ++end_it;
            }

            if (end_it != it)
            {
                // There was at least one locator with this heuristic value
                if (externality.first > 0 &&
                        num_exactly_matched == cost.second.size() &&
                        end_it != locators.end() &&
                        static_cast<size_t>(std::distance(it, end_it)) == num_exactly_matched)
                {
                    // All locators on this heuristic were the local locators, ignore this heuristic
                    it = locators.erase(it, end_it);
                }
                else
                {
                    // We should keep this locators, remove the rest and return
                    it = locators.erase(end_it, locators.end());
                    return;
                }
            }
        }
    }

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
