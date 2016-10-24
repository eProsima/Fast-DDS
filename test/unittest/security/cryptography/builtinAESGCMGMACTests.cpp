


#include <iostream>
#include "CryptographyPluginTests.hpp"

TEST_F(CryptographyPluginTest, mocktest){
    uint8_t mock = 7;

    ASSERT_TRUE(mock == 7);

}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

