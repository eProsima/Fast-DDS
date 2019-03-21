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

        virtual void TearDown() override
        {
            TypeObjectFactory::delete_instance();
            DynamicDataFactory::delete_instance();
            DynamicTypeBuilderFactory::delete_instance();
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

    // TODO Bitset serialization isn't compatible.
    const TypeIdentifier* identifier = GetNoBitsetStructTestIdentifier(true);
    const TypeObject* object = GetCompleteNoBitsetStructTestObject();

    DynamicType_ptr dyn_type =
        TypeObjectFactory::get_instance()->build_dynamic_type("NoBitsetStructTest", identifier, object);

    TypeIdentifier conv_identifier;
    TypeObject conv_object;
    DynamicTypeBuilderFactory::get_instance()->build_type_object(dyn_type, conv_object, true, true); // Avoid factory
    DynamicTypeBuilderFactory::get_instance()->build_type_identifier(dyn_type, conv_identifier, true);

    ASSERT_TRUE(*identifier == conv_identifier);
    ASSERT_TRUE(*object == conv_object);

    // Serialize static <-> dynamic

    NoBitsetStructTest struct_test;
    DynamicData_ptr dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);

    DynamicPubSubType pst_dynamic(dyn_type);
    uint32_t payload_dyn_size = static_cast<uint32_t>(pst_dynamic.getSerializedSizeProvider(dyn_data.get())());
    SerializedPayload_t payload(payload_dyn_size);
    ASSERT_TRUE(pst_dynamic.serialize(dyn_data.get(), &payload));
    ASSERT_TRUE(payload.length == payload_dyn_size);

    NoBitsetStructTestPubSubType pst_static;
    uint32_t payload_size = static_cast<uint32_t>(pst_static.getSerializedSizeProvider(&struct_test)());
    SerializedPayload_t st_payload(payload_size);
    ASSERT_TRUE(pst_static.serialize(&struct_test, &st_payload));
    ASSERT_TRUE(st_payload.length == payload_size);

    DynamicData_ptr dyn_data_from_dynamic = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_TRUE(pst_dynamic.deserialize(&payload, dyn_data_from_dynamic.get()));

    types::DynamicData_ptr dyn_data_from_static = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_TRUE(pst_dynamic.deserialize(&st_payload, dyn_data_from_static.get()));

    // DEBUG Printing payloads
    /*
    std::cout << "Payload: " << std::endl;
    for (int i = 0; i < payload_size; ++i)
    {
        std::cout << std::hex << (uint32_t)(payload.data[i]) << " ";
    }
    std::cout << std::endl;
    std::cout << "ST_Payload: " << std::endl;
    for (int i = 0; i < st_payload.length; ++i)
    {
        std::cout << std::hex << (uint32_t)(st_payload.data[i]) << " ";
    }
    std::cout << "--------------------------" << std::endl;
    */
    ASSERT_TRUE(dyn_data_from_static->equals(dyn_data_from_dynamic.get()));
}

int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
