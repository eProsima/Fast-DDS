#include <fastrtps/rtps/filters/ThrottleFilter.h>
#include <gtest/gtest.h>
#include <vector>
#include <chrono>

using namespace std;
using namespace eprosima::fastrtps::rtps;

static const unsigned int testThrottlePeriodInMs = 100;
static const unsigned int testPayloadSize = 256;
static const unsigned int numberOfTestChanges = 10;

class ThrottleFilterTests: public ::testing::Test 
{
   public:

   ThrottleFilterTests():
      throttle(testThrottlePeriodInMs)
   {
      for (unsigned int i = 0; i < numberOfTestChanges; i++)
      {
         testChangesForGroup.emplace_back(&testChanges[i]);
         otherChangesForGroup.emplace_back(&otherChanges[i]);
      }
   }

   ThrottleFilter throttle;
   CacheChange_t testChanges[numberOfTestChanges];
   std::vector<CacheChangeForGroup_t> testChangesForGroup;

   CacheChange_t otherChanges[numberOfTestChanges];
   std::vector<CacheChangeForGroup_t> otherChangesForGroup;
};

TEST_F(ThrottleFilterTests, throttle_filter_lets_changes_through_as_long_as_nothing_is_sent)
{
   // When
   throttle(testChangesForGroup);

   // Then
   ASSERT_EQ(numberOfTestChanges, testChangesForGroup.size());

   // When
   throttle(testChangesForGroup);

   // Then
   ASSERT_EQ(numberOfTestChanges, testChangesForGroup.size());
}

TEST_F(ThrottleFilterTests, throttle_period_kicks_in_when_sending_a_change_we_just_cleared)
{
   // Given we cleared the thest change vector
   throttle(testChangesForGroup);
   ASSERT_EQ(numberOfTestChanges, testChangesForGroup.size());

   // When we send one of those changes succesfully
   FlowFilter::NotifyFiltersChangeSent(&testChangesForGroup[0]);

   // Then we start throttling (filtering everything out)
   throttle(testChangesForGroup);
   ASSERT_EQ(0, testChangesForGroup.size());
}

TEST_F(ThrottleFilterTests, no_throttling_if_we_sent_a_change_the_filter_has_not_touched)
{
   // Given we cleared the "other" change vector
   throttle(otherChangesForGroup);
   ASSERT_EQ(numberOfTestChanges, otherChangesForGroup.size());

   // When we send a test change succesfully
   FlowFilter::NotifyFiltersChangeSent(&testChangesForGroup[0]);

   // Then we don't filter anything out (we haven't cleared that particular change)
   throttle(testChangesForGroup);
   ASSERT_EQ(numberOfTestChanges, testChangesForGroup.size());
}

TEST_F(ThrottleFilterTests, throttling_lasts_for_the_time_specified_in_construction)
{
   // Given we got to the point of throttling
   throttle(testChangesForGroup);
   ASSERT_EQ(numberOfTestChanges, testChangesForGroup.size());
   FlowFilter::NotifyFiltersChangeSent(&testChangesForGroup[0]);

   // when we wait less than the specified time, throttling is still active
   std::this_thread::sleep_for(std::chrono::milliseconds(testThrottlePeriodInMs - 20));
   throttle(testChangesForGroup);
   ASSERT_EQ(0, testChangesForGroup.size());

   // Then after the specified time, throttling is over
   std::this_thread::sleep_for(std::chrono::milliseconds(40));
   throttle(otherChangesForGroup);
   ASSERT_EQ(numberOfTestChanges, otherChangesForGroup.size());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
