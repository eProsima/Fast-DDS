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
#include <fastdds/dds/log/Log.hpp>
#include <fastcdr/exceptions/BadParamException.h>
#include "idl/new_features_4_2PubSubTypes.h"
#include "idl/new_features_4_2TypeObject.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;
using namespace eprosima::fastcdr::exception;

class DynamicTypes_4_2_Tests: public ::testing::Test
{
    const std::string config_file_ = "types.xml";

    public:
        DynamicTypes_4_2_Tests()
        {
        }

        ~DynamicTypes_4_2_Tests()
        {
            eprosima::fastdds::dds::Log::KillThread();
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
    ASSERT_TRUE(struct_test.uint64_() == 555);
    ASSERT_TRUE(struct_test.int64_() == 0);
}

TEST_F(DynamicTypes_4_2_Tests, Bitmask)
{
    using namespace bitmodule;
    MyBitMask bitmask(static_cast<MyBitMask>(0));

    ASSERT_TRUE(bitmask == static_cast<MyBitMask>(0));
    bitmask = MyBitMask::flag0;
    ASSERT_TRUE(bitmask == static_cast<MyBitMask>(1));
    bitmask = MyBitMask::flag1;
    ASSERT_TRUE(bitmask == static_cast<MyBitMask>(2));
    bitmask = MyBitMask::flag4;
    ASSERT_TRUE(bitmask == static_cast<MyBitMask>(16));
    bitmask = MyBitMask::flag6;
    ASSERT_TRUE(bitmask == static_cast<MyBitMask>(64));
    bitmask = MyBitMask::flag7;
    ASSERT_TRUE(bitmask == static_cast<MyBitMask>(128));

    bitmask = static_cast<MyBitMask>(MyBitMask::flag0 | MyBitMask::flag4);
    ASSERT_TRUE(bitmask == static_cast<MyBitMask>(17));

    bitmask = static_cast<MyBitMask>(9);
    ASSERT_TRUE(bitmask & MyBitMask::flag0);
    ASSERT_FALSE(bitmask & MyBitMask::flag1);
    ASSERT_FALSE(bitmask & MyBitMask::flag4);
}

TEST_F(DynamicTypes_4_2_Tests, Bitset)
{
    using namespace bitmodule;
    MyBitset mybitset;

    ASSERT_TRUE(mybitset.a() == 0);
    ASSERT_TRUE(mybitset.b() == 0);
    ASSERT_TRUE(mybitset.c() == 0);
    ASSERT_TRUE(mybitset.d() == 0);
    ASSERT_TRUE(mybitset.e() == 0);
    ASSERT_TRUE(mybitset.f() == 0);
    ASSERT_TRUE(mybitset.parent_bitfield() == 0);

    mybitset.a(static_cast<char>(15));    // 00001111
    ASSERT_TRUE(mybitset.a() == 7);       // 00000111
    ASSERT_TRUE(mybitset.b() == 0);       // b unaffected
    ASSERT_FALSE(mybitset.b());
    mybitset.b(true);
    ASSERT_TRUE(mybitset.b());
    mybitset.c(static_cast<uint16_t>(-1));// 1111111111111111
    ASSERT_TRUE(mybitset.c() == 1023);    // 0000001111111111 (2^10 - 1)
    mybitset.d(-1);                       // 1111111111111111
    mybitset.e(5555);                     // 0001010110110011 (The most significant bit is not stored)
    mybitset.f(3851);
    ASSERT_TRUE(mybitset.d() == 4095);    // 0000111111111111 (2^12 - 1)
    ASSERT_TRUE(mybitset.e() == 1459);    // 0000010110110011 (The most significant bit is not stored)
    ASSERT_TRUE(mybitset.f() == 3851);
    mybitset.parent_bitfield(static_cast<uint32_t>(-1));// 11111111111111111111111111111111
    ASSERT_TRUE(mybitset.parent_bitfield() == 131071);  // 00000000000000111111111111111111 (2^17 - 1)
}

TEST_F(DynamicTypes_4_2_Tests, Non_Serialized_Annotation)
{
    NewAliases struct_test;
    struct_test.int8_(-8);
    struct_test.uint8_(8);
    struct_test.int16_(-16);
    struct_test.uint16_(16);
    struct_test.int32_(-32);
    struct_test.uint32_(32);
    struct_test.int64_(-64);
    struct_test.uint64_(64);
    struct_test.local_string("DON'T_SERIALIZE");

    NewAliasesPubSubType pst;
    NewAliases destination;
    uint32_t payloadSize = static_cast<uint32_t>(pst.getSerializedSizeProvider(&struct_test)());
    SerializedPayload_t payload(payloadSize);

    pst.serialize(&struct_test, &payload);
    pst.deserialize(&payload, &destination);

    ASSERT_TRUE(destination.int8_() == -8);
    ASSERT_TRUE(destination.uint8_() == 8);
    ASSERT_TRUE(destination.int16_() == -16);
    ASSERT_TRUE(destination.uint16_() == 16);
    ASSERT_TRUE(destination.int32_() == -32);
    ASSERT_TRUE(destination.uint32_() == 32);
    ASSERT_TRUE(destination.int64_() == -64);
    ASSERT_TRUE(destination.uint64_() == 64);
    ASSERT_FALSE(destination.local_string() == "DON'T_SERIALIZE"); // Is non_serialized annotated
}

TEST_F(DynamicTypes_4_2_Tests, New_Union_Discriminators)
{
    StructTest struct_test;

    ASSERT_TRUE(sizeof(struct_test.int8Union()._d()) == 1);
    ASSERT_TRUE(sizeof(struct_test.octetUnion()._d()) == 1);
    ASSERT_TRUE(sizeof(struct_test.charUnion()._d()) == sizeof(wchar_t));

    // int8Union
    struct_test.int8Union().case_three(333);
    struct_test.int8Union().case_six(666);

    struct_test.int8Union()._d() = 3;
    ASSERT_TRUE(struct_test.int8Union().case_three() == 333);
    try
    {
        ASSERT_FALSE(struct_test.int8Union().case_six() == 666);
        ASSERT_TRUE(false);
    }
    catch(const BadParamException&)
    {
    }

    struct_test.int8Union()._d() = 6;
    try
    {
        ASSERT_FALSE(struct_test.int8Union().case_three() == 333);
        ASSERT_TRUE(false);
    }
    catch(const BadParamException&)
    {
    }
    ASSERT_TRUE(struct_test.int8Union().case_six() == 666);

    // octetUnion
    struct_test.octetUnion().case_five(555);
    struct_test.octetUnion().case_seven(777);

    struct_test.octetUnion()._d() = 5;
    ASSERT_TRUE(struct_test.octetUnion().case_five() == 555);
    try
    {
        ASSERT_FALSE(struct_test.octetUnion().case_seven() == 777);
        ASSERT_TRUE(false);
    }
    catch(const BadParamException&)
    {
    }

    struct_test.octetUnion()._d() = 7;
    try
    {
        ASSERT_FALSE(struct_test.octetUnion().case_five() == 555);
        ASSERT_TRUE(false);
    }
    catch(const BadParamException&)
    {
    }
    ASSERT_TRUE(struct_test.octetUnion().case_seven() == 777);

    // charUnion
    struct_test.charUnion().case_zero(111);
    struct_test.charUnion().case_one(222);

    struct_test.charUnion()._d() = L'a';
    ASSERT_TRUE(struct_test.charUnion().case_zero() == 111);
    try
    {
        ASSERT_FALSE(struct_test.charUnion().case_one() == 222);
        ASSERT_TRUE(false);
    }
    catch(const BadParamException&)
    {
    }

    struct_test.charUnion()._d() = L'b';
    try
    {
        ASSERT_FALSE(struct_test.charUnion().case_zero() == 111);
        ASSERT_TRUE(false);
    }
    catch(const BadParamException&)
    {
    }
    ASSERT_TRUE(struct_test.charUnion().case_one() == 222);
}

TEST_F(DynamicTypes_4_2_Tests, TypeObject_DynamicType_Conversion)
{
    registernew_features_4_2Types();

    // TODO Bitset serialization isn't compatible.
    const TypeIdentifier* identifier = GetStructTestIdentifier(true);
    const TypeObject* object = GetCompleteStructTestObject();

    DynamicType_ptr dyn_type =
        TypeObjectFactory::get_instance()->build_dynamic_type("StructTest", identifier, object);

    TypeIdentifier conv_identifier;
    TypeObject conv_object;
    DynamicTypeBuilderFactory::get_instance()->build_type_object(dyn_type, conv_object, true, true); // Avoid factory
    DynamicTypeBuilderFactory::get_instance()->build_type_identifier(dyn_type, conv_identifier, true);

    ASSERT_TRUE(*identifier == conv_identifier);
    ASSERT_TRUE(*object == conv_object);

    // Serialize static <-> dynamic

    StructTest struct_test;
    DynamicData_ptr dyn_data(DynamicDataFactory::get_instance()->create_data(dyn_type));

    DynamicPubSubType pst_dynamic(dyn_type);
    uint32_t payload_dyn_size = static_cast<uint32_t>(pst_dynamic.getSerializedSizeProvider(dyn_data.get())());
    SerializedPayload_t payload(payload_dyn_size);
    ASSERT_TRUE(pst_dynamic.serialize(dyn_data.get(), &payload));
    ASSERT_TRUE(payload.length == payload_dyn_size);

    StructTestPubSubType pst_static;
    uint32_t payload_size = static_cast<uint32_t>(pst_static.getSerializedSizeProvider(&struct_test)());
    SerializedPayload_t st_payload(payload_size);
    ASSERT_TRUE(pst_static.serialize(&struct_test, &st_payload));
    ASSERT_TRUE(st_payload.length == payload_size);

    DynamicData_ptr dyn_data_from_dynamic(DynamicDataFactory::get_instance()->create_data(dyn_type));
    ASSERT_TRUE(pst_dynamic.deserialize(&payload, dyn_data_from_dynamic.get()));

    types::DynamicData_ptr dyn_data_from_static(DynamicDataFactory::get_instance()->create_data(dyn_type));
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

TEST_F(DynamicTypes_4_2_Tests, Static_Dynamic_Values)
{
    registernew_features_4_2Types();

    const TypeIdentifier* identifier = GetStructTestIdentifier(true);
    const TypeObject* object = GetCompleteStructTestObject();

    DynamicType_ptr dyn_type =
        TypeObjectFactory::get_instance()->build_dynamic_type("StructTest", identifier, object);

    // Serialize static initialization with values
    StructTest struct_test;
    StructTestPubSubType pst_static;

    struct_test.int8_(-8);
    struct_test.uint8_(8);
    struct_test.int16_(-16);
    struct_test.uint16_(16);
    struct_test.int32_(-32);
    struct_test.uint32_(32);
    struct_test.int64_(-64);
    struct_test.uint64_(64);
    struct_test.local_string("DON'T_SERIALIZE");
    struct_test.charUnion().case_one(11111);
    struct_test.octetUnion().case_seven(77777);
    struct_test.int8Union().case_three(33333);
    struct_test.myStructBits().mybitset().parent_bitfield(2121);
    struct_test.myStructBits().mybitset().a(5);
    struct_test.myStructBits().mybitset().b(true);
    struct_test.myStructBits().mybitset().c(333);
    struct_test.myStructBits().mybitset().d(4000);
    struct_test.myStructBits().mybitset().e(4001);
    struct_test.myStructBits().mybitset().f(3001);
    struct_test.myStructBits().mybitmask(
        static_cast<bitmodule::MyBitMask>(bitmodule::MyBitMask::flag0 | bitmodule::MyBitMask::flag4));
    struct_test.myStructBits().two(
        static_cast<bitmodule::MyBitMask>(bitmodule::MyBitMask::flag1 | bitmodule::MyBitMask::flag6));
    struct_test.myStructBits().mylong(static_cast<uint32_t>(struct_test.myStructBits().two()));

    // Static serialization
    uint32_t payload_size = static_cast<uint32_t>(pst_static.getSerializedSizeProvider(&struct_test)());
    SerializedPayload_t st_payload(payload_size);
    ASSERT_TRUE(pst_static.serialize(&struct_test, &st_payload));
    ASSERT_TRUE(st_payload.length == payload_size);

    // Dynamic deserialization from static
    DynamicPubSubType pst_dynamic(dyn_type);
    DynamicData_ptr dyn_data(DynamicDataFactory::get_instance()->create_data(dyn_type));
    ASSERT_TRUE(pst_dynamic.deserialize(&st_payload, dyn_data.get()));

    // Dynamic serialization
    uint32_t payload_dyn_size = static_cast<uint32_t>(pst_dynamic.getSerializedSizeProvider(dyn_data.get())());
    SerializedPayload_t dyn_payload(payload_dyn_size);
    ASSERT_TRUE(pst_dynamic.serialize(dyn_data.get(), &dyn_payload));
    ASSERT_TRUE(dyn_payload.length == payload_dyn_size);

    // Static deserialization from dynamic
    StructTest struct_test_from_dynamic;
    ASSERT_TRUE(pst_static.deserialize(&dyn_payload, &struct_test_from_dynamic));

    // Check values
    ASSERT_TRUE(struct_test_from_dynamic.int8_() == struct_test.int8_());
    ASSERT_TRUE(struct_test_from_dynamic.uint8_() == struct_test.uint8_());
    ASSERT_TRUE(struct_test_from_dynamic.int16_() == struct_test.int16_());
    ASSERT_TRUE(struct_test_from_dynamic.uint16_() == struct_test.uint16_());
    ASSERT_TRUE(struct_test_from_dynamic.int32_() == struct_test.int32_());
    ASSERT_TRUE(struct_test_from_dynamic.uint32_() == struct_test.uint32_());
    ASSERT_TRUE(struct_test_from_dynamic.int64_() == struct_test.int64_());
    ASSERT_TRUE(struct_test_from_dynamic.uint64_() == struct_test.uint64_());
    ASSERT_FALSE(struct_test_from_dynamic.local_string() == struct_test.local_string()); // Non serialized
    ASSERT_TRUE(struct_test_from_dynamic.charUnion().case_one() == struct_test.charUnion().case_one());
    ASSERT_TRUE(struct_test_from_dynamic.octetUnion().case_seven() == struct_test.octetUnion().case_seven());
    ASSERT_TRUE(struct_test_from_dynamic.int8Union().case_three() == struct_test.int8Union().case_three());
    ASSERT_TRUE(struct_test_from_dynamic.myStructBits().mybitset().parent_bitfield() == struct_test.myStructBits().mybitset().parent_bitfield());
    ASSERT_TRUE(struct_test_from_dynamic.myStructBits().mybitset().a() == struct_test.myStructBits().mybitset().a());
    ASSERT_TRUE(struct_test_from_dynamic.myStructBits().mybitset().b() == struct_test.myStructBits().mybitset().b());
    ASSERT_TRUE(struct_test_from_dynamic.myStructBits().mybitset().c() == struct_test.myStructBits().mybitset().c());
    ASSERT_TRUE(struct_test_from_dynamic.myStructBits().mybitset().d() == struct_test.myStructBits().mybitset().d());
    ASSERT_TRUE(struct_test_from_dynamic.myStructBits().mybitset().e() == struct_test.myStructBits().mybitset().e());
    ASSERT_TRUE(struct_test_from_dynamic.myStructBits().mybitset().f() == struct_test.myStructBits().mybitset().f());
    ASSERT_TRUE(struct_test_from_dynamic.myStructBits().mybitmask() == struct_test.myStructBits().mybitmask());
    ASSERT_TRUE(struct_test_from_dynamic.myStructBits().two() == struct_test.myStructBits().two());
    ASSERT_TRUE(struct_test_from_dynamic.myStructBits().mylong() == struct_test.myStructBits().mylong());

    ASSERT_TRUE(dyn_data->get_int8_value(dyn_data->get_member_id_by_name("int8_")) == struct_test.int8_());
    ASSERT_TRUE(dyn_data->get_uint8_value(dyn_data->get_member_id_by_name("uint8_")) == struct_test.uint8_());
    ASSERT_TRUE(dyn_data->get_int16_value(dyn_data->get_member_id_by_name("int16_")) == struct_test.int16_());
    ASSERT_TRUE(dyn_data->get_uint16_value(dyn_data->get_member_id_by_name("uint16_")) == struct_test.uint16_());
    ASSERT_TRUE(dyn_data->get_int32_value(dyn_data->get_member_id_by_name("int32_")) == struct_test.int32_());
    ASSERT_TRUE(dyn_data->get_uint32_value(dyn_data->get_member_id_by_name("uint32_")) == struct_test.uint32_());
    ASSERT_TRUE(dyn_data->get_int64_value(dyn_data->get_member_id_by_name("int64_")) == struct_test.int64_());
    ASSERT_TRUE(dyn_data->get_uint64_value(dyn_data->get_member_id_by_name("uint64_")) == struct_test.uint64_());
    ASSERT_FALSE(dyn_data->get_string_value(dyn_data->get_member_id_by_name("local_string")) ==
        struct_test.local_string()); // Non serialized

    DynamicData* charUnion = dyn_data->loan_value(dyn_data->get_member_id_by_name("charUnion"));
    ASSERT_TRUE(charUnion->get_int32_value(charUnion->get_member_id_by_name("case_one")) ==
        struct_test.charUnion().case_one());
    dyn_data->return_loaned_value(charUnion);

    DynamicData* octetUnion = dyn_data->loan_value(dyn_data->get_member_id_by_name("octetUnion"));
    ASSERT_TRUE(octetUnion->get_int32_value(octetUnion->get_member_id_by_name("case_seven")) ==
        struct_test.octetUnion().case_seven());
    dyn_data->return_loaned_value(octetUnion);

    DynamicData* int8Union = dyn_data->loan_value(dyn_data->get_member_id_by_name("int8Union"));
    ASSERT_TRUE(int8Union->get_int32_value(int8Union->get_member_id_by_name("case_three")) ==
        struct_test.int8Union().case_three());
    dyn_data->return_loaned_value(int8Union);

    DynamicData* myStructBits = dyn_data->loan_value(dyn_data->get_member_id_by_name("myStructBits"));
    DynamicData* mybitset = myStructBits->loan_value(myStructBits->get_member_id_by_name("mybitset"));

    ASSERT_TRUE(mybitset->get_uint32_value(mybitset->get_member_id_by_name("parent_bitfield"))
        == struct_test.myStructBits().mybitset().parent_bitfield());
    ASSERT_TRUE(mybitset->get_char8_value(mybitset->get_member_id_by_name("a"))
        == struct_test.myStructBits().mybitset().a());
    ASSERT_TRUE(mybitset->get_bool_value(mybitset->get_member_id_by_name("b"))
        == struct_test.myStructBits().mybitset().b());
    ASSERT_TRUE(mybitset->get_uint16_value(mybitset->get_member_id_by_name("c"))
        == struct_test.myStructBits().mybitset().c());
    ASSERT_TRUE(mybitset->get_int16_value(mybitset->get_member_id_by_name("d"))
        == struct_test.myStructBits().mybitset().d());
    ASSERT_TRUE(mybitset->get_int16_value(mybitset->get_member_id_by_name("e"))
        == struct_test.myStructBits().mybitset().e());
    ASSERT_TRUE(mybitset->get_int16_value(mybitset->get_member_id_by_name("f"))
        == struct_test.myStructBits().mybitset().f());

    myStructBits->return_loaned_value(mybitset);
    ASSERT_TRUE(myStructBits->get_uint64_value(myStructBits->get_member_id_by_name("mybitmask")) ==
        struct_test.myStructBits().mybitmask());
    ASSERT_TRUE(myStructBits->get_uint64_value(myStructBits->get_member_id_by_name("two")) ==
        struct_test.myStructBits().two());
    ASSERT_TRUE(myStructBits->get_int32_value(myStructBits->get_member_id_by_name("mylong")) ==
        struct_test.myStructBits().mylong());
    dyn_data->return_loaned_value(myStructBits);
}

int main(int argc, char **argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
