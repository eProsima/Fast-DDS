#include <fastrtps/rtps/filters/QuantityFilter.h>
#include <gtest/gtest.h>

using namespace std;
using namespace eprosima::fastrtps::rtps;

static const unsigned int testQuantity = 3;
static const unsigned int numberOfTestChanges = 10;

class QuantityFilterTests: public ::testing::Test 
{
   public:

   QuantityFilterTests():
      qFilter(testQuantity)
   {
      for (unsigned int i = 0; i < numberOfTestChanges; i++)
         testChangesForGroup.emplace_back(&testChanges[i]);
   }

   QuantityFilter qFilter;
   CacheChange_t testChanges[numberOfTestChanges];
   std::vector<CacheChangeForGroup_t> testChangesForGroup;
};

TEST_F(QuantityFilterTests, quantity_filter_lets_only_some_elements_through)
{
   // When
   qFilter(testChangesForGroup);

   // Then
   ASSERT_EQ(testQuantity, testChangesForGroup.size());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
