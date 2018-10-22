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
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/log/Log.h>

#include "idl/Test.h"
#include "idl/TestPubSubTypes.h"
#include "idl/TestTypeObject.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::types;

class DynamicComplexTypesTests: public ::testing::Test
{
    public:
        DynamicComplexTypesTests()
        {
            CompleteStruct toRegisterStatic;
            m_factory = DynamicTypeBuilderFactory::GetInstance();
            init();
        }

        ~DynamicComplexTypesTests()
        {
            m_DynAutoType = nullptr;
            //DynamicDataFactory::GetInstance()->DeleteData(m_DynAuto);

            m_DynManualType = nullptr;
            //DynamicDataFactory::GetInstance()->DeleteData(m_DynManual);

            if (!DynamicTypeBuilderFactory::GetInstance()->IsEmpty())
            {
                logError(DYN_TEST, "DynamicTypeBuilderFactory is not empty.");
            }

            if (!DynamicDataFactory::GetInstance()->IsEmpty())
            {
                logError(DYN_TEST, "DynamicDataFactory is not empty.");
            }

            DynamicDataFactory::DeleteInstance();
            DynamicTypeBuilderFactory::DeleteInstance();
            TypeObjectFactory::DeleteInstance();

            Log::KillThread();
        }

        virtual void TearDown()
        {
        }

        void init();

        types::DynamicType_ptr GetMyEnumType();
        types::DynamicType_ptr GetMyAliasEnumType();
        types::DynamicType_ptr GetMyAliasEnum2Type();
        types::DynamicType_ptr GetMyAliasEnum3Type();
        types::DynamicType_ptr GetMyOctetArray500Type();
        types::DynamicType_ptr GetBSAlias5Type();
        types::DynamicType_ptr GetMA3Type();
        types::DynamicType_ptr GetMyMiniArrayType();
        types::DynamicType_ptr GetMySequenceLongType();
        types::DynamicType_ptr GetBasicStructType();
        types::DynamicType_ptr GetComplexStructType();
        types::DynamicType_ptr GetUnionSwitchType();
        types::DynamicType_ptr GetUnion2SwitchType();
        types::DynamicType_ptr GetCompleteStructType();
        types::DynamicType_ptr GetKeyedStructType();

        // Static types
        //CompleteStruct m_Static;
        CompleteStructPubSubType m_StaticType;
        // Dynamic Types
        //DynamicData* m_DynAuto;
        types::DynamicType_ptr m_DynAutoType;
        //DynamicData* m_DynManual;
        types::DynamicType_ptr m_DynManualType;
        DynamicTypeBuilderFactory* m_factory;

    private:
        types::DynamicType_ptr m_MyEnumType;
        types::DynamicType_ptr m_MyAliasEnumType;
        types::DynamicType_ptr m_MyAliasEnum2Type;
        types::DynamicType_ptr m_MyAliasEnum3Type;
        types::DynamicType_ptr m_MyOctetArray500;
        types::DynamicType_ptr m_BSAlias5;
        types::DynamicType_ptr m_MA3;
        types::DynamicType_ptr m_MyMiniArray;
        types::DynamicType_ptr m_MySequenceLong;
        types::DynamicType_ptr m_BasicStructType;
        types::DynamicType_ptr m_ComplexStructType;
        types::DynamicType_ptr m_UnionSwitchType;
        types::DynamicType_ptr m_Union2SwitchType;
        types::DynamicType_ptr m_CompleteStructType;
        types::DynamicType_ptr m_KeyedStructType;
};

/*

struct KeyedStruct
{
    @Key octet key;
    BasicStruct basic;
};
 */
