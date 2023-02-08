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

#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>

#include "idl/Test.hpp"
#include "idl/TestPubSubTypes.h"
#include "idl/TestTypeObjectSupport.hpp"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastdds::dds::xtypes;

/*TODO(richiware)
   class DynamicComplexTypesTests : public ::testing::Test
   {
   public:

    DynamicComplexTypesTests()
        : m_factory{DynamicTypeBuilderFactory::get_instance()}
    {
        register_Test_type_objects();
        init();
    }

    ~DynamicComplexTypesTests()
    {
        m_DynAutoType = nullptr;
        //DynamicDataFactory::get_instance()->delete_data(m_DynAuto);

        m_DynManualType = nullptr;
        //DynamicDataFactory::get_instance()->delete_data(m_DynManual);

        if (!m_factory.is_empty())
        {
            EPROSIMA_LOG_ERROR(DYN_TEST, "DynamicTypeBuilderFactory is not empty.");
        }

        if (!DynamicDataFactory::get_instance().is_empty())
        {
            EPROSIMA_LOG_ERROR(DYN_TEST, "DynamicDataFactory is not empty.");
        }

        DynamicDataFactory::delete_instance();
        DynamicTypeBuilderFactory::delete_instance();

        eprosima::fastdds::dds::Log::KillThread();
    }

    virtual void TearDown()
    {
    }

    void init();

    std::unique_ptr<const DynamicType>& GetMyEnumType();
    std::unique_ptr<const DynamicType>& GetMyAliasEnumType();
    std::unique_ptr<const DynamicType>& GetMyAliasEnum2Type();
    std::unique_ptr<const DynamicType>& GetMyAliasEnum3Type();
    std::unique_ptr<const DynamicType>& GetMyOctetArray500Type();
    std::unique_ptr<const DynamicType>& GetBSAlias5Type();
    std::unique_ptr<const DynamicType>& GetMA3Type();
    std::unique_ptr<const DynamicType>& GetMyMiniArrayType();
    std::unique_ptr<const DynamicType>& GetMySequenceLongType();
    std::unique_ptr<const DynamicType>& GetBasicStructType();
    std::unique_ptr<const DynamicType>& GetComplexStructType();
    std::unique_ptr<const DynamicType>& GetUnionSwitchType();
    std::unique_ptr<const DynamicType>& GetUnion2SwitchType();
    std::unique_ptr<const DynamicType>& GetCompleteStructType();
    std::unique_ptr<const DynamicType>& GetKeyedStructType();

    // Static types
    //CompleteStruct m_Static;
    CompleteStructPubSubType m_StaticType;
    // Dynamic Types
    //DynamicData* m_DynAuto;
    std::unique_ptr<const DynamicType> m_DynAutoType;
    //DynamicData* m_DynManual;
    std::unique_ptr<const DynamicType> m_DynManualType;
    DynamicTypeBuilderFactory& m_factory;

   private:

    std::unique_ptr<const DynamicType> m_MyEnumType;
    std::unique_ptr<const DynamicType> m_MyAliasEnumType;
    std::unique_ptr<const DynamicType> m_MyAliasEnum2Type;
    std::unique_ptr<const DynamicType> m_MyAliasEnum3Type;
    std::unique_ptr<const DynamicType> m_MyOctetArray500;
    std::unique_ptr<const DynamicType> m_BSAlias5;
    std::unique_ptr<const DynamicType> m_MA3;
    std::unique_ptr<const DynamicType> m_MyMiniArray;
    std::unique_ptr<const DynamicType> m_MySequenceLong;
    std::unique_ptr<const DynamicType> m_BasicStructType;
    std::unique_ptr<const DynamicType> m_ComplexStructType;
    std::unique_ptr<const DynamicType> m_UnionSwitchType;
    std::unique_ptr<const DynamicType> m_Union2SwitchType;
    std::unique_ptr<const DynamicType> m_CompleteStructType;
    std::unique_ptr<const DynamicType> m_KeyedStructType;
   };
 */

/*

   struct KeyedStruct
   {
    @Key octet key;
    BasicStruct basic;
   };
 */
