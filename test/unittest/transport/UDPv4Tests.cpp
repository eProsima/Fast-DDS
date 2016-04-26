#include <fastrtps/transport/UDPv4Transport.h>
#include <gtest/gtest.h>

using namespace std;
using namespace eprosima::fastrtps::rtps;

class UDPv4Tests: public ::testing::Test 
{
   public:
};

TEST_F(UDPv4Tests, trivial_test)
{
   ASSERT_TRUE(false);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