types::DynamicType_ptr DynamicComplexTypesTests::GetKeyedStructType()
{
    if (m_KeyedStructType.get() == nullptr)
    {
        DynamicTypeBuilder_ptr keyedStruct_builder = m_factory->CreateStructBuilder();
        DynamicTypeBuilder_ptr octet_builder = m_factory->CreateByteBuilder();
        octet_builder->ApplyAnnotation("@Key", "true");
        keyedStruct_builder->AddMember(0, "key", octet_builder->Build());
        keyedStruct_builder->AddMember(1, "basic", GetBasicStructType());
        keyedStruct_builder->ApplyAnnotation("@Key", "true");
        keyedStruct_builder->SetName("KeyedStruct");
        m_KeyedStructType = keyedStruct_builder->Build();
    }

    return m_KeyedStructType;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetMyEnumType()
{
    if (m_MyEnumType.get() == nullptr)
    {
        DynamicTypeBuilder_ptr myEnum_builder = m_factory->CreateEnumBuilder();
        myEnum_builder->SetName("MyEnum");
        myEnum_builder->AddEmptyMember(0, "A");
        myEnum_builder->AddEmptyMember(1, "B");
        myEnum_builder->AddEmptyMember(2, "C");
        m_MyEnumType = myEnum_builder->Build();
    }

    return m_MyEnumType;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetMyAliasEnumType()
{
    if (m_MyAliasEnumType.get() == nullptr)
    {
        DynamicTypeBuilder_ptr myAliasEnum_builder = m_factory->CreateAliasBuilder(GetMyEnumType(), "MyAliasEnum");
        m_MyAliasEnumType = myAliasEnum_builder->Build();
    }

    return m_MyAliasEnumType;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetMyAliasEnum2Type()
{
    if (m_MyAliasEnum2Type.get() == nullptr)
    {
        DynamicTypeBuilder_ptr myAliasEnum2_builder = m_factory->CreateAliasBuilder(GetMyAliasEnumType(), "MyAliasEnum2");
        m_MyAliasEnum2Type = myAliasEnum2_builder->Build();
    }

    return m_MyAliasEnum2Type;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetMyAliasEnum3Type()
{
    if (m_MyAliasEnum3Type.get() == nullptr)
    {
        DynamicTypeBuilder_ptr myAliasEnum3_builder = m_factory->CreateAliasBuilder(GetMyAliasEnum2Type(), "MyAliasEnum3");
        m_MyAliasEnum3Type = myAliasEnum3_builder->Build();
    }

    return m_MyAliasEnum3Type;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetMyOctetArray500Type()
{
    if (m_MyOctetArray500.get() == nullptr)
    {
        DynamicTypeBuilder_ptr octet_builder = m_factory->CreateByteBuilder();
        DynamicTypeBuilder_ptr myOctetArray500_builder = m_factory->CreateArrayBuilder(octet_builder.get(), { 500 });
        myOctetArray500_builder->SetName("MyOctetArray500");
        m_MyOctetArray500 = myOctetArray500_builder->Build();
    }

    return m_MyOctetArray500;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetBSAlias5Type()
{
    if (m_BSAlias5.get() == nullptr)
    {
        DynamicTypeBuilder_ptr bSAlias5_builder = m_factory->CreateArrayBuilder(GetBasicStructType(), { 5 });
        bSAlias5_builder->SetName("BSAlias5");
        m_BSAlias5 = bSAlias5_builder->Build();
    }

    return m_BSAlias5;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetMA3Type()
{
    if (m_MA3.get() == nullptr)
    {
        DynamicTypeBuilder_ptr mA3_builder = m_factory->CreateArrayBuilder(GetMyAliasEnum3Type(), { 42 });
        mA3_builder->SetName("MA3");
        m_MA3 = mA3_builder->Build();
    }

    return m_MA3;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetMyMiniArrayType()
{
    if (m_MyMiniArray.get() == nullptr)
    {
        DynamicTypeBuilder_ptr int32_builder = m_factory->CreateInt32Builder();
        DynamicTypeBuilder_ptr myMiniArray_builder = m_factory->CreateArrayBuilder(int32_builder.get(), { 2 });
        myMiniArray_builder->SetName("MyMiniArray");
        m_MyMiniArray = myMiniArray_builder->Build();
    }

    return m_MyMiniArray;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetMySequenceLongType()
{
    if (m_MySequenceLong.get() == nullptr)
    {
        DynamicTypeBuilder_ptr int32_builder = m_factory->CreateInt32Builder();
        DynamicTypeBuilder_ptr seqLong_builder = m_factory->CreateSequenceBuilder(int32_builder.get());
        DynamicTypeBuilder_ptr mySequenceLong_builder = m_factory->CreateAliasBuilder(seqLong_builder.get(), "MySequenceLong");
        m_MySequenceLong = mySequenceLong_builder->Build();
    }

    return m_MySequenceLong;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetBasicStructType()
{
    if (m_BasicStructType.get() == nullptr)
    {
        // Members
        DynamicTypeBuilder_ptr bool_builder = m_factory->CreateBoolBuilder();
        DynamicTypeBuilder_ptr octet_builder = m_factory->CreateByteBuilder();
        DynamicTypeBuilder_ptr int16_builder = m_factory->CreateInt16Builder();
        DynamicTypeBuilder_ptr int32_builder = m_factory->CreateInt32Builder();
        DynamicTypeBuilder_ptr int64_builder = m_factory->CreateInt64Builder();
        DynamicTypeBuilder_ptr uint16_builder = m_factory->CreateUint16Builder();
        DynamicTypeBuilder_ptr uint32_builder = m_factory->CreateUint32Builder();
        DynamicTypeBuilder_ptr uint64_builder = m_factory->CreateUint64Builder();
        DynamicTypeBuilder_ptr float_builder = m_factory->CreateFloat32Builder();
        DynamicTypeBuilder_ptr double_builder = m_factory->CreateFloat64Builder();
        DynamicTypeBuilder_ptr ldouble_builder = m_factory->CreateFloat128Builder();
        DynamicTypeBuilder_ptr char_builder = m_factory->CreateChar8Builder();
        DynamicTypeBuilder_ptr wchar_builder = m_factory->CreateChar16Builder();
        DynamicTypeBuilder_ptr string_builder = m_factory->CreateStringBuilder();
        DynamicTypeBuilder_ptr wstring_builder = m_factory->CreateWstringBuilder();
        DynamicTypeBuilder_ptr basicStruct_builder = m_factory->CreateStructBuilder();

        // Add members to the struct.
        int idx = 0;
        basicStruct_builder->AddMember(idx++, "my_bool", bool_builder.get());
        basicStruct_builder->AddMember(idx++, "my_octet", octet_builder.get());
        basicStruct_builder->AddMember(idx++, "my_int16", int16_builder.get());
        basicStruct_builder->AddMember(idx++, "my_int32", int32_builder.get());
        basicStruct_builder->AddMember(idx++, "my_int64", int64_builder.get());
        basicStruct_builder->AddMember(idx++, "my_uint16", uint16_builder.get());
        basicStruct_builder->AddMember(idx++, "my_uint32", uint32_builder.get());
        basicStruct_builder->AddMember(idx++, "my_uint64", uint64_builder.get());
        basicStruct_builder->AddMember(idx++, "my_float32", float_builder.get());
        basicStruct_builder->AddMember(idx++, "my_float64", double_builder.get());
        basicStruct_builder->AddMember(idx++, "my_float128", ldouble_builder.get());
        basicStruct_builder->AddMember(idx++, "my_char", char_builder.get());
        basicStruct_builder->AddMember(idx++, "my_wchar", wchar_builder.get());
        basicStruct_builder->AddMember(idx++, "my_string", string_builder.get());
        basicStruct_builder->AddMember(idx++, "my_wstring", wstring_builder.get());
        basicStruct_builder->SetName("BasicStruct");

        m_BasicStructType = basicStruct_builder->Build();
    }

    return m_BasicStructType;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetComplexStructType()
{
    if (m_ComplexStructType.get() == nullptr)
    {
        // Members (auxiliar types are tab)
        DynamicTypeBuilder_ptr octet_builder = m_factory->CreateByteBuilder();
        DynamicTypeBuilder_ptr my_sequence_octet_builder = m_factory->CreateSequenceBuilder(octet_builder.get(), 55);
        DynamicTypeBuilder_ptr my_sequence_struct_builder = m_factory->CreateSequenceBuilder(GetBasicStructType());
        DynamicTypeBuilder_ptr char_builder = m_factory->CreateChar8Builder();
        DynamicTypeBuilder_ptr byte_builder = m_factory->CreateByteBuilder();
        DynamicTypeBuilder_ptr my_array_octet_builder = m_factory->CreateArrayBuilder(byte_builder.get(), { 500, 5, 4 });
        // MyOctetArray500 is already created
            // We reuse the bounds... { 5 }
        DynamicTypeBuilder_ptr my_array_struct_builder = m_factory->CreateArrayBuilder(GetBasicStructType(), { 5 });
        DynamicTypeBuilder_ptr int16_builder = m_factory->CreateInt16Builder();
        DynamicTypeBuilder_ptr my_map_octet_short_builder = m_factory->CreateMapBuilder(octet_builder.get(), int16_builder.get());
        DynamicTypeBuilder_ptr int32_builder = m_factory->CreateInt32Builder();
        DynamicTypeBuilder_ptr my_map_long_struct_builder = m_factory->CreateMapBuilder(int32_builder.get()->Build(), GetBasicStructType());
        DynamicTypeBuilder_ptr seqOctet_builder = m_factory->CreateSequenceBuilder(octet_builder.get());
        DynamicTypeBuilder_ptr seqSeqOctet_builder = m_factory->CreateSequenceBuilder(seqOctet_builder.get());
        DynamicTypeBuilder_ptr my_map_long_seq_octet_builder = m_factory->CreateMapBuilder(int32_builder.get(), seqSeqOctet_builder.get());
        DynamicTypeBuilder_ptr my_map_long_octet_array_500_builder = m_factory->CreateMapBuilder(int32_builder.get()->Build(), GetMyOctetArray500Type());
        DynamicTypeBuilder_ptr map_octet_bsalias5_builder = m_factory->CreateMapBuilder(octet_builder.get()->Build(), GetBSAlias5Type());
        DynamicTypeBuilder_ptr my_map_long_lol_type_builder = m_factory->CreateMapBuilder(int32_builder.get(), map_octet_bsalias5_builder.get());
        DynamicTypeBuilder_ptr my_small_string_8_builder = m_factory->CreateStringBuilder(128);
        DynamicTypeBuilder_ptr my_small_string_16_builder = m_factory->CreateWstringBuilder(64);
        DynamicTypeBuilder_ptr my_large_string_8_builder = m_factory->CreateStringBuilder(500);
        DynamicTypeBuilder_ptr my_large_string_16_builder = m_factory->CreateWstringBuilder(1024);
        DynamicTypeBuilder_ptr string75_8_builder = m_factory->CreateStringBuilder(75);
        DynamicTypeBuilder_ptr my_array_string_builder = m_factory->CreateArrayBuilder(string75_8_builder.get(), { 5, 5 });

        // MA3 is already defined.
        // { 5 } being reused
        DynamicTypeBuilder_ptr my_array_arrays_builder = m_factory->CreateArrayBuilder(GetMyMiniArrayType(), { 5 });
        DynamicTypeBuilder_ptr my_sequences_array_builder = m_factory->CreateArrayBuilder(GetMySequenceLongType(), { 23 });
        DynamicTypeBuilder_ptr complexStruct_builder = m_factory->CreateStructBuilder();

            // Add members to the struct.
            int idx = 0;
            complexStruct_builder->AddMember(idx++, "my_octet", octet_builder.get());
            complexStruct_builder->AddMember(idx++, "my_basic_struct", GetBasicStructType());
            complexStruct_builder->AddMember(idx++, "my_alias_enum", GetMyAliasEnumType());
            complexStruct_builder->AddMember(idx++, "my_enum", GetMyEnumType());
            complexStruct_builder->AddMember(idx++, "my_sequence_octet", my_sequence_octet_builder.get());
            complexStruct_builder->AddMember(idx++, "my_sequence_struct", my_sequence_struct_builder.get());
            complexStruct_builder->AddMember(idx++, "my_array_octet", my_array_octet_builder.get());
            complexStruct_builder->AddMember(idx++, "my_octet_array_500", GetMyOctetArray500Type());
            complexStruct_builder->AddMember(idx++, "my_array_struct", my_array_struct_builder.get());
            complexStruct_builder->AddMember(idx++, "my_map_octet_short", my_map_octet_short_builder.get());
            complexStruct_builder->AddMember(idx++, "my_map_long_struct", my_map_long_struct_builder.get());
            complexStruct_builder->AddMember(idx++, "my_map_long_seq_octet", my_map_long_seq_octet_builder.get());
            complexStruct_builder->AddMember(idx++, "my_map_long_octet_array_500", my_map_long_octet_array_500_builder.get());
            complexStruct_builder->AddMember(idx++, "my_map_long_lol_type", my_map_long_lol_type_builder.get());
            complexStruct_builder->AddMember(idx++, "my_small_string_8", my_small_string_8_builder.get());
            complexStruct_builder->AddMember(idx++, "my_small_string_16", my_small_string_16_builder.get());
            complexStruct_builder->AddMember(idx++, "my_large_string_8", my_large_string_8_builder.get());
            complexStruct_builder->AddMember(idx++, "my_large_string_16", my_large_string_16_builder.get());
            complexStruct_builder->AddMember(idx++, "my_array_string", my_array_string_builder.get());
            complexStruct_builder->AddMember(idx++, "multi_alias_array_42", GetMA3Type());
            complexStruct_builder->AddMember(idx++, "my_array_arrays", my_array_arrays_builder.get());
            complexStruct_builder->AddMember(idx++, "my_sequences_array", my_sequences_array_builder.get());
        complexStruct_builder->SetName("ComplexStruct");
        m_ComplexStructType = complexStruct_builder->Build();
    }

    return m_ComplexStructType;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetUnionSwitchType()
{
    if (m_UnionSwitchType.get() == nullptr)
    {
        DynamicTypeBuilder_ptr myUnion_builder = m_factory->CreateUnionBuilder(GetMyEnumType());
        myUnion_builder->AddMember(0, "basic", GetBasicStructType(), "A", { 0 }, false);
        myUnion_builder->AddMember(1, "complex", GetComplexStructType(), "B", { 1, 2 }, false);
        myUnion_builder->SetName("MyUnion");
        m_UnionSwitchType = myUnion_builder->Build();
    }

    return m_UnionSwitchType;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetUnion2SwitchType()
{
    if (m_Union2SwitchType.get() == nullptr)
    {
        DynamicTypeBuilder_ptr octet_builder = m_factory->CreateByteBuilder();
        DynamicTypeBuilder_ptr int32_builder = m_factory->CreateInt32Builder();
        DynamicTypeBuilder_ptr string_builder = m_factory->CreateStringBuilder();
        DynamicTypeBuilder_ptr myUnion2_builder = m_factory->CreateUnionBuilder(octet_builder.get());
        myUnion2_builder->AddMember(0, "uno", int32_builder.get(), "0", { 0 }, false);
        myUnion2_builder->AddMember(1, "imString", string_builder.get(), "1", { 1 }, false);
        myUnion2_builder->AddMember(2, "tres", int32_builder.get(), "2", { 2 }, false);
        myUnion2_builder->SetName("MyUnion2");
        m_Union2SwitchType = myUnion2_builder->Build();
    }

    return m_Union2SwitchType;
}

types::DynamicType_ptr DynamicComplexTypesTests::GetCompleteStructType()
{
    if (m_CompleteStructType.get() == nullptr)
    {
        DynamicTypeBuilder_ptr completeStruct_builder = m_factory->CreateStructBuilder();
        // Add members to the struct.
        int idx = 0;
        completeStruct_builder->AddMember(idx++, "my_union", GetUnionSwitchType());
        completeStruct_builder->AddMember(idx++, "my_union_2", GetUnion2SwitchType());
        completeStruct_builder->SetName("CompleteStruct");
        m_CompleteStructType = completeStruct_builder->Build();
    }

    return m_CompleteStructType;
}

void DynamicComplexTypesTests::init()
{
    const TypeIdentifier *id = TypeObjectFactory::GetInstance()->GetTypeIdentifier("CompleteStruct", true);
    const TypeObject *obj = TypeObjectFactory::GetInstance()->GetTypeObject(id);
    m_DynAutoType = TypeObjectFactory::GetInstance()->BuildDynamicType("CompleteStruct", id, obj);

    m_DynManualType = GetCompleteStructType();
}

TEST_F(DynamicComplexTypesTests, Static_Manual_Comparison)
{
    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    types::DynamicData_ptr dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);
    types::DynamicData_ptr dynData2 = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);
    ASSERT_TRUE(dynData2->Equals(dynData.get()));

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
    ASSERT_TRUE(dynData2->Equals(dynData.get()));
}

TEST_F(DynamicComplexTypesTests, Manual_Auto_Comparision)
{
    types::DynamicData* dynAutoData = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);
    types::DynamicData* dynManualData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);

    ASSERT_TRUE(dynManualData->Equals(dynAutoData));

    DynamicDataFactory::GetInstance()->DeleteData(dynAutoData);
    DynamicDataFactory::GetInstance()->DeleteData(dynManualData);
}

TEST_F(DynamicComplexTypesTests, Static_Auto_Comparision)
{
    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubtype(m_DynAutoType);
    types::DynamicData_ptr dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);
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

    types::DynamicData_ptr dynData2 = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);
    ASSERT_TRUE(pubsubtype.deserialize(&payload2, dynData2.get()));

    ASSERT_TRUE(dynData2->Equals(dynData.get()));
}

TEST_F(DynamicComplexTypesTests, Conversions_Test)
{
    TypeObject newObject;
    DynamicTypeBuilderFactory::GetInstance()->BuildTypeObject(m_DynManualType, newObject, true);

    const TypeIdentifier* identifier = TypeObjectFactory::GetInstance()->GetTypeIdentifier(m_DynManualType->GetName(),
        true);
    DynamicType_ptr newAutoType = TypeObjectFactory::GetInstance()->BuildDynamicType(m_DynManualType->GetName(),
        identifier, &newObject);
    types::DynamicData* dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);
    types::DynamicData* dynData2 = DynamicDataFactory::GetInstance()->CreateData(newAutoType);

    ASSERT_TRUE(dynData2->Equals(dynData));

    DynamicDataFactory::GetInstance()->DeleteData(dynData);
    DynamicDataFactory::GetInstance()->DeleteData(dynData2);
}

TEST_F(DynamicComplexTypesTests, DynamicDiscoveryTest)
{
    TypeObject typeObject1, typeObject2, typeObject3;
    DynamicTypeBuilder_ptr type1, type2, type3;
    {
        type1 = DynamicTypeBuilderFactory::GetInstance()->CreateUint16Builder();
        //types::DynamicData_ptr data = DynamicDataFactory::GetInstance()->CreateData(type1);
        DynamicTypeBuilderFactory::GetInstance()->BuildTypeObject(type1->getTypeDescriptor(), typeObject1);
    }
    {
        type2 = DynamicTypeBuilderFactory::GetInstance()->CreateInt16Builder();
        //types::DynamicData_ptr data2 = DynamicDataFactory::GetInstance()->CreateData(type2);
        DynamicTypeBuilderFactory::GetInstance()->BuildTypeObject(type2->getTypeDescriptor(), typeObject2);
    }

    {
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateInt16Builder();
        //types::DynamicData_ptr data2 = DynamicDataFactory::GetInstance()->CreateData(type3);
        DynamicTypeBuilderFactory::GetInstance()->BuildTypeObject(type3->getTypeDescriptor(), typeObject3);
    }

    const TypeIdentifier* identifier1 = TypeObjectFactory::GetInstance()->GetTypeIdentifier(type1->GetName());
    const TypeIdentifier* identifier2 = TypeObjectFactory::GetInstance()->GetTypeIdentifier(type2->GetName());
    const TypeIdentifier* identifier3 = TypeObjectFactory::GetInstance()->GetTypeIdentifier(type3->GetName());
    ASSERT_FALSE(*identifier1 == *identifier2);
    ASSERT_FALSE(*identifier1 == *identifier3);
    ASSERT_TRUE(*identifier2 == *identifier3);
}

TEST_F(DynamicComplexTypesTests, Data_Comparison_A_A)
{
    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    types::DynamicData_ptr dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);
    types::DynamicData_ptr dynDataFromStatic = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);

    CompleteStruct staticData;
    staticData.my_union()._d(A);
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
    staticData.my_union().basic().my_wchar(L'G');
    staticData.my_union().basic().my_string("Luis@eProsima");
    staticData.my_union().basic().my_wstring(L"LuisGasco@eProsima");

    //staticData.my_union_2()._d(A);
    staticData.my_union_2().uno(156);

    DynamicData *my_union = dynData->LoanValue(dynData->GetMemberIdByName("my_union"));
    DynamicData *basic = my_union->LoanValue(my_union->GetMemberIdByName("basic"));

    basic->SetBoolValue(true, basic->GetMemberIdByName("my_bool"));
    basic->SetByteValue(166, basic->GetMemberIdByName("my_octet"));
    basic->SetInt16Value(-10401, basic->GetMemberIdByName("my_int16"));
    basic->SetInt32Value(5884001, basic->GetMemberIdByName("my_int32"));
    basic->SetInt64Value(884481567, basic->GetMemberIdByName("my_int64"));
    basic->SetUint16Value(250, basic->GetMemberIdByName("my_uint16"));
    basic->SetUint32Value(15884, basic->GetMemberIdByName("my_uint32"));
    basic->SetUint64Value(765241, basic->GetMemberIdByName("my_uint64"));
    basic->SetFloat32Value(158.55f, basic->GetMemberIdByName("my_float32"));
    basic->SetFloat64Value(765241.58, basic->GetMemberIdByName("my_float64"));
    basic->SetFloat128Value(765241878.154874, basic->GetMemberIdByName("my_float128"));
    basic->SetChar8Value('L', basic->GetMemberIdByName("my_char"));
    basic->SetChar16Value(L'G', basic->GetMemberIdByName("my_wchar"));
    basic->SetStringValue("Luis@eProsima", basic->GetMemberIdByName("my_string"));
    basic->SetWstringValue(L"LuisGasco@eProsima", basic->GetMemberIdByName("my_wstring"));

    my_union->ReturnLoanedValue(basic);
    dynData->ReturnLoanedValue(my_union);

    DynamicData *my_union_2 = dynData->LoanValue(dynData->GetMemberIdByName("my_union_2"));
    my_union_2->SetInt32Value(156, my_union_2->GetMemberIdByName("uno"));


    dynData->ReturnLoanedValue(my_union_2);

    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);

    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    ASSERT_TRUE(payloadSize == payloadSize2);

    CompleteStructPubSubType pbComplete;
    ASSERT_TRUE(pbComplete.serialize(&staticData, &payload));
    ASSERT_TRUE(pubsubType.deserialize(&payload, dynDataFromStatic.get()));

    ASSERT_TRUE(dynDataFromStatic->Equals(dynData.get()));
}

TEST_F(DynamicComplexTypesTests, Data_Comparison_A_B)
{
    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    types::DynamicData_ptr dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);
    types::DynamicData_ptr dynDataFromStatic = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);

    CompleteStruct staticData;
    staticData.my_union()._d(A);
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
    staticData.my_union().basic().my_wchar(L'G');
    staticData.my_union().basic().my_string("Luis@eProsima");
    staticData.my_union().basic().my_wstring(L"LuisGasco@eProsima");

    staticData.my_union_2().imString("JuanCarlosArcereredekljnjkds");

    DynamicData *my_union = dynData->LoanValue(dynData->GetMemberIdByName("my_union"));
    DynamicData *basic = my_union->LoanValue(my_union->GetMemberIdByName("basic"));

    basic->SetBoolValue(true, basic->GetMemberIdByName("my_bool"));
    basic->SetByteValue(166, basic->GetMemberIdByName("my_octet"));
    basic->SetInt16Value(-10401, basic->GetMemberIdByName("my_int16"));
    basic->SetInt32Value(5884001, basic->GetMemberIdByName("my_int32"));
    basic->SetInt64Value(884481567, basic->GetMemberIdByName("my_int64"));
    basic->SetUint16Value(250, basic->GetMemberIdByName("my_uint16"));
    basic->SetUint32Value(15884, basic->GetMemberIdByName("my_uint32"));
    basic->SetUint64Value(765241, basic->GetMemberIdByName("my_uint64"));
    basic->SetFloat32Value(158.55f, basic->GetMemberIdByName("my_float32"));
    basic->SetFloat64Value(765241.58, basic->GetMemberIdByName("my_float64"));
    basic->SetFloat128Value(765241878.154874, basic->GetMemberIdByName("my_float128"));
    basic->SetChar8Value('L', basic->GetMemberIdByName("my_char"));
    basic->SetChar16Value(L'G', basic->GetMemberIdByName("my_wchar"));
    basic->SetStringValue("Luis@eProsima", basic->GetMemberIdByName("my_string"));
    basic->SetWstringValue(L"LuisGasco@eProsima", basic->GetMemberIdByName("my_wstring"));

    my_union->ReturnLoanedValue(basic);
    dynData->ReturnLoanedValue(my_union);

    DynamicData *my_union_2 = dynData->LoanValue(dynData->GetMemberIdByName("my_union_2"));
    my_union_2->SetStringValue("JuanCarlosArcereredekljnjkds", my_union_2->GetMemberIdByName("imString"));


    dynData->ReturnLoanedValue(my_union_2);

    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(dynData.get(), &payload));

    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    ASSERT_TRUE(payloadSize == payloadSize2);

    CompleteStructPubSubType pbComplete;
    ASSERT_TRUE(pbComplete.serialize(&staticData, &payload));
    ASSERT_TRUE(pubsubType.deserialize(&payload, dynDataFromStatic.get()));

    ASSERT_TRUE(dynDataFromStatic->Equals(dynData.get()));
}

