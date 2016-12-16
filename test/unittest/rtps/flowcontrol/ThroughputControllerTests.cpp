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

#include <fastrtps/rtps/flowcontrol/ThroughputController.h>
#include <fastrtps/rtps/flowcontrol/ThroughputControllerDescriptor.h>
#include <gtest/gtest.h>

using namespace std;
using namespace eprosima::fastrtps::rtps;

static const unsigned int testPayloadSize = 1000;
static const unsigned int controllerSize = 5500;
static const unsigned int periodMillisecs = 100;
static const unsigned int numberOfTestChanges = 10;

static const ThroughputControllerDescriptor testDescriptor = {controllerSize, periodMillisecs};

class ThroughputControllerTests: public ::testing::Test
{
   public:

   ThroughputControllerTests():
      sController(testDescriptor, (const RTPSWriter*)nullptr)
   {
      for (unsigned int i = 0; i < numberOfTestChanges; i++)
      {
         testChanges.emplace_back(new CacheChange_t(testPayloadSize));
         testChanges.back()->serializedPayload.length = testPayloadSize;
         testChangesForGroup.emplace_back(testChanges.back().get());

         otherChanges.emplace_back(new CacheChange_t(testPayloadSize));
         otherChanges.back()->serializedPayload.length = testPayloadSize;
         otherChangesForGroup.emplace_back(otherChanges.back().get());
      }
   }

   ThroughputController sController;
   std::vector<std::unique_ptr<CacheChange_t>> testChanges;
   std::vector<std::unique_ptr<CacheChange_t>> otherChanges;
   std::vector<CacheChangeForGroup_t> testChangesForGroup;
   std::vector<CacheChangeForGroup_t> otherChangesForGroup;
};

TEST_F(ThroughputControllerTests, throughput_controller_lets_only_some_elements_through)
{
   // When
   sController(testChangesForGroup);

   // Then
   ASSERT_EQ(controllerSize/testPayloadSize, testChangesForGroup.size());

   std::this_thread::sleep_for(std::chrono::milliseconds(periodMillisecs + 50));
}

TEST_F(ThroughputControllerTests, if_changes_are_fragmented_throughput_controller_provides_granularity)
{
   // Given fragmented changes
   testChangesForGroup.clear();
   for (auto& change : testChanges)
   {
      change->setFragmentSize(100);
      testChangesForGroup.emplace_back(change.get());
   }

   // When
   sController(testChangesForGroup);

   // Then
   ASSERT_EQ(6, testChangesForGroup.size());

   // The first 5 are completely cleared
   for (int i = 0; i < 5; i++)
   {
      ASSERT_EQ(testChangesForGroup[i].getFragmentsClearedForSending().set.size(), 10);
   }

   // And the last one is partially cleared
   ASSERT_EQ(testChangesForGroup[5].getFragmentsClearedForSending().set.size(), 5);
   std::this_thread::sleep_for(std::chrono::milliseconds(periodMillisecs + 50));
}

TEST_F(ThroughputControllerTests, throughput_controller_carries_over_multiple_attempts)
{
   // Given
   sController(testChangesForGroup);

   // when
   sController(otherChangesForGroup);

   // Then
   ASSERT_EQ(0, otherChangesForGroup.size());
   std::this_thread::sleep_for(std::chrono::milliseconds(periodMillisecs + 50));
}

TEST_F(ThroughputControllerTests, throughput_controller_resets_completely_after_its_refresh_period)
{
   // Given
   sController(testChangesForGroup);
   ASSERT_EQ(5, testChangesForGroup.size());

   // The controller is now fully closed, so controllering anything will throw all changes away.
   sController(testChangesForGroup);
   ASSERT_EQ(0, testChangesForGroup.size());

   // When
   std::this_thread::sleep_for(std::chrono::milliseconds(periodMillisecs + 100));

   // The controller should be open now
   sController(otherChangesForGroup);
   EXPECT_EQ(5, otherChangesForGroup.size());
   std::this_thread::sleep_for(std::chrono::milliseconds(periodMillisecs + 50));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
