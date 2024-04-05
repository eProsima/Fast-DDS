// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <gtest/gtest.h>

// Include mocks first
#include <fastdds/core/condition/ConditionNotifier.hpp>

// Include UUT
#include <fastdds/core/condition/StatusConditionImpl.hpp>

// Other includes
#include <fastdds/dds/core/status/StatusMask.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

TEST(StatusConditionImplTests, enabled_status_management)
{
    ConditionNotifier notifier;
    StatusConditionImpl uut(&notifier);

    StatusMask mask_none = StatusMask::none();
    StatusMask mask_all = StatusMask::all();
    StatusMask mask_single = StatusMask::inconsistent_topic();

    // According to the DDS standard, StatusCondition should start with all statuses enabled
    EXPECT_EQ(mask_all.to_string(), uut.get_enabled_statuses().to_string());
    EXPECT_EQ(RETCODE_OK, uut.set_enabled_statuses(mask_single));
    EXPECT_EQ(mask_single.to_string(), uut.get_enabled_statuses().to_string());
    EXPECT_EQ(RETCODE_OK, uut.set_enabled_statuses(mask_none));
    EXPECT_EQ(mask_none.to_string(), uut.get_enabled_statuses().to_string());
    EXPECT_EQ(RETCODE_OK, uut.set_enabled_statuses(mask_all));
    EXPECT_EQ(mask_all.to_string(), uut.get_enabled_statuses().to_string());
}

TEST(StatusConditionImplTests, notify_trigger)
{
    ::testing::StrictMock<ConditionNotifier> notifier;
    StatusConditionImpl uut(&notifier);

    StatusMask mask_none = StatusMask::none();
    StatusMask mask_all = StatusMask::all();
    StatusMask one_mask = StatusMask::inconsistent_topic();
    StatusMask other_mask = StatusMask::data_on_readers();
    StatusMask both_mask = one_mask;
    both_mask |= other_mask;

    // Condition should be untriggered upon creation
    EXPECT_FALSE(uut.get_trigger_value());
    EXPECT_EQ(mask_all.to_string(), uut.get_enabled_statuses().to_string());
    EXPECT_EQ(mask_none.to_string(), uut.get_raw_status().to_string());

    // Triggering other_mask should trigger
    auto& call1 = EXPECT_CALL(notifier, notify()).Times(1);
    uut.set_status(other_mask, true);
    EXPECT_EQ(other_mask.to_string(), uut.get_raw_status().to_string());
    EXPECT_TRUE(uut.get_trigger_value());

    // Setting mask to one_mask should untrigger
    EXPECT_EQ(RETCODE_OK, uut.set_enabled_statuses(one_mask));
    EXPECT_EQ(one_mask.to_string(), uut.get_enabled_statuses().to_string());
    EXPECT_FALSE(uut.get_trigger_value());

    // Triggering one_mask should trigger
    auto& call2 = EXPECT_CALL(notifier, notify()).Times(1).After(call1);
    uut.set_status(one_mask, true);
    EXPECT_EQ(both_mask.to_string(), uut.get_raw_status().to_string());
    EXPECT_TRUE(uut.get_trigger_value());

    // Triggering twice should not affect trigger
    uut.set_status(one_mask, true);
    EXPECT_EQ(both_mask.to_string(), uut.get_raw_status().to_string());
    EXPECT_TRUE(uut.get_trigger_value());

    // Untriggering other_mask should not affect trigger
    uut.set_status(other_mask, false);
    EXPECT_EQ(one_mask.to_string(), uut.get_raw_status().to_string());
    EXPECT_TRUE(uut.get_trigger_value());

    // Triggering other_mask should not affect trigger
    uut.set_status(other_mask, true);
    EXPECT_EQ(both_mask.to_string(), uut.get_raw_status().to_string());
    EXPECT_TRUE(uut.get_trigger_value());

    // Untriggering one_mask should untrigger
    uut.set_status(one_mask, false);
    EXPECT_EQ(other_mask.to_string(), uut.get_raw_status().to_string());
    EXPECT_FALSE(uut.get_trigger_value());

    // Untriggering other_mask should not trigger
    uut.set_status(other_mask, false);
    EXPECT_EQ(mask_none.to_string(), uut.get_raw_status().to_string());
    EXPECT_FALSE(uut.get_trigger_value());

    // Triggering other_mask should not trigger
    uut.set_status(other_mask, true);
    EXPECT_EQ(other_mask.to_string(), uut.get_raw_status().to_string());
    EXPECT_FALSE(uut.get_trigger_value());

    // Setting mask to other_mask should trigger
    auto& call3 = EXPECT_CALL(notifier, notify()).Times(1).After(call2);
    EXPECT_EQ(RETCODE_OK, uut.set_enabled_statuses(other_mask));
    EXPECT_EQ(other_mask.to_string(), uut.get_enabled_statuses().to_string());

    // Triggering one_mask should not affect trigger
    uut.set_status(one_mask, true);
    EXPECT_EQ(both_mask.to_string(), uut.get_raw_status().to_string());
    EXPECT_TRUE(uut.get_trigger_value());

    // Setting mask to one_mask should not affect trigger
    EXPECT_EQ(RETCODE_OK, uut.set_enabled_statuses(one_mask));
    EXPECT_EQ(one_mask.to_string(), uut.get_enabled_statuses().to_string());
    EXPECT_TRUE(uut.get_trigger_value());

    // Untriggering other_mask should not affect trigger
    uut.set_status(other_mask, false);
    EXPECT_EQ(one_mask.to_string(), uut.get_raw_status().to_string());
    EXPECT_TRUE(uut.get_trigger_value());

    // Setting mask to other_mask should untrigger
    EXPECT_EQ(RETCODE_OK, uut.set_enabled_statuses(other_mask));
    EXPECT_EQ(other_mask.to_string(), uut.get_enabled_statuses().to_string());
    EXPECT_FALSE(uut.get_trigger_value());

    // Setting mask to one_mask should trigger
    EXPECT_CALL(notifier, notify()).Times(1).After(call3);
    EXPECT_EQ(RETCODE_OK, uut.set_enabled_statuses(one_mask));
    EXPECT_EQ(one_mask.to_string(), uut.get_enabled_statuses().to_string());
    EXPECT_TRUE(uut.get_trigger_value());
}

} // namespace detail
} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