TEST_F(DynamicComplexTypesTests, Data_Comparison_A_C)
{
    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    types::DynamicData_ptr dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);
    types::DynamicData_ptr dynDataFromStatic = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);

    CompleteStruct staticData;
    staticData.my_union()._d(A);
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
    staticData.my_union().basic().my_wchar(L'G');
    staticData.my_union().basic().my_string("Luis@eProsima");
    staticData.my_union().basic().my_wstring(L"LuisGasco@eProsima");

    //staticData.my_union_2()._d(A);
    staticData.my_union_2().tres(333);

    DynamicData *my_union = dynData->LoanValue(dynData->GetMemberIdByName("my_union"));
    DynamicData *basic = my_union->LoanValue(my_union->GetMemberIdByName("basic"));

    basic->SetBoolValue(true, basic->GetMemberIdByName("my_bool"));
    basic->SetByteValue(166, basic->GetMemberIdByName("my_octet"));
    basic->SetInt16Value(-10401, basic->GetMemberIdByName("my_int16"));
    basic->SetInt32Value(5884001, basic->GetMemberIdByName("my_int32"));
    basic->SetInt64Value(884481567, basic->GetMemberIdByName("my_int64"));
    basic->SetUint16Value(250, basic->GetMemberIdByName("my_uint16"));
    basic->SetUint32Value(15884, basic->GetMemberIdByName("my_uint32"));
    basic->SetUint64Value(765241, basic->GetMemberIdByName("my_uint64"));
    basic->SetFloat32Value(158.55f, basic->GetMemberIdByName("my_float32"));
    basic->SetFloat64Value(765241.58, basic->GetMemberIdByName("my_float64"));
    basic->SetFloat128Value(765241878.154874, basic->GetMemberIdByName("my_float128"));
    basic->SetChar8Value('L', basic->GetMemberIdByName("my_char"));
    basic->SetChar16Value(L'G', basic->GetMemberIdByName("my_wchar"));
    basic->SetStringValue("Luis@eProsima", basic->GetMemberIdByName("my_string"));
    basic->SetWstringValue(L"LuisGasco@eProsima", basic->GetMemberIdByName("my_wstring"));

    my_union->ReturnLoanedValue(basic);
    dynData->ReturnLoanedValue(my_union);

    DynamicData *my_union_2 = dynData->LoanValue(dynData->GetMemberIdByName("my_union_2"));
    my_union_2->SetInt32Value(333, my_union_2->GetMemberIdByName("tres"));

    dynData->ReturnLoanedValue(my_union_2);

    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);

    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    ASSERT_TRUE(payloadSize == payloadSize2);

    CompleteStructPubSubType pbComplete;
    ASSERT_TRUE(pbComplete.serialize(&staticData, &payload));
    ASSERT_TRUE(pubsubType.deserialize(&payload, dynDataFromStatic.get()));

    ASSERT_TRUE(dynDataFromStatic->Equals(dynData.get()));
}

