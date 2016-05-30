
#include <fastrtps/rtps/filters/SizeFilter.h>
#include <gtest/gtest.h>

using namespace std;
using namespace eprosima::fastrtps::rtps;

static const unsigned int testPayloadSize = 1000;
static const unsigned int filterSize = 5500;
static const unsigned int refreshTimeMS = 100;
static const unsigned int numberOfTestChanges = 10;

static const SizeFilterDescriptor testDescriptor = {filterSize, refreshTimeMS};

class SizeFilterTests: public ::testing::Test 
{
   public:

   SizeFilterTests():
      sFilter(testDescriptor, (const RTPSWriter*)nullptr)
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

   SizeFilter sFilter;
   std::vector<std::unique_ptr<CacheChange_t>> testChanges;
   std::vector<std::unique_ptr<CacheChange_t>> otherChanges;
   std::vector<CacheChangeForGroup_t> testChangesForGroup;
   std::vector<CacheChangeForGroup_t> otherChangesForGroup;
};

TEST_F(SizeFilterTests, size_filter_lets_only_some_elements_through)
{
   // When
   sFilter(testChangesForGroup);

   // Then
   ASSERT_EQ(filterSize/testPayloadSize, testChangesForGroup.size());

   std::this_thread::sleep_for(std::chrono::milliseconds(refreshTimeMS + 20));
}

TEST_F(SizeFilterTests, if_changes_are_fragmented_size_filter_provides_granularity)
{
   // Given fragmented changes
   testChangesForGroup.clear();
   for (auto& change : testChanges)
   {
      change->setFragmentSize(100);
      testChangesForGroup.emplace_back(change.get());
   }

   // When
   sFilter(testChangesForGroup);

   // Then
   ASSERT_EQ(6, testChangesForGroup.size());

   // The first 5 are completely cleared
   for (int i = 0; i < 5; i++)
   {
      ASSERT_EQ(testChangesForGroup[i].getFragmentsClearedForSending().set.size(), 10);
   }

   // And the last one is partially cleared
   ASSERT_EQ(testChangesForGroup[5].getFragmentsClearedForSending().set.size(), 5); 
   std::this_thread::sleep_for(std::chrono::milliseconds(refreshTimeMS + 20));
}

TEST_F(SizeFilterTests, size_filter_carries_over_multiple_attempts)
{
   // Given
   sFilter(testChangesForGroup);

   // when
   sFilter(otherChangesForGroup);

   // Then
   ASSERT_EQ(0, otherChangesForGroup.size());
   std::this_thread::sleep_for(std::chrono::milliseconds(refreshTimeMS + 20));
}

TEST_F(SizeFilterTests, size_filter_resets_completely_after_its_refresh_period)
{
   // Given
   sFilter(testChangesForGroup);
   ASSERT_EQ(5, testChangesForGroup.size());

   // The filter is now fully closed, so filtering anything will throw all changes away.
   sFilter(testChangesForGroup);
   ASSERT_EQ(0, testChangesForGroup.size());

   // When
   std::this_thread::sleep_for(std::chrono::milliseconds(refreshTimeMS + 20));
   
   // The filter should be open now
   sFilter(otherChangesForGroup);
   EXPECT_EQ(5, otherChangesForGroup.size());
   std::this_thread::sleep_for(std::chrono::milliseconds(refreshTimeMS + 20));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
