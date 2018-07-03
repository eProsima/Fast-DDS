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

#include <fastrtps/types/TypesBase.h>
#include <gtest/gtest.h>
#include <fastrtps/log/Log.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::types;

class DynamicTypesTests: public ::testing::Test
{
    public:
        DynamicTypesTests()
        {
            HELPER_SetDescriptorDefaults();
        }

        ~DynamicTypesTests()
        {
            Log::KillThread();
        }

        void HELPER_SetDescriptorDefaults();
};

TEST_F(DynamicTypesTests, locators_with_kind_1_supported)
{
    //// Given
    //TCPv4Transport transportUnderTest(descriptor);
    //transportUnderTest.init();

    //Locator_t supportedLocator;
    //supportedLocator.kind = LOCATOR_KIND_TCPv4;
    //Locator_t unsupportedLocatorv4;
    //unsupportedLocatorv4.kind = LOCATOR_KIND_UDPv4;
    //Locator_t unsupportedLocatorv6;
    //unsupportedLocatorv6.kind = LOCATOR_KIND_UDPv6;

    //// Then
    //ASSERT_TRUE(transportUnderTest.IsLocatorSupported(supportedLocator));
    //ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocatorv4));
    //ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocatorv6));
}

void DynamicTypesTests::HELPER_SetDescriptorDefaults()
{
}

//TODO: //ARCE:

/*
PENDING TESTS

TypeBuilderFactory: -> create a type of each basic kind, create a builder of each type.
Create structs of structs
Create combined types using members.
DynamicType-> Create, clone, compare

*/
int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