/*TODO(richiware)
   DynamicType_ptr DynamicComplexTypesTests::GetKeyedStructType()
   {
    if (m_KeyedStructType.get() == nullptr)
    {
        DynamicTypeBuilder_ptr keyedStruct_builder = m_factory.create_struct_type();
        DynamicTypeBuilder_ptr octet_builder = m_factory.create_type(
 * m_factory.create_byte_type());
        octet_builder->apply_annotation(ANNOTATION_KEY_ID, "value", "true");
        keyedStruct_builder->add_member(0_id, "key", octet_builder->build());
        keyedStruct_builder->add_member(1_id, "basic", GetBasicStructType());
        keyedStruct_builder->apply_annotation(ANNOTATION_KEY_ID, "value", "true");
        keyedStruct_builder->set_name("KeyedStruct");
        m_KeyedStructType = keyedStruct_builder->build();
    }

    return m_KeyedStructType;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetMyEnumType()
   {
    if (m_MyEnumType.get() == nullptr)
    {
        DynamicTypeBuilder_ptr myEnum_builder = m_factory.create_enum_type();
        myEnum_builder->set_name("MyEnum");
        myEnum_builder->add_member(0_id, "A");
        myEnum_builder->add_member(1_id, "B");
        myEnum_builder->add_member(2_id, "C");
        m_MyEnumType = myEnum_builder->build();
    }

    return m_MyEnumType;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetMyAliasEnumType()
   {
    if (m_MyAliasEnumType.get() == nullptr)
    {
        DynamicTypeBuilder_ptr myAliasEnum_builder = m_factory.create_alias_type(*GetMyEnumType(), "MyAliasEnum");
        m_MyAliasEnumType = myAliasEnum_builder->build();
    }

    return m_MyAliasEnumType;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetMyAliasEnum2Type()
   {
    if (m_MyAliasEnum2Type.get() == nullptr)
    {
        DynamicTypeBuilder_ptr myAliasEnum2_builder = m_factory.create_alias_type(
 * GetMyAliasEnumType(), "MyAliasEnum2");
        m_MyAliasEnum2Type = myAliasEnum2_builder->build();
    }

    return m_MyAliasEnum2Type;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetMyAliasEnum3Type()
   {
    if (m_MyAliasEnum3Type.get() == nullptr)
    {
        DynamicTypeBuilder_ptr myAliasEnum3_builder = m_factory.create_alias_type(
 * GetMyAliasEnum2Type(), "MyAliasEnum3");
        m_MyAliasEnum3Type = myAliasEnum3_builder->build();
    }

    return m_MyAliasEnum3Type;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetMyOctetArray500Type()
   {
    if (m_MyOctetArray500.get() == nullptr)
    {
        DynamicType_ptr octet_type = m_factory.get_byte_type();
        DynamicTypeBuilder_ptr myOctetArray500_builder = m_factory.create_array_type(*octet_type, { 500 });
        m_MyOctetArray500 = m_factory.get_alias_type(*myOctetArray500_builder->build(), "MyOctetArray500");
    }

    return m_MyOctetArray500;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetBSAlias5Type()
   {
    if (m_BSAlias5.get() == nullptr)
    {
        DynamicTypeBuilder_ptr bSAlias5_builder = m_factory.create_array_type(*GetBasicStructType(), { 5 });
        m_BSAlias5 = m_factory.get_alias_type(*bSAlias5_builder->build(), "BSAlias5");
    }

    return m_BSAlias5;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetMA3Type()
   {
    if (m_MA3.get() == nullptr)
    {
        DynamicTypeBuilder_ptr mA3_builder = m_factory.create_array_type(*GetMyAliasEnum3Type(), { 42 });
        m_MA3 = m_factory.get_alias_type(*mA3_builder->build(), "MA3");
    }

    return m_MA3;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetMyMiniArrayType()
   {
    if (m_MyMiniArray.get() == nullptr)
    {
        DynamicType_ptr int32_type = m_factory.get_int32_type();
        DynamicTypeBuilder_ptr myMiniArray_builder = m_factory.create_array_type(*int32_type, { 2 });
        m_MyMiniArray = m_factory.get_alias_type(*myMiniArray_builder->build(), "MyMiniArray");
    }

    return m_MyMiniArray;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetMySequenceLongType()
   {
    if (m_MySequenceLong.get() == nullptr)
    {
        DynamicType_ptr int32_type = m_factory.get_int32_type();
        DynamicTypeBuilder_ptr seqLong_builder = m_factory.create_sequence_type(*int32_type);
        DynamicTypeBuilder_ptr mySequenceLong_builder = m_factory.create_alias_type(
 * seqLong_builder->build(), "MySequenceLong");
        m_MySequenceLong = mySequenceLong_builder->build();
    }

    return m_MySequenceLong;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetBasicStructType()
   {
    if (m_BasicStructType.get() == nullptr)
    {
        // Members
        DynamicType_ptr bool_type = m_factory.get_bool_type();
        DynamicType_ptr octet_type = m_factory.get_byte_type();
        DynamicType_ptr int16_type = m_factory.get_int16_type();
        DynamicType_ptr int32_type = m_factory.get_int32_type();
        DynamicType_ptr int64_type = m_factory.get_int64_type();
        DynamicType_ptr uint16_type = m_factory.get_uint16_type();
        DynamicType_ptr uint32_type = m_factory.get_uint32_type();
        DynamicType_ptr uint64_type = m_factory.get_uint64_type();
        DynamicType_ptr float_type = m_factory.get_float32_type();
        DynamicType_ptr double_type = m_factory.get_float64_type();
        DynamicType_ptr ldouble_type = m_factory.get_float128_type();
        DynamicType_ptr char_type = m_factory.get_char8_type();
        DynamicType_ptr wchar_type = m_factory.get_char16_type();
        DynamicType_ptr string_type = m_factory.get_string_type();
        DynamicType_ptr wstring_type = m_factory.get_wstring_type();
        DynamicTypeBuilder_ptr basicStruct_builder = m_factory.create_struct_type();

        // Add members to the struct.
        int idx = 0;
        basicStruct_builder->add_member(idx++, "my_bool", bool_type);
        basicStruct_builder->add_member(idx++, "my_octet", octet_type);
        basicStruct_builder->add_member(idx++, "my_int16", int16_type);
        basicStruct_builder->add_member(idx++, "my_int32", int32_type);
        basicStruct_builder->add_member(idx++, "my_int64", int64_type);
        basicStruct_builder->add_member(idx++, "my_uint16", uint16_type);
        basicStruct_builder->add_member(idx++, "my_uint32", uint32_type);
        basicStruct_builder->add_member(idx++, "my_uint64", uint64_type);
        basicStruct_builder->add_member(idx++, "my_float32", float_type);
        basicStruct_builder->add_member(idx++, "my_float64", double_type);
        basicStruct_builder->add_member(idx++, "my_float128", ldouble_type);
        basicStruct_builder->add_member(idx++, "my_char", char_type);
        basicStruct_builder->add_member(idx++, "my_wchar", wchar_type);
        basicStruct_builder->add_member(idx++, "my_string", string_type);
        basicStruct_builder->add_member(idx++, "my_wstring", wstring_type);
        basicStruct_builder->set_name("BasicStruct");

        m_BasicStructType = basicStruct_builder->build();
    }

    return m_BasicStructType;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetComplexStructType()
   {
    if (m_ComplexStructType.get() == nullptr)
    {
        // Members (auxiliar types are tab)
        DynamicType_ptr octet_type = m_factory.get_byte_type();
        DynamicTypeBuilder_ptr my_sequence_octet_builder = m_factory.create_sequence_type(*octet_type, 55);
        DynamicType_ptr my_sequence_octet_type = my_sequence_octet_builder->build();

        DynamicTypeBuilder_ptr my_sequence_struct_builder = m_factory.create_sequence_type(*GetBasicStructType());
        DynamicType_ptr my_sequence_struct_type = my_sequence_struct_builder->build();

        DynamicType_ptr char_type = m_factory.get_char8_type();
        DynamicTypeBuilder_ptr my_array_octet_builder = m_factory.create_array_type(
 * char_type, { 500, 5, 4 });
        DynamicType_ptr my_array_octet_type = my_array_octet_builder->build();

        // MyOctetArray500 is already created
        // We reuse the bounds... { 5 }
        DynamicTypeBuilder_ptr my_array_struct_builder =
                DynamicTypeBuilderFactory::get_instance().create_array_type(*GetBasicStructType(), { 5 });
        DynamicType_ptr int16_type = DynamicTypeBuilderFactory::get_instance().get_int16_type();
        DynamicTypeBuilder_ptr my_map_octet_short_builder =
                DynamicTypeBuilderFactory::get_instance().create_map_type(*octet_type, *int16_type);
        DynamicType_ptr int32_type = DynamicTypeBuilderFactory::get_instance().get_int32_type();
        DynamicTypeBuilder_ptr my_map_long_struct_builder =
                DynamicTypeBuilderFactory::get_instance().create_map_type(*int32_type, *GetBasicStructType());
        DynamicTypeBuilder_ptr seqOctet_builder =
                DynamicTypeBuilderFactory::get_instance().create_sequence_type(*octet_type);
        DynamicTypeBuilder_ptr seqSeqOctet_builder =
                DynamicTypeBuilderFactory::get_instance().create_sequence_type(*seqOctet_builder->build());
        DynamicTypeBuilder_ptr my_map_long_seq_octet_builder =
                DynamicTypeBuilderFactory::get_instance().create_map_type(*int32_type,
 * seqSeqOctet_builder->build());
        DynamicType_ptr my_map_long_seq_octet_type = my_map_long_seq_octet_builder->build();
        DynamicTypeBuilder_ptr my_map_long_octet_array_500_builder =
                DynamicTypeBuilderFactory::get_instance().create_map_type(*int32_type, *GetMyOctetArray500Type());
        DynamicTypeBuilder_ptr map_octet_bsalias5_builder =
                DynamicTypeBuilderFactory::get_instance().create_map_type(*octet_type, *GetBSAlias5Type());
        DynamicTypeBuilder_ptr my_map_long_lol_type_builder =
                DynamicTypeBuilderFactory::get_instance().create_map_type(*int32_type,
 * map_octet_bsalias5_builder->build());
        DynamicType_ptr my_small_string_8_type =
                DynamicTypeBuilderFactory::get_instance().get_string_type(128);
        DynamicType_ptr my_small_string_16_type =
                DynamicTypeBuilderFactory::get_instance().get_wstring_type(64);
        DynamicType_ptr my_large_string_8_type =
                DynamicTypeBuilderFactory::get_instance().get_string_type(500);
        DynamicType_ptr my_large_string_16_type =
                DynamicTypeBuilderFactory::get_instance().get_wstring_type(1024);
        DynamicType_ptr string75_8_type =
                DynamicTypeBuilderFactory::get_instance().get_string_type(75);
        DynamicTypeBuilder_ptr my_array_string_builder =
                DynamicTypeBuilderFactory::get_instance().create_array_type(*string75_8_type, { 5, 5 });

        // MA3 is already defined.
        // { 5 } being reused
        DynamicTypeBuilder_ptr my_array_arrays_builder =
                DynamicTypeBuilderFactory::get_instance().create_array_type(*GetMyMiniArrayType(), { 5 });
        DynamicTypeBuilder_ptr my_sequences_array_builder =
                DynamicTypeBuilderFactory::get_instance().create_array_type(*GetMySequenceLongType(), { 23 });
        DynamicTypeBuilder_ptr complexStruct_builder =
                DynamicTypeBuilderFactory::get_instance().create_struct_type();

        // Add members to the struct.
        int idx = 0;
        complexStruct_builder->add_member(idx++, "my_octet", octet_type);
        complexStruct_builder->add_member(idx++, "my_basic_struct", GetBasicStructType());
        complexStruct_builder->add_member(idx++, "my_alias_enum", GetMyAliasEnumType());
        complexStruct_builder->add_member(idx++, "my_enum", GetMyEnumType());
        complexStruct_builder->add_member(idx++, "my_sequence_octet", my_sequence_octet_type);
        complexStruct_builder->add_member(idx++, "my_sequence_struct", my_sequence_struct_builder->build());
        complexStruct_builder->add_member(idx++, "my_array_octet", my_array_octet_type);
        complexStruct_builder->add_member(idx++, "my_octet_array_500", GetMyOctetArray500Type());
        complexStruct_builder->add_member(idx++, "my_array_struct", my_array_struct_builder->build());
        complexStruct_builder->add_member(idx++, "my_map_octet_short", my_map_octet_short_builder->build());
        complexStruct_builder->add_member(idx++, "my_map_long_struct", my_map_long_struct_builder->build());
        complexStruct_builder->add_member(idx++, "my_map_long_seq_octet", my_map_long_seq_octet_type);
        complexStruct_builder->add_member(idx++, "my_map_long_octet_array_500",
                my_map_long_octet_array_500_builder->build());
        complexStruct_builder->add_member(idx++, "my_map_long_lol_type", my_map_long_lol_type_builder->build());
        complexStruct_builder->add_member(idx++, "my_small_string_8", my_small_string_8_type);
        complexStruct_builder->add_member(idx++, "my_small_string_16", my_small_string_16_type);
        complexStruct_builder->add_member(idx++, "my_large_string_8", my_large_string_8_type);
        complexStruct_builder->add_member(idx++, "my_large_string_16", my_large_string_16_type);
        complexStruct_builder->add_member(idx++, "my_array_string", my_array_string_builder->build());
        complexStruct_builder->add_member(idx++, "multi_alias_array_42", GetMA3Type());
        complexStruct_builder->add_member(idx++, "my_array_arrays", my_array_arrays_builder->build());
        complexStruct_builder->add_member(idx++, "my_sequences_array", my_sequences_array_builder->build());
        complexStruct_builder->set_name("ComplexStruct");
        m_ComplexStructType = complexStruct_builder->build();
    }

    return m_ComplexStructType;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetUnionSwitchType()
   {
    if (m_UnionSwitchType.get() == nullptr)
    {
        DynamicTypeBuilder_ptr myUnion_builder = DynamicTypeBuilderFactory::get_instance().create_union_type(
 * GetMyEnumType());
        myUnion_builder->add_member(0_id, "basic", GetBasicStructType(), "A", std::vector<uint64_t>{ 0 }, false);
        myUnion_builder->add_member(1_id, "complex", GetComplexStructType(), "B", std::vector<uint64_t>{ 1, 2 }, false);
        myUnion_builder->set_name("MyUnion");
        m_UnionSwitchType = myUnion_builder->build();
    }

    return m_UnionSwitchType;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetUnion2SwitchType()
   {
    if (!m_Union2SwitchType)
    {
        DynamicType_ptr octet_type = DynamicTypeBuilderFactory::get_instance().get_byte_type();
        DynamicType_ptr int32_type = DynamicTypeBuilderFactory::get_instance().get_int32_type();
        DynamicType_ptr string_type = DynamicTypeBuilderFactory::get_instance().get_string_type();
        DynamicTypeBuilder_ptr myUnion2_builder = DynamicTypeBuilderFactory::get_instance().create_union_type(
 * octet_type);
        myUnion2_builder->add_member(0_id, "uno", int32_type, "0", std::vector<uint64_t>{ 0 }, false);
        myUnion2_builder->add_member(1_id, "imString", string_type, "1", std::vector<uint64_t>{ 1 }, false);
        myUnion2_builder->add_member(2_id, "tres", int32_type, "2", std::vector<uint64_t>{ 2 }, false);
        myUnion2_builder->set_name("MyUnion2");
        m_Union2SwitchType = myUnion2_builder->build();
    }

    return m_Union2SwitchType;
   }

   DynamicType_ptr DynamicComplexTypesTests::GetCompleteStructType()
   {
    if (m_CompleteStructType.get() == nullptr)
    {
        DynamicTypeBuilder_ptr completeStruct_builder = m_factory.create_struct_type();
        // Add members to the struct.
        int idx = 0;
        completeStruct_builder->add_member(idx++, "my_union", GetUnionSwitchType());
        completeStruct_builder->add_member(idx++, "my_union_2", GetUnion2SwitchType());
        completeStruct_builder->set_name("CompleteStruct");
        m_CompleteStructType = completeStruct_builder->build();
    }

    return m_CompleteStructType;
   }

   void DynamicComplexTypesTests::init()
   {
    // TODO(XTypes): PENDING implementation DynamicTypeBuilderFactory::create_type_w_type_object
    // m_DynAutoType = TypeObjectFactory::get_instance()->build_dynamic_type("CompleteStruct", id, obj);

    m_DynManualType = GetCompleteStructType();
   }

   TEST_F(DynamicComplexTypesTests, Static_Manual_Comparison)
   {
    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    DynamicData_ptr dynData(DynamicDataFactory::get_instance()->create_data(m_DynManualType));
    DynamicData_ptr dynData2(DynamicDataFactory::get_instance()->create_data(m_DynManualType));
    ASSERT_TRUE(dynData2->equals(dynData.get()));

    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(dynData.get(), &payload));

    CompleteStruct staticData;
    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    ASSERT_TRUE(payloadSize == payloadSize2);
    SerializedPayload_t payload2(payloadSize2);
    ASSERT_TRUE(m_StaticType.deserialize(&payload, &staticData));
    ASSERT_TRUE(m_StaticType.serialize(&staticData, &payload2));

    ASSERT_TRUE(pubsubType.deserialize(&payload2, dynData2.get()));
    ASSERT_TRUE(dynData2->equals(dynData.get()));
   }

   TEST_F(DynamicComplexTypesTests, Manual_Auto_Comparision)
   {
    EXPECT_EQ(*m_DynAutoType, *m_DynManualType);

    DynamicData* dynAutoData = DynamicDataFactory::get_instance()->create_data(m_DynAutoType);
    DynamicData* dynManualData = DynamicDataFactory::get_instance()->create_data(m_DynManualType);

    EXPECT_TRUE(dynManualData->equals(dynAutoData));

    DynamicDataFactory::get_instance()->delete_data(dynAutoData);
    DynamicDataFactory::get_instance()->delete_data(dynManualData);
   }

   TEST_F(DynamicComplexTypesTests, Static_Auto_Comparision)
   {
    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubtype(m_DynAutoType);
    DynamicData_ptr dynData(DynamicDataFactory::get_instance()->create_data(m_DynAutoType));
    uint32_t payloadSize = static_cast<uint32_t>(pubsubtype.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubtype.serialize(dynData.get(), &payload));
    ASSERT_TRUE(payloadSize == payload.length);

    CompleteStruct staticData;
    ASSERT_TRUE(m_StaticType.deserialize(&payload, &staticData));

    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    SerializedPayload_t payload2 = SerializedPayload_t(payloadSize2);
    ASSERT_TRUE(m_StaticType.serialize(&staticData, &payload2));
    ASSERT_TRUE(payloadSize2 == payload2.length);

    DynamicData_ptr dynData2(DynamicDataFactory::get_instance()->create_data(m_DynAutoType));
    ASSERT_TRUE(pubsubtype.deserialize(&payload2, dynData2.get()));

    ASSERT_TRUE(dynData2->equals(dynData.get()));
   }

   TEST_F(DynamicComplexTypesTests, Data_Comparison_A_A)
   {
    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    DynamicData_ptr dynData(DynamicDataFactory::get_instance()->create_data(m_DynManualType));
    DynamicData_ptr dynDataFromStatic(DynamicDataFactory::get_instance()->create_data(m_DynAutoType));

    CompleteStruct staticData;
    staticData.my_union()._d(MyEnum::A);
    staticData.my_union().basic().my_bool(true);
    staticData.my_union().basic().my_octet(166);
    staticData.my_union().basic().my_int16(-10401);
    staticData.my_union().basic().my_int32(5884001);
    staticData.my_union().basic().my_int64(884481567);
    staticData.my_union().basic().my_uint16(250);
    staticData.my_union().basic().my_uint32(15884);
    staticData.my_union().basic().my_uint64(765241);
    staticData.my_union().basic().my_float32(158.55f);
    staticData.my_union().basic().my_float64(765241.58);
    staticData.my_union().basic().my_float128(765241878.154874);
    staticData.my_union().basic().my_char('L');
    //staticData.my_union().basic().my_wchar(L'G');
    staticData.my_union().basic().my_string("Luis@eProsima");
    //staticData.my_union().basic().my_wstring(L"LuisGasco@eProsima");

    //staticData.my_union_2()._d(A);
    staticData.my_union_2().uno(156);

    DynamicData* my_union = dynData->loan_value(dynData->get_member_id_by_name("my_union"));
    DynamicData* basic = my_union->loan_value(my_union->get_member_id_by_name("basic"));

    basic->set_bool_value(true, basic->get_member_id_by_name("my_bool"));
    basic->set_byte_value(166, basic->get_member_id_by_name("my_octet"));
    basic->set_int16_value(-10401, basic->get_member_id_by_name("my_int16"));
    basic->set_int32_value(5884001, basic->get_member_id_by_name("my_int32"));
    basic->set_int64_value(884481567, basic->get_member_id_by_name("my_int64"));
    basic->set_uint16_value(250, basic->get_member_id_by_name("my_uint16"));
    basic->set_uint32_value(15884, basic->get_member_id_by_name("my_uint32"));
    basic->set_uint64_value(765241, basic->get_member_id_by_name("my_uint64"));
    basic->set_float32_value(158.55f, basic->get_member_id_by_name("my_float32"));
    basic->set_float64_value(765241.58, basic->get_member_id_by_name("my_float64"));
    basic->set_float128_value(765241878.154874, basic->get_member_id_by_name("my_float128"));
    basic->set_char8_value('L', basic->get_member_id_by_name("my_char"));
    //basic->set_char16_value(L'G', basic->get_member_id_by_name("my_wchar"));
    basic->set_string_value("Luis@eProsima", basic->get_member_id_by_name("my_string"));
    //basic->set_wstring_value(L"LuisGasco@eProsima", basic->get_member_id_by_name("my_wstring"));

    my_union->return_loaned_value(basic);
    dynData->return_loaned_value(my_union);

    DynamicData* my_union_2 = dynData->loan_value(dynData->get_member_id_by_name("my_union_2"));
    my_union_2->set_int32_value(156, my_union_2->get_member_id_by_name("uno"));


    dynData->return_loaned_value(my_union_2);

    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);

    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    ASSERT_TRUE(payloadSize == payloadSize2);

    CompleteStructPubSubType pbComplete;
    ASSERT_TRUE(pbComplete.serialize(&staticData, &payload));
    ASSERT_TRUE(pubsubType.deserialize(&payload, dynDataFromStatic.get()));

    ASSERT_TRUE(dynDataFromStatic->equals(dynData.get()));
   }

   TEST_F(DynamicComplexTypesTests, Data_Comparison_A_B)
   {
    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    DynamicData_ptr dynData(DynamicDataFactory::get_instance()->create_data(m_DynManualType));
    DynamicData_ptr dynDataFromStatic(DynamicDataFactory::get_instance()->create_data(m_DynAutoType));

    CompleteStruct staticData;
    staticData.my_union()._d(MyEnum::A);
    staticData.my_union().basic().my_bool(true);
    staticData.my_union().basic().my_octet(166);
    staticData.my_union().basic().my_int16(-10401);
    staticData.my_union().basic().my_int32(5884001);
    staticData.my_union().basic().my_int64(884481567);
    staticData.my_union().basic().my_uint16(250);
    staticData.my_union().basic().my_uint32(15884);
    staticData.my_union().basic().my_uint64(765241);
    staticData.my_union().basic().my_float32(158.55f);
    staticData.my_union().basic().my_float64(765241.58);
    staticData.my_union().basic().my_float128(765241878.154874);
    staticData.my_union().basic().my_char('L');
    //staticData.my_union().basic().my_wchar(L'G');
    staticData.my_union().basic().my_string("Luis@eProsima");
    //staticData.my_union().basic().my_wstring(L"LuisGasco@eProsima");

    staticData.my_union_2().imString("JuanCarlosArcereredekljnjkds");

    DynamicData* my_union = dynData->loan_value(dynData->get_member_id_by_name("my_union"));
    DynamicData* basic = my_union->loan_value(my_union->get_member_id_by_name("basic"));

    basic->set_bool_value(true, basic->get_member_id_by_name("my_bool"));
    basic->set_byte_value(166, basic->get_member_id_by_name("my_octet"));
    basic->set_int16_value(-10401, basic->get_member_id_by_name("my_int16"));
    basic->set_int32_value(5884001, basic->get_member_id_by_name("my_int32"));
    basic->set_int64_value(884481567, basic->get_member_id_by_name("my_int64"));
    basic->set_uint16_value(250, basic->get_member_id_by_name("my_uint16"));
    basic->set_uint32_value(15884, basic->get_member_id_by_name("my_uint32"));
    basic->set_uint64_value(765241, basic->get_member_id_by_name("my_uint64"));
    basic->set_float32_value(158.55f, basic->get_member_id_by_name("my_float32"));
    basic->set_float64_value(765241.58, basic->get_member_id_by_name("my_float64"));
    basic->set_float128_value(765241878.154874, basic->get_member_id_by_name("my_float128"));
    basic->set_char8_value('L', basic->get_member_id_by_name("my_char"));
    //basic->set_char16_value(L'G', basic->get_member_id_by_name("my_wchar"));
    basic->set_string_value("Luis@eProsima", basic->get_member_id_by_name("my_string"));
    //basic->set_wstring_value(L"LuisGasco@eProsima", basic->get_member_id_by_name("my_wstring"));

    my_union->return_loaned_value(basic);
    dynData->return_loaned_value(my_union);

    DynamicData* my_union_2 = dynData->loan_value(dynData->get_member_id_by_name("my_union_2"));
    my_union_2->set_string_value("JuanCarlosArcereredekljnjkds", my_union_2->get_member_id_by_name("imString"));


    dynData->return_loaned_value(my_union_2);

    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(dynData.get(), &payload));

    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    ASSERT_TRUE(payloadSize == payloadSize2);

    CompleteStructPubSubType pbComplete;
    ASSERT_TRUE(pbComplete.serialize(&staticData, &payload));
    ASSERT_TRUE(pubsubType.deserialize(&payload, dynDataFromStatic.get()));

    ASSERT_TRUE(dynDataFromStatic->equals(dynData.get()));
   }

   TEST_F(DynamicComplexTypesTests, Data_Comparison_A_C)
   {
    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    DynamicData_ptr dynData(DynamicDataFactory::get_instance()->create_data(m_DynManualType));
    DynamicData_ptr dynDataFromStatic(DynamicDataFactory::get_instance()->create_data(m_DynAutoType));

    CompleteStruct staticData;
    staticData.my_union()._d(MyEnum::A);
    staticData.my_union().basic().my_bool(true);
    staticData.my_union().basic().my_octet(166);
    staticData.my_union().basic().my_int16(-10401);
    staticData.my_union().basic().my_int32(5884001);
    staticData.my_union().basic().my_int64(884481567);
    staticData.my_union().basic().my_uint16(250);
    staticData.my_union().basic().my_uint32(15884);
    staticData.my_union().basic().my_uint64(765241);
    staticData.my_union().basic().my_float32(158.55f);
    staticData.my_union().basic().my_float64(765241.58);
    staticData.my_union().basic().my_float128(765241878.154874);
    staticData.my_union().basic().my_char('L');
    //staticData.my_union().basic().my_wchar(L'G');
    staticData.my_union().basic().my_string("Luis@eProsima");
    //staticData.my_union().basic().my_wstring(L"LuisGasco@eProsima");

    //staticData.my_union_2()._d(A);
    staticData.my_union_2().tres(333);

    DynamicData* my_union = dynData->loan_value(dynData->get_member_id_by_name("my_union"));
    DynamicData* basic = my_union->loan_value(my_union->get_member_id_by_name("basic"));

    basic->set_bool_value(true, basic->get_member_id_by_name("my_bool"));
    basic->set_byte_value(166, basic->get_member_id_by_name("my_octet"));
    basic->set_int16_value(-10401, basic->get_member_id_by_name("my_int16"));
    basic->set_int32_value(5884001, basic->get_member_id_by_name("my_int32"));
    basic->set_int64_value(884481567, basic->get_member_id_by_name("my_int64"));
    basic->set_uint16_value(250, basic->get_member_id_by_name("my_uint16"));
    basic->set_uint32_value(15884, basic->get_member_id_by_name("my_uint32"));
    basic->set_uint64_value(765241, basic->get_member_id_by_name("my_uint64"));
    basic->set_float32_value(158.55f, basic->get_member_id_by_name("my_float32"));
    basic->set_float64_value(765241.58, basic->get_member_id_by_name("my_float64"));
    basic->set_float128_value(765241878.154874, basic->get_member_id_by_name("my_float128"));
    basic->set_char8_value('L', basic->get_member_id_by_name("my_char"));
    //basic->set_char16_value(L'G', basic->get_member_id_by_name("my_wchar"));
    basic->set_string_value("Luis@eProsima", basic->get_member_id_by_name("my_string"));
    //basic->set_wstring_value(L"LuisGasco@eProsima", basic->get_member_id_by_name("my_wstring"));

    my_union->return_loaned_value(basic);
    dynData->return_loaned_value(my_union);

    DynamicData* my_union_2 = dynData->loan_value(dynData->get_member_id_by_name("my_union_2"));
    my_union_2->set_int32_value(333, my_union_2->get_member_id_by_name("tres"));

    dynData->return_loaned_value(my_union_2);

    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);

    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    ASSERT_TRUE(payloadSize == payloadSize2);

    CompleteStructPubSubType pbComplete;
    ASSERT_TRUE(pbComplete.serialize(&staticData, &payload));
    ASSERT_TRUE(pubsubType.deserialize(&payload, dynDataFromStatic.get()));

    ASSERT_TRUE(dynDataFromStatic->equals(dynData.get()));
   }

   TEST_F(DynamicComplexTypesTests, Data_Comparison_B_A)
   {
    DynamicData_ptr dynData(DynamicDataFactory::get_instance()->create_data(m_DynManualType));

    CompleteStruct staticData;
    staticData.my_union()._d() = MyEnum::B;
    staticData.my_union().complex().my_octet(66);
    staticData.my_union().complex().my_basic_struct().my_bool(true);
    staticData.my_union().complex().my_basic_struct().my_octet(166);
    staticData.my_union().complex().my_basic_struct().my_int16(-10401);
    staticData.my_union().complex().my_basic_struct().my_int32(5884001);
    staticData.my_union().complex().my_basic_struct().my_int64(884481567);
    staticData.my_union().complex().my_basic_struct().my_uint16(250);
    staticData.my_union().complex().my_basic_struct().my_uint32(15884);
    staticData.my_union().complex().my_basic_struct().my_uint64(765241);
    staticData.my_union().complex().my_basic_struct().my_float32(158.55f);
    staticData.my_union().complex().my_basic_struct().my_float64(765241.58);
    staticData.my_union().complex().my_basic_struct().my_float128(765241878.154874);
    staticData.my_union().complex().my_basic_struct().my_char('L');
    //staticData.my_union().complex().my_basic_struct().my_wchar(L'G');
    staticData.my_union().complex().my_basic_struct().my_string("Luis@eProsima");
    //staticData.my_union().complex().my_basic_struct().my_wstring(L"LuisGasco@eProsima");

    staticData.my_union().complex().my_alias_enum(MyEnum::C);
    staticData.my_union().complex().my_enum(MyEnum::B);
    staticData.my_union().complex().my_sequence_octet().push_back(88);
    staticData.my_union().complex().my_sequence_octet().push_back(99);
    staticData.my_union().complex().my_sequence_struct().push_back(staticData.my_union().complex().my_basic_struct());
    for (int i = 0; i < 500; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                staticData.my_union().complex().my_array_octet()[i][j][k] = static_cast<uint8_t>(j * k);
            }
        }
    }

    for (int i = 0; i < 5; ++i)
    {
        staticData.my_union().complex().my_array_struct()[i].my_bool(i % 2 == 1);
        staticData.my_union().complex().my_array_struct()[i].my_octet(static_cast<uint8_t>(i));
        staticData.my_union().complex().my_array_struct()[i].my_int16(static_cast<int16_t>(-i));
        staticData.my_union().complex().my_array_struct()[i].my_int32(i);
        staticData.my_union().complex().my_array_struct()[i].my_int64(i * 1000);
        staticData.my_union().complex().my_array_struct()[i].my_uint16(static_cast<uint16_t>(i));
        staticData.my_union().complex().my_array_struct()[i].my_uint32(i);
        staticData.my_union().complex().my_array_struct()[i].my_uint64(i * 10005);
        staticData.my_union().complex().my_array_struct()[i].my_float32(i * 5.5f);
        staticData.my_union().complex().my_array_struct()[i].my_float64(i * 8.8);
        staticData.my_union().complex().my_array_struct()[i].my_float128(i * 10.0);
        staticData.my_union().complex().my_array_struct()[i].my_char('J');
        //staticData.my_union().complex().my_array_struct()[i].my_wchar(L'C');
        staticData.my_union().complex().my_array_struct()[i].my_string("JC@eProsima");
        //staticData.my_union().complex().my_array_struct()[i].my_wstring(L"JC-BOOM-Armadilo!@eProsima");
    }

    staticData.my_union().complex().my_map_octet_short()[0] = 1340;
    staticData.my_union().complex().my_map_octet_short()[1] = 1341;
    staticData.my_union().complex().my_map_long_struct()[1000] = staticData.my_union().complex().my_array_struct()[3];
    staticData.my_union().complex().my_map_long_struct()[55] = staticData.my_union().complex().my_basic_struct();

    staticData.my_union().complex().my_map_long_seq_octet()[55].push_back({1, 2, 3, 4, 5});
    staticData.my_union().complex().my_map_long_seq_octet()[55].push_back({1, 2, 3, 4, 5});
    staticData.my_union().complex().my_map_long_seq_octet()[0].push_back({1, 2, 3, 4, 5});
    for (int i = 0; i < 500; ++i)
    {
        staticData.my_union().complex().my_map_long_octet_array_500()[0][i] = i % 256;
        staticData.my_union().complex().my_map_long_octet_array_500()[10][i] = (i + 55) % 256;
    }

    staticData.my_union().complex().my_small_string_8(
        "Bv7EMffURwGNqePoujdSfkF9PXN9TH125X5nGpNLfzya53tZtNJdgMROlYdZnTE1SLWzBdIU7ZyjjGvsGHkmuJUROwVPcNa9q5dRUV3KZAKNx1exL7BjhqIgQFconhd");
    //staticData.my_union().complex().my_small_string_16(
    //    L"AgzñgXsI9pXbWjYLDvvn8JUFWhxZhk9t92rdsTqylvdpqtXA6hy9dHkoBTgmF2c");
    staticData.my_union().complex().my_large_string_8(
        "hYE5vjcLJe6ML5DmoqQwh9ns866dAbnjkVKIKu2VF6lbkvh91ZOG2enEcdoRa8T43hR0Ym0k7tI621EQGufvzmLqxKCPgiXSp2zUTTmIWtn4fM8tC3aP1Yd0dKvn0tDobyp6p3156KvxqG3BKQ6VjFiHlMFoEyz8pjCclhXLl2cfAi97sQzXLUoPYUC5BWKyQTrA2JF6HXZM6vrbw5dc3B4AOJNGdPJ9ai6weF43h1RhnXE9MOFxPNoQnJ8gqSXYbMtpG6ZzqhUyoz0XhFDt7EOqXIgvc9SCejQTVMPeRcF5Zy57hrYZiKrCQqFWidS4BdfEAkuwESgBmEpEFOpZotwDt0TGDaLktSt3dKRsURO6TpuZ2nZNdiEJyc597ZjjQXtyKU7OCyRRqllzAnHEtoU3zd3OLTOvT5uk32N1Y64tpUte63De2EMwDNYb2eGAQfATdSt8VcGBOzJQjsmrMwMumtk48JzXXLxjo6s2vl2rNK9WQM1");
    //staticData.my_union().complex().my_large_string_16(
    //    L"nosYBfFr1s3t8rUsuUrVCWFi6moDk7GULFj6XnkebIDkjl3n2ykKxUIaLj3qNNUx0ny8DvFbdfxZBdMhBNW3fHbKrig4GkHnN1JoEo0ACiPxrARusDs3xKzvaQQrls6lVUFAUXzDOtw5f2CNVJKiruGjXUO2Lq5Mmy8ygW3eUiTlueAHA2dRXXryOFi47jS3DkmBH4aAOKcmR27KhhJnXaY0gWy3XdSnaGQNB3XvbmxQ7xXDsf1wz860WMEKP3VhdOLsmS6tKCb4sshuOlmUSyTggY7vNoxfpG1EUFP5iPro9E0tHLLdHlWf2NwU8OXCYx6KKEbs5pFMvgEstnQglsdTk0lOv6riaFkFOwx83gW1l6Pg4eXjacnJKoVh1pOeZxULLZpCECw8yRZ9z4JPHxh2C7ytkCHMKp9O4MwQwYvvvgWWLWfJgb7Ecy2tgvWLpNDzgkFrEFhaCTKitChlG422CnLSsXvTBNnF52sULH6rcwOVx3mbhqte3ld3fObtAuH3zPzjOF4vVbvUXxgZh1Zx1cey0iGfnhOZHUfUwJ3Qv0WZNcuVLvMMhhg85A3620b84MAIc2UoW9Hl4BIT7pHo41ApF0DxIPJL0QdIdAOjn0JTPZqAhoHVBQoYvivPHftk5Crd1a1J8L7hSs0s4uSQKAMTKDxy3gKLaGAg277h4iEsEZRCI4RPlPTo9nZ48s8OO2KzqrUbMkoPSTgaJEXq8GsozAzh0wtL4P3gPeHO5nQzoytoXAkiXoPph0GaTLiahYQksYeK1eVQADDqZPXC55teXKKdX4aomCufr1ZizgzkGwAmnsFmhmBSF0gvbm56NDaUVT0UqXxKxAfRjkILeWR1mW8jfn6RYJH3IWiHxEfyB23rr78NySfgzIchhrm7jEFtmwPpKPKAwzajLv0HpkrtTr38YwWeT5LzHokFAQEc6l3aWdJWapVyt9wX89dEkmPPG9torCV2ddjyF4jAKsxKvzU4pCxV6B3m16IIdnksemJ0xG8iKh4ZPsX");
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            staticData.my_union().complex().my_array_string()[i][j] =
                    "Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42";
        }
    }
    for (int i = 0; i < 42; ++i)
    {
        staticData.my_union().complex().multi_alias_array_42()[i] = (MyEnum)(i % 3);
    }

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            staticData.my_union().complex().my_array_arrays()[i][j] = i * j;
        }
    }

    for (int i = 0; i < 23; ++i)
    {
        staticData.my_union().complex().my_sequences_array()[i].push_back(i);
        staticData.my_union().complex().my_sequences_array()[i].push_back(i * 10);
        staticData.my_union().complex().my_sequences_array()[i].push_back(i * 100);
    }
    staticData.my_union_2()._d(0);
    staticData.my_union_2().uno(156);

    DynamicData* my_union = dynData->loan_value(dynData->get_member_id_by_name("my_union"));

    DynamicData* complex = my_union->loan_value(my_union->get_member_id_by_name("complex"));
    complex->set_byte_value(66, complex->get_member_id_by_name("my_octet"));

    DynamicData* basic = complex->loan_value(complex->get_member_id_by_name("my_basic_struct"));
    basic->set_bool_value(true, basic->get_member_id_by_name("my_bool"));
    basic->set_byte_value(166, basic->get_member_id_by_name("my_octet"));
    basic->set_int16_value(-10401, basic->get_member_id_by_name("my_int16"));
    basic->set_int32_value(5884001, basic->get_member_id_by_name("my_int32"));
    basic->set_int64_value(884481567, basic->get_member_id_by_name("my_int64"));
    basic->set_uint16_value(250, basic->get_member_id_by_name("my_uint16"));
    basic->set_uint32_value(15884, basic->get_member_id_by_name("my_uint32"));
    basic->set_uint64_value(765241, basic->get_member_id_by_name("my_uint64"));
    basic->set_float32_value(158.55f, basic->get_member_id_by_name("my_float32"));
    basic->set_float64_value(765241.58, basic->get_member_id_by_name("my_float64"));
    basic->set_float128_value(765241878.154874, basic->get_member_id_by_name("my_float128"));
    basic->set_char8_value('L', basic->get_member_id_by_name("my_char"));
    //basic->set_char16_value(L'G', basic->get_member_id_by_name("my_wchar"));
    basic->set_string_value("Luis@eProsima", basic->get_member_id_by_name("my_string"));
    //basic->set_wstring_value(L"LuisGasco@eProsima", basic->get_member_id_by_name("my_wstring"));
    complex->return_loaned_value(basic);

    complex->set_enum_value("C", complex->get_member_id_by_name("my_alias_enum"));
    complex->set_enum_value("B", complex->get_member_id_by_name("my_enum"));

    DynamicData* my_seq_octet = complex->loan_value(complex->get_member_id_by_name("my_sequence_octet"));
    MemberId id;
    my_seq_octet->insert_sequence_data(id);
    my_seq_octet->set_byte_value(88, id);
    my_seq_octet->insert_sequence_data(id);
    my_seq_octet->set_byte_value(99, id);
    //staticData.my_union().complex().my_sequence_octet().push_back(88);
    //staticData.my_union().complex().my_sequence_octet().push_back(99);
    complex->return_loaned_value(my_seq_octet);

    DynamicData* my_seq_struct = complex->loan_value(complex->get_member_id_by_name("my_sequence_struct"));
    my_seq_struct->insert_sequence_data(id);
    my_seq_struct->set_complex_value(DynamicDataFactory::get_instance()->create_copy(basic), id);
    //staticData.my_union().complex().my_sequence_struct().push_back(staticData.my_union().complex().my_basic_struct());
    complex->return_loaned_value(my_seq_struct);

    DynamicData* my_array_octet = complex->loan_value(complex->get_member_id_by_name("my_array_octet"));
    for (unsigned int i = 0; i < 500; ++i)
    {
        for (unsigned int j = 0; j < 5; ++j)
        {
            for (unsigned int k = 0; k < 4; ++k)
            {
                MemberId array_idx = my_array_octet->get_array_index({ i, j, k });
                my_array_octet->set_char8_value(static_cast<uint8_t>(j * k), array_idx);
            }
        }
        //staticData.my_union().complex().my_array_octet()[i][j][k] = j*k;
    }
    complex->return_loaned_value(my_array_octet);

    DynamicData* my_array_struct = complex->loan_value(complex->get_member_id_by_name("my_array_struct"));
    for (int i = 0; i < 5; ++i)
    {
        DynamicData* tempBasic = DynamicDataFactory::get_instance()->create_data(GetBasicStructType());
        tempBasic->set_bool_value(i % 2 == 1, tempBasic->get_member_id_by_name("my_bool"));
        tempBasic->set_byte_value(static_cast<uint8_t>(i), tempBasic->get_member_id_by_name("my_octet"));
        tempBasic->set_int16_value(static_cast<int16_t>(-i), tempBasic->get_member_id_by_name("my_int16"));
        tempBasic->set_int32_value(i, tempBasic->get_member_id_by_name("my_int32"));
        tempBasic->set_int64_value(i * 1000, tempBasic->get_member_id_by_name("my_int64"));
        tempBasic->set_uint16_value(static_cast<uint16_t>(i), tempBasic->get_member_id_by_name("my_uint16"));
        tempBasic->set_uint32_value(i, tempBasic->get_member_id_by_name("my_uint32"));
        tempBasic->set_uint64_value(i * 10005, tempBasic->get_member_id_by_name("my_uint64"));
        tempBasic->set_float32_value(i * 5.5f, tempBasic->get_member_id_by_name("my_float32"));
        tempBasic->set_float64_value(i * 8.8, tempBasic->get_member_id_by_name("my_float64"));
        tempBasic->set_float128_value(i * 10.0, tempBasic->get_member_id_by_name("my_float128"));
        tempBasic->set_char8_value('J', tempBasic->get_member_id_by_name("my_char"));
        //tempBasic->set_char16_value(L'C', tempBasic->get_member_id_by_name("my_wchar"));
        tempBasic->set_string_value("JC@eProsima", tempBasic->get_member_id_by_name("my_string"));
        tempBasic->set_wstring_value(L"JC-BOOM-Armadilo!@eProsima", tempBasic->get_member_id_by_name("my_wstring"));
        my_array_struct->set_complex_value(tempBasic, MemberId{i});
    }
    complex->return_loaned_value(my_array_struct);

    DynamicType_ptr octet_type = m_factory.get_byte_type();
    DynamicData_ptr key_oct(DynamicDataFactory::get_instance()->create_data(octet_type));
    MemberId kId;
    MemberId vId;
    MemberId ssId;
    MemberId sId;
    DynamicData* my_map_octet_short = complex->loan_value(complex->get_member_id_by_name("my_map_octet_short"));
    key_oct->set_byte_value(0);
    my_map_octet_short->insert_map_data(key_oct.get(), kId, vId);
    my_map_octet_short->set_int16_value((short)1340, vId);
    key_oct = DynamicDataFactory::get_instance()->create_data(octet_type);
    key_oct->set_byte_value(1);
    my_map_octet_short->insert_map_data(key_oct.get(), kId, vId);
    my_map_octet_short->set_int16_value((short)1341, vId);
    //staticData.my_union().complex().my_map_octet_short()[0] = 1340;
    //staticData.my_union().complex().my_map_octet_short()[1] = 1341;
    complex->return_loaned_value(my_map_octet_short);

    DynamicType_ptr long_type = m_factory.get_int32_type();
    DynamicData_ptr key(DynamicDataFactory::get_instance()->create_data(long_type));
    DynamicData* my_map_long_struct = complex->loan_value(complex->get_member_id_by_name("my_map_long_struct"));

    //DynamicData *mas3 = my_array_struct->loan_value(3);
    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(55);
    my_map_long_struct->insert_map_data(key.get(), kId, vId);
    basic = my_map_long_struct->loan_value(vId);
    basic->set_bool_value(true, basic->get_member_id_by_name("my_bool"));
    basic->set_byte_value(166, basic->get_member_id_by_name("my_octet"));
    basic->set_int16_value(-10401, basic->get_member_id_by_name("my_int16"));
    basic->set_int32_value(5884001, basic->get_member_id_by_name("my_int32"));
    basic->set_int64_value(884481567, basic->get_member_id_by_name("my_int64"));
    basic->set_uint16_value(250, basic->get_member_id_by_name("my_uint16"));
    basic->set_uint32_value(15884, basic->get_member_id_by_name("my_uint32"));
    basic->set_uint64_value(765241, basic->get_member_id_by_name("my_uint64"));
    basic->set_float32_value(158.55f, basic->get_member_id_by_name("my_float32"));
    basic->set_float64_value(765241.58, basic->get_member_id_by_name("my_float64"));
    basic->set_float128_value(765241878.154874, basic->get_member_id_by_name("my_float128"));
    basic->set_char8_value('L', basic->get_member_id_by_name("my_char"));
    //basic->set_char16_value(L'G', basic->get_member_id_by_name("my_wchar"));
    basic->set_string_value("Luis@eProsima", basic->get_member_id_by_name("my_string"));
    //basic->set_wstring_value(L"LuisGasco@eProsima", basic->get_member_id_by_name("my_wstring"));
    my_map_long_struct->return_loaned_value(basic);
    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(1000);
    my_map_long_struct->insert_map_data(key.get(), kId, vId);
    DynamicData* mas3 = my_map_long_struct->loan_value(vId);
    int i = 3;
    mas3->set_bool_value(i % 2 == 1, mas3->get_member_id_by_name("my_bool"));
    mas3->set_byte_value(static_cast<uint8_t>(i), mas3->get_member_id_by_name("my_octet"));
    mas3->set_int16_value(static_cast<int16_t>(-i), mas3->get_member_id_by_name("my_int16"));
    mas3->set_int32_value(i, mas3->get_member_id_by_name("my_int32"));
    mas3->set_int64_value(i * 1000, mas3->get_member_id_by_name("my_int64"));
    mas3->set_uint16_value(static_cast<uint8_t>(i), mas3->get_member_id_by_name("my_uint16"));
    mas3->set_uint32_value(i, mas3->get_member_id_by_name("my_uint32"));
    mas3->set_uint64_value(i * 10005, mas3->get_member_id_by_name("my_uint64"));
    mas3->set_float32_value(i * 5.5f, mas3->get_member_id_by_name("my_float32"));
    mas3->set_float64_value(i * 8.8, mas3->get_member_id_by_name("my_float64"));
    mas3->set_float128_value(i * 10.0, mas3->get_member_id_by_name("my_float128"));
    mas3->set_char8_value('J', mas3->get_member_id_by_name("my_char"));
    //mas3->set_char16_value(L'C', mas3->get_member_id_by_name("my_wchar"));
    mas3->set_string_value("JC@eProsima", mas3->get_member_id_by_name("my_string"));
    //mas3->set_wstring_value(L"JC-BOOM-Armadilo!@eProsima", mas3->get_member_id_by_name("my_wstring"));
    my_map_long_struct->return_loaned_value(mas3);

    // staticData.my_union().complex().my_map_long_struct()[1000] = staticData.my_union().complex().my_array_struct()[3];
    // staticData.my_union().complex().my_map_long_struct()[55] = staticData.my_union().complex().my_basic_struct();
    complex->return_loaned_value(my_map_long_struct);

    DynamicData* my_map_long_seq_octet = complex->loan_value(complex->get_member_id_by_name("my_map_long_seq_octet"));
    //std::vector my_vector_octet = {1, 2, 3, 4, 5};
    //MemberId id;
    //DynamicType_ptr octet_type = m_factory.get_byte_type();
       types::DynamicTypeBuilder_ptr seqOctet_builder = m_factory.create_sequence_type(*octet_type);
       types::DynamicType_ptr seqSeqOctet_builder = m_factory.get_sequence_type(seqOctet_builder->build())->build();
       DynamicData *dataSeqOctet = seqOctet_builder->build();
       DynamicData *dataSeqSeqOctet = seqSeqOctet_builder->build();
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(1, id);
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(2, id);
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(3, id);
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(4, id);
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(5, id);
       dataSeqSeqOctet->insert_sequence_data(id);
       dataSeqSeqOctet->set_complex_value(dataSeqOctet, id);//

    // insert_map_data(DynamicData_ptr key, MemberId& outKeyId, MemberId& outValueId);
    // TODO De la muerte para Juan Carlos - Esto no es NADA práctico...

    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(0);
    my_map_long_seq_octet->insert_map_data(key.get(), kId, vId);

    DynamicData* seq_seq_oct = my_map_long_seq_octet->loan_value(vId);
    seq_seq_oct->insert_sequence_data(ssId);
    DynamicData* seq_oct = seq_seq_oct->loan_value(ssId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(1, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(2, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(3, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(4, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(5, sId);
    seq_seq_oct->return_loaned_value(seq_oct);
    my_map_long_seq_octet->return_loaned_value(seq_seq_oct);

    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(55);
    my_map_long_seq_octet->insert_map_data(key.get(), kId, vId);

    seq_seq_oct = my_map_long_seq_octet->loan_value(vId);
    seq_seq_oct->insert_sequence_data(ssId);
    seq_oct = seq_seq_oct->loan_value(ssId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(1, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(2, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(3, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(4, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(5, sId);
    seq_seq_oct->return_loaned_value(seq_oct);
    seq_seq_oct->insert_sequence_data(ssId);
    seq_oct = seq_seq_oct->loan_value(ssId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(1, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(2, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(3, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(4, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(5, sId);
    seq_seq_oct->return_loaned_value(seq_oct);
    my_map_long_seq_octet->return_loaned_value(seq_seq_oct);
    //staticData.my_union().complex().my_map_long_seq_octet()[55].push_back(my_vector_octet);
    //staticData.my_union().complex().my_map_long_seq_octet()[55].push_back(my_vector_octet);
    //staticData.my_union().complex().my_map_long_seq_octet()[0].push_back(my_vector_octet);
    complex->return_loaned_value(my_map_long_seq_octet);

    DynamicData* my_map_long_octet_array_500 =
            complex->loan_value(complex->get_member_id_by_name("my_map_long_octet_array_500"));

    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(0);
    my_map_long_octet_array_500->insert_map_data(key.get(), kId, vId);

    DynamicData* oct_array_500 = my_map_long_octet_array_500->loan_value(vId);
    for (int j = 0; j < 500; ++j)
    {
        oct_array_500->set_byte_value(j % 256, MemberId{j});
        //staticData.my_union().complex().my_map_long_octet_array_500()[0][i] = i%256;
    }
    my_map_long_octet_array_500->return_loaned_value(oct_array_500);

    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(10);
    my_map_long_octet_array_500->insert_map_data(key.get(), kId, vId);
    oct_array_500 = my_map_long_octet_array_500->loan_value(vId);

    for (int j = 0; j < 500; ++j)
    {
        oct_array_500->set_byte_value((j + 55) % 256, MemberId{j});
        //staticData.my_union().complex().my_map_long_octet_array_500()[10][i] = (i+55)%256;
    }
    my_map_long_octet_array_500->return_loaned_value(oct_array_500);
    complex->return_loaned_value(my_map_long_octet_array_500);

    complex->set_string_value(
        "Bv7EMffURwGNqePoujdSfkF9PXN9TH125X5nGpNLfzya53tZtNJdgMROlYdZnTE1SLWzBdIU7ZyjjGvsGHkmuJUROwVPcNa9q5dRUV3KZAKNx1exL7BjhqIgQFconhd", complex->get_member_id_by_name(
            "my_small_string_8"));
    //complex->set_wstring_value(L"AgzñgXsI9pXbWjYLDvvn8JUFWhxZhk9t92rdsTqylvdpqtXA6hy9dHkoBTgmF2c", complex->get_member_id_by_name(
    //            "my_small_string_16"));
    complex->set_string_value(
        "hYE5vjcLJe6ML5DmoqQwh9ns866dAbnjkVKIKu2VF6lbkvh91ZOG2enEcdoRa8T43hR0Ym0k7tI621EQGufvzmLqxKCPgiXSp2zUTTmIWtn4fM8tC3aP1Yd0dKvn0tDobyp6p3156KvxqG3BKQ6VjFiHlMFoEyz8pjCclhXLl2cfAi97sQzXLUoPYUC5BWKyQTrA2JF6HXZM6vrbw5dc3B4AOJNGdPJ9ai6weF43h1RhnXE9MOFxPNoQnJ8gqSXYbMtpG6ZzqhUyoz0XhFDt7EOqXIgvc9SCejQTVMPeRcF5Zy57hrYZiKrCQqFWidS4BdfEAkuwESgBmEpEFOpZotwDt0TGDaLktSt3dKRsURO6TpuZ2nZNdiEJyc597ZjjQXtyKU7OCyRRqllzAnHEtoU3zd3OLTOvT5uk32N1Y64tpUte63De2EMwDNYb2eGAQfATdSt8VcGBOzJQjsmrMwMumtk48JzXXLxjo6s2vl2rNK9WQM1", complex->get_member_id_by_name(
            "my_large_string_8"));
    //complex->set_wstring_value(
    //    L"nosYBfFr1s3t8rUsuUrVCWFi6moDk7GULFj6XnkebIDkjl3n2ykKxUIaLj3qNNUx0ny8DvFbdfxZBdMhBNW3fHbKrig4GkHnN1JoEo0ACiPxrARusDs3xKzvaQQrls6lVUFAUXzDOtw5f2CNVJKiruGjXUO2Lq5Mmy8ygW3eUiTlueAHA2dRXXryOFi47jS3DkmBH4aAOKcmR27KhhJnXaY0gWy3XdSnaGQNB3XvbmxQ7xXDsf1wz860WMEKP3VhdOLsmS6tKCb4sshuOlmUSyTggY7vNoxfpG1EUFP5iPro9E0tHLLdHlWf2NwU8OXCYx6KKEbs5pFMvgEstnQglsdTk0lOv6riaFkFOwx83gW1l6Pg4eXjacnJKoVh1pOeZxULLZpCECw8yRZ9z4JPHxh2C7ytkCHMKp9O4MwQwYvvvgWWLWfJgb7Ecy2tgvWLpNDzgkFrEFhaCTKitChlG422CnLSsXvTBNnF52sULH6rcwOVx3mbhqte3ld3fObtAuH3zPzjOF4vVbvUXxgZh1Zx1cey0iGfnhOZHUfUwJ3Qv0WZNcuVLvMMhhg85A3620b84MAIc2UoW9Hl4BIT7pHo41ApF0DxIPJL0QdIdAOjn0JTPZqAhoHVBQoYvivPHftk5Crd1a1J8L7hSs0s4uSQKAMTKDxy3gKLaGAg277h4iEsEZRCI4RPlPTo9nZ48s8OO2KzqrUbMkoPSTgaJEXq8GsozAzh0wtL4P3gPeHO5nQzoytoXAkiXoPph0GaTLiahYQksYeK1eVQADDqZPXC55teXKKdX4aomCufr1ZizgzkGwAmnsFmhmBSF0gvbm56NDaUVT0UqXxKxAfRjkILeWR1mW8jfn6RYJH3IWiHxEfyB23rr78NySfgzIchhrm7jEFtmwPpKPKAwzajLv0HpkrtTr38YwWeT5LzHokFAQEc6l3aWdJWapVyt9wX89dEkmPPG9torCV2ddjyF4jAKsxKvzU4pCxV6B3m16IIdnksemJ0xG8iKh4ZPsX", complex->get_member_id_by_name(
    //        "my_large_string_16"));

    DynamicData* my_array_string = complex->loan_value(complex->get_member_id_by_name("my_array_string"));
    for (unsigned int j = 0; j < 5; ++j)
    {
        for (unsigned int k = 0; k < 5; ++k)
        {
            MemberId array_idx = my_array_string->get_array_index({ j, k });
            my_array_string->set_string_value(
                "Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42", array_idx);
            //staticData.my_union().complex().my_array_string()[i][j]("Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42");
        }
    }
    complex->return_loaned_value(my_array_string);

    DynamicData* multi_alias_array_42 = complex->loan_value(complex->get_member_id_by_name("multi_alias_array_42"));
    for (int j = 0; j < 42; ++j)
    {
        multi_alias_array_42->set_enum_value(j % 3, MemberId{j});
        //staticData.my_union().complex().multi_alias_array_42()[i](i%3);
    }
    complex->return_loaned_value(multi_alias_array_42);

    DynamicData* my_array_arrays = complex->loan_value(complex->get_member_id_by_name("my_array_arrays"));
    for (unsigned int j = 0; j < 5; ++j)
    {
        DynamicData* myMiniArray = my_array_arrays->loan_value(MemberId{j});
        for (unsigned int k = 0; k < 2; ++k)
        {
            myMiniArray->set_int32_value(j * k, MemberId{k});
            //staticData.my_union().complex().my_array_arrays()[i][j](i*j);
        }
        my_array_arrays->return_loaned_value(myMiniArray);
    }
    complex->return_loaned_value(my_array_arrays);

    DynamicData* my_sequences_array = complex->loan_value(complex->get_member_id_by_name("my_sequences_array"));
    for (int j = 0; j < 23; ++j)
    {
        DynamicData* seq = DynamicDataFactory::get_instance()->create_data(GetMySequenceLongType());
        seq->insert_sequence_data(id);
        seq->set_int32_value(j, id);
        seq->insert_sequence_data(id);
        seq->set_int32_value(j * 10, id);
        seq->insert_sequence_data(id);
        seq->set_int32_value(j * 100, id);
        my_sequences_array->set_complex_value(seq, MemberId{j});
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i*10);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i*100);
    }
    complex->return_loaned_value(my_sequences_array);

    my_union->return_loaned_value(complex);
    dynData->return_loaned_value(my_union);

    DynamicData* my_union_2 = dynData->loan_value(dynData->get_member_id_by_name("my_union_2"));
    my_union_2->set_int32_value(156, my_union_2->get_member_id_by_name("uno"));

    dynData->return_loaned_value(my_union_2);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);
    EXPECT_TRUE(pubsubType.serialize(dynData.get(), &payload));
    EXPECT_TRUE(payload.length == payloadSize);
    //
       std::cout << "BEGIN" << std::endl;
       for (uint32_t j = 0; j < payload.length; j += 100)
       {
        std::cout << std::endl;
        for (uint32_t k = 0; k < 100; k++)
        {
            if (j + k < payload.length)
            {
                if ((int)payload.data[j + k] == 204)
                {
                    std::cout << 0 << " ";
                }
                else
                {
                    std::cout << (int)payload.data[j + k] << " ";
                }
            }
        }
       }
       std::cout << "END" << std::endl;
     //
    CompleteStructPubSubType pbComplete;
    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    SerializedPayload_t stPayload(payloadSize2);
    EXPECT_TRUE(pbComplete.serialize(&staticData, &stPayload));
    EXPECT_TRUE(stPayload.length == payloadSize2);

    //       std::cout << "BEGIN" << std::endl;
    //       for (uint32_t j = 0; j < stPayload.length; j += 100)
    //       {
    //        std::cout << std::endl;
    //        for (uint32_t k = 0; k < 100; k++)
    //        {
    //            if (j + k < stPayload.length)
    //            {
    //                if ((int)stPayload.data[j + k] == 204)
    //                {
    //                    std::cout << 0 << " ";
    //                }
    //                else
    //                {
    //                    std::cout << (int)stPayload.data[j + k] << " ";
    //                }
    //            }
    //        }
    //       }
    //       std::cout << "END" << std::endl;

    DynamicData_ptr dynDataFromDynamic(DynamicDataFactory::get_instance()->create_data(m_DynAutoType));
    EXPECT_TRUE(pubsubType.deserialize(&payload, dynDataFromDynamic.get()));

    DynamicData_ptr dynDataFromStatic(DynamicDataFactory::get_instance()->create_data(m_DynAutoType));
    EXPECT_TRUE(pubsubType.deserialize(&stPayload, dynDataFromStatic.get()));

    EXPECT_TRUE(dynDataFromStatic->equals(dynDataFromDynamic.get()));
   }

   TEST_F(DynamicComplexTypesTests, Data_Comparison_B_B)
   {
    DynamicData_ptr dynData(DynamicDataFactory::get_instance()->create_data(m_DynManualType));

    CompleteStruct staticData;
    staticData.my_union()._d() = MyEnum::B;
    staticData.my_union().complex().my_octet(66);
    staticData.my_union().complex().my_basic_struct().my_bool(true);
    staticData.my_union().complex().my_basic_struct().my_octet(166);
    staticData.my_union().complex().my_basic_struct().my_int16(-10401);
    staticData.my_union().complex().my_basic_struct().my_int32(5884001);
    staticData.my_union().complex().my_basic_struct().my_int64(884481567);
    staticData.my_union().complex().my_basic_struct().my_uint16(250);
    staticData.my_union().complex().my_basic_struct().my_uint32(15884);
    staticData.my_union().complex().my_basic_struct().my_uint64(765241);
    staticData.my_union().complex().my_basic_struct().my_float32(158.55f);
    staticData.my_union().complex().my_basic_struct().my_float64(765241.58);
    staticData.my_union().complex().my_basic_struct().my_float128(765241878.154874);
    staticData.my_union().complex().my_basic_struct().my_char('L');
    //staticData.my_union().complex().my_basic_struct().my_wchar(L'G');
    staticData.my_union().complex().my_basic_struct().my_string("Luis@eProsima");
    //staticData.my_union().complex().my_basic_struct().my_wstring(L"LuisGasco@eProsima");

    staticData.my_union().complex().my_alias_enum(MyEnum::C);
    staticData.my_union().complex().my_enum(MyEnum::B);
    staticData.my_union().complex().my_sequence_octet().push_back(88);
    staticData.my_union().complex().my_sequence_octet().push_back(99);
    staticData.my_union().complex().my_sequence_struct().push_back(staticData.my_union().complex().my_basic_struct());
    for (int i = 0; i < 500; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                staticData.my_union().complex().my_array_octet()[i][j][k] = static_cast<uint8_t>(j * k);
            }
        }
    }

    for (int i = 0; i < 5; ++i)
    {
        staticData.my_union().complex().my_array_struct()[i].my_bool(i % 2 == 1);
        staticData.my_union().complex().my_array_struct()[i].my_octet(static_cast<uint8_t>(i));
        staticData.my_union().complex().my_array_struct()[i].my_int16(static_cast<int16_t>(-i));
        staticData.my_union().complex().my_array_struct()[i].my_int32(i);
        staticData.my_union().complex().my_array_struct()[i].my_int64(i * 1000);
        staticData.my_union().complex().my_array_struct()[i].my_uint16(static_cast<uint16_t>(i));
        staticData.my_union().complex().my_array_struct()[i].my_uint32(i);
        staticData.my_union().complex().my_array_struct()[i].my_uint64(i * 10005);
        staticData.my_union().complex().my_array_struct()[i].my_float32(i * 5.5f);
        staticData.my_union().complex().my_array_struct()[i].my_float64(i * 8.8);
        staticData.my_union().complex().my_array_struct()[i].my_float128(i * 10.0);
        staticData.my_union().complex().my_array_struct()[i].my_char('J');
        //staticData.my_union().complex().my_array_struct()[i].my_wchar(L'C');
        staticData.my_union().complex().my_array_struct()[i].my_string("JC@eProsima");
        //staticData.my_union().complex().my_array_struct()[i].my_wstring(L"JC-BOOM-Armadilo!@eProsima");
    }

    staticData.my_union().complex().my_map_octet_short()[0] = 1340;
    staticData.my_union().complex().my_map_octet_short()[1] = 1341;
    staticData.my_union().complex().my_map_long_struct()[1000] = staticData.my_union().complex().my_array_struct()[3];
    staticData.my_union().complex().my_map_long_struct()[55] = staticData.my_union().complex().my_basic_struct();

    staticData.my_union().complex().my_map_long_seq_octet()[55].push_back({ 1, 2, 3, 4, 5 });
    staticData.my_union().complex().my_map_long_seq_octet()[55].push_back({ 1, 2, 3, 4, 5 });
    staticData.my_union().complex().my_map_long_seq_octet()[0].push_back({ 1, 2, 3, 4, 5 });
    for (int i = 0; i < 500; ++i)
    {
        staticData.my_union().complex().my_map_long_octet_array_500()[0][i] = i % 256;
        staticData.my_union().complex().my_map_long_octet_array_500()[10][i] = (i + 55) % 256;
    }

    staticData.my_union().complex().my_small_string_8(
        "Bv7EMffURwGNqePoujdSfkF9PXN9TH125X5nGpNLfzya53tZtNJdgMROlYdZnTE1SLWzBdIU7ZyjjGvsGHkmuJUROwVPcNa9q5dRUV3KZAKNx1exL7BjhqIgQFconhd");
    //staticData.my_union().complex().my_small_string_16(
    //    L"AgzñgXsI9pXbWjYLDvvn8JUFWhxZhk9t92rdsTqylvdpqtXA6hy9dHkoBTgmF2c");
    staticData.my_union().complex().my_large_string_8(
        "hYE5vjcLJe6ML5DmoqQwh9ns866dAbnjkVKIKu2VF6lbkvh91ZOG2enEcdoRa8T43hR0Ym0k7tI621EQGufvzmLqxKCPgiXSp2zUTTmIWtn4fM8tC3aP1Yd0dKvn0tDobyp6p3156KvxqG3BKQ6VjFiHlMFoEyz8pjCclhXLl2cfAi97sQzXLUoPYUC5BWKyQTrA2JF6HXZM6vrbw5dc3B4AOJNGdPJ9ai6weF43h1RhnXE9MOFxPNoQnJ8gqSXYbMtpG6ZzqhUyoz0XhFDt7EOqXIgvc9SCejQTVMPeRcF5Zy57hrYZiKrCQqFWidS4BdfEAkuwESgBmEpEFOpZotwDt0TGDaLktSt3dKRsURO6TpuZ2nZNdiEJyc597ZjjQXtyKU7OCyRRqllzAnHEtoU3zd3OLTOvT5uk32N1Y64tpUte63De2EMwDNYb2eGAQfATdSt8VcGBOzJQjsmrMwMumtk48JzXXLxjo6s2vl2rNK9WQM1");
    //staticData.my_union().complex().my_large_string_16(
    //    L"nosYBfFr1s3t8rUsuUrVCWFi6moDk7GULFj6XnkebIDkjl3n2ykKxUIaLj3qNNUx0ny8DvFbdfxZBdMhBNW3fHbKrig4GkHnN1JoEo0ACiPxrARusDs3xKzvaQQrls6lVUFAUXzDOtw5f2CNVJKiruGjXUO2Lq5Mmy8ygW3eUiTlueAHA2dRXXryOFi47jS3DkmBH4aAOKcmR27KhhJnXaY0gWy3XdSnaGQNB3XvbmxQ7xXDsf1wz860WMEKP3VhdOLsmS6tKCb4sshuOlmUSyTggY7vNoxfpG1EUFP5iPro9E0tHLLdHlWf2NwU8OXCYx6KKEbs5pFMvgEstnQglsdTk0lOv6riaFkFOwx83gW1l6Pg4eXjacnJKoVh1pOeZxULLZpCECw8yRZ9z4JPHxh2C7ytkCHMKp9O4MwQwYvvvgWWLWfJgb7Ecy2tgvWLpNDzgkFrEFhaCTKitChlG422CnLSsXvTBNnF52sULH6rcwOVx3mbhqte3ld3fObtAuH3zPzjOF4vVbvUXxgZh1Zx1cey0iGfnhOZHUfUwJ3Qv0WZNcuVLvMMhhg85A3620b84MAIc2UoW9Hl4BIT7pHo41ApF0DxIPJL0QdIdAOjn0JTPZqAhoHVBQoYvivPHftk5Crd1a1J8L7hSs0s4uSQKAMTKDxy3gKLaGAg277h4iEsEZRCI4RPlPTo9nZ48s8OO2KzqrUbMkoPSTgaJEXq8GsozAzh0wtL4P3gPeHO5nQzoytoXAkiXoPph0GaTLiahYQksYeK1eVQADDqZPXC55teXKKdX4aomCufr1ZizgzkGwAmnsFmhmBSF0gvbm56NDaUVT0UqXxKxAfRjkILeWR1mW8jfn6RYJH3IWiHxEfyB23rr78NySfgzIchhrm7jEFtmwPpKPKAwzajLv0HpkrtTr38YwWeT5LzHokFAQEc6l3aWdJWapVyt9wX89dEkmPPG9torCV2ddjyF4jAKsxKvzU4pCxV6B3m16IIdnksemJ0xG8iKh4ZPsX");
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            staticData.my_union().complex().my_array_string()[i][j] =
                    "Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42";
        }
    }
    for (int i = 0; i < 42; ++i)
    {
        staticData.my_union().complex().multi_alias_array_42()[i] = (MyEnum)(i % 3);
    }

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            staticData.my_union().complex().my_array_arrays()[i][j] = i * j;
        }
    }

    for (int i = 0; i < 23; ++i)
    {
        staticData.my_union().complex().my_sequences_array()[i].push_back(i);
        staticData.my_union().complex().my_sequences_array()[i].push_back(i * 10);
        staticData.my_union().complex().my_sequences_array()[i].push_back(i * 100);
    }
    //staticData.my_union_2()._d() = B;
    staticData.my_union_2().imString("GASCO!");

    DynamicData* my_union = dynData->loan_value(dynData->get_member_id_by_name("my_union"));

    DynamicData* complex = my_union->loan_value(my_union->get_member_id_by_name("complex"));
    complex->set_byte_value(66, complex->get_member_id_by_name("my_octet"));

    DynamicData* basic = complex->loan_value(complex->get_member_id_by_name("my_basic_struct"));
    basic->set_bool_value(true, basic->get_member_id_by_name("my_bool"));
    basic->set_byte_value(166, basic->get_member_id_by_name("my_octet"));
    basic->set_int16_value(-10401, basic->get_member_id_by_name("my_int16"));
    basic->set_int32_value(5884001, basic->get_member_id_by_name("my_int32"));
    basic->set_int64_value(884481567, basic->get_member_id_by_name("my_int64"));
    basic->set_uint16_value(250, basic->get_member_id_by_name("my_uint16"));
    basic->set_uint32_value(15884, basic->get_member_id_by_name("my_uint32"));
    basic->set_uint64_value(765241, basic->get_member_id_by_name("my_uint64"));
    basic->set_float32_value(158.55f, basic->get_member_id_by_name("my_float32"));
    basic->set_float64_value(765241.58, basic->get_member_id_by_name("my_float64"));
    basic->set_float128_value(765241878.154874, basic->get_member_id_by_name("my_float128"));
    basic->set_char8_value('L', basic->get_member_id_by_name("my_char"));
    //basic->set_char16_value(L'G', basic->get_member_id_by_name("my_wchar"));
    basic->set_string_value("Luis@eProsima", basic->get_member_id_by_name("my_string"));
    //basic->set_wstring_value(L"LuisGasco@eProsima", basic->get_member_id_by_name("my_wstring"));
    complex->return_loaned_value(basic);

    complex->set_enum_value("C", complex->get_member_id_by_name("my_alias_enum"));
    complex->set_enum_value("B", complex->get_member_id_by_name("my_enum"));

    DynamicData* my_seq_octet = complex->loan_value(complex->get_member_id_by_name("my_sequence_octet"));
    MemberId id;
    my_seq_octet->insert_sequence_data(id);
    my_seq_octet->set_byte_value(88, id);
    my_seq_octet->insert_sequence_data(id);
    my_seq_octet->set_byte_value(99, id);
    //staticData.my_union().complex().my_sequence_octet().push_back(88);
    //staticData.my_union().complex().my_sequence_octet().push_back(99);
    complex->return_loaned_value(my_seq_octet);

    DynamicData* my_seq_struct = complex->loan_value(complex->get_member_id_by_name("my_sequence_struct"));
    my_seq_struct->insert_sequence_data(id);
    my_seq_struct->set_complex_value(DynamicDataFactory::get_instance()->create_copy(basic), id);
    //staticData.my_union().complex().my_sequence_struct().push_back(staticData.my_union().complex().my_basic_struct());
    complex->return_loaned_value(my_seq_struct);

    DynamicData* my_array_octet = complex->loan_value(complex->get_member_id_by_name("my_array_octet"));
    for (unsigned int i = 0; i < 500; ++i)
    {
        for (unsigned int j = 0; j < 5; ++j)
        {
            for (unsigned int k = 0; k < 4; ++k)
            {
                MemberId array_idx = my_array_octet->get_array_index({ i, j, k });
                my_array_octet->set_char8_value(static_cast<uint8_t>(j * k), array_idx);
            }
        }
        //staticData.my_union().complex().my_array_octet()[i][j][k] = j*k;
    }
    complex->return_loaned_value(my_array_octet);

    DynamicData* my_array_struct = complex->loan_value(complex->get_member_id_by_name("my_array_struct"));
    for (int i = 0; i < 5; ++i)
    {
        DynamicData* tempBasic = DynamicDataFactory::get_instance()->create_data(GetBasicStructType());
        tempBasic->set_bool_value(i % 2 == 1, tempBasic->get_member_id_by_name("my_bool"));
        tempBasic->set_byte_value(static_cast<uint8_t>(i), tempBasic->get_member_id_by_name("my_octet"));
        tempBasic->set_int16_value(static_cast<int16_t>(-i), tempBasic->get_member_id_by_name("my_int16"));
        tempBasic->set_int32_value(i, tempBasic->get_member_id_by_name("my_int32"));
        tempBasic->set_int64_value(i * 1000, tempBasic->get_member_id_by_name("my_int64"));
        tempBasic->set_uint16_value(static_cast<uint16_t>(i), tempBasic->get_member_id_by_name("my_uint16"));
        tempBasic->set_uint32_value(i, tempBasic->get_member_id_by_name("my_uint32"));
        tempBasic->set_uint64_value(i * 10005, tempBasic->get_member_id_by_name("my_uint64"));
        tempBasic->set_float32_value(i * 5.5f, tempBasic->get_member_id_by_name("my_float32"));
        tempBasic->set_float64_value(i * 8.8, tempBasic->get_member_id_by_name("my_float64"));
        tempBasic->set_float128_value(i * 10.0, tempBasic->get_member_id_by_name("my_float128"));
        tempBasic->set_char8_value('J', tempBasic->get_member_id_by_name("my_char"));
        //tempBasic->set_char16_value(L'C', tempBasic->get_member_id_by_name("my_wchar"));
        tempBasic->set_string_value("JC@eProsima", tempBasic->get_member_id_by_name("my_string"));
        tempBasic->set_wstring_value(L"JC-BOOM-Armadilo!@eProsima", tempBasic->get_member_id_by_name("my_wstring"));
        my_array_struct->set_complex_value(tempBasic, MemberId{i});
    }
    complex->return_loaned_value(my_array_struct);

    DynamicType_ptr octet_type = m_factory.get_byte_type();
    DynamicData_ptr key_oct(DynamicDataFactory::get_instance()->create_data(octet_type));
    MemberId kId;
    MemberId vId;
    MemberId ssId;
    MemberId sId;
    DynamicData* my_map_octet_short = complex->loan_value(complex->get_member_id_by_name("my_map_octet_short"));
    key_oct->set_byte_value(0);
    my_map_octet_short->insert_map_data(key_oct.get(), kId, vId);
    my_map_octet_short->set_int16_value((short)1340, vId);
    key_oct = DynamicDataFactory::get_instance()->create_data(octet_type);
    key_oct->set_byte_value(1);
    my_map_octet_short->insert_map_data(key_oct.get(), kId, vId);
    my_map_octet_short->set_int16_value((short)1341, vId);
    //staticData.my_union().complex().my_map_octet_short()[0] = 1340;
    //staticData.my_union().complex().my_map_octet_short()[1] = 1341;
    complex->return_loaned_value(my_map_octet_short);

    DynamicType_ptr long_type = m_factory.get_int32_type();
    DynamicData_ptr key(DynamicDataFactory::get_instance()->create_data(long_type));
    DynamicData* my_map_long_struct = complex->loan_value(complex->get_member_id_by_name("my_map_long_struct"));

    //DynamicData *mas3 = my_array_struct->loan_value(3);
    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(55);
    my_map_long_struct->insert_map_data(key.get(), kId, vId);
    basic = my_map_long_struct->loan_value(vId);
    basic->set_bool_value(true, basic->get_member_id_by_name("my_bool"));
    basic->set_byte_value(166, basic->get_member_id_by_name("my_octet"));
    basic->set_int16_value(-10401, basic->get_member_id_by_name("my_int16"));
    basic->set_int32_value(5884001, basic->get_member_id_by_name("my_int32"));
    basic->set_int64_value(884481567, basic->get_member_id_by_name("my_int64"));
    basic->set_uint16_value(250, basic->get_member_id_by_name("my_uint16"));
    basic->set_uint32_value(15884, basic->get_member_id_by_name("my_uint32"));
    basic->set_uint64_value(765241, basic->get_member_id_by_name("my_uint64"));
    basic->set_float32_value(158.55f, basic->get_member_id_by_name("my_float32"));
    basic->set_float64_value(765241.58, basic->get_member_id_by_name("my_float64"));
    basic->set_float128_value(765241878.154874, basic->get_member_id_by_name("my_float128"));
    basic->set_char8_value('L', basic->get_member_id_by_name("my_char"));
    //basic->set_char16_value(L'G', basic->get_member_id_by_name("my_wchar"));
    basic->set_string_value("Luis@eProsima", basic->get_member_id_by_name("my_string"));
    //basic->set_wstring_value(L"LuisGasco@eProsima", basic->get_member_id_by_name("my_wstring"));
    my_map_long_struct->return_loaned_value(basic);
    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(1000);
    my_map_long_struct->insert_map_data(key.get(), kId, vId);
    DynamicData* mas3 = my_map_long_struct->loan_value(vId);
    int i = 3;
    mas3->set_bool_value(i % 2 == 1, mas3->get_member_id_by_name("my_bool"));
    mas3->set_byte_value(static_cast<uint8_t>(i), mas3->get_member_id_by_name("my_octet"));
    mas3->set_int16_value(static_cast<int16_t>(-i), mas3->get_member_id_by_name("my_int16"));
    mas3->set_int32_value(i, mas3->get_member_id_by_name("my_int32"));
    mas3->set_int64_value(i * 1000, mas3->get_member_id_by_name("my_int64"));
    mas3->set_uint16_value(static_cast<uint8_t>(i), mas3->get_member_id_by_name("my_uint16"));
    mas3->set_uint32_value(i, mas3->get_member_id_by_name("my_uint32"));
    mas3->set_uint64_value(i * 10005, mas3->get_member_id_by_name("my_uint64"));
    mas3->set_float32_value(i * 5.5f, mas3->get_member_id_by_name("my_float32"));
    mas3->set_float64_value(i * 8.8, mas3->get_member_id_by_name("my_float64"));
    mas3->set_float128_value(i * 10.0, mas3->get_member_id_by_name("my_float128"));
    mas3->set_char8_value('J', mas3->get_member_id_by_name("my_char"));
    //mas3->set_char16_value(L'C', mas3->get_member_id_by_name("my_wchar"));
    mas3->set_string_value("JC@eProsima", mas3->get_member_id_by_name("my_string"));
    //mas3->set_wstring_value(L"JC-BOOM-Armadilo!@eProsima", mas3->get_member_id_by_name("my_wstring"));
    my_map_long_struct->return_loaned_value(mas3);

    // staticData.my_union().complex().my_map_long_struct()[1000] = staticData.my_union().complex().my_array_struct()[3];
    // staticData.my_union().complex().my_map_long_struct()[55] = staticData.my_union().complex().my_basic_struct();
    complex->return_loaned_value(my_map_long_struct);

    DynamicData* my_map_long_seq_octet = complex->loan_value(complex->get_member_id_by_name("my_map_long_seq_octet"));
    //std::vector my_vector_octet = {1, 2, 3, 4, 5};
    //MemberId id;
    //DynamicType_ptr octet_type = m_factory.get_byte_type();
       types::DynamicTypeBuilder_ptr seqOctet_builder = m_factory.create_sequence_type(octet_type);
       types::DynamicType_ptr seqSeqOctet_builder = m_factory.create_sequence_type(seqOctet_builder.get())->build();
       DynamicData *dataSeqOctet = seqOctet_builder->build();
       DynamicData *dataSeqSeqOctet = seqSeqOctet_builder->build();
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(1, id);
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(2, id);
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(3, id);
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(4, id);
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(5, id);
       dataSeqSeqOctet->insert_sequence_data(id);
       dataSeqSeqOctet->set_complex_value(dataSeqOctet, id);//
    // insert_map_data(DynamicData_ptr key, MemberId& outKeyId, MemberId& outValueId);
    // TODO De la muerte para Juan Carlos - Esto no es NADA práctico...

    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(0);
    my_map_long_seq_octet->insert_map_data(key.get(), kId, vId);

    DynamicData* seq_seq_oct = my_map_long_seq_octet->loan_value(vId);
    seq_seq_oct->insert_sequence_data(ssId);
    DynamicData* seq_oct = seq_seq_oct->loan_value(ssId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(1, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(2, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(3, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(4, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(5, sId);
    seq_seq_oct->return_loaned_value(seq_oct);
    my_map_long_seq_octet->return_loaned_value(seq_seq_oct);

    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(55);
    my_map_long_seq_octet->insert_map_data(key.get(), kId, vId);

    seq_seq_oct = my_map_long_seq_octet->loan_value(vId);
    seq_seq_oct->insert_sequence_data(ssId);
    seq_oct = seq_seq_oct->loan_value(ssId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(1, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(2, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(3, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(4, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(5, sId);
    seq_seq_oct->return_loaned_value(seq_oct);
    seq_seq_oct->insert_sequence_data(ssId);
    seq_oct = seq_seq_oct->loan_value(ssId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(1, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(2, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(3, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(4, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(5, sId);
    seq_seq_oct->return_loaned_value(seq_oct);
    my_map_long_seq_octet->return_loaned_value(seq_seq_oct);
    //staticData.my_union().complex().my_map_long_seq_octet()[55].push_back(my_vector_octet);
    //staticData.my_union().complex().my_map_long_seq_octet()[55].push_back(my_vector_octet);
    //staticData.my_union().complex().my_map_long_seq_octet()[0].push_back(my_vector_octet);
    complex->return_loaned_value(my_map_long_seq_octet);

    DynamicData* my_map_long_octet_array_500 =
            complex->loan_value(complex->get_member_id_by_name("my_map_long_octet_array_500"));

    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(0);
    my_map_long_octet_array_500->insert_map_data(key.get(), kId, vId);

    DynamicData* oct_array_500 = my_map_long_octet_array_500->loan_value(vId);
    for (int j = 0; j < 500; ++j)
    {
        oct_array_500->set_byte_value(j % 256, MemberId{j});
        //staticData.my_union().complex().my_map_long_octet_array_500()[0][i] = i%256;
    }
    my_map_long_octet_array_500->return_loaned_value(oct_array_500);

    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(10);
    my_map_long_octet_array_500->insert_map_data(key.get(), kId, vId);
    oct_array_500 = my_map_long_octet_array_500->loan_value(vId);

    for (int j = 0; j < 500; ++j)
    {
        oct_array_500->set_byte_value((j + 55) % 256, MemberId{j});
        //staticData.my_union().complex().my_map_long_octet_array_500()[10][i] = (i+55)%256;
    }
    my_map_long_octet_array_500->return_loaned_value(oct_array_500);
    complex->return_loaned_value(my_map_long_octet_array_500);

    complex->set_string_value(
        "Bv7EMffURwGNqePoujdSfkF9PXN9TH125X5nGpNLfzya53tZtNJdgMROlYdZnTE1SLWzBdIU7ZyjjGvsGHkmuJUROwVPcNa9q5dRUV3KZAKNx1exL7BjhqIgQFconhd", complex->get_member_id_by_name(
            "my_small_string_8"));
    //complex->set_wstring_value(L"AgzñgXsI9pXbWjYLDvvn8JUFWhxZhk9t92rdsTqylvdpqtXA6hy9dHkoBTgmF2c", complex->get_member_id_by_name(
    //            "my_small_string_16"));
    complex->set_string_value(
        "hYE5vjcLJe6ML5DmoqQwh9ns866dAbnjkVKIKu2VF6lbkvh91ZOG2enEcdoRa8T43hR0Ym0k7tI621EQGufvzmLqxKCPgiXSp2zUTTmIWtn4fM8tC3aP1Yd0dKvn0tDobyp6p3156KvxqG3BKQ6VjFiHlMFoEyz8pjCclhXLl2cfAi97sQzXLUoPYUC5BWKyQTrA2JF6HXZM6vrbw5dc3B4AOJNGdPJ9ai6weF43h1RhnXE9MOFxPNoQnJ8gqSXYbMtpG6ZzqhUyoz0XhFDt7EOqXIgvc9SCejQTVMPeRcF5Zy57hrYZiKrCQqFWidS4BdfEAkuwESgBmEpEFOpZotwDt0TGDaLktSt3dKRsURO6TpuZ2nZNdiEJyc597ZjjQXtyKU7OCyRRqllzAnHEtoU3zd3OLTOvT5uk32N1Y64tpUte63De2EMwDNYb2eGAQfATdSt8VcGBOzJQjsmrMwMumtk48JzXXLxjo6s2vl2rNK9WQM1", complex->get_member_id_by_name(
            "my_large_string_8"));
    //complex->set_wstring_value(
    //    L"nosYBfFr1s3t8rUsuUrVCWFi6moDk7GULFj6XnkebIDkjl3n2ykKxUIaLj3qNNUx0ny8DvFbdfxZBdMhBNW3fHbKrig4GkHnN1JoEo0ACiPxrARusDs3xKzvaQQrls6lVUFAUXzDOtw5f2CNVJKiruGjXUO2Lq5Mmy8ygW3eUiTlueAHA2dRXXryOFi47jS3DkmBH4aAOKcmR27KhhJnXaY0gWy3XdSnaGQNB3XvbmxQ7xXDsf1wz860WMEKP3VhdOLsmS6tKCb4sshuOlmUSyTggY7vNoxfpG1EUFP5iPro9E0tHLLdHlWf2NwU8OXCYx6KKEbs5pFMvgEstnQglsdTk0lOv6riaFkFOwx83gW1l6Pg4eXjacnJKoVh1pOeZxULLZpCECw8yRZ9z4JPHxh2C7ytkCHMKp9O4MwQwYvvvgWWLWfJgb7Ecy2tgvWLpNDzgkFrEFhaCTKitChlG422CnLSsXvTBNnF52sULH6rcwOVx3mbhqte3ld3fObtAuH3zPzjOF4vVbvUXxgZh1Zx1cey0iGfnhOZHUfUwJ3Qv0WZNcuVLvMMhhg85A3620b84MAIc2UoW9Hl4BIT7pHo41ApF0DxIPJL0QdIdAOjn0JTPZqAhoHVBQoYvivPHftk5Crd1a1J8L7hSs0s4uSQKAMTKDxy3gKLaGAg277h4iEsEZRCI4RPlPTo9nZ48s8OO2KzqrUbMkoPSTgaJEXq8GsozAzh0wtL4P3gPeHO5nQzoytoXAkiXoPph0GaTLiahYQksYeK1eVQADDqZPXC55teXKKdX4aomCufr1ZizgzkGwAmnsFmhmBSF0gvbm56NDaUVT0UqXxKxAfRjkILeWR1mW8jfn6RYJH3IWiHxEfyB23rr78NySfgzIchhrm7jEFtmwPpKPKAwzajLv0HpkrtTr38YwWeT5LzHokFAQEc6l3aWdJWapVyt9wX89dEkmPPG9torCV2ddjyF4jAKsxKvzU4pCxV6B3m16IIdnksemJ0xG8iKh4ZPsX", complex->get_member_id_by_name(
    //"my_large_string_16"));

    DynamicData* my_array_string = complex->loan_value(complex->get_member_id_by_name("my_array_string"));
    for (unsigned int j = 0; j < 5; ++j)
    {
        for (unsigned int k = 0; k < 5; ++k)
        {
            MemberId array_idx = my_array_string->get_array_index({ j, k });
            my_array_string->set_string_value(
                "Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42", array_idx);
            //staticData.my_union().complex().my_array_string()[i][j]("Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42");
        }
    }
    complex->return_loaned_value(my_array_string);

    DynamicData* multi_alias_array_42 = complex->loan_value(complex->get_member_id_by_name("multi_alias_array_42"));
    for (int j = 0; j < 42; ++j)
    {
        multi_alias_array_42->set_enum_value(j % 3, MemberId{j});
        //staticData.my_union().complex().multi_alias_array_42()[i](i%3);
    }
    complex->return_loaned_value(multi_alias_array_42);

    DynamicData* my_array_arrays = complex->loan_value(complex->get_member_id_by_name("my_array_arrays"));
    for (unsigned int j = 0; j < 5; ++j)
    {
        DynamicData* myMiniArray = my_array_arrays->loan_value(MemberId{j});
        for (unsigned int k = 0; k < 2; ++k)
        {
            myMiniArray->set_int32_value(j * k, MemberId{k});
            //staticData.my_union().complex().my_array_arrays()[i][j](i*j);
        }
        my_array_arrays->return_loaned_value(myMiniArray);
    }
    complex->return_loaned_value(my_array_arrays);

    DynamicData* my_sequences_array = complex->loan_value(complex->get_member_id_by_name("my_sequences_array"));
    for (int j = 0; j < 23; ++j)
    {
        DynamicData* seq = DynamicDataFactory::get_instance()->create_data(GetMySequenceLongType());
        seq->insert_sequence_data(id);
        seq->set_int32_value(j, id);
        seq->insert_sequence_data(id);
        seq->set_int32_value(j * 10, id);
        seq->insert_sequence_data(id);
        seq->set_int32_value(j * 100, id);
        my_sequences_array->set_complex_value(seq, MemberId{j});
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i*10);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i*100);
    }
    complex->return_loaned_value(my_sequences_array);

    my_union->return_loaned_value(complex);
    dynData->return_loaned_value(my_union);

    DynamicData* my_union_2 = dynData->loan_value(dynData->get_member_id_by_name("my_union_2"));
    my_union_2->set_string_value("GASCO!", my_union_2->get_member_id_by_name("imString"));

    dynData->return_loaned_value(my_union_2);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(dynData.get(), &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    CompleteStructPubSubType pbComplete;
    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    SerializedPayload_t stPayload(payloadSize2);
    ASSERT_TRUE(pbComplete.serialize(&staticData, &stPayload));
    ASSERT_TRUE(stPayload.length == payloadSize2);

    DynamicData_ptr dynDataFromDynamic(DynamicDataFactory::get_instance()->create_data(m_DynAutoType));
    ASSERT_TRUE(pubsubType.deserialize(&payload, dynDataFromDynamic.get()));

    DynamicData_ptr dynDataFromStatic(DynamicDataFactory::get_instance()->create_data(m_DynAutoType));
    ASSERT_TRUE(pubsubType.deserialize(&stPayload, dynDataFromStatic.get()));

    ASSERT_TRUE(dynDataFromStatic->equals(dynDataFromDynamic.get()));
   }

   TEST_F(DynamicComplexTypesTests, Data_Comparison_B_C)
   {
    DynamicData_ptr dynData(DynamicDataFactory::get_instance()->create_data(m_DynManualType));

    CompleteStruct staticData;
    staticData.my_union()._d() = MyEnum::B;
    staticData.my_union().complex().my_octet(66);
    staticData.my_union().complex().my_basic_struct().my_bool(true);
    staticData.my_union().complex().my_basic_struct().my_octet(166);
    staticData.my_union().complex().my_basic_struct().my_int16(-10401);
    staticData.my_union().complex().my_basic_struct().my_int32(5884001);
    staticData.my_union().complex().my_basic_struct().my_int64(884481567);
    staticData.my_union().complex().my_basic_struct().my_uint16(250);
    staticData.my_union().complex().my_basic_struct().my_uint32(15884);
    staticData.my_union().complex().my_basic_struct().my_uint64(765241);
    staticData.my_union().complex().my_basic_struct().my_float32(158.55f);
    staticData.my_union().complex().my_basic_struct().my_float64(765241.58);
    staticData.my_union().complex().my_basic_struct().my_float128(765241878.154874);
    staticData.my_union().complex().my_basic_struct().my_char('L');
    //staticData.my_union().complex().my_basic_struct().my_wchar(L'G');
    staticData.my_union().complex().my_basic_struct().my_string("Luis@eProsima");
    //staticData.my_union().complex().my_basic_struct().my_wstring(L"LuisGasco@eProsima");

    staticData.my_union().complex().my_alias_enum(MyEnum::C);
    staticData.my_union().complex().my_enum(MyEnum::B);
    staticData.my_union().complex().my_sequence_octet().push_back(88);
    staticData.my_union().complex().my_sequence_octet().push_back(99);
    staticData.my_union().complex().my_sequence_struct().push_back(staticData.my_union().complex().my_basic_struct());
    for (int i = 0; i < 500; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                staticData.my_union().complex().my_array_octet()[i][j][k] = static_cast<uint8_t>(j * k);
            }
        }
    }

    for (int i = 0; i < 5; ++i)
    {
        staticData.my_union().complex().my_array_struct()[i].my_bool(i % 2 == 1);
        staticData.my_union().complex().my_array_struct()[i].my_octet(static_cast<uint8_t>(i));
        staticData.my_union().complex().my_array_struct()[i].my_int16(static_cast<int16_t>(-i));
        staticData.my_union().complex().my_array_struct()[i].my_int32(i);
        staticData.my_union().complex().my_array_struct()[i].my_int64(i * 1000);
        staticData.my_union().complex().my_array_struct()[i].my_uint16(static_cast<uint16_t>(i));
        staticData.my_union().complex().my_array_struct()[i].my_uint32(i);
        staticData.my_union().complex().my_array_struct()[i].my_uint64(i * 10005);
        staticData.my_union().complex().my_array_struct()[i].my_float32(i * 5.5f);
        staticData.my_union().complex().my_array_struct()[i].my_float64(i * 8.8);
        staticData.my_union().complex().my_array_struct()[i].my_float128(i * 10.0);
        staticData.my_union().complex().my_array_struct()[i].my_char('J');
        //staticData.my_union().complex().my_array_struct()[i].my_wchar(L'C');
        staticData.my_union().complex().my_array_struct()[i].my_string("JC@eProsima");
        //staticData.my_union().complex().my_array_struct()[i].my_wstring(L"JC-BOOM-Armadilo!@eProsima");
    }

    staticData.my_union().complex().my_map_octet_short()[0] = 1340;
    staticData.my_union().complex().my_map_octet_short()[1] = 1341;
    staticData.my_union().complex().my_map_long_struct()[1000] = staticData.my_union().complex().my_array_struct()[3];
    staticData.my_union().complex().my_map_long_struct()[55] = staticData.my_union().complex().my_basic_struct();

    staticData.my_union().complex().my_map_long_seq_octet()[55].push_back({ 1, 2, 3, 4, 5 });
    staticData.my_union().complex().my_map_long_seq_octet()[55].push_back({ 1, 2, 3, 4, 5 });
    staticData.my_union().complex().my_map_long_seq_octet()[0].push_back({ 1, 2, 3, 4, 5 });
    for (int i = 0; i < 500; ++i)
    {
        staticData.my_union().complex().my_map_long_octet_array_500()[0][i] = i % 256;
        staticData.my_union().complex().my_map_long_octet_array_500()[10][i] = (i + 55) % 256;
    }

    staticData.my_union().complex().my_small_string_8(
        "Bv7EMffURwGNqePoujdSfkF9PXN9TH125X5nGpNLfzya53tZtNJdgMROlYdZnTE1SLWzBdIU7ZyjjGvsGHkmuJUROwVPcNa9q5dRUV3KZAKNx1exL7BjhqIgQFconhd");
    //staticData.my_union().complex().my_small_string_16(
    //    L"AgzñgXsI9pXbWjYLDvvn8JUFWhxZhk9t92rdsTqylvdpqtXA6hy9dHkoBTgmF2c");
    staticData.my_union().complex().my_large_string_8(
        "hYE5vjcLJe6ML5DmoqQwh9ns866dAbnjkVKIKu2VF6lbkvh91ZOG2enEcdoRa8T43hR0Ym0k7tI621EQGufvzmLqxKCPgiXSp2zUTTmIWtn4fM8tC3aP1Yd0dKvn0tDobyp6p3156KvxqG3BKQ6VjFiHlMFoEyz8pjCclhXLl2cfAi97sQzXLUoPYUC5BWKyQTrA2JF6HXZM6vrbw5dc3B4AOJNGdPJ9ai6weF43h1RhnXE9MOFxPNoQnJ8gqSXYbMtpG6ZzqhUyoz0XhFDt7EOqXIgvc9SCejQTVMPeRcF5Zy57hrYZiKrCQqFWidS4BdfEAkuwESgBmEpEFOpZotwDt0TGDaLktSt3dKRsURO6TpuZ2nZNdiEJyc597ZjjQXtyKU7OCyRRqllzAnHEtoU3zd3OLTOvT5uk32N1Y64tpUte63De2EMwDNYb2eGAQfATdSt8VcGBOzJQjsmrMwMumtk48JzXXLxjo6s2vl2rNK9WQM1");
    //staticData.my_union().complex().my_large_string_16(
    //    L"nosYBfFr1s3t8rUsuUrVCWFi6moDk7GULFj6XnkebIDkjl3n2ykKxUIaLj3qNNUx0ny8DvFbdfxZBdMhBNW3fHbKrig4GkHnN1JoEo0ACiPxrARusDs3xKzvaQQrls6lVUFAUXzDOtw5f2CNVJKiruGjXUO2Lq5Mmy8ygW3eUiTlueAHA2dRXXryOFi47jS3DkmBH4aAOKcmR27KhhJnXaY0gWy3XdSnaGQNB3XvbmxQ7xXDsf1wz860WMEKP3VhdOLsmS6tKCb4sshuOlmUSyTggY7vNoxfpG1EUFP5iPro9E0tHLLdHlWf2NwU8OXCYx6KKEbs5pFMvgEstnQglsdTk0lOv6riaFkFOwx83gW1l6Pg4eXjacnJKoVh1pOeZxULLZpCECw8yRZ9z4JPHxh2C7ytkCHMKp9O4MwQwYvvvgWWLWfJgb7Ecy2tgvWLpNDzgkFrEFhaCTKitChlG422CnLSsXvTBNnF52sULH6rcwOVx3mbhqte3ld3fObtAuH3zPzjOF4vVbvUXxgZh1Zx1cey0iGfnhOZHUfUwJ3Qv0WZNcuVLvMMhhg85A3620b84MAIc2UoW9Hl4BIT7pHo41ApF0DxIPJL0QdIdAOjn0JTPZqAhoHVBQoYvivPHftk5Crd1a1J8L7hSs0s4uSQKAMTKDxy3gKLaGAg277h4iEsEZRCI4RPlPTo9nZ48s8OO2KzqrUbMkoPSTgaJEXq8GsozAzh0wtL4P3gPeHO5nQzoytoXAkiXoPph0GaTLiahYQksYeK1eVQADDqZPXC55teXKKdX4aomCufr1ZizgzkGwAmnsFmhmBSF0gvbm56NDaUVT0UqXxKxAfRjkILeWR1mW8jfn6RYJH3IWiHxEfyB23rr78NySfgzIchhrm7jEFtmwPpKPKAwzajLv0HpkrtTr38YwWeT5LzHokFAQEc6l3aWdJWapVyt9wX89dEkmPPG9torCV2ddjyF4jAKsxKvzU4pCxV6B3m16IIdnksemJ0xG8iKh4ZPsX");
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            staticData.my_union().complex().my_array_string()[i][j] =
                    "Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42";
        }
    }
    for (int i = 0; i < 42; ++i)
    {
        staticData.my_union().complex().multi_alias_array_42()[i] = (MyEnum)(i % 3);
    }

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            staticData.my_union().complex().my_array_arrays()[i][j] = i * j;
        }
    }

    for (int i = 0; i < 23; ++i)
    {
        staticData.my_union().complex().my_sequences_array()[i].push_back(i);
        staticData.my_union().complex().my_sequences_array()[i].push_back(i * 10);
        staticData.my_union().complex().my_sequences_array()[i].push_back(i * 100);
    }
    staticData.my_union_2().tres(156);

    DynamicData* my_union = dynData->loan_value(dynData->get_member_id_by_name("my_union"));

    DynamicData* complex = my_union->loan_value(my_union->get_member_id_by_name("complex"));
    complex->set_byte_value(66, complex->get_member_id_by_name("my_octet"));

    DynamicData* basic = complex->loan_value(complex->get_member_id_by_name("my_basic_struct"));
    basic->set_bool_value(true, basic->get_member_id_by_name("my_bool"));
    basic->set_byte_value(166, basic->get_member_id_by_name("my_octet"));
    basic->set_int16_value(-10401, basic->get_member_id_by_name("my_int16"));
    basic->set_int32_value(5884001, basic->get_member_id_by_name("my_int32"));
    basic->set_int64_value(884481567, basic->get_member_id_by_name("my_int64"));
    basic->set_uint16_value(250, basic->get_member_id_by_name("my_uint16"));
    basic->set_uint32_value(15884, basic->get_member_id_by_name("my_uint32"));
    basic->set_uint64_value(765241, basic->get_member_id_by_name("my_uint64"));
    basic->set_float32_value(158.55f, basic->get_member_id_by_name("my_float32"));
    basic->set_float64_value(765241.58, basic->get_member_id_by_name("my_float64"));
    basic->set_float128_value(765241878.154874, basic->get_member_id_by_name("my_float128"));
    basic->set_char8_value('L', basic->get_member_id_by_name("my_char"));
    //basic->set_char16_value(L'G', basic->get_member_id_by_name("my_wchar"));
    basic->set_string_value("Luis@eProsima", basic->get_member_id_by_name("my_string"));
    //basic->set_wstring_value(L"LuisGasco@eProsima", basic->get_member_id_by_name("my_wstring"));
    complex->return_loaned_value(basic);

    complex->set_enum_value("C", complex->get_member_id_by_name("my_alias_enum"));
    complex->set_enum_value("B", complex->get_member_id_by_name("my_enum"));

    DynamicData* my_seq_octet = complex->loan_value(complex->get_member_id_by_name("my_sequence_octet"));
    MemberId id;
    my_seq_octet->insert_sequence_data(id);
    my_seq_octet->set_byte_value(88, id);
    my_seq_octet->insert_sequence_data(id);
    my_seq_octet->set_byte_value(99, id);
    //staticData.my_union().complex().my_sequence_octet().push_back(88);
    //staticData.my_union().complex().my_sequence_octet().push_back(99);
    complex->return_loaned_value(my_seq_octet);

    DynamicData* my_seq_struct = complex->loan_value(complex->get_member_id_by_name("my_sequence_struct"));
    my_seq_struct->insert_sequence_data(id);
    my_seq_struct->set_complex_value(DynamicDataFactory::get_instance()->create_copy(basic), id);
    //staticData.my_union().complex().my_sequence_struct().push_back(staticData.my_union().complex().my_basic_struct());
    complex->return_loaned_value(my_seq_struct);

    DynamicData* my_array_octet = complex->loan_value(complex->get_member_id_by_name("my_array_octet"));
    for (unsigned int i = 0; i < 500; ++i)
    {
        for (unsigned int j = 0; j < 5; ++j)
        {
            for (unsigned int k = 0; k < 4; ++k)
            {
                MemberId array_idx = my_array_octet->get_array_index({ i, j, k });
                my_array_octet->set_char8_value(static_cast<uint8_t>(j * k), array_idx);
            }
        }
        //staticData.my_union().complex().my_array_octet()[i][j][k] = j*k;
    }
    complex->return_loaned_value(my_array_octet);

    DynamicData* my_array_struct = complex->loan_value(complex->get_member_id_by_name("my_array_struct"));
    for (int i = 0; i < 5; ++i)
    {
        DynamicData* tempBasic = DynamicDataFactory::get_instance()->create_data(GetBasicStructType());
        tempBasic->set_bool_value(i % 2 == 1, tempBasic->get_member_id_by_name("my_bool"));
        tempBasic->set_byte_value(static_cast<uint8_t>(i), tempBasic->get_member_id_by_name("my_octet"));
        tempBasic->set_int16_value(static_cast<int16_t>(-i), tempBasic->get_member_id_by_name("my_int16"));
        tempBasic->set_int32_value(i, tempBasic->get_member_id_by_name("my_int32"));
        tempBasic->set_int64_value(i * 1000, tempBasic->get_member_id_by_name("my_int64"));
        tempBasic->set_uint16_value(static_cast<uint16_t>(i), tempBasic->get_member_id_by_name("my_uint16"));
        tempBasic->set_uint32_value(i, tempBasic->get_member_id_by_name("my_uint32"));
        tempBasic->set_uint64_value(i * 10005, tempBasic->get_member_id_by_name("my_uint64"));
        tempBasic->set_float32_value(i * 5.5f, tempBasic->get_member_id_by_name("my_float32"));
        tempBasic->set_float64_value(i * 8.8, tempBasic->get_member_id_by_name("my_float64"));
        tempBasic->set_float128_value(i * 10.0, tempBasic->get_member_id_by_name("my_float128"));
        tempBasic->set_char8_value('J', tempBasic->get_member_id_by_name("my_char"));
        //tempBasic->set_char16_value(L'C', tempBasic->get_member_id_by_name("my_wchar"));
        tempBasic->set_string_value("JC@eProsima", tempBasic->get_member_id_by_name("my_string"));
        tempBasic->set_wstring_value(L"JC-BOOM-Armadilo!@eProsima", tempBasic->get_member_id_by_name("my_wstring"));
        my_array_struct->set_complex_value(tempBasic, MemberId{i});
    }
    complex->return_loaned_value(my_array_struct);

    DynamicType_ptr octet_type = m_factory.get_byte_type();
    DynamicData_ptr key_oct(DynamicDataFactory::get_instance()->create_data(octet_type));
    MemberId kId;
    MemberId vId;
    MemberId ssId;
    MemberId sId;
    DynamicData* my_map_octet_short = complex->loan_value(complex->get_member_id_by_name("my_map_octet_short"));
    key_oct->set_byte_value(0);
    my_map_octet_short->insert_map_data(key_oct.get(), kId, vId);
    my_map_octet_short->set_int16_value((short)1340, vId);
    key_oct = DynamicDataFactory::get_instance()->create_data(octet_type);
    key_oct->set_byte_value(1);
    my_map_octet_short->insert_map_data(key_oct.get(), kId, vId);
    my_map_octet_short->set_int16_value((short)1341, vId);
    //staticData.my_union().complex().my_map_octet_short()[0] = 1340;
    //staticData.my_union().complex().my_map_octet_short()[1] = 1341;
    complex->return_loaned_value(my_map_octet_short);

    DynamicType_ptr long_type = m_factory.get_int32_type();
    DynamicData_ptr key(DynamicDataFactory::get_instance()->create_data(long_type));
    DynamicData* my_map_long_struct = complex->loan_value(complex->get_member_id_by_name("my_map_long_struct"));

    //DynamicData *mas3 = my_array_struct->loan_value(3);
    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(55);
    my_map_long_struct->insert_map_data(key.get(), kId, vId);
    basic = my_map_long_struct->loan_value(vId);
    basic->set_bool_value(true, basic->get_member_id_by_name("my_bool"));
    basic->set_byte_value(166, basic->get_member_id_by_name("my_octet"));
    basic->set_int16_value(-10401, basic->get_member_id_by_name("my_int16"));
    basic->set_int32_value(5884001, basic->get_member_id_by_name("my_int32"));
    basic->set_int64_value(884481567, basic->get_member_id_by_name("my_int64"));
    basic->set_uint16_value(250, basic->get_member_id_by_name("my_uint16"));
    basic->set_uint32_value(15884, basic->get_member_id_by_name("my_uint32"));
    basic->set_uint64_value(765241, basic->get_member_id_by_name("my_uint64"));
    basic->set_float32_value(158.55f, basic->get_member_id_by_name("my_float32"));
    basic->set_float64_value(765241.58, basic->get_member_id_by_name("my_float64"));
    basic->set_float128_value(765241878.154874, basic->get_member_id_by_name("my_float128"));
    basic->set_char8_value('L', basic->get_member_id_by_name("my_char"));
    //basic->set_char16_value(L'G', basic->get_member_id_by_name("my_wchar"));
    basic->set_string_value("Luis@eProsima", basic->get_member_id_by_name("my_string"));
    //basic->set_wstring_value(L"LuisGasco@eProsima", basic->get_member_id_by_name("my_wstring"));
    my_map_long_struct->return_loaned_value(basic);
    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(1000);
    my_map_long_struct->insert_map_data(key.get(), kId, vId);
    DynamicData* mas3 = my_map_long_struct->loan_value(vId);
    int i = 3;
    mas3->set_bool_value(i % 2 == 1, mas3->get_member_id_by_name("my_bool"));
    mas3->set_byte_value(static_cast<uint8_t>(i), mas3->get_member_id_by_name("my_octet"));
    mas3->set_int16_value(static_cast<int16_t>(-i), mas3->get_member_id_by_name("my_int16"));
    mas3->set_int32_value(i, mas3->get_member_id_by_name("my_int32"));
    mas3->set_int64_value(i * 1000, mas3->get_member_id_by_name("my_int64"));
    mas3->set_uint16_value(static_cast<uint8_t>(i), mas3->get_member_id_by_name("my_uint16"));
    mas3->set_uint32_value(i, mas3->get_member_id_by_name("my_uint32"));
    mas3->set_uint64_value(i * 10005, mas3->get_member_id_by_name("my_uint64"));
    mas3->set_float32_value(i * 5.5f, mas3->get_member_id_by_name("my_float32"));
    mas3->set_float64_value(i * 8.8, mas3->get_member_id_by_name("my_float64"));
    mas3->set_float128_value(i * 10.0, mas3->get_member_id_by_name("my_float128"));
    mas3->set_char8_value('J', mas3->get_member_id_by_name("my_char"));
    //mas3->set_char16_value(L'C', mas3->get_member_id_by_name("my_wchar"));
    mas3->set_string_value("JC@eProsima", mas3->get_member_id_by_name("my_string"));
    //mas3->set_wstring_value(L"JC-BOOM-Armadilo!@eProsima", mas3->get_member_id_by_name("my_wstring"));
    my_map_long_struct->return_loaned_value(mas3);

    // staticData.my_union().complex().my_map_long_struct()[1000] = staticData.my_union().complex().my_array_struct()[3];
    // staticData.my_union().complex().my_map_long_struct()[55] = staticData.my_union().complex().my_basic_struct();
    complex->return_loaned_value(my_map_long_struct);

    DynamicData* my_map_long_seq_octet = complex->loan_value(complex->get_member_id_by_name("my_map_long_seq_octet"));
    //std::vector my_vector_octet = {1, 2, 3, 4, 5};
    //MemberId id;
    //DynamicType_ptr octet_type = m_factory.get_byte_type();
       types::DynamicTypeBuilder_ptr seqOctet_builder = m_factory.create_sequence_type(octet_type);
       types::DynamicType_ptr seqSeqOctet_builder = m_factory.create_sequence_type(seqOctet_builder.get())->build();
       DynamicData *dataSeqOctet = seqOctet_builder->build();
       DynamicData *dataSeqSeqOctet = seqSeqOctet_builder->build();
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(1, id);
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(2, id);
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(3, id);
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(4, id);
       dataSeqOctet->insert_sequence_data(id);
       dataSeqOctet->set_byte_value(5, id);
       dataSeqSeqOctet->insert_sequence_data(id);
       dataSeqSeqOctet->set_complex_value(dataSeqOctet, id);//
    // insert_map_data(DynamicData_ptr key, MemberId& outKeyId, MemberId& outValueId);

    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(0);
    my_map_long_seq_octet->insert_map_data(key.get(), kId, vId);

    DynamicData* seq_seq_oct = my_map_long_seq_octet->loan_value(vId);
    seq_seq_oct->insert_sequence_data(ssId);
    DynamicData* seq_oct = seq_seq_oct->loan_value(ssId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(1, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(2, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(3, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(4, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(5, sId);
    seq_seq_oct->return_loaned_value(seq_oct);
    my_map_long_seq_octet->return_loaned_value(seq_seq_oct);

    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(55);
    my_map_long_seq_octet->insert_map_data(key.get(), kId, vId);

    seq_seq_oct = my_map_long_seq_octet->loan_value(vId);
    seq_seq_oct->insert_sequence_data(ssId);
    seq_oct = seq_seq_oct->loan_value(ssId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(1, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(2, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(3, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(4, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(5, sId);
    seq_seq_oct->return_loaned_value(seq_oct);
    seq_seq_oct->insert_sequence_data(ssId);
    seq_oct = seq_seq_oct->loan_value(ssId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(1, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(2, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(3, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(4, sId);
    seq_oct->insert_sequence_data(sId);
    seq_oct->set_byte_value(5, sId);
    seq_seq_oct->return_loaned_value(seq_oct);
    my_map_long_seq_octet->return_loaned_value(seq_seq_oct);
    //staticData.my_union().complex().my_map_long_seq_octet()[55].push_back(my_vector_octet);
    //staticData.my_union().complex().my_map_long_seq_octet()[55].push_back(my_vector_octet);
    //staticData.my_union().complex().my_map_long_seq_octet()[0].push_back(my_vector_octet);
    complex->return_loaned_value(my_map_long_seq_octet);

    DynamicData* my_map_long_octet_array_500 =
            complex->loan_value(complex->get_member_id_by_name("my_map_long_octet_array_500"));

    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(0);
    my_map_long_octet_array_500->insert_map_data(key.get(), kId, vId);

    DynamicData* oct_array_500 = my_map_long_octet_array_500->loan_value(vId);
    for (int j = 0; j < 500; ++j)
    {
        oct_array_500->set_byte_value(j % 256, MemberId{j});
        //staticData.my_union().complex().my_map_long_octet_array_500()[0][i] = i%256;
    }
    my_map_long_octet_array_500->return_loaned_value(oct_array_500);

    key = DynamicDataFactory::get_instance()->create_data(long_type);
    key->set_int32_value(10);
    my_map_long_octet_array_500->insert_map_data(key.get(), kId, vId);
    oct_array_500 = my_map_long_octet_array_500->loan_value(vId);

    for (int j = 0; j < 500; ++j)
    {
        oct_array_500->set_byte_value((j + 55) % 256, MemberId{j});
        //staticData.my_union().complex().my_map_long_octet_array_500()[10][i] = (i+55)%256;
    }
    my_map_long_octet_array_500->return_loaned_value(oct_array_500);
    complex->return_loaned_value(my_map_long_octet_array_500);

    complex->set_string_value(
        "Bv7EMffURwGNqePoujdSfkF9PXN9TH125X5nGpNLfzya53tZtNJdgMROlYdZnTE1SLWzBdIU7ZyjjGvsGHkmuJUROwVPcNa9q5dRUV3KZAKNx1exL7BjhqIgQFconhd", complex->get_member_id_by_name(
            "my_small_string_8"));
    //complex->set_wstring_value(L"AgzñgXsI9pXbWjYLDvvn8JUFWhxZhk9t92rdsTqylvdpqtXA6hy9dHkoBTgmF2c", complex->get_member_id_by_name(
    //            "my_small_string_16"));
    complex->set_string_value(
        "hYE5vjcLJe6ML5DmoqQwh9ns866dAbnjkVKIKu2VF6lbkvh91ZOG2enEcdoRa8T43hR0Ym0k7tI621EQGufvzmLqxKCPgiXSp2zUTTmIWtn4fM8tC3aP1Yd0dKvn0tDobyp6p3156KvxqG3BKQ6VjFiHlMFoEyz8pjCclhXLl2cfAi97sQzXLUoPYUC5BWKyQTrA2JF6HXZM6vrbw5dc3B4AOJNGdPJ9ai6weF43h1RhnXE9MOFxPNoQnJ8gqSXYbMtpG6ZzqhUyoz0XhFDt7EOqXIgvc9SCejQTVMPeRcF5Zy57hrYZiKrCQqFWidS4BdfEAkuwESgBmEpEFOpZotwDt0TGDaLktSt3dKRsURO6TpuZ2nZNdiEJyc597ZjjQXtyKU7OCyRRqllzAnHEtoU3zd3OLTOvT5uk32N1Y64tpUte63De2EMwDNYb2eGAQfATdSt8VcGBOzJQjsmrMwMumtk48JzXXLxjo6s2vl2rNK9WQM1", complex->get_member_id_by_name(
            "my_large_string_8"));
    //complex->set_wstring_value(
    //    L"nosYBfFr1s3t8rUsuUrVCWFi6moDk7GULFj6XnkebIDkjl3n2ykKxUIaLj3qNNUx0ny8DvFbdfxZBdMhBNW3fHbKrig4GkHnN1JoEo0ACiPxrARusDs3xKzvaQQrls6lVUFAUXzDOtw5f2CNVJKiruGjXUO2Lq5Mmy8ygW3eUiTlueAHA2dRXXryOFi47jS3DkmBH4aAOKcmR27KhhJnXaY0gWy3XdSnaGQNB3XvbmxQ7xXDsf1wz860WMEKP3VhdOLsmS6tKCb4sshuOlmUSyTggY7vNoxfpG1EUFP5iPro9E0tHLLdHlWf2NwU8OXCYx6KKEbs5pFMvgEstnQglsdTk0lOv6riaFkFOwx83gW1l6Pg4eXjacnJKoVh1pOeZxULLZpCECw8yRZ9z4JPHxh2C7ytkCHMKp9O4MwQwYvvvgWWLWfJgb7Ecy2tgvWLpNDzgkFrEFhaCTKitChlG422CnLSsXvTBNnF52sULH6rcwOVx3mbhqte3ld3fObtAuH3zPzjOF4vVbvUXxgZh1Zx1cey0iGfnhOZHUfUwJ3Qv0WZNcuVLvMMhhg85A3620b84MAIc2UoW9Hl4BIT7pHo41ApF0DxIPJL0QdIdAOjn0JTPZqAhoHVBQoYvivPHftk5Crd1a1J8L7hSs0s4uSQKAMTKDxy3gKLaGAg277h4iEsEZRCI4RPlPTo9nZ48s8OO2KzqrUbMkoPSTgaJEXq8GsozAzh0wtL4P3gPeHO5nQzoytoXAkiXoPph0GaTLiahYQksYeK1eVQADDqZPXC55teXKKdX4aomCufr1ZizgzkGwAmnsFmhmBSF0gvbm56NDaUVT0UqXxKxAfRjkILeWR1mW8jfn6RYJH3IWiHxEfyB23rr78NySfgzIchhrm7jEFtmwPpKPKAwzajLv0HpkrtTr38YwWeT5LzHokFAQEc6l3aWdJWapVyt9wX89dEkmPPG9torCV2ddjyF4jAKsxKvzU4pCxV6B3m16IIdnksemJ0xG8iKh4ZPsX", complex->get_member_id_by_name(
    //        "my_large_string_16"));

    DynamicData* my_array_string = complex->loan_value(complex->get_member_id_by_name("my_array_string"));
    for (unsigned int j = 0; j < 5; ++j)
    {
        for (unsigned int k = 0; k < 5; ++k)
        {
            MemberId array_idx = my_array_string->get_array_index({ j, k });
            my_array_string->set_string_value(
                "Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42", array_idx);
            //staticData.my_union().complex().my_array_string()[i][j]("Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42");
        }
    }
    complex->return_loaned_value(my_array_string);

    DynamicData* multi_alias_array_42 = complex->loan_value(complex->get_member_id_by_name("multi_alias_array_42"));
    for (int j = 0; j < 42; ++j)
    {
        multi_alias_array_42->set_enum_value(j % 3, MemberId{j});
        //staticData.my_union().complex().multi_alias_array_42()[i](i%3);
    }
    complex->return_loaned_value(multi_alias_array_42);

    DynamicData* my_array_arrays = complex->loan_value(complex->get_member_id_by_name("my_array_arrays"));
    for (unsigned int j = 0; j < 5; ++j)
    {
        DynamicData* myMiniArray = my_array_arrays->loan_value(MemberId{j});
        for (unsigned int k = 0; k < 2; ++k)
        {
            myMiniArray->set_int32_value(j * k, MemberId{k});
            //staticData.my_union().complex().my_array_arrays()[i][j](i*j);
        }
        my_array_arrays->return_loaned_value(myMiniArray);
    }
    complex->return_loaned_value(my_array_arrays);

    DynamicData* my_sequences_array = complex->loan_value(complex->get_member_id_by_name("my_sequences_array"));
    for (int j = 0; j < 23; ++j)
    {
        DynamicData* seq = DynamicDataFactory::get_instance()->create_data(GetMySequenceLongType());
        seq->insert_sequence_data(id);
        seq->set_int32_value(j, id);
        seq->insert_sequence_data(id);
        seq->set_int32_value(j * 10, id);
        seq->insert_sequence_data(id);
        seq->set_int32_value(j * 100, id);
        my_sequences_array->set_complex_value(seq, MemberId{j});
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i*10);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i*100);
    }
    complex->return_loaned_value(my_sequences_array);

    my_union->return_loaned_value(complex);
    dynData->return_loaned_value(my_union);

    DynamicData* my_union_2 = dynData->loan_value(dynData->get_member_id_by_name("my_union_2"));
    my_union_2->set_int32_value(156, my_union_2->get_member_id_by_name("tres"));

    dynData->return_loaned_value(my_union_2);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(dynData.get(), &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    CompleteStructPubSubType pbComplete;
    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    SerializedPayload_t stPayload(payloadSize2);
    ASSERT_TRUE(pbComplete.serialize(&staticData, &stPayload));
    ASSERT_TRUE(stPayload.length == payloadSize2);
    DynamicData_ptr dynDataFromDynamic(DynamicDataFactory::get_instance()->create_data(m_DynAutoType));
    ASSERT_TRUE(pubsubType.deserialize(&payload, dynDataFromDynamic.get()));

    DynamicData_ptr dynDataFromStatic(DynamicDataFactory::get_instance()->create_data(m_DynAutoType));
    ASSERT_TRUE(pubsubType.deserialize(&stPayload, dynDataFromStatic.get()));

    ASSERT_TRUE(dynDataFromStatic->equals(dynDataFromDynamic.get()));
   }

   TEST_F(DynamicComplexTypesTests, Data_Comparison_with_Keys)
   {
    KeyedStruct staticData;

    staticData.basic().my_bool(true);
    staticData.basic().my_octet(100);
    staticData.basic().my_int16(-12000);
    staticData.basic().my_int32(-12000000);
    staticData.basic().my_int64(-1200000000);
    staticData.basic().my_uint16(12000);
    staticData.basic().my_uint32(12000000);
    staticData.basic().my_uint64(1200000000);
    staticData.basic().my_float32(5.5f);
    staticData.basic().my_float64(8.888);
    staticData.basic().my_float128(1005.1005);
    staticData.basic().my_char('O');
    //staticData.basic().my_wchar(L'M');
    staticData.basic().my_string("G It's");
    //staticData.basic().my_wstring(L" Working");
    //staticData.key(88);

    DynamicData* dynData = DynamicDataFactory::get_instance()->create_data(GetKeyedStructType());
    DynamicData* basic = dynData->loan_value(dynData->get_member_id_by_name("basic"));
    basic->set_bool_value(true, basic->get_member_id_by_name("my_bool"));
    basic->set_byte_value(100, basic->get_member_id_by_name("my_octet"));
    basic->set_int16_value(-12000, basic->get_member_id_by_name("my_int16"));
    basic->set_int32_value(-12000000, basic->get_member_id_by_name("my_int32"));
    basic->set_int64_value(-1200000000, basic->get_member_id_by_name("my_int64"));
    basic->set_uint16_value(12000, basic->get_member_id_by_name("my_uint16"));
    basic->set_uint32_value(12000000, basic->get_member_id_by_name("my_uint32"));
    basic->set_uint64_value(1200000000, basic->get_member_id_by_name("my_uint64"));
    basic->set_float32_value(5.5f, basic->get_member_id_by_name("my_float32"));
    basic->set_float64_value(8.888, basic->get_member_id_by_name("my_float64"));
    basic->set_float128_value(1005.1005, basic->get_member_id_by_name("my_float128"));
    basic->set_char8_value('O', basic->get_member_id_by_name("my_char"));
    //basic->set_char16_value(L'M', basic->get_member_id_by_name("my_wchar"));
    basic->set_string_value("G It's", basic->get_member_id_by_name("my_string"));
    //basic->set_wstring_value(L" Working", basic->get_member_id_by_name("my_wstring"));
    dynData->return_loaned_value(basic);
    //dynData->set_byte_value(88, dynData->get_member_id_by_name("key"));

    KeyedStructPubSubType pbKeyed;
    DynamicPubSubType pubsubType(GetKeyedStructType());
    uint32_t payloadSize = static_cast<uint32_t>(pbKeyed.getSerializedSizeProvider(&staticData)());
    SerializedPayload_t stPayload(payloadSize);
    ASSERT_TRUE(pbKeyed.serialize(&staticData, &stPayload));
    ASSERT_TRUE(payloadSize == stPayload.length);

    DynamicPubSubType dynPubSub;
    uint32_t payloadSize2 = static_cast<uint32_t>(dynPubSub.getSerializedSizeProvider(dynData)());
    SerializedPayload_t dynPayload(payloadSize2);
    ASSERT_TRUE(dynPubSub.serialize(dynData, &dynPayload));
    ASSERT_TRUE(payloadSize2 == dynPayload.length);

    DynamicData* dynDataFromStatic = DynamicDataFactory::get_instance()->create_data(GetKeyedStructType());
    ASSERT_TRUE(pubsubType.deserialize(&stPayload, dynDataFromStatic));

    DynamicData* dynDataFromDynamic = DynamicDataFactory::get_instance()->create_data(GetKeyedStructType());
    ASSERT_TRUE(dynPubSub.deserialize(&dynPayload, dynDataFromDynamic));

    ASSERT_TRUE(dynDataFromStatic->equals(dynDataFromDynamic));

    DynamicDataFactory::get_instance()->delete_data(dynData);
    DynamicDataFactory::get_instance()->delete_data(dynDataFromStatic);
    DynamicDataFactory::get_instance()->delete_data(dynDataFromDynamic);
   }

 */

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
