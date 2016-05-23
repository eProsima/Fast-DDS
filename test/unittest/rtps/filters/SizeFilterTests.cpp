
#include <fastrtps/rtps/filters/SizeFilter.h>
#include <gtest/gtest.h>

using namespace std;
using namespace eprosima::fastrtps::rtps;

static const unsigned int testPayloadSize = 1000;
static const unsigned int filterSize = 5500;
static const unsigned int numberOfTestChanges = 10;

class SizeFilterTests: public ::testing::Test 
{
   public:

   SizeFilterTests():
      sFilter(filterSize)
   {
      for (unsigned int i = 0; i < numberOfTestChanges; i++)
      {
         testChanges.emplace_back(new CacheChange_t(testPayloadSize));
         testChanges.back()->serializedPayload.length = testPayloadSize;
         testChangesForGroup.emplace_back(testChanges.back().get());
      }
   }

   SizeFilter sFilter;
   std::vector<std::unique_ptr<CacheChange_t>> testChanges;
   std::vector<CacheChangeForGroup_t> testChangesForGroup;
};

TEST_F(SizeFilterTests, quantity_filter_lets_only_some_elements_through)
{
   // When
   sFilter(testChangesForGroup);

   // Then
   ASSERT_EQ(filterSize/testPayloadSize, testChangesForGroup.size());
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
      ASSERT_EQ(testChangesForGroup[i].getFragmentsClearedForSending(), 10);
   }

   // And the last one is partially cleared
   ASSERT_EQ(testChangesForGroup[6].getFragmentsClearedForSending(), 5); 
}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
