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
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/log/Log.h>
#include "idl/new_features_4_2PubSubTypes.h"
#include "idl/new_features_4_2TypeObject.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::types;

class DynamicTypes_4_2_Tests: public ::testing::Test
{
    const std::string config_file_ = "types.xml";

    public:
        DynamicTypes_4_2_Tests()
        {
        }

        ~DynamicTypes_4_2_Tests()
        {
            Log::KillThread();
        }

        virtual void TearDown()
        {
            DynamicDataFactory::delete_instance();
            DynamicTypeBuilderFactory::delete_instance();
        }

        const std::string& config_file()
        {
            return config_file_;
        }
};

TEST_F(DynamicTypes_4_2_Tests, Inheritance_And_Default_Value)
{
    StructTest struct_test;
    ASSERT_TRUE(struct_test._uint64() == 555);
    ASSERT_TRUE(struct_test._int64() == 0);
}

TEST_F(DynamicTypes_4_2_Tests, Non_Serialized_Annotation)
{
    NewAliases struct_test;
    struct_test._int8(-8);
    struct_test._uint8(8);
    struct_test._int16(-16);
    struct_test._uint16(16);
    struct_test._int32(-32);
    struct_test._uint32(32);
    struct_test._int64(-64);
    struct_test._uint64(64);
    struct_test.local_string("DON'T_SERIALIZE");

    NewAliasesPubSubType pst;
    NewAliases destination;
    uint32_t payloadSize = static_cast<uint32_t>(pst.getSerializedSizeProvider(&struct_test)());
    SerializedPayload_t payload(payloadSize);

    pst.serialize(&struct_test, &payload);
    pst.deserialize(&payload, &destination);

    ASSERT_TRUE(destination._int8() == -8);
    ASSERT_TRUE(destination._uint8() == 8);
    ASSERT_TRUE(destination._int16() == -16);
    ASSERT_TRUE(destination._uint16() == 16);
    ASSERT_TRUE(destination._int32() == -32);
    ASSERT_TRUE(destination._uint32() == 32);
    ASSERT_TRUE(destination._int64() == -64);
    ASSERT_TRUE(destination._uint64() == 64);
    ASSERT_FALSE(destination.local_string() == "DON'T_SERIALIZE"); // Is non_serialized annotated
}

TEST_F(DynamicTypes_4_2_Tests, New_Union_Discriminators)
{
    StructTest struct_test;

    ASSERT_TRUE(sizeof(struct_test.int8Union()._d()) == 1);
    ASSERT_TRUE(sizeof(struct_test.octetUnion()._d()) == 1);
    ASSERT_TRUE(sizeof(struct_test.charUnion()._d()) == sizeof(wchar_t));
}

TEST_F(DynamicTypes_4_2_Tests, TypeObject_DynamicType_Conversion)
{
    registernew_features_4_2Types();

    const TypeIdentifier* identifier = GetStructTestIdentifier(true);
    const TypeObject* object = GetCompleteStructTestObject();

    DynamicType_ptr dyn_type = TypeObjectFactory::get_instance()->build_dynamic_type("StructTest", identifier, object);

    TypeIdentifier conv_identifier;
    TypeObject conv_object;
    DynamicTypeBuilderFactory::get_instance()->build_type_identifier(dyn_type, conv_identifier, true);
    DynamicTypeBuilderFactory::get_instance()->build_type_object(dyn_type, conv_object, true);

    //ASSERT_TRUE(memcmp(object, &conv_object, sizeof(TypeObject)) == 0);
    ASSERT_TRUE(identifier->_d() == conv_identifier._d());
    ASSERT_TRUE(memcmp(identifier->equivalence_hash(), conv_identifier.equivalence_hash(), 14) == 0);

    //ASSERT_TRUE(memcmp(identifier, &conv_identifier, sizeof(TypeIdentifier)) == 0);

    //StructTest struct_test;

}

int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
