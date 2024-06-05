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

#include <iostream>
#include <algorithm>
#include "CryptographyPluginTests.hpp"
#include "security/OpenSSLInit.hpp"

TEST_F(CryptographyPluginTest, mocktest){
    static eprosima::fastdds::rtps::security::OpenSSLInit openssl_init;
    uint8_t mock = 7;

    ASSERT_TRUE(mock == 7);

}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
