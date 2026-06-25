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

#include <cstddef>
#include <cstdint>
#include <deque>

#include <gtest/gtest.h>

#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/LocatorSelector.hpp>
#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>
#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

namespace {

LocatorSelectorEntry make_reader_entry(
        uint32_t entity_id,
        uint16_t port)
{
    Locator_t loc;
    loc.kind = LOCATOR_KIND_UDPv4;
    loc.port = port;
    loc.address[12] = 127;
    loc.address[15] = 1;

    LocatorList_t unicast;
    unicast.push_back(loc);

    LocatorSelectorEntry entry = LocatorSelectorEntry::create_fully_selected_entry(unicast);
    // A non-unknown remote_guid is required for LocatorSelectorEntry::enable() to take effect.
    entry.remote_guid.entityId = entity_id;
    return entry;
}

/*!
 * Emulates NetworkFactory::select_locators() followed by a UDP transport that selects the single
 * unicast locator of every entry flagged transport_should_process. This is the only path that
 * (re)builds selections_, and it clears force_reset_ via transport_starts().
 */
void simulate_select_locators(
        LocatorSelector& selector)
{
    selector.selection_start();                                       // clears selections_ and per-entry state
    auto& entries = selector.transport_starts();                      // clears force_reset_; flags enabled entries
    for (size_t i = 0; i < entries.size(); ++i)
    {
        LocatorSelectorEntry* entry = entries.at(i);
        if (entry->transport_should_process && !entry->unicast.empty())
        {
            entry->state.unicast.push_back(0);                        // transport selects first unicast locator
            selector.select(i);
        }
    }
}

} // anonymous namespace

/*!
 * Regression test for the GrainedFlowController wedge (Refs #24363).
 *
 * When a reader exceeds its per-period byte budget, GrainedFlowController calls
 * LocatorSelector::unselect(), which mutates selections_ outside of the select_locators() flow.
 * state_has_changed() tracks only the per-entry enabled/allowed_to_send flags, so once the period
 * reset re-arms allowed_to_send back to its previous value, the change tracker reported "no change",
 * select_locators() was never re-run, selected_size() stayed 0, and the writer was wedged forever.
 *
 * The fix makes unselect() flag the selector dirty (and reset() preserve that flag), so the next
 * delivery pass re-runs select_locators().
 */
TEST(LocatorSelectorTests, reselects_after_flow_controller_unselect)
{
    LocatorSelectorEntry entry = make_reader_entry(1, 7400);

    LocatorSelector selector(ResourceLimitedContainerConfig::fixed_size_configuration(1));
    selector.add_entry(&entry);

    // ---------- Pass 1: normal delivery; reader selected and data put on the wire ----------
    selector.reset(false);
    selector.enable(entry.remote_guid);
    ASSERT_TRUE(selector.state_has_changed());      // freshly added entry -> dirty
    simulate_select_locators(selector);
    ASSERT_GT(selector.selected_size(), 0u);
    selector.initial_allow_to_send(true);           // an actual send() resets this flag

    // ---------- Pass 2: reader over budget; GrainedFlowController disables and unselects it ----------
    selector.reset(false);
    selector.enable(entry.remote_guid);
    // State did not change vs. pass 1, so the writer does NOT re-run select_locators();
    // selections_ still holds the reader from pass 1. data_exceeds_limitation then does:
    entry.allowed_to_send = false;
    selector.unselect(0);
    ASSERT_EQ(selector.selected_size(), 0u);        // reader dropped from the selection

    // ---------- Period reset: prepare_locator_selector re-arms allowed_to_send ----------
    entry.allowed_to_send = true;

    // ---------- Pass 3: recovery delivery; the selection MUST be rebuilt ----------
    selector.reset(false);
    selector.enable(entry.remote_guid);

    // Regression point, state_has_changed() should be true to trigger select_locators()
    EXPECT_TRUE(selector.state_has_changed());

    simulate_select_locators(selector);
    EXPECT_GT(selector.selected_size(), 0u);        // reader receives data again
}

/*!
 * Same GrainedFlowController wedge (Refs #24363), but with several matched readers and only one over budget.
 * This guards against fixes that only react when selected_size() drops to 0
 */
TEST(LocatorSelectorTests, reselects_single_over_budget_reader_among_many)
{
    constexpr size_t num_readers = 5;
    constexpr size_t over_budget_idx = 2;

    std::deque<LocatorSelectorEntry> entries;       // deque keeps element addresses stable
    LocatorSelector selector(ResourceLimitedContainerConfig::fixed_size_configuration(num_readers));
    for (size_t i = 0; i < num_readers; ++i)
    {
        entries.push_back(make_reader_entry(static_cast<uint32_t>(i + 1), static_cast<uint16_t>(7400 + i)));
        selector.add_entry(&entries.back());
    }

    auto enable_all = [&]()
            {
                for (LocatorSelectorEntry& e : entries)
                {
                    selector.enable(e.remote_guid);
                }
            };

    // ---------- Pass 1: all readers selected and sent ----------
    selector.reset(false);
    enable_all();
    ASSERT_TRUE(selector.state_has_changed());
    simulate_select_locators(selector);
    ASSERT_EQ(selector.selected_size(), num_readers);
    selector.initial_allow_to_send(true);

    // ---------- Pass 2: only reader 'over_budget_idx' exceeds its budget ----------
    selector.reset(false);
    enable_all();
    entries[over_budget_idx].allowed_to_send = false;
    selector.unselect(over_budget_idx);
    ASSERT_EQ(selector.selected_size(), num_readers - 1);   // the other readers stay selected

    // ---------- Period reset re-arms the over-budget reader ----------
    entries[over_budget_idx].allowed_to_send = true;

    // ---------- Pass 3: recovery; the dropped reader MUST be reselected ----------
    selector.reset(false);
    enable_all();

    // Regression point, state_has_changed() should be true to trigger select_locators()
    EXPECT_TRUE(selector.state_has_changed());

    simulate_select_locators(selector);
    EXPECT_EQ(selector.selected_size(), num_readers);       // all readers receiving again
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
