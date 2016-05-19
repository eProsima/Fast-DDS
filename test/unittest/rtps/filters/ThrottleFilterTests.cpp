#include <fastrtps/rtps/filters/ThrottleFilter.h>
#include <fastrtps/rtps/messages/RTPSMessageGroup.h>
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
         testChangeReferences.push_back(&testChanges[i]);
         otherChangeReferences.push_back(&otherChanges[i]);
      }
   }

   ThrottleFilter throttle;
   CacheChange_t testChanges[numberOfTestChanges];
   std::vector<const CacheChange_t*> testChangeReferences;

   CacheChange_t otherChanges[numberOfTestChanges];
   std::vector<const CacheChange_t*> otherChangeReferences;

   void HELPER_Send_change(const CacheChange_t* change)
   {
      std::vector<CacheChangeForGroup_t> changesToSend;
      changesToSend.emplace_back(change);
      LocatorList_t ll;
      RTPSMessageGroup::send_Changes_AsData(0, 0, changesToSend, GuidPrefix_t(), EntityId_t(), 
                                            ll, ll, false);
   
   }
};

TEST_F(ThrottleFilterTests, throttle_filter_lets_changes_through_as_long_as_nothing_is_sent)
{
   // When
   auto filteredChanges = throttle(testChangeReferences);

   // Then
   ASSERT_EQ(numberOfTestChanges, filteredChanges.size());

   // When
   filteredChanges = throttle(filteredChanges);

   // Then
   ASSERT_EQ(numberOfTestChanges, filteredChanges.size());
}

TEST_F(ThrottleFilterTests, throttle_period_kicks_in_when_sending_a_change_we_just_cleared)
{
   // Given we cleared the thest change vector
   auto filteredChanges = throttle(testChangeReferences);
   ASSERT_EQ(numberOfTestChanges, filteredChanges.size());

   // When when send one of those changes succesfully
   HELPER_Send_change(&testChanges[0]);

   // Then we start throttling (filtering everything out)
   filteredChanges = throttle(testChangeReferences);
   ASSERT_EQ(0, filteredChanges.size());
}

TEST_F(ThrottleFilterTests, no_throttling_if_we_sent_a_change_the_filter_has_not_touched)
{
   // Given we cleared the "other" change vector
   auto filteredChanges = throttle(otherChangeReferences);
   ASSERT_EQ(numberOfTestChanges, filteredChanges.size());

   // When when send a test change succesfully
   HELPER_Send_change(&testChanges[0]);

   // Then we don't filter anything out (we haven't cleared that particular change)
   filteredChanges = throttle(testChangeReferences);
   ASSERT_EQ(10, filteredChanges.size());
}

TEST_F(ThrottleFilterTests, throttling_lasts_for_the_time_specified_in_construction)
{
   // Given we got to the point of throttling
   auto filteredChanges = throttle(testChangeReferences);
   ASSERT_EQ(numberOfTestChanges, filteredChanges.size());
   HELPER_Send_change(&testChanges[0]);

   // when we wait less than the specified time, throttling is still active
   std::this_thread::sleep_for(std::chrono::milliseconds(testThrottlePeriodInMs - 20));
   filteredChanges = throttle(testChangeReferences);
   ASSERT_EQ(0, filteredChanges.size());

   // Then after the specified time, throttling is over
   std::this_thread::sleep_for(std::chrono::milliseconds(40));
   filteredChanges = throttle(testChangeReferences);
   ASSERT_EQ(10, filteredChanges.size());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
