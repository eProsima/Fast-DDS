#include <fastrtps/rtps/filters/QuantityFilter.h>
#include <gtest/gtest.h>

using namespace std;
using namespace eprosima::fastrtps::rtps;

static const testQuantity = 3;

class QuantityFilterTests: public ::testing::Test 
{
   public:

   QuantityFilterTests():
      qFilter(testQuantity)
   {
      for (unsigned int i = 0; i < numberOfTestChanges; i++)
         testChangeReferences.push_back(&testChanges[i]);
   }

   QuantityFilter qFilter;
   CacheChange_t testChanges[numberOfTestChanges];
   std::vector<const CacheChange_t*> testChangeReferences;
};

TEST_F(QuantityFilterTests, quantity_filter_lets_only_some_elements_through)
{
   // When
   auto filteredChanges = qFilter(testChangeReferences);

   // Then
   ASSERT_EQ(testQuantity, filteredChanges.size());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