TEST_F(DynamicComplexTypesTests, Data_Comparison_B_A)
{
    types::DynamicData_ptr dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);

    CompleteStruct staticData;
    staticData.my_union()._d() = B;
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
    staticData.my_union().complex().my_basic_struct().my_wchar(L'G');
    staticData.my_union().complex().my_basic_struct().my_string("Luis@eProsima");
    staticData.my_union().complex().my_basic_struct().my_wstring(L"LuisGasco@eProsima");

    staticData.my_union().complex().my_alias_enum(C);
    staticData.my_union().complex().my_enum(B);
    staticData.my_union().complex().my_sequence_octet().push_back(88);
    staticData.my_union().complex().my_sequence_octet().push_back(99);
    staticData.my_union().complex().my_sequence_struct().push_back(staticData.my_union().complex().my_basic_struct());
    for (int i = 0; i < 500; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                staticData.my_union().complex().my_array_octet()[i][j][k] = static_cast<uint8_t>(j*k);
            }
        }
    }

    for (int i = 0; i < 5; ++i)
    {
        staticData.my_union().complex().my_array_struct()[i].my_bool(i % 2 == 1);
        staticData.my_union().complex().my_array_struct()[i].my_octet(static_cast<uint8_t>(i));
        staticData.my_union().complex().my_array_struct()[i].my_int16(static_cast<int16_t>(-i));
        staticData.my_union().complex().my_array_struct()[i].my_int32(i);
        staticData.my_union().complex().my_array_struct()[i].my_int64(i*1000);
        staticData.my_union().complex().my_array_struct()[i].my_uint16(static_cast<uint16_t>(i));
        staticData.my_union().complex().my_array_struct()[i].my_uint32(i);
        staticData.my_union().complex().my_array_struct()[i].my_uint64(i*10005);
        staticData.my_union().complex().my_array_struct()[i].my_float32(i*5.5f);
        staticData.my_union().complex().my_array_struct()[i].my_float64(i*8.8);
        staticData.my_union().complex().my_array_struct()[i].my_float128(i*10.0);
        staticData.my_union().complex().my_array_struct()[i].my_char('J');
        staticData.my_union().complex().my_array_struct()[i].my_wchar(L'C');
        staticData.my_union().complex().my_array_struct()[i].my_string("JC@eProsima");
        staticData.my_union().complex().my_array_struct()[i].my_wstring(L"JC-BOOM-Armadilo!@eProsima");
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
        staticData.my_union().complex().my_map_long_octet_array_500()[0][i] = i%256;
        staticData.my_union().complex().my_map_long_octet_array_500()[10][i] = (i+55)%256;
    }

    staticData.my_union().complex().my_small_string_8("Bv7EMffURwGNqePoujdSfkF9PXN9TH125X5nGpNLfzya53tZtNJdgMROlYdZnTE1SLWzBdIU7ZyjjGvsGHkmuJUROwVPcNa9q5dRUV3KZAKNx1exL7BjhqIgQFconhd");
    staticData.my_union().complex().my_small_string_16(L"AgzñgXsI9pXbWjYLDvvn8JUFWhxZhk9t92rdsTqylvdpqtXA6hy9dHkoBTgmF2c");
    staticData.my_union().complex().my_large_string_8("hYE5vjcLJe6ML5DmoqQwh9ns866dAbnjkVKIKu2VF6lbkvh91ZOG2enEcdoRa8T43hR0Ym0k7tI621EQGufvzmLqxKCPgiXSp2zUTTmIWtn4fM8tC3aP1Yd0dKvn0tDobyp6p3156KvxqG3BKQ6VjFiHlMFoEyz8pjCclhXLl2cfAi97sQzXLUoPYUC5BWKyQTrA2JF6HXZM6vrbw5dc3B4AOJNGdPJ9ai6weF43h1RhnXE9MOFxPNoQnJ8gqSXYbMtpG6ZzqhUyoz0XhFDt7EOqXIgvc9SCejQTVMPeRcF5Zy57hrYZiKrCQqFWidS4BdfEAkuwESgBmEpEFOpZotwDt0TGDaLktSt3dKRsURO6TpuZ2nZNdiEJyc597ZjjQXtyKU7OCyRRqllzAnHEtoU3zd3OLTOvT5uk32N1Y64tpUte63De2EMwDNYb2eGAQfATdSt8VcGBOzJQjsmrMwMumtk48JzXXLxjo6s2vl2rNK9WQM1");
    staticData.my_union().complex().my_large_string_16(L"nosYBfFr1s3t8rUsuUrVCWFi6moDk7GULFj6XnkebIDkjl3n2ykKxUIaLj3qNNUx0ny8DvFbdfxZBdMhBNW3fHbKrig4GkHnN1JoEo0ACiPxrARusDs3xKzvaQQrls6lVUFAUXzDOtw5f2CNVJKiruGjXUO2Lq5Mmy8ygW3eUiTlueAHA2dRXXryOFi47jS3DkmBH4aAOKcmR27KhhJnXaY0gWy3XdSnaGQNB3XvbmxQ7xXDsf1wz860WMEKP3VhdOLsmS6tKCb4sshuOlmUSyTggY7vNoxfpG1EUFP5iPro9E0tHLLdHlWf2NwU8OXCYx6KKEbs5pFMvgEstnQglsdTk0lOv6riaFkFOwx83gW1l6Pg4eXjacnJKoVh1pOeZxULLZpCECw8yRZ9z4JPHxh2C7ytkCHMKp9O4MwQwYvvvgWWLWfJgb7Ecy2tgvWLpNDzgkFrEFhaCTKitChlG422CnLSsXvTBNnF52sULH6rcwOVx3mbhqte3ld3fObtAuH3zPzjOF4vVbvUXxgZh1Zx1cey0iGfnhOZHUfUwJ3Qv0WZNcuVLvMMhhg85A3620b84MAIc2UoW9Hl4BIT7pHo41ApF0DxIPJL0QdIdAOjn0JTPZqAhoHVBQoYvivPHftk5Crd1a1J8L7hSs0s4uSQKAMTKDxy3gKLaGAg277h4iEsEZRCI4RPlPTo9nZ48s8OO2KzqrUbMkoPSTgaJEXq8GsozAzh0wtL4P3gPeHO5nQzoytoXAkiXoPph0GaTLiahYQksYeK1eVQADDqZPXC55teXKKdX4aomCufr1ZizgzkGwAmnsFmhmBSF0gvbm56NDaUVT0UqXxKxAfRjkILeWR1mW8jfn6RYJH3IWiHxEfyB23rr78NySfgzIchhrm7jEFtmwPpKPKAwzajLv0HpkrtTr38YwWeT5LzHokFAQEc6l3aWdJWapVyt9wX89dEkmPPG9torCV2ddjyF4jAKsxKvzU4pCxV6B3m16IIdnksemJ0xG8iKh4ZPsX");
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j)
            staticData.my_union().complex().my_array_string()[i][j]="Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42";
    for (int i = 0; i < 42; ++i)
        staticData.my_union().complex().multi_alias_array_42()[i] = (MyEnum)(i%3);

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            staticData.my_union().complex().my_array_arrays()[i][j] = i*j;
        }
    }

    for (int i = 0; i < 23; ++i)
    {
        staticData.my_union().complex().my_sequences_array()[i].push_back(i);
        staticData.my_union().complex().my_sequences_array()[i].push_back(i*10);
        staticData.my_union().complex().my_sequences_array()[i].push_back(i*100);
    }
    staticData.my_union_2()._d(A);
    staticData.my_union_2().uno(156);

    DynamicData *my_union = dynData->LoanValue(dynData->GetMemberIdByName("my_union"));

    DynamicData *complex = my_union->LoanValue(my_union->GetMemberIdByName("complex"));
    complex->SetByteValue(66, complex->GetMemberIdByName("my_octet"));

    DynamicData *basic = complex->LoanValue(complex->GetMemberIdByName("my_basic_struct"));
    basic->SetBoolValue(true, basic->GetMemberIdByName("my_bool"));
    basic->SetByteValue(166, basic->GetMemberIdByName("my_octet"));
    basic->SetInt16Value(-10401, basic->GetMemberIdByName("my_int16"));
    basic->SetInt32Value(5884001, basic->GetMemberIdByName("my_int32"));
    basic->SetInt64Value(884481567, basic->GetMemberIdByName("my_int64"));
    basic->SetUint16Value(250, basic->GetMemberIdByName("my_uint16"));
    basic->SetUint32Value(15884, basic->GetMemberIdByName("my_uint32"));
    basic->SetUint64Value(765241, basic->GetMemberIdByName("my_uint64"));
    basic->SetFloat32Value(158.55f, basic->GetMemberIdByName("my_float32"));
    basic->SetFloat64Value(765241.58, basic->GetMemberIdByName("my_float64"));
    basic->SetFloat128Value(765241878.154874, basic->GetMemberIdByName("my_float128"));
    basic->SetChar8Value('L', basic->GetMemberIdByName("my_char"));
    basic->SetChar16Value(L'G', basic->GetMemberIdByName("my_wchar"));
    basic->SetStringValue("Luis@eProsima", basic->GetMemberIdByName("my_string"));
    basic->SetWstringValue(L"LuisGasco@eProsima", basic->GetMemberIdByName("my_wstring"));
    complex->ReturnLoanedValue(basic);

    complex->SetEnumValue("C", complex->GetMemberIdByName("my_alias_enum"));
    complex->SetEnumValue("B", complex->GetMemberIdByName("my_enum"));

    DynamicData *my_seq_octet = complex->LoanValue(complex->GetMemberIdByName("my_sequence_octet"));
    MemberId id;
    my_seq_octet->InsertSequenceData(id);
    my_seq_octet->SetByteValue(88, id);
    my_seq_octet->InsertSequenceData(id);
    my_seq_octet->SetByteValue(99, id);
    //staticData.my_union().complex().my_sequence_octet().push_back(88);
    //staticData.my_union().complex().my_sequence_octet().push_back(99);
    complex->ReturnLoanedValue(my_seq_octet);

    DynamicData *my_seq_struct = complex->LoanValue(complex->GetMemberIdByName("my_sequence_struct"));
    my_seq_struct->InsertSequenceData(id);
    my_seq_struct->SetComplexValue(DynamicDataFactory::GetInstance()->CreateCopy(basic), id);
    //staticData.my_union().complex().my_sequence_struct().push_back(staticData.my_union().complex().my_basic_struct());
    complex->ReturnLoanedValue(my_seq_struct);

    DynamicData *my_array_octet = complex->LoanValue(complex->GetMemberIdByName("my_array_octet"));
    for (unsigned int i = 0; i < 500; ++i)
    {
        for (unsigned int j = 0; j < 5; ++j)
        {
            for (unsigned int k = 0; k < 4; ++k)
            {
                MemberId array_idx = my_array_octet->GetArrayIndex({ i, j, k });
                my_array_octet->SetByteValue(static_cast<uint8_t>(j*k), array_idx);
            }
        }
        //staticData.my_union().complex().my_array_octet()[i][j][k] = j*k;
    }
    complex->ReturnLoanedValue(my_array_octet);

    DynamicData *my_array_struct = complex->LoanValue(complex->GetMemberIdByName("my_array_struct"));
    for (int i = 0; i < 5; ++i)
    {
        DynamicData *tempBasic = DynamicDataFactory::GetInstance()->CreateData(GetBasicStructType());
        tempBasic->SetBoolValue(i % 2 == 1, tempBasic->GetMemberIdByName("my_bool"));
        tempBasic->SetByteValue(static_cast<uint8_t>(i), tempBasic->GetMemberIdByName("my_octet"));
        tempBasic->SetInt16Value(static_cast<int16_t>(-i), tempBasic->GetMemberIdByName("my_int16"));
        tempBasic->SetInt32Value(i, tempBasic->GetMemberIdByName("my_int32"));
        tempBasic->SetInt64Value(i*1000, tempBasic->GetMemberIdByName("my_int64"));
        tempBasic->SetUint16Value(static_cast<uint16_t>(i), tempBasic->GetMemberIdByName("my_uint16"));
        tempBasic->SetUint32Value(i, tempBasic->GetMemberIdByName("my_uint32"));
        tempBasic->SetUint64Value(i*10005, tempBasic->GetMemberIdByName("my_uint64"));
        tempBasic->SetFloat32Value(i*5.5f, tempBasic->GetMemberIdByName("my_float32"));
        tempBasic->SetFloat64Value(i*8.8, tempBasic->GetMemberIdByName("my_float64"));
        tempBasic->SetFloat128Value(i*10.0, tempBasic->GetMemberIdByName("my_float128"));
        tempBasic->SetChar8Value('J', tempBasic->GetMemberIdByName("my_char"));
        tempBasic->SetChar16Value(L'C', tempBasic->GetMemberIdByName("my_wchar"));
        tempBasic->SetStringValue("JC@eProsima", tempBasic->GetMemberIdByName("my_string"));
        tempBasic->SetWstringValue(L"JC-BOOM-Armadilo!@eProsima", tempBasic->GetMemberIdByName("my_wstring"));
        my_array_struct->SetComplexValue(tempBasic, i);
    }
    complex->ReturnLoanedValue(my_array_struct);

    DynamicTypeBuilder_ptr octet_builder = m_factory->CreateByteBuilder();
    DynamicData_ptr key_oct = DynamicDataFactory::GetInstance()->CreateData(octet_builder->Build());
    MemberId kId;
    MemberId vId;
    MemberId ssId;
    MemberId sId;
    DynamicData *my_map_octet_short = complex->LoanValue(complex->GetMemberIdByName("my_map_octet_short"));
    key_oct->SetByteValue(0);
    my_map_octet_short->InsertMapData(key_oct.get(), kId, vId);
    my_map_octet_short->SetInt16Value((short)1340, vId);
    key_oct = DynamicDataFactory::GetInstance()->CreateData(octet_builder->Build());
    key_oct->SetByteValue(1);
    my_map_octet_short->InsertMapData(key_oct.get(), kId, vId);
    my_map_octet_short->SetInt16Value((short)1341, vId);
    //staticData.my_union().complex().my_map_octet_short()[0] = 1340;
    //staticData.my_union().complex().my_map_octet_short()[1] = 1341;
    complex->ReturnLoanedValue(my_map_octet_short);

    DynamicTypeBuilder_ptr long_builder = m_factory->CreateInt32Builder();
    DynamicData_ptr key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    DynamicData *my_map_long_struct = complex->LoanValue(complex->GetMemberIdByName("my_map_long_struct"));

    //DynamicData *mas3 = my_array_struct->LoanValue(3);
    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(55);
    my_map_long_struct->InsertMapData(key.get(), kId, vId);
    basic = my_map_long_struct->LoanValue(vId);
    basic->SetBoolValue(true, basic->GetMemberIdByName("my_bool"));
    basic->SetByteValue(166, basic->GetMemberIdByName("my_octet"));
    basic->SetInt16Value(-10401, basic->GetMemberIdByName("my_int16"));
    basic->SetInt32Value(5884001, basic->GetMemberIdByName("my_int32"));
    basic->SetInt64Value(884481567, basic->GetMemberIdByName("my_int64"));
    basic->SetUint16Value(250, basic->GetMemberIdByName("my_uint16"));
    basic->SetUint32Value(15884, basic->GetMemberIdByName("my_uint32"));
    basic->SetUint64Value(765241, basic->GetMemberIdByName("my_uint64"));
    basic->SetFloat32Value(158.55f, basic->GetMemberIdByName("my_float32"));
    basic->SetFloat64Value(765241.58, basic->GetMemberIdByName("my_float64"));
    basic->SetFloat128Value(765241878.154874, basic->GetMemberIdByName("my_float128"));
    basic->SetChar8Value('L', basic->GetMemberIdByName("my_char"));
    basic->SetChar16Value(L'G', basic->GetMemberIdByName("my_wchar"));
    basic->SetStringValue("Luis@eProsima", basic->GetMemberIdByName("my_string"));
    basic->SetWstringValue(L"LuisGasco@eProsima", basic->GetMemberIdByName("my_wstring"));
    my_map_long_struct->ReturnLoanedValue(basic);
    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(1000);
    my_map_long_struct->InsertMapData(key.get(), kId, vId);
    DynamicData *mas3 = my_map_long_struct->LoanValue(vId);
    int i = 3;
    mas3->SetBoolValue(i % 2 == 1, mas3->GetMemberIdByName("my_bool"));
    mas3->SetByteValue(static_cast<uint8_t>(i), mas3->GetMemberIdByName("my_octet"));
    mas3->SetInt16Value(static_cast<int16_t>(-i), mas3->GetMemberIdByName("my_int16"));
    mas3->SetInt32Value(i, mas3->GetMemberIdByName("my_int32"));
    mas3->SetInt64Value(i*1000, mas3->GetMemberIdByName("my_int64"));
    mas3->SetUint16Value(static_cast<uint8_t>(i), mas3->GetMemberIdByName("my_uint16"));
    mas3->SetUint32Value(i, mas3->GetMemberIdByName("my_uint32"));
    mas3->SetUint64Value(i*10005, mas3->GetMemberIdByName("my_uint64"));
    mas3->SetFloat32Value(i*5.5f, mas3->GetMemberIdByName("my_float32"));
    mas3->SetFloat64Value(i*8.8, mas3->GetMemberIdByName("my_float64"));
    mas3->SetFloat128Value(i*10.0, mas3->GetMemberIdByName("my_float128"));
    mas3->SetChar8Value('J', mas3->GetMemberIdByName("my_char"));
    mas3->SetChar16Value(L'C', mas3->GetMemberIdByName("my_wchar"));
    mas3->SetStringValue("JC@eProsima", mas3->GetMemberIdByName("my_string"));
    mas3->SetWstringValue(L"JC-BOOM-Armadilo!@eProsima", mas3->GetMemberIdByName("my_wstring"));
    my_map_long_struct->ReturnLoanedValue(mas3);

    // staticData.my_union().complex().my_map_long_struct()[1000] = staticData.my_union().complex().my_array_struct()[3];
    // staticData.my_union().complex().my_map_long_struct()[55] = staticData.my_union().complex().my_basic_struct();
    complex->ReturnLoanedValue(my_map_long_struct);

    DynamicData *my_map_long_seq_octet = complex->LoanValue(complex->GetMemberIdByName("my_map_long_seq_octet"));
    //std::vector my_vector_octet = {1, 2, 3, 4, 5};
    //MemberId id;
    /*DynamicTypeBuilder_ptr octet_builder = m_factory->CreateByteBuilder();
    types::DynamicTypeBuilder_ptr seqOctet_builder = m_factory->CreateSequenceBuilder(octet_builder.get());
    types::DynamicType_ptr seqSeqOctet_builder = m_factory->CreateSequenceBuilder(seqOctet_builder.get())->Build();
    DynamicData *dataSeqOctet = seqOctet_builder->Build();
    DynamicData *dataSeqSeqOctet = seqSeqOctet_builder->Build();
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(1, id);
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(2, id);
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(3, id);
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(4, id);
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(5, id);
    dataSeqSeqOctet->InsertSequenceData(id);
    dataSeqSeqOctet->SetComplexValue(dataSeqOctet, id);*/
    // InsertMapData(DynamicData_ptr key, MemberId& outKeyId, MemberId& outValueId);
    // TODO De la muerte para Juan Carlos - Esto no es NADA práctico...

    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(0);
    my_map_long_seq_octet->InsertMapData(key.get(), kId, vId);

    DynamicData* seq_seq_oct = my_map_long_seq_octet->LoanValue(vId);
    seq_seq_oct->InsertSequenceData(ssId);
    DynamicData* seq_oct = seq_seq_oct->LoanValue(ssId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(1, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(2, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(3, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(4, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(5, sId);
    seq_seq_oct->ReturnLoanedValue(seq_oct);
    my_map_long_seq_octet->ReturnLoanedValue(seq_seq_oct);

    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(55);
    my_map_long_seq_octet->InsertMapData(key.get(), kId, vId);

    seq_seq_oct = my_map_long_seq_octet->LoanValue(vId);
    seq_seq_oct->InsertSequenceData(ssId);
    seq_oct = seq_seq_oct->LoanValue(ssId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(1, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(2, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(3, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(4, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(5, sId);
    seq_seq_oct->ReturnLoanedValue(seq_oct);
    seq_seq_oct->InsertSequenceData(ssId);
    seq_oct = seq_seq_oct->LoanValue(ssId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(1, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(2, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(3, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(4, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(5, sId);
    seq_seq_oct->ReturnLoanedValue(seq_oct);
    my_map_long_seq_octet->ReturnLoanedValue(seq_seq_oct);
    //staticData.my_union().complex().my_map_long_seq_octet()[55].push_back(my_vector_octet);
    //staticData.my_union().complex().my_map_long_seq_octet()[55].push_back(my_vector_octet);
    //staticData.my_union().complex().my_map_long_seq_octet()[0].push_back(my_vector_octet);
    complex->ReturnLoanedValue(my_map_long_seq_octet);

    DynamicData *my_map_long_octet_array_500 =
        complex->LoanValue(complex->GetMemberIdByName("my_map_long_octet_array_500"));

    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(0);
    my_map_long_octet_array_500->InsertMapData(key.get(), kId, vId);

    DynamicData* oct_array_500 = my_map_long_octet_array_500->LoanValue(vId);
    for (int j = 0; j < 500; ++j)
    {
        oct_array_500->SetByteValue(j%256, j);
        //staticData.my_union().complex().my_map_long_octet_array_500()[0][i] = i%256;
    }
    my_map_long_octet_array_500->ReturnLoanedValue(oct_array_500);

    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(10);
    my_map_long_octet_array_500->InsertMapData(key.get(), kId, vId);
    oct_array_500 = my_map_long_octet_array_500->LoanValue(vId);

    for (int j = 0; j < 500; ++j)
    {
        oct_array_500->SetByteValue((j+55)%256, j);
        //staticData.my_union().complex().my_map_long_octet_array_500()[10][i] = (i+55)%256;
    }
    my_map_long_octet_array_500->ReturnLoanedValue(oct_array_500);
    complex->ReturnLoanedValue(my_map_long_octet_array_500);

    complex->SetStringValue("Bv7EMffURwGNqePoujdSfkF9PXN9TH125X5nGpNLfzya53tZtNJdgMROlYdZnTE1SLWzBdIU7ZyjjGvsGHkmuJUROwVPcNa9q5dRUV3KZAKNx1exL7BjhqIgQFconhd", complex->GetMemberIdByName("my_small_string_8"));
    complex->SetWstringValue(L"AgzñgXsI9pXbWjYLDvvn8JUFWhxZhk9t92rdsTqylvdpqtXA6hy9dHkoBTgmF2c", complex->GetMemberIdByName("my_small_string_16"));
    complex->SetStringValue("hYE5vjcLJe6ML5DmoqQwh9ns866dAbnjkVKIKu2VF6lbkvh91ZOG2enEcdoRa8T43hR0Ym0k7tI621EQGufvzmLqxKCPgiXSp2zUTTmIWtn4fM8tC3aP1Yd0dKvn0tDobyp6p3156KvxqG3BKQ6VjFiHlMFoEyz8pjCclhXLl2cfAi97sQzXLUoPYUC5BWKyQTrA2JF6HXZM6vrbw5dc3B4AOJNGdPJ9ai6weF43h1RhnXE9MOFxPNoQnJ8gqSXYbMtpG6ZzqhUyoz0XhFDt7EOqXIgvc9SCejQTVMPeRcF5Zy57hrYZiKrCQqFWidS4BdfEAkuwESgBmEpEFOpZotwDt0TGDaLktSt3dKRsURO6TpuZ2nZNdiEJyc597ZjjQXtyKU7OCyRRqllzAnHEtoU3zd3OLTOvT5uk32N1Y64tpUte63De2EMwDNYb2eGAQfATdSt8VcGBOzJQjsmrMwMumtk48JzXXLxjo6s2vl2rNK9WQM1", complex->GetMemberIdByName("my_large_string_8"));
    complex->SetWstringValue(L"nosYBfFr1s3t8rUsuUrVCWFi6moDk7GULFj6XnkebIDkjl3n2ykKxUIaLj3qNNUx0ny8DvFbdfxZBdMhBNW3fHbKrig4GkHnN1JoEo0ACiPxrARusDs3xKzvaQQrls6lVUFAUXzDOtw5f2CNVJKiruGjXUO2Lq5Mmy8ygW3eUiTlueAHA2dRXXryOFi47jS3DkmBH4aAOKcmR27KhhJnXaY0gWy3XdSnaGQNB3XvbmxQ7xXDsf1wz860WMEKP3VhdOLsmS6tKCb4sshuOlmUSyTggY7vNoxfpG1EUFP5iPro9E0tHLLdHlWf2NwU8OXCYx6KKEbs5pFMvgEstnQglsdTk0lOv6riaFkFOwx83gW1l6Pg4eXjacnJKoVh1pOeZxULLZpCECw8yRZ9z4JPHxh2C7ytkCHMKp9O4MwQwYvvvgWWLWfJgb7Ecy2tgvWLpNDzgkFrEFhaCTKitChlG422CnLSsXvTBNnF52sULH6rcwOVx3mbhqte3ld3fObtAuH3zPzjOF4vVbvUXxgZh1Zx1cey0iGfnhOZHUfUwJ3Qv0WZNcuVLvMMhhg85A3620b84MAIc2UoW9Hl4BIT7pHo41ApF0DxIPJL0QdIdAOjn0JTPZqAhoHVBQoYvivPHftk5Crd1a1J8L7hSs0s4uSQKAMTKDxy3gKLaGAg277h4iEsEZRCI4RPlPTo9nZ48s8OO2KzqrUbMkoPSTgaJEXq8GsozAzh0wtL4P3gPeHO5nQzoytoXAkiXoPph0GaTLiahYQksYeK1eVQADDqZPXC55teXKKdX4aomCufr1ZizgzkGwAmnsFmhmBSF0gvbm56NDaUVT0UqXxKxAfRjkILeWR1mW8jfn6RYJH3IWiHxEfyB23rr78NySfgzIchhrm7jEFtmwPpKPKAwzajLv0HpkrtTr38YwWeT5LzHokFAQEc6l3aWdJWapVyt9wX89dEkmPPG9torCV2ddjyF4jAKsxKvzU4pCxV6B3m16IIdnksemJ0xG8iKh4ZPsX", complex->GetMemberIdByName("my_large_string_16"));

    DynamicData *my_array_string = complex->LoanValue(complex->GetMemberIdByName("my_array_string"));
    for (unsigned int j = 0; j < 5; ++j)
    {
        for (unsigned int k = 0; k < 5; ++k)
        {
            MemberId array_idx = my_array_string->GetArrayIndex({ j, k });
            my_array_string->SetStringValue("Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42", array_idx);
            //staticData.my_union().complex().my_array_string()[i][j]("Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42");
        }
    }
    complex->ReturnLoanedValue(my_array_string);

    DynamicData *multi_alias_array_42 = complex->LoanValue(complex->GetMemberIdByName("multi_alias_array_42"));
    for (int j = 0; j < 42; ++j)
    {
        multi_alias_array_42->SetEnumValue(j % 3, j);
        //staticData.my_union().complex().multi_alias_array_42()[i](i%3);
    }
    complex->ReturnLoanedValue(multi_alias_array_42);

    DynamicData *my_array_arrays = complex->LoanValue(complex->GetMemberIdByName("my_array_arrays"));
    for (unsigned int j = 0; j < 5; ++j)
    {
        DynamicData *myMiniArray = my_array_arrays->LoanValue(j);
        for (unsigned int k = 0; k < 2; ++k)
        {
            myMiniArray->SetInt32Value(j*k, k);
            //staticData.my_union().complex().my_array_arrays()[i][j](i*j);
        }
        my_array_arrays->ReturnLoanedValue(myMiniArray);
    }
    complex->ReturnLoanedValue(my_array_arrays);

    DynamicData *my_sequences_array = complex->LoanValue(complex->GetMemberIdByName("my_sequences_array"));
    for (int j = 0; j < 23; ++j)
    {
        DynamicData *seq = DynamicDataFactory::GetInstance()->CreateData(GetMySequenceLongType());
        seq->InsertSequenceData(id);
        seq->SetInt32Value(j, id);
        seq->InsertSequenceData(id);
        seq->SetInt32Value(j*10, id);
        seq->InsertSequenceData(id);
        seq->SetInt32Value(j*100, id);
        my_sequences_array->SetComplexValue(seq, j);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i*10);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i*100);
    }
    complex->ReturnLoanedValue(my_sequences_array);

    my_union->ReturnLoanedValue(complex);
    dynData->ReturnLoanedValue(my_union);

    DynamicData *my_union_2 = dynData->LoanValue(dynData->GetMemberIdByName("my_union_2"));
    my_union_2->SetInt32Value(156, my_union_2->GetMemberIdByName("uno"));

    dynData->ReturnLoanedValue(my_union_2);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(dynData.get(), &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    /*
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
    */
    CompleteStructPubSubType pbComplete;
    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    SerializedPayload_t stPayload(payloadSize2);
    ASSERT_TRUE(pbComplete.serialize(&staticData, &stPayload));
    ASSERT_TRUE(stPayload.length == payloadSize2);
    /*
    std::cout << "BEGIN" << std::endl;
    for (uint32_t j = 0; j < stPayload.length; j += 100)
    {
        std::cout << std::endl;
        for (uint32_t k = 0; k < 100; k++)
        {
            if (j + k < stPayload.length)
            {
                if ((int)stPayload.data[j + k] == 204)
                {
                    std::cout << 0 << " ";
                }
                else
                {
                    std::cout << (int)stPayload.data[j + k] << " ";
                }
            }
        }
    }
    std::cout << "END" << std::endl;
    */
    types::DynamicData_ptr dynDataFromDynamic = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);
    ASSERT_TRUE(pubsubType.deserialize(&payload, dynDataFromDynamic.get()));

    types::DynamicData_ptr dynDataFromStatic = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);
    ASSERT_TRUE(pubsubType.deserialize(&stPayload, dynDataFromStatic.get()));

    ASSERT_TRUE(dynDataFromStatic->Equals(dynDataFromDynamic.get()));
}

TEST_F(DynamicComplexTypesTests, Data_Comparison_B_B)
{
    types::DynamicData_ptr dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);

    CompleteStruct staticData;
    staticData.my_union()._d() = B;
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
    staticData.my_union().complex().my_basic_struct().my_wchar(L'G');
    staticData.my_union().complex().my_basic_struct().my_string("Luis@eProsima");
    staticData.my_union().complex().my_basic_struct().my_wstring(L"LuisGasco@eProsima");

    staticData.my_union().complex().my_alias_enum(C);
    staticData.my_union().complex().my_enum(B);
    staticData.my_union().complex().my_sequence_octet().push_back(88);
    staticData.my_union().complex().my_sequence_octet().push_back(99);
    staticData.my_union().complex().my_sequence_struct().push_back(staticData.my_union().complex().my_basic_struct());
    for (int i = 0; i < 500; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                staticData.my_union().complex().my_array_octet()[i][j][k] = static_cast<uint8_t>(j*k);
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
        staticData.my_union().complex().my_array_struct()[i].my_float32(i*5.5f);
        staticData.my_union().complex().my_array_struct()[i].my_float64(i*8.8);
        staticData.my_union().complex().my_array_struct()[i].my_float128(i*10.0);
        staticData.my_union().complex().my_array_struct()[i].my_char('J');
        staticData.my_union().complex().my_array_struct()[i].my_wchar(L'C');
        staticData.my_union().complex().my_array_struct()[i].my_string("JC@eProsima");
        staticData.my_union().complex().my_array_struct()[i].my_wstring(L"JC-BOOM-Armadilo!@eProsima");
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

    staticData.my_union().complex().my_small_string_8("Bv7EMffURwGNqePoujdSfkF9PXN9TH125X5nGpNLfzya53tZtNJdgMROlYdZnTE1SLWzBdIU7ZyjjGvsGHkmuJUROwVPcNa9q5dRUV3KZAKNx1exL7BjhqIgQFconhd");
    staticData.my_union().complex().my_small_string_16(L"AgzñgXsI9pXbWjYLDvvn8JUFWhxZhk9t92rdsTqylvdpqtXA6hy9dHkoBTgmF2c");
    staticData.my_union().complex().my_large_string_8("hYE5vjcLJe6ML5DmoqQwh9ns866dAbnjkVKIKu2VF6lbkvh91ZOG2enEcdoRa8T43hR0Ym0k7tI621EQGufvzmLqxKCPgiXSp2zUTTmIWtn4fM8tC3aP1Yd0dKvn0tDobyp6p3156KvxqG3BKQ6VjFiHlMFoEyz8pjCclhXLl2cfAi97sQzXLUoPYUC5BWKyQTrA2JF6HXZM6vrbw5dc3B4AOJNGdPJ9ai6weF43h1RhnXE9MOFxPNoQnJ8gqSXYbMtpG6ZzqhUyoz0XhFDt7EOqXIgvc9SCejQTVMPeRcF5Zy57hrYZiKrCQqFWidS4BdfEAkuwESgBmEpEFOpZotwDt0TGDaLktSt3dKRsURO6TpuZ2nZNdiEJyc597ZjjQXtyKU7OCyRRqllzAnHEtoU3zd3OLTOvT5uk32N1Y64tpUte63De2EMwDNYb2eGAQfATdSt8VcGBOzJQjsmrMwMumtk48JzXXLxjo6s2vl2rNK9WQM1");
    staticData.my_union().complex().my_large_string_16(L"nosYBfFr1s3t8rUsuUrVCWFi6moDk7GULFj6XnkebIDkjl3n2ykKxUIaLj3qNNUx0ny8DvFbdfxZBdMhBNW3fHbKrig4GkHnN1JoEo0ACiPxrARusDs3xKzvaQQrls6lVUFAUXzDOtw5f2CNVJKiruGjXUO2Lq5Mmy8ygW3eUiTlueAHA2dRXXryOFi47jS3DkmBH4aAOKcmR27KhhJnXaY0gWy3XdSnaGQNB3XvbmxQ7xXDsf1wz860WMEKP3VhdOLsmS6tKCb4sshuOlmUSyTggY7vNoxfpG1EUFP5iPro9E0tHLLdHlWf2NwU8OXCYx6KKEbs5pFMvgEstnQglsdTk0lOv6riaFkFOwx83gW1l6Pg4eXjacnJKoVh1pOeZxULLZpCECw8yRZ9z4JPHxh2C7ytkCHMKp9O4MwQwYvvvgWWLWfJgb7Ecy2tgvWLpNDzgkFrEFhaCTKitChlG422CnLSsXvTBNnF52sULH6rcwOVx3mbhqte3ld3fObtAuH3zPzjOF4vVbvUXxgZh1Zx1cey0iGfnhOZHUfUwJ3Qv0WZNcuVLvMMhhg85A3620b84MAIc2UoW9Hl4BIT7pHo41ApF0DxIPJL0QdIdAOjn0JTPZqAhoHVBQoYvivPHftk5Crd1a1J8L7hSs0s4uSQKAMTKDxy3gKLaGAg277h4iEsEZRCI4RPlPTo9nZ48s8OO2KzqrUbMkoPSTgaJEXq8GsozAzh0wtL4P3gPeHO5nQzoytoXAkiXoPph0GaTLiahYQksYeK1eVQADDqZPXC55teXKKdX4aomCufr1ZizgzkGwAmnsFmhmBSF0gvbm56NDaUVT0UqXxKxAfRjkILeWR1mW8jfn6RYJH3IWiHxEfyB23rr78NySfgzIchhrm7jEFtmwPpKPKAwzajLv0HpkrtTr38YwWeT5LzHokFAQEc6l3aWdJWapVyt9wX89dEkmPPG9torCV2ddjyF4jAKsxKvzU4pCxV6B3m16IIdnksemJ0xG8iKh4ZPsX");
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j)
            staticData.my_union().complex().my_array_string()[i][j] = "Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42";
    for (int i = 0; i < 42; ++i)
        staticData.my_union().complex().multi_alias_array_42()[i] = (MyEnum)(i % 3);

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            staticData.my_union().complex().my_array_arrays()[i][j] = i*j;
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

    DynamicData *my_union = dynData->LoanValue(dynData->GetMemberIdByName("my_union"));

    DynamicData *complex = my_union->LoanValue(my_union->GetMemberIdByName("complex"));
    complex->SetByteValue(66, complex->GetMemberIdByName("my_octet"));

    DynamicData *basic = complex->LoanValue(complex->GetMemberIdByName("my_basic_struct"));
    basic->SetBoolValue(true, basic->GetMemberIdByName("my_bool"));
    basic->SetByteValue(166, basic->GetMemberIdByName("my_octet"));
    basic->SetInt16Value(-10401, basic->GetMemberIdByName("my_int16"));
    basic->SetInt32Value(5884001, basic->GetMemberIdByName("my_int32"));
    basic->SetInt64Value(884481567, basic->GetMemberIdByName("my_int64"));
    basic->SetUint16Value(250, basic->GetMemberIdByName("my_uint16"));
    basic->SetUint32Value(15884, basic->GetMemberIdByName("my_uint32"));
    basic->SetUint64Value(765241, basic->GetMemberIdByName("my_uint64"));
    basic->SetFloat32Value(158.55f, basic->GetMemberIdByName("my_float32"));
    basic->SetFloat64Value(765241.58, basic->GetMemberIdByName("my_float64"));
    basic->SetFloat128Value(765241878.154874, basic->GetMemberIdByName("my_float128"));
    basic->SetChar8Value('L', basic->GetMemberIdByName("my_char"));
    basic->SetChar16Value(L'G', basic->GetMemberIdByName("my_wchar"));
    basic->SetStringValue("Luis@eProsima", basic->GetMemberIdByName("my_string"));
    basic->SetWstringValue(L"LuisGasco@eProsima", basic->GetMemberIdByName("my_wstring"));
    complex->ReturnLoanedValue(basic);

    complex->SetEnumValue("C", complex->GetMemberIdByName("my_alias_enum"));
    complex->SetEnumValue("B", complex->GetMemberIdByName("my_enum"));

    DynamicData *my_seq_octet = complex->LoanValue(complex->GetMemberIdByName("my_sequence_octet"));
    MemberId id;
    my_seq_octet->InsertSequenceData(id);
    my_seq_octet->SetByteValue(88, id);
    my_seq_octet->InsertSequenceData(id);
    my_seq_octet->SetByteValue(99, id);
    //staticData.my_union().complex().my_sequence_octet().push_back(88);
    //staticData.my_union().complex().my_sequence_octet().push_back(99);
    complex->ReturnLoanedValue(my_seq_octet);

    DynamicData *my_seq_struct = complex->LoanValue(complex->GetMemberIdByName("my_sequence_struct"));
    my_seq_struct->InsertSequenceData(id);
    my_seq_struct->SetComplexValue(DynamicDataFactory::GetInstance()->CreateCopy(basic), id);
    //staticData.my_union().complex().my_sequence_struct().push_back(staticData.my_union().complex().my_basic_struct());
    complex->ReturnLoanedValue(my_seq_struct);

    DynamicData *my_array_octet = complex->LoanValue(complex->GetMemberIdByName("my_array_octet"));
    for (unsigned int i = 0; i < 500; ++i)
    {
        for (unsigned int j = 0; j < 5; ++j)
        {
            for (unsigned int k = 0; k < 4; ++k)
            {
                MemberId array_idx = my_array_octet->GetArrayIndex({ i, j, k });
                my_array_octet->SetByteValue(static_cast<uint8_t>(j*k), array_idx);
            }
        }
        //staticData.my_union().complex().my_array_octet()[i][j][k] = j*k;
    }
    complex->ReturnLoanedValue(my_array_octet);

    DynamicData *my_array_struct = complex->LoanValue(complex->GetMemberIdByName("my_array_struct"));
    for (int i = 0; i < 5; ++i)
    {
        DynamicData *tempBasic = DynamicDataFactory::GetInstance()->CreateData(GetBasicStructType());
        tempBasic->SetBoolValue(i % 2 == 1, tempBasic->GetMemberIdByName("my_bool"));
        tempBasic->SetByteValue(static_cast<uint8_t>(i), tempBasic->GetMemberIdByName("my_octet"));
        tempBasic->SetInt16Value(static_cast<int16_t>(-i), tempBasic->GetMemberIdByName("my_int16"));
        tempBasic->SetInt32Value(i, tempBasic->GetMemberIdByName("my_int32"));
        tempBasic->SetInt64Value(i * 1000, tempBasic->GetMemberIdByName("my_int64"));
        tempBasic->SetUint16Value(static_cast<uint16_t>(i), tempBasic->GetMemberIdByName("my_uint16"));
        tempBasic->SetUint32Value(i, tempBasic->GetMemberIdByName("my_uint32"));
        tempBasic->SetUint64Value(i * 10005, tempBasic->GetMemberIdByName("my_uint64"));
        tempBasic->SetFloat32Value(i*5.5f, tempBasic->GetMemberIdByName("my_float32"));
        tempBasic->SetFloat64Value(i*8.8, tempBasic->GetMemberIdByName("my_float64"));
        tempBasic->SetFloat128Value(i*10.0, tempBasic->GetMemberIdByName("my_float128"));
        tempBasic->SetChar8Value('J', tempBasic->GetMemberIdByName("my_char"));
        tempBasic->SetChar16Value(L'C', tempBasic->GetMemberIdByName("my_wchar"));
        tempBasic->SetStringValue("JC@eProsima", tempBasic->GetMemberIdByName("my_string"));
        tempBasic->SetWstringValue(L"JC-BOOM-Armadilo!@eProsima", tempBasic->GetMemberIdByName("my_wstring"));
        my_array_struct->SetComplexValue(tempBasic, i);
    }
    complex->ReturnLoanedValue(my_array_struct);

    DynamicTypeBuilder_ptr octet_builder = m_factory->CreateByteBuilder();
    DynamicData_ptr key_oct = DynamicDataFactory::GetInstance()->CreateData(octet_builder->Build());
    MemberId kId;
    MemberId vId;
    MemberId ssId;
    MemberId sId;
    DynamicData *my_map_octet_short = complex->LoanValue(complex->GetMemberIdByName("my_map_octet_short"));
    key_oct->SetByteValue(0);
    my_map_octet_short->InsertMapData(key_oct.get(), kId, vId);
    my_map_octet_short->SetInt16Value((short)1340, vId);
    key_oct = DynamicDataFactory::GetInstance()->CreateData(octet_builder->Build());
    key_oct->SetByteValue(1);
    my_map_octet_short->InsertMapData(key_oct.get(), kId, vId);
    my_map_octet_short->SetInt16Value((short)1341, vId);
    //staticData.my_union().complex().my_map_octet_short()[0] = 1340;
    //staticData.my_union().complex().my_map_octet_short()[1] = 1341;
    complex->ReturnLoanedValue(my_map_octet_short);

    DynamicTypeBuilder_ptr long_builder = m_factory->CreateInt32Builder();
    DynamicData_ptr key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    DynamicData *my_map_long_struct = complex->LoanValue(complex->GetMemberIdByName("my_map_long_struct"));

    //DynamicData *mas3 = my_array_struct->LoanValue(3);
    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(55);
    my_map_long_struct->InsertMapData(key.get(), kId, vId);
    basic = my_map_long_struct->LoanValue(vId);
    basic->SetBoolValue(true, basic->GetMemberIdByName("my_bool"));
    basic->SetByteValue(166, basic->GetMemberIdByName("my_octet"));
    basic->SetInt16Value(-10401, basic->GetMemberIdByName("my_int16"));
    basic->SetInt32Value(5884001, basic->GetMemberIdByName("my_int32"));
    basic->SetInt64Value(884481567, basic->GetMemberIdByName("my_int64"));
    basic->SetUint16Value(250, basic->GetMemberIdByName("my_uint16"));
    basic->SetUint32Value(15884, basic->GetMemberIdByName("my_uint32"));
    basic->SetUint64Value(765241, basic->GetMemberIdByName("my_uint64"));
    basic->SetFloat32Value(158.55f, basic->GetMemberIdByName("my_float32"));
    basic->SetFloat64Value(765241.58, basic->GetMemberIdByName("my_float64"));
    basic->SetFloat128Value(765241878.154874, basic->GetMemberIdByName("my_float128"));
    basic->SetChar8Value('L', basic->GetMemberIdByName("my_char"));
    basic->SetChar16Value(L'G', basic->GetMemberIdByName("my_wchar"));
    basic->SetStringValue("Luis@eProsima", basic->GetMemberIdByName("my_string"));
    basic->SetWstringValue(L"LuisGasco@eProsima", basic->GetMemberIdByName("my_wstring"));
    my_map_long_struct->ReturnLoanedValue(basic);
    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(1000);
    my_map_long_struct->InsertMapData(key.get(), kId, vId);
    DynamicData *mas3 = my_map_long_struct->LoanValue(vId);
    int i = 3;
    mas3->SetBoolValue(i % 2 == 1, mas3->GetMemberIdByName("my_bool"));
    mas3->SetByteValue(static_cast<uint8_t>(i), mas3->GetMemberIdByName("my_octet"));
    mas3->SetInt16Value(static_cast<int16_t>(-i), mas3->GetMemberIdByName("my_int16"));
    mas3->SetInt32Value(i, mas3->GetMemberIdByName("my_int32"));
    mas3->SetInt64Value(i * 1000, mas3->GetMemberIdByName("my_int64"));
    mas3->SetUint16Value(static_cast<uint8_t>(i), mas3->GetMemberIdByName("my_uint16"));
    mas3->SetUint32Value(i, mas3->GetMemberIdByName("my_uint32"));
    mas3->SetUint64Value(i * 10005, mas3->GetMemberIdByName("my_uint64"));
    mas3->SetFloat32Value(i*5.5f, mas3->GetMemberIdByName("my_float32"));
    mas3->SetFloat64Value(i*8.8, mas3->GetMemberIdByName("my_float64"));
    mas3->SetFloat128Value(i*10.0, mas3->GetMemberIdByName("my_float128"));
    mas3->SetChar8Value('J', mas3->GetMemberIdByName("my_char"));
    mas3->SetChar16Value(L'C', mas3->GetMemberIdByName("my_wchar"));
    mas3->SetStringValue("JC@eProsima", mas3->GetMemberIdByName("my_string"));
    mas3->SetWstringValue(L"JC-BOOM-Armadilo!@eProsima", mas3->GetMemberIdByName("my_wstring"));
    my_map_long_struct->ReturnLoanedValue(mas3);

    // staticData.my_union().complex().my_map_long_struct()[1000] = staticData.my_union().complex().my_array_struct()[3];
    // staticData.my_union().complex().my_map_long_struct()[55] = staticData.my_union().complex().my_basic_struct();
    complex->ReturnLoanedValue(my_map_long_struct);

    DynamicData *my_map_long_seq_octet = complex->LoanValue(complex->GetMemberIdByName("my_map_long_seq_octet"));
    //std::vector my_vector_octet = {1, 2, 3, 4, 5};
    //MemberId id;
    /*DynamicTypeBuilder_ptr octet_builder = m_factory->CreateByteBuilder();
    types::DynamicTypeBuilder_ptr seqOctet_builder = m_factory->CreateSequenceBuilder(octet_builder.get());
    types::DynamicType_ptr seqSeqOctet_builder = m_factory->CreateSequenceBuilder(seqOctet_builder.get())->Build();
    DynamicData *dataSeqOctet = seqOctet_builder->Build();
    DynamicData *dataSeqSeqOctet = seqSeqOctet_builder->Build();
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(1, id);
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(2, id);
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(3, id);
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(4, id);
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(5, id);
    dataSeqSeqOctet->InsertSequenceData(id);
    dataSeqSeqOctet->SetComplexValue(dataSeqOctet, id);*/
    // InsertMapData(DynamicData_ptr key, MemberId& outKeyId, MemberId& outValueId);
    // TODO De la muerte para Juan Carlos - Esto no es NADA práctico...

    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(0);
    my_map_long_seq_octet->InsertMapData(key.get(), kId, vId);

    DynamicData* seq_seq_oct = my_map_long_seq_octet->LoanValue(vId);
    seq_seq_oct->InsertSequenceData(ssId);
    DynamicData* seq_oct = seq_seq_oct->LoanValue(ssId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(1, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(2, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(3, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(4, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(5, sId);
    seq_seq_oct->ReturnLoanedValue(seq_oct);
    my_map_long_seq_octet->ReturnLoanedValue(seq_seq_oct);

    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(55);
    my_map_long_seq_octet->InsertMapData(key.get(), kId, vId);

    seq_seq_oct = my_map_long_seq_octet->LoanValue(vId);
    seq_seq_oct->InsertSequenceData(ssId);
    seq_oct = seq_seq_oct->LoanValue(ssId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(1, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(2, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(3, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(4, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(5, sId);
    seq_seq_oct->ReturnLoanedValue(seq_oct);
    seq_seq_oct->InsertSequenceData(ssId);
    seq_oct = seq_seq_oct->LoanValue(ssId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(1, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(2, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(3, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(4, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(5, sId);
    seq_seq_oct->ReturnLoanedValue(seq_oct);
    my_map_long_seq_octet->ReturnLoanedValue(seq_seq_oct);
    //staticData.my_union().complex().my_map_long_seq_octet()[55].push_back(my_vector_octet);
    //staticData.my_union().complex().my_map_long_seq_octet()[55].push_back(my_vector_octet);
    //staticData.my_union().complex().my_map_long_seq_octet()[0].push_back(my_vector_octet);
    complex->ReturnLoanedValue(my_map_long_seq_octet);

    DynamicData *my_map_long_octet_array_500 =
        complex->LoanValue(complex->GetMemberIdByName("my_map_long_octet_array_500"));

    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(0);
    my_map_long_octet_array_500->InsertMapData(key.get(), kId, vId);

    DynamicData* oct_array_500 = my_map_long_octet_array_500->LoanValue(vId);
    for (int j = 0; j < 500; ++j)
    {
        oct_array_500->SetByteValue(j % 256, j);
        //staticData.my_union().complex().my_map_long_octet_array_500()[0][i] = i%256;
    }
    my_map_long_octet_array_500->ReturnLoanedValue(oct_array_500);

    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(10);
    my_map_long_octet_array_500->InsertMapData(key.get(), kId, vId);
    oct_array_500 = my_map_long_octet_array_500->LoanValue(vId);

    for (int j = 0; j < 500; ++j)
    {
        oct_array_500->SetByteValue((j + 55) % 256, j);
        //staticData.my_union().complex().my_map_long_octet_array_500()[10][i] = (i+55)%256;
    }
    my_map_long_octet_array_500->ReturnLoanedValue(oct_array_500);
    complex->ReturnLoanedValue(my_map_long_octet_array_500);

    complex->SetStringValue("Bv7EMffURwGNqePoujdSfkF9PXN9TH125X5nGpNLfzya53tZtNJdgMROlYdZnTE1SLWzBdIU7ZyjjGvsGHkmuJUROwVPcNa9q5dRUV3KZAKNx1exL7BjhqIgQFconhd", complex->GetMemberIdByName("my_small_string_8"));
    complex->SetWstringValue(L"AgzñgXsI9pXbWjYLDvvn8JUFWhxZhk9t92rdsTqylvdpqtXA6hy9dHkoBTgmF2c", complex->GetMemberIdByName("my_small_string_16"));
    complex->SetStringValue("hYE5vjcLJe6ML5DmoqQwh9ns866dAbnjkVKIKu2VF6lbkvh91ZOG2enEcdoRa8T43hR0Ym0k7tI621EQGufvzmLqxKCPgiXSp2zUTTmIWtn4fM8tC3aP1Yd0dKvn0tDobyp6p3156KvxqG3BKQ6VjFiHlMFoEyz8pjCclhXLl2cfAi97sQzXLUoPYUC5BWKyQTrA2JF6HXZM6vrbw5dc3B4AOJNGdPJ9ai6weF43h1RhnXE9MOFxPNoQnJ8gqSXYbMtpG6ZzqhUyoz0XhFDt7EOqXIgvc9SCejQTVMPeRcF5Zy57hrYZiKrCQqFWidS4BdfEAkuwESgBmEpEFOpZotwDt0TGDaLktSt3dKRsURO6TpuZ2nZNdiEJyc597ZjjQXtyKU7OCyRRqllzAnHEtoU3zd3OLTOvT5uk32N1Y64tpUte63De2EMwDNYb2eGAQfATdSt8VcGBOzJQjsmrMwMumtk48JzXXLxjo6s2vl2rNK9WQM1", complex->GetMemberIdByName("my_large_string_8"));
    complex->SetWstringValue(L"nosYBfFr1s3t8rUsuUrVCWFi6moDk7GULFj6XnkebIDkjl3n2ykKxUIaLj3qNNUx0ny8DvFbdfxZBdMhBNW3fHbKrig4GkHnN1JoEo0ACiPxrARusDs3xKzvaQQrls6lVUFAUXzDOtw5f2CNVJKiruGjXUO2Lq5Mmy8ygW3eUiTlueAHA2dRXXryOFi47jS3DkmBH4aAOKcmR27KhhJnXaY0gWy3XdSnaGQNB3XvbmxQ7xXDsf1wz860WMEKP3VhdOLsmS6tKCb4sshuOlmUSyTggY7vNoxfpG1EUFP5iPro9E0tHLLdHlWf2NwU8OXCYx6KKEbs5pFMvgEstnQglsdTk0lOv6riaFkFOwx83gW1l6Pg4eXjacnJKoVh1pOeZxULLZpCECw8yRZ9z4JPHxh2C7ytkCHMKp9O4MwQwYvvvgWWLWfJgb7Ecy2tgvWLpNDzgkFrEFhaCTKitChlG422CnLSsXvTBNnF52sULH6rcwOVx3mbhqte3ld3fObtAuH3zPzjOF4vVbvUXxgZh1Zx1cey0iGfnhOZHUfUwJ3Qv0WZNcuVLvMMhhg85A3620b84MAIc2UoW9Hl4BIT7pHo41ApF0DxIPJL0QdIdAOjn0JTPZqAhoHVBQoYvivPHftk5Crd1a1J8L7hSs0s4uSQKAMTKDxy3gKLaGAg277h4iEsEZRCI4RPlPTo9nZ48s8OO2KzqrUbMkoPSTgaJEXq8GsozAzh0wtL4P3gPeHO5nQzoytoXAkiXoPph0GaTLiahYQksYeK1eVQADDqZPXC55teXKKdX4aomCufr1ZizgzkGwAmnsFmhmBSF0gvbm56NDaUVT0UqXxKxAfRjkILeWR1mW8jfn6RYJH3IWiHxEfyB23rr78NySfgzIchhrm7jEFtmwPpKPKAwzajLv0HpkrtTr38YwWeT5LzHokFAQEc6l3aWdJWapVyt9wX89dEkmPPG9torCV2ddjyF4jAKsxKvzU4pCxV6B3m16IIdnksemJ0xG8iKh4ZPsX", complex->GetMemberIdByName("my_large_string_16"));

    DynamicData *my_array_string = complex->LoanValue(complex->GetMemberIdByName("my_array_string"));
    for (unsigned int j = 0; j < 5; ++j)
    {
        for (unsigned int k = 0; k < 5; ++k)
        {
            MemberId array_idx = my_array_string->GetArrayIndex({ j, k });
            my_array_string->SetStringValue("Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42", array_idx);
            //staticData.my_union().complex().my_array_string()[i][j]("Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42");
        }
    }
    complex->ReturnLoanedValue(my_array_string);

    DynamicData *multi_alias_array_42 = complex->LoanValue(complex->GetMemberIdByName("multi_alias_array_42"));
    for (int j = 0; j < 42; ++j)
    {
        multi_alias_array_42->SetEnumValue(j % 3, j);
        //staticData.my_union().complex().multi_alias_array_42()[i](i%3);
    }
    complex->ReturnLoanedValue(multi_alias_array_42);

    DynamicData *my_array_arrays = complex->LoanValue(complex->GetMemberIdByName("my_array_arrays"));
    for (unsigned int j = 0; j < 5; ++j)
    {
        DynamicData *myMiniArray = my_array_arrays->LoanValue(j);
        for (unsigned int k = 0; k < 2; ++k)
        {
            myMiniArray->SetInt32Value(j*k, k);
            //staticData.my_union().complex().my_array_arrays()[i][j](i*j);
        }
        my_array_arrays->ReturnLoanedValue(myMiniArray);
    }
    complex->ReturnLoanedValue(my_array_arrays);

    DynamicData *my_sequences_array = complex->LoanValue(complex->GetMemberIdByName("my_sequences_array"));
    for (int j = 0; j < 23; ++j)
    {
        DynamicData *seq = DynamicDataFactory::GetInstance()->CreateData(GetMySequenceLongType());
        seq->InsertSequenceData(id);
        seq->SetInt32Value(j, id);
        seq->InsertSequenceData(id);
        seq->SetInt32Value(j * 10, id);
        seq->InsertSequenceData(id);
        seq->SetInt32Value(j * 100, id);
        my_sequences_array->SetComplexValue(seq, j);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i*10);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i*100);
    }
    complex->ReturnLoanedValue(my_sequences_array);

    my_union->ReturnLoanedValue(complex);
    dynData->ReturnLoanedValue(my_union);

    DynamicData *my_union_2 = dynData->LoanValue(dynData->GetMemberIdByName("my_union_2"));
    my_union_2->SetStringValue("GASCO!", my_union_2->GetMemberIdByName("imString"));

    dynData->ReturnLoanedValue(my_union_2);

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

    types::DynamicData_ptr dynDataFromDynamic = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);
    ASSERT_TRUE(pubsubType.deserialize(&payload, dynDataFromDynamic.get()));

    types::DynamicData_ptr dynDataFromStatic = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);
    ASSERT_TRUE(pubsubType.deserialize(&stPayload, dynDataFromStatic.get()));

    ASSERT_TRUE(dynDataFromStatic->Equals(dynDataFromDynamic.get()));
}

TEST_F(DynamicComplexTypesTests, Data_Comparison_B_C)
{
    types::DynamicData_ptr dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);

    CompleteStruct staticData;
    staticData.my_union()._d() = B;
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
    staticData.my_union().complex().my_basic_struct().my_wchar(L'G');
    staticData.my_union().complex().my_basic_struct().my_string("Luis@eProsima");
    staticData.my_union().complex().my_basic_struct().my_wstring(L"LuisGasco@eProsima");

    staticData.my_union().complex().my_alias_enum(C);
    staticData.my_union().complex().my_enum(B);
    staticData.my_union().complex().my_sequence_octet().push_back(88);
    staticData.my_union().complex().my_sequence_octet().push_back(99);
    staticData.my_union().complex().my_sequence_struct().push_back(staticData.my_union().complex().my_basic_struct());
    for (int i = 0; i < 500; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                staticData.my_union().complex().my_array_octet()[i][j][k] = static_cast<uint8_t>(j*k);
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
        staticData.my_union().complex().my_array_struct()[i].my_float32(i*5.5f);
        staticData.my_union().complex().my_array_struct()[i].my_float64(i*8.8);
        staticData.my_union().complex().my_array_struct()[i].my_float128(i*10.0);
        staticData.my_union().complex().my_array_struct()[i].my_char('J');
        staticData.my_union().complex().my_array_struct()[i].my_wchar(L'C');
        staticData.my_union().complex().my_array_struct()[i].my_string("JC@eProsima");
        staticData.my_union().complex().my_array_struct()[i].my_wstring(L"JC-BOOM-Armadilo!@eProsima");
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

    staticData.my_union().complex().my_small_string_8("Bv7EMffURwGNqePoujdSfkF9PXN9TH125X5nGpNLfzya53tZtNJdgMROlYdZnTE1SLWzBdIU7ZyjjGvsGHkmuJUROwVPcNa9q5dRUV3KZAKNx1exL7BjhqIgQFconhd");
    staticData.my_union().complex().my_small_string_16(L"AgzñgXsI9pXbWjYLDvvn8JUFWhxZhk9t92rdsTqylvdpqtXA6hy9dHkoBTgmF2c");
    staticData.my_union().complex().my_large_string_8("hYE5vjcLJe6ML5DmoqQwh9ns866dAbnjkVKIKu2VF6lbkvh91ZOG2enEcdoRa8T43hR0Ym0k7tI621EQGufvzmLqxKCPgiXSp2zUTTmIWtn4fM8tC3aP1Yd0dKvn0tDobyp6p3156KvxqG3BKQ6VjFiHlMFoEyz8pjCclhXLl2cfAi97sQzXLUoPYUC5BWKyQTrA2JF6HXZM6vrbw5dc3B4AOJNGdPJ9ai6weF43h1RhnXE9MOFxPNoQnJ8gqSXYbMtpG6ZzqhUyoz0XhFDt7EOqXIgvc9SCejQTVMPeRcF5Zy57hrYZiKrCQqFWidS4BdfEAkuwESgBmEpEFOpZotwDt0TGDaLktSt3dKRsURO6TpuZ2nZNdiEJyc597ZjjQXtyKU7OCyRRqllzAnHEtoU3zd3OLTOvT5uk32N1Y64tpUte63De2EMwDNYb2eGAQfATdSt8VcGBOzJQjsmrMwMumtk48JzXXLxjo6s2vl2rNK9WQM1");
    staticData.my_union().complex().my_large_string_16(L"nosYBfFr1s3t8rUsuUrVCWFi6moDk7GULFj6XnkebIDkjl3n2ykKxUIaLj3qNNUx0ny8DvFbdfxZBdMhBNW3fHbKrig4GkHnN1JoEo0ACiPxrARusDs3xKzvaQQrls6lVUFAUXzDOtw5f2CNVJKiruGjXUO2Lq5Mmy8ygW3eUiTlueAHA2dRXXryOFi47jS3DkmBH4aAOKcmR27KhhJnXaY0gWy3XdSnaGQNB3XvbmxQ7xXDsf1wz860WMEKP3VhdOLsmS6tKCb4sshuOlmUSyTggY7vNoxfpG1EUFP5iPro9E0tHLLdHlWf2NwU8OXCYx6KKEbs5pFMvgEstnQglsdTk0lOv6riaFkFOwx83gW1l6Pg4eXjacnJKoVh1pOeZxULLZpCECw8yRZ9z4JPHxh2C7ytkCHMKp9O4MwQwYvvvgWWLWfJgb7Ecy2tgvWLpNDzgkFrEFhaCTKitChlG422CnLSsXvTBNnF52sULH6rcwOVx3mbhqte3ld3fObtAuH3zPzjOF4vVbvUXxgZh1Zx1cey0iGfnhOZHUfUwJ3Qv0WZNcuVLvMMhhg85A3620b84MAIc2UoW9Hl4BIT7pHo41ApF0DxIPJL0QdIdAOjn0JTPZqAhoHVBQoYvivPHftk5Crd1a1J8L7hSs0s4uSQKAMTKDxy3gKLaGAg277h4iEsEZRCI4RPlPTo9nZ48s8OO2KzqrUbMkoPSTgaJEXq8GsozAzh0wtL4P3gPeHO5nQzoytoXAkiXoPph0GaTLiahYQksYeK1eVQADDqZPXC55teXKKdX4aomCufr1ZizgzkGwAmnsFmhmBSF0gvbm56NDaUVT0UqXxKxAfRjkILeWR1mW8jfn6RYJH3IWiHxEfyB23rr78NySfgzIchhrm7jEFtmwPpKPKAwzajLv0HpkrtTr38YwWeT5LzHokFAQEc6l3aWdJWapVyt9wX89dEkmPPG9torCV2ddjyF4jAKsxKvzU4pCxV6B3m16IIdnksemJ0xG8iKh4ZPsX");
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j)
            staticData.my_union().complex().my_array_string()[i][j] = "Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42";
    for (int i = 0; i < 42; ++i)
        staticData.my_union().complex().multi_alias_array_42()[i] = (MyEnum)(i % 3);

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            staticData.my_union().complex().my_array_arrays()[i][j] = i*j;
        }
    }

    for (int i = 0; i < 23; ++i)
    {
        staticData.my_union().complex().my_sequences_array()[i].push_back(i);
        staticData.my_union().complex().my_sequences_array()[i].push_back(i * 10);
        staticData.my_union().complex().my_sequences_array()[i].push_back(i * 100);
    }
    staticData.my_union_2().tres(156);

    DynamicData *my_union = dynData->LoanValue(dynData->GetMemberIdByName("my_union"));

    DynamicData *complex = my_union->LoanValue(my_union->GetMemberIdByName("complex"));
    complex->SetByteValue(66, complex->GetMemberIdByName("my_octet"));

    DynamicData *basic = complex->LoanValue(complex->GetMemberIdByName("my_basic_struct"));
    basic->SetBoolValue(true, basic->GetMemberIdByName("my_bool"));
    basic->SetByteValue(166, basic->GetMemberIdByName("my_octet"));
    basic->SetInt16Value(-10401, basic->GetMemberIdByName("my_int16"));
    basic->SetInt32Value(5884001, basic->GetMemberIdByName("my_int32"));
    basic->SetInt64Value(884481567, basic->GetMemberIdByName("my_int64"));
    basic->SetUint16Value(250, basic->GetMemberIdByName("my_uint16"));
    basic->SetUint32Value(15884, basic->GetMemberIdByName("my_uint32"));
    basic->SetUint64Value(765241, basic->GetMemberIdByName("my_uint64"));
    basic->SetFloat32Value(158.55f, basic->GetMemberIdByName("my_float32"));
    basic->SetFloat64Value(765241.58, basic->GetMemberIdByName("my_float64"));
    basic->SetFloat128Value(765241878.154874, basic->GetMemberIdByName("my_float128"));
    basic->SetChar8Value('L', basic->GetMemberIdByName("my_char"));
    basic->SetChar16Value(L'G', basic->GetMemberIdByName("my_wchar"));
    basic->SetStringValue("Luis@eProsima", basic->GetMemberIdByName("my_string"));
    basic->SetWstringValue(L"LuisGasco@eProsima", basic->GetMemberIdByName("my_wstring"));
    complex->ReturnLoanedValue(basic);

    complex->SetEnumValue("C", complex->GetMemberIdByName("my_alias_enum"));
    complex->SetEnumValue("B", complex->GetMemberIdByName("my_enum"));

    DynamicData *my_seq_octet = complex->LoanValue(complex->GetMemberIdByName("my_sequence_octet"));
    MemberId id;
    my_seq_octet->InsertSequenceData(id);
    my_seq_octet->SetByteValue(88, id);
    my_seq_octet->InsertSequenceData(id);
    my_seq_octet->SetByteValue(99, id);
    //staticData.my_union().complex().my_sequence_octet().push_back(88);
    //staticData.my_union().complex().my_sequence_octet().push_back(99);
    complex->ReturnLoanedValue(my_seq_octet);

    DynamicData *my_seq_struct = complex->LoanValue(complex->GetMemberIdByName("my_sequence_struct"));
    my_seq_struct->InsertSequenceData(id);
    my_seq_struct->SetComplexValue(DynamicDataFactory::GetInstance()->CreateCopy(basic), id);
    //staticData.my_union().complex().my_sequence_struct().push_back(staticData.my_union().complex().my_basic_struct());
    complex->ReturnLoanedValue(my_seq_struct);

    DynamicData *my_array_octet = complex->LoanValue(complex->GetMemberIdByName("my_array_octet"));
    for (unsigned int i = 0; i < 500; ++i)
    {
        for (unsigned int j = 0; j < 5; ++j)
        {
            for (unsigned int k = 0; k < 4; ++k)
            {
                MemberId array_idx = my_array_octet->GetArrayIndex({ i, j, k });
                my_array_octet->SetByteValue(static_cast<uint8_t>(j*k), array_idx);
            }
        }
        //staticData.my_union().complex().my_array_octet()[i][j][k] = j*k;
    }
    complex->ReturnLoanedValue(my_array_octet);

    DynamicData *my_array_struct = complex->LoanValue(complex->GetMemberIdByName("my_array_struct"));
    for (int i = 0; i < 5; ++i)
    {
        DynamicData *tempBasic = DynamicDataFactory::GetInstance()->CreateData(GetBasicStructType());
        tempBasic->SetBoolValue(i % 2 == 1, tempBasic->GetMemberIdByName("my_bool"));
        tempBasic->SetByteValue(static_cast<uint8_t>(i), tempBasic->GetMemberIdByName("my_octet"));
        tempBasic->SetInt16Value(static_cast<int16_t>(-i), tempBasic->GetMemberIdByName("my_int16"));
        tempBasic->SetInt32Value(i, tempBasic->GetMemberIdByName("my_int32"));
        tempBasic->SetInt64Value(i * 1000, tempBasic->GetMemberIdByName("my_int64"));
        tempBasic->SetUint16Value(static_cast<uint16_t>(i), tempBasic->GetMemberIdByName("my_uint16"));
        tempBasic->SetUint32Value(i, tempBasic->GetMemberIdByName("my_uint32"));
        tempBasic->SetUint64Value(i * 10005, tempBasic->GetMemberIdByName("my_uint64"));
        tempBasic->SetFloat32Value(i*5.5f, tempBasic->GetMemberIdByName("my_float32"));
        tempBasic->SetFloat64Value(i*8.8, tempBasic->GetMemberIdByName("my_float64"));
        tempBasic->SetFloat128Value(i*10.0, tempBasic->GetMemberIdByName("my_float128"));
        tempBasic->SetChar8Value('J', tempBasic->GetMemberIdByName("my_char"));
        tempBasic->SetChar16Value(L'C', tempBasic->GetMemberIdByName("my_wchar"));
        tempBasic->SetStringValue("JC@eProsima", tempBasic->GetMemberIdByName("my_string"));
        tempBasic->SetWstringValue(L"JC-BOOM-Armadilo!@eProsima", tempBasic->GetMemberIdByName("my_wstring"));
        my_array_struct->SetComplexValue(tempBasic, i);
    }
    complex->ReturnLoanedValue(my_array_struct);

    DynamicTypeBuilder_ptr octet_builder = m_factory->CreateByteBuilder();
    DynamicData_ptr key_oct = DynamicDataFactory::GetInstance()->CreateData(octet_builder->Build());
    MemberId kId;
    MemberId vId;
    MemberId ssId;
    MemberId sId;
    DynamicData *my_map_octet_short = complex->LoanValue(complex->GetMemberIdByName("my_map_octet_short"));
    key_oct->SetByteValue(0);
    my_map_octet_short->InsertMapData(key_oct.get(), kId, vId);
    my_map_octet_short->SetInt16Value((short)1340, vId);
    key_oct = DynamicDataFactory::GetInstance()->CreateData(octet_builder->Build());
    key_oct->SetByteValue(1);
    my_map_octet_short->InsertMapData(key_oct.get(), kId, vId);
    my_map_octet_short->SetInt16Value((short)1341, vId);
    //staticData.my_union().complex().my_map_octet_short()[0] = 1340;
    //staticData.my_union().complex().my_map_octet_short()[1] = 1341;
    complex->ReturnLoanedValue(my_map_octet_short);

    DynamicTypeBuilder_ptr long_builder = m_factory->CreateInt32Builder();
    DynamicData_ptr key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    DynamicData *my_map_long_struct = complex->LoanValue(complex->GetMemberIdByName("my_map_long_struct"));

    //DynamicData *mas3 = my_array_struct->LoanValue(3);
    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(55);
    my_map_long_struct->InsertMapData(key.get(), kId, vId);
    basic = my_map_long_struct->LoanValue(vId);
    basic->SetBoolValue(true, basic->GetMemberIdByName("my_bool"));
    basic->SetByteValue(166, basic->GetMemberIdByName("my_octet"));
    basic->SetInt16Value(-10401, basic->GetMemberIdByName("my_int16"));
    basic->SetInt32Value(5884001, basic->GetMemberIdByName("my_int32"));
    basic->SetInt64Value(884481567, basic->GetMemberIdByName("my_int64"));
    basic->SetUint16Value(250, basic->GetMemberIdByName("my_uint16"));
    basic->SetUint32Value(15884, basic->GetMemberIdByName("my_uint32"));
    basic->SetUint64Value(765241, basic->GetMemberIdByName("my_uint64"));
    basic->SetFloat32Value(158.55f, basic->GetMemberIdByName("my_float32"));
    basic->SetFloat64Value(765241.58, basic->GetMemberIdByName("my_float64"));
    basic->SetFloat128Value(765241878.154874, basic->GetMemberIdByName("my_float128"));
    basic->SetChar8Value('L', basic->GetMemberIdByName("my_char"));
    basic->SetChar16Value(L'G', basic->GetMemberIdByName("my_wchar"));
    basic->SetStringValue("Luis@eProsima", basic->GetMemberIdByName("my_string"));
    basic->SetWstringValue(L"LuisGasco@eProsima", basic->GetMemberIdByName("my_wstring"));
    my_map_long_struct->ReturnLoanedValue(basic);
    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(1000);
    my_map_long_struct->InsertMapData(key.get(), kId, vId);
    DynamicData *mas3 = my_map_long_struct->LoanValue(vId);
    int i = 3;
    mas3->SetBoolValue(i % 2 == 1, mas3->GetMemberIdByName("my_bool"));
    mas3->SetByteValue(static_cast<uint8_t>(i), mas3->GetMemberIdByName("my_octet"));
    mas3->SetInt16Value(static_cast<int16_t>(-i), mas3->GetMemberIdByName("my_int16"));
    mas3->SetInt32Value(i, mas3->GetMemberIdByName("my_int32"));
    mas3->SetInt64Value(i * 1000, mas3->GetMemberIdByName("my_int64"));
    mas3->SetUint16Value(static_cast<uint8_t>(i), mas3->GetMemberIdByName("my_uint16"));
    mas3->SetUint32Value(i, mas3->GetMemberIdByName("my_uint32"));
    mas3->SetUint64Value(i * 10005, mas3->GetMemberIdByName("my_uint64"));
    mas3->SetFloat32Value(i*5.5f, mas3->GetMemberIdByName("my_float32"));
    mas3->SetFloat64Value(i*8.8, mas3->GetMemberIdByName("my_float64"));
    mas3->SetFloat128Value(i*10.0, mas3->GetMemberIdByName("my_float128"));
    mas3->SetChar8Value('J', mas3->GetMemberIdByName("my_char"));
    mas3->SetChar16Value(L'C', mas3->GetMemberIdByName("my_wchar"));
    mas3->SetStringValue("JC@eProsima", mas3->GetMemberIdByName("my_string"));
    mas3->SetWstringValue(L"JC-BOOM-Armadilo!@eProsima", mas3->GetMemberIdByName("my_wstring"));
    my_map_long_struct->ReturnLoanedValue(mas3);

    // staticData.my_union().complex().my_map_long_struct()[1000] = staticData.my_union().complex().my_array_struct()[3];
    // staticData.my_union().complex().my_map_long_struct()[55] = staticData.my_union().complex().my_basic_struct();
    complex->ReturnLoanedValue(my_map_long_struct);

    DynamicData *my_map_long_seq_octet = complex->LoanValue(complex->GetMemberIdByName("my_map_long_seq_octet"));
    //std::vector my_vector_octet = {1, 2, 3, 4, 5};
    //MemberId id;
    /*DynamicTypeBuilder_ptr octet_builder = m_factory->CreateByteBuilder();
    types::DynamicTypeBuilder_ptr seqOctet_builder = m_factory->CreateSequenceBuilder(octet_builder.get());
    types::DynamicType_ptr seqSeqOctet_builder = m_factory->CreateSequenceBuilder(seqOctet_builder.get())->Build();
    DynamicData *dataSeqOctet = seqOctet_builder->Build();
    DynamicData *dataSeqSeqOctet = seqSeqOctet_builder->Build();
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(1, id);
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(2, id);
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(3, id);
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(4, id);
    dataSeqOctet->InsertSequenceData(id);
    dataSeqOctet->SetByteValue(5, id);
    dataSeqSeqOctet->InsertSequenceData(id);
    dataSeqSeqOctet->SetComplexValue(dataSeqOctet, id);*/
    // InsertMapData(DynamicData_ptr key, MemberId& outKeyId, MemberId& outValueId);
    // TODO De la muerte para Juan Carlos - Esto no es NADA práctico...

    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(0);
    my_map_long_seq_octet->InsertMapData(key.get(), kId, vId);

    DynamicData* seq_seq_oct = my_map_long_seq_octet->LoanValue(vId);
    seq_seq_oct->InsertSequenceData(ssId);
    DynamicData* seq_oct = seq_seq_oct->LoanValue(ssId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(1, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(2, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(3, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(4, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(5, sId);
    seq_seq_oct->ReturnLoanedValue(seq_oct);
    my_map_long_seq_octet->ReturnLoanedValue(seq_seq_oct);

    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(55);
    my_map_long_seq_octet->InsertMapData(key.get(), kId, vId);

    seq_seq_oct = my_map_long_seq_octet->LoanValue(vId);
    seq_seq_oct->InsertSequenceData(ssId);
    seq_oct = seq_seq_oct->LoanValue(ssId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(1, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(2, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(3, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(4, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(5, sId);
    seq_seq_oct->ReturnLoanedValue(seq_oct);
    seq_seq_oct->InsertSequenceData(ssId);
    seq_oct = seq_seq_oct->LoanValue(ssId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(1, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(2, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(3, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(4, sId);
    seq_oct->InsertSequenceData(sId);
    seq_oct->SetByteValue(5, sId);
    seq_seq_oct->ReturnLoanedValue(seq_oct);
    my_map_long_seq_octet->ReturnLoanedValue(seq_seq_oct);
    //staticData.my_union().complex().my_map_long_seq_octet()[55].push_back(my_vector_octet);
    //staticData.my_union().complex().my_map_long_seq_octet()[55].push_back(my_vector_octet);
    //staticData.my_union().complex().my_map_long_seq_octet()[0].push_back(my_vector_octet);
    complex->ReturnLoanedValue(my_map_long_seq_octet);

    DynamicData *my_map_long_octet_array_500 =
        complex->LoanValue(complex->GetMemberIdByName("my_map_long_octet_array_500"));

    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(0);
    my_map_long_octet_array_500->InsertMapData(key.get(), kId, vId);

    DynamicData* oct_array_500 = my_map_long_octet_array_500->LoanValue(vId);
    for (int j = 0; j < 500; ++j)
    {
        oct_array_500->SetByteValue(j % 256, j);
        //staticData.my_union().complex().my_map_long_octet_array_500()[0][i] = i%256;
    }
    my_map_long_octet_array_500->ReturnLoanedValue(oct_array_500);

    key = DynamicDataFactory::GetInstance()->CreateData(long_builder->Build());
    key->SetInt32Value(10);
    my_map_long_octet_array_500->InsertMapData(key.get(), kId, vId);
    oct_array_500 = my_map_long_octet_array_500->LoanValue(vId);

    for (int j = 0; j < 500; ++j)
    {
        oct_array_500->SetByteValue((j + 55) % 256, j);
        //staticData.my_union().complex().my_map_long_octet_array_500()[10][i] = (i+55)%256;
    }
    my_map_long_octet_array_500->ReturnLoanedValue(oct_array_500);
    complex->ReturnLoanedValue(my_map_long_octet_array_500);

    complex->SetStringValue("Bv7EMffURwGNqePoujdSfkF9PXN9TH125X5nGpNLfzya53tZtNJdgMROlYdZnTE1SLWzBdIU7ZyjjGvsGHkmuJUROwVPcNa9q5dRUV3KZAKNx1exL7BjhqIgQFconhd", complex->GetMemberIdByName("my_small_string_8"));
    complex->SetWstringValue(L"AgzñgXsI9pXbWjYLDvvn8JUFWhxZhk9t92rdsTqylvdpqtXA6hy9dHkoBTgmF2c", complex->GetMemberIdByName("my_small_string_16"));
    complex->SetStringValue("hYE5vjcLJe6ML5DmoqQwh9ns866dAbnjkVKIKu2VF6lbkvh91ZOG2enEcdoRa8T43hR0Ym0k7tI621EQGufvzmLqxKCPgiXSp2zUTTmIWtn4fM8tC3aP1Yd0dKvn0tDobyp6p3156KvxqG3BKQ6VjFiHlMFoEyz8pjCclhXLl2cfAi97sQzXLUoPYUC5BWKyQTrA2JF6HXZM6vrbw5dc3B4AOJNGdPJ9ai6weF43h1RhnXE9MOFxPNoQnJ8gqSXYbMtpG6ZzqhUyoz0XhFDt7EOqXIgvc9SCejQTVMPeRcF5Zy57hrYZiKrCQqFWidS4BdfEAkuwESgBmEpEFOpZotwDt0TGDaLktSt3dKRsURO6TpuZ2nZNdiEJyc597ZjjQXtyKU7OCyRRqllzAnHEtoU3zd3OLTOvT5uk32N1Y64tpUte63De2EMwDNYb2eGAQfATdSt8VcGBOzJQjsmrMwMumtk48JzXXLxjo6s2vl2rNK9WQM1", complex->GetMemberIdByName("my_large_string_8"));
    complex->SetWstringValue(L"nosYBfFr1s3t8rUsuUrVCWFi6moDk7GULFj6XnkebIDkjl3n2ykKxUIaLj3qNNUx0ny8DvFbdfxZBdMhBNW3fHbKrig4GkHnN1JoEo0ACiPxrARusDs3xKzvaQQrls6lVUFAUXzDOtw5f2CNVJKiruGjXUO2Lq5Mmy8ygW3eUiTlueAHA2dRXXryOFi47jS3DkmBH4aAOKcmR27KhhJnXaY0gWy3XdSnaGQNB3XvbmxQ7xXDsf1wz860WMEKP3VhdOLsmS6tKCb4sshuOlmUSyTggY7vNoxfpG1EUFP5iPro9E0tHLLdHlWf2NwU8OXCYx6KKEbs5pFMvgEstnQglsdTk0lOv6riaFkFOwx83gW1l6Pg4eXjacnJKoVh1pOeZxULLZpCECw8yRZ9z4JPHxh2C7ytkCHMKp9O4MwQwYvvvgWWLWfJgb7Ecy2tgvWLpNDzgkFrEFhaCTKitChlG422CnLSsXvTBNnF52sULH6rcwOVx3mbhqte3ld3fObtAuH3zPzjOF4vVbvUXxgZh1Zx1cey0iGfnhOZHUfUwJ3Qv0WZNcuVLvMMhhg85A3620b84MAIc2UoW9Hl4BIT7pHo41ApF0DxIPJL0QdIdAOjn0JTPZqAhoHVBQoYvivPHftk5Crd1a1J8L7hSs0s4uSQKAMTKDxy3gKLaGAg277h4iEsEZRCI4RPlPTo9nZ48s8OO2KzqrUbMkoPSTgaJEXq8GsozAzh0wtL4P3gPeHO5nQzoytoXAkiXoPph0GaTLiahYQksYeK1eVQADDqZPXC55teXKKdX4aomCufr1ZizgzkGwAmnsFmhmBSF0gvbm56NDaUVT0UqXxKxAfRjkILeWR1mW8jfn6RYJH3IWiHxEfyB23rr78NySfgzIchhrm7jEFtmwPpKPKAwzajLv0HpkrtTr38YwWeT5LzHokFAQEc6l3aWdJWapVyt9wX89dEkmPPG9torCV2ddjyF4jAKsxKvzU4pCxV6B3m16IIdnksemJ0xG8iKh4ZPsX", complex->GetMemberIdByName("my_large_string_16"));

    DynamicData *my_array_string = complex->LoanValue(complex->GetMemberIdByName("my_array_string"));
    for (unsigned int j = 0; j < 5; ++j)
    {
        for (unsigned int k = 0; k < 5; ++k)
        {
            MemberId array_idx = my_array_string->GetArrayIndex({ j, k });
            my_array_string->SetStringValue("Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42", array_idx);
            //staticData.my_union().complex().my_array_string()[i][j]("Ee4rH8nSX1xnWrlDqDJjKWtWntMia9RrZqZPznr0yIDjeroWxUUzpPVV8UK4qUF4eilYR3Dz42");
        }
    }
    complex->ReturnLoanedValue(my_array_string);

    DynamicData *multi_alias_array_42 = complex->LoanValue(complex->GetMemberIdByName("multi_alias_array_42"));
    for (int j = 0; j < 42; ++j)
    {
        multi_alias_array_42->SetEnumValue(j % 3, j);
        //staticData.my_union().complex().multi_alias_array_42()[i](i%3);
    }
    complex->ReturnLoanedValue(multi_alias_array_42);

    DynamicData *my_array_arrays = complex->LoanValue(complex->GetMemberIdByName("my_array_arrays"));
    for (unsigned int j = 0; j < 5; ++j)
    {
        DynamicData *myMiniArray = my_array_arrays->LoanValue(j);
        for (unsigned int k = 0; k < 2; ++k)
        {
            myMiniArray->SetInt32Value(j*k, k);
            //staticData.my_union().complex().my_array_arrays()[i][j](i*j);
        }
        my_array_arrays->ReturnLoanedValue(myMiniArray);
    }
    complex->ReturnLoanedValue(my_array_arrays);

    DynamicData *my_sequences_array = complex->LoanValue(complex->GetMemberIdByName("my_sequences_array"));
    for (int j = 0; j < 23; ++j)
    {
        DynamicData *seq = DynamicDataFactory::GetInstance()->CreateData(GetMySequenceLongType());
        seq->InsertSequenceData(id);
        seq->SetInt32Value(j, id);
        seq->InsertSequenceData(id);
        seq->SetInt32Value(j * 10, id);
        seq->InsertSequenceData(id);
        seq->SetInt32Value(j * 100, id);
        my_sequences_array->SetComplexValue(seq, j);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i*10);
        // staticData.my_union().complex().my_sequences_array()[i].push_back(i*100);
    }
    complex->ReturnLoanedValue(my_sequences_array);

    my_union->ReturnLoanedValue(complex);
    dynData->ReturnLoanedValue(my_union);

    DynamicData *my_union_2 = dynData->LoanValue(dynData->GetMemberIdByName("my_union_2"));
    my_union_2->SetInt32Value(156, my_union_2->GetMemberIdByName("tres"));

    dynData->ReturnLoanedValue(my_union_2);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData.get())());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(dynData.get(), &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    /*
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
    */
    CompleteStructPubSubType pbComplete;
    uint32_t payloadSize2 = static_cast<uint32_t>(m_StaticType.getSerializedSizeProvider(&staticData)());
    SerializedPayload_t stPayload(payloadSize2);
    ASSERT_TRUE(pbComplete.serialize(&staticData, &stPayload));
    ASSERT_TRUE(stPayload.length == payloadSize2);
    /*
    std::cout << "BEGIN" << std::endl;
    for (uint32_t j = 0; j < stPayload.length; j += 100)
    {
    std::cout << std::endl;
    for (uint32_t k = 0; k < 100; k++)
    {
    if (j + k < stPayload.length)
    {
    if ((int)stPayload.data[j + k] == 204)
    {
    std::cout << 0 << " ";
    }
    else
    {
    std::cout << (int)stPayload.data[j + k] << " ";
    }
    }
    }
    }
    std::cout << "END" << std::endl;
    */
    types::DynamicData_ptr dynDataFromDynamic = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);
    ASSERT_TRUE(pubsubType.deserialize(&payload, dynDataFromDynamic.get()));

    types::DynamicData_ptr dynDataFromStatic = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);
    ASSERT_TRUE(pubsubType.deserialize(&stPayload, dynDataFromStatic.get()));

    ASSERT_TRUE(dynDataFromStatic->Equals(dynDataFromDynamic.get()));
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
    staticData.basic().my_wchar(L'M');
    staticData.basic().my_string("G It's");
    staticData.basic().my_wstring(L" Working");
    //staticData.key(88);

    DynamicData *dynData = DynamicDataFactory::GetInstance()->CreateData(GetKeyedStructType());
    DynamicData *basic = dynData->LoanValue(dynData->GetMemberIdByName("basic"));
    basic->SetBoolValue(true, basic->GetMemberIdByName("my_bool"));
    basic->SetByteValue(100, basic->GetMemberIdByName("my_octet"));
    basic->SetInt16Value(-12000, basic->GetMemberIdByName("my_int16"));
    basic->SetInt32Value(-12000000, basic->GetMemberIdByName("my_int32"));
    basic->SetInt64Value(-1200000000, basic->GetMemberIdByName("my_int64"));
    basic->SetUint16Value(12000, basic->GetMemberIdByName("my_uint16"));
    basic->SetUint32Value(12000000, basic->GetMemberIdByName("my_uint32"));
    basic->SetUint64Value(1200000000, basic->GetMemberIdByName("my_uint64"));
    basic->SetFloat32Value(5.5f, basic->GetMemberIdByName("my_float32"));
    basic->SetFloat64Value(8.888, basic->GetMemberIdByName("my_float64"));
    basic->SetFloat128Value(1005.1005, basic->GetMemberIdByName("my_float128"));
    basic->SetChar8Value('O', basic->GetMemberIdByName("my_char"));
    basic->SetChar16Value(L'M', basic->GetMemberIdByName("my_wchar"));
    basic->SetStringValue("G It's", basic->GetMemberIdByName("my_string"));
    basic->SetWstringValue(L" Working", basic->GetMemberIdByName("my_wstring"));
    dynData->ReturnLoanedValue(basic);
    //dynData->SetByteValue(88, dynData->GetMemberIdByName("key"));

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

    types::DynamicData* dynDataFromStatic = DynamicDataFactory::GetInstance()->CreateData(GetKeyedStructType());
    ASSERT_TRUE(pubsubType.deserialize(&stPayload, dynDataFromStatic));

    types::DynamicData* dynDataFromDynamic = DynamicDataFactory::GetInstance()->CreateData(GetKeyedStructType());
    ASSERT_TRUE(dynPubSub.deserialize(&dynPayload, dynDataFromDynamic));

    ASSERT_TRUE(dynDataFromStatic->Equals(dynDataFromDynamic));

    DynamicDataFactory::GetInstance()->DeleteData(dynData);
    DynamicDataFactory::GetInstance()->DeleteData(dynDataFromStatic);
    DynamicDataFactory::GetInstance()->DeleteData(dynDataFromDynamic);
}

int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
