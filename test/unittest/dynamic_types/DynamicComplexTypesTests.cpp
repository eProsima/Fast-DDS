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

        // Static types
        //CompleteStruct m_Static;
        CompleteStructPubSubType m_StaticType;
        // Dynamic Types
        //DynamicData* m_DynAuto;
        types::DynamicType_ptr m_DynAutoType;
        //DynamicData* m_DynManual;
        types::DynamicType_ptr m_DynManualType;
        DynamicTypeBuilderFactory* m_factory;
};

void DynamicComplexTypesTests::init()
{
    m_factory = DynamicTypeBuilderFactory::GetInstance();

    const TypeIdentifier *id = TypeObjectFactory::GetInstance()->GetTypeIdentifier("CompleteStruct", true);
    const TypeObject *obj = TypeObjectFactory::GetInstance()->GetTypeObject(id);
    m_DynAutoType = TypeObjectFactory::GetInstance()->BuildDynamicType("CompleteStruct", id, obj)->Build();
    //m_DynAutoType = nullptr;

    // Manual creation
    // MyEnum
    DynamicTypeBuilder_ptr myEnum_builder = m_factory->CreateUint32Builder();
    myEnum_builder->SetName("MyEnum");

    // MyAliasEnum
    DynamicTypeBuilder_ptr myAliasEnum_builder = m_factory->CreateAliasBuilder(myEnum_builder.get(), "MyAliasEnum");

    // MyAliasEnum2
    DynamicTypeBuilder_ptr myAliasEnum2_builder = m_factory->CreateAliasBuilder(myAliasEnum_builder.get(), "MyAliasEnum2");

    // MyAliasEnum3
    DynamicTypeBuilder_ptr myAliasEnum3_builder = m_factory->CreateAliasBuilder(myAliasEnum2_builder.get(), "MyAliasEnum3");

    // BasicStruct

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

    // MyOctetArray500
    std::vector<uint32_t> myOctetArray500_lengths = { 500 };
    DynamicTypeBuilder_ptr myOctetArray500_builder = m_factory->CreateArrayBuilder(octet_builder.get(), myOctetArray500_lengths);
    myOctetArray500_builder->SetName("MyOctetArray500");

    // BSAlias5
    std::vector<uint32_t> bSAlias5_lengths = { 5 };
    DynamicTypeBuilder_ptr bSAlias5_builder = m_factory->CreateArrayBuilder(basicStruct_builder.get(), bSAlias5_lengths);
    bSAlias5_builder->SetName("BSAlias5");

    // MA3
    std::vector<uint32_t> mA3_lengths = { 42 };
    DynamicTypeBuilder_ptr mA3_builder = m_factory->CreateArrayBuilder(myAliasEnum3_builder.get(), mA3_lengths);
    mA3_builder->SetName("MA3");

    // MyMiniArray
    std::vector<uint32_t> myMiniArray_lengths = { 2 };
    DynamicTypeBuilder_ptr myMiniArray_builder = m_factory->CreateArrayBuilder(int32_builder.get(), myMiniArray_lengths);
    myMiniArray_builder->SetName("MyMiniArray");

    // MySequenceLong
    DynamicTypeBuilder_ptr seqLong_builder = m_factory->CreateSequenceBuilder(int32_builder.get());
    DynamicTypeBuilder_ptr mySequenceLong_builder = m_factory->CreateAliasBuilder(seqLong_builder.get(), "MySequenceLong");

    // ComplexStruct
        // Members (auxiliar types are tab)
        // octet, BasicStruct, MyAliasEnum and MyEnum are already created.
    DynamicTypeBuilder_ptr my_sequence_octet_builder = m_factory->CreateSequenceBuilder(octet_builder.get(), 55);
    DynamicTypeBuilder_ptr my_sequence_struct_builder = m_factory->CreateSequenceBuilder(basicStruct_builder.get());
    DynamicTypeBuilder_ptr my_array_octet_builder = m_factory->CreateArrayBuilder(char_builder.get(), { 500, 5, 4 });
        // MyOctetArray500 is already created
            // We reuse the bounds... bSAlias5_lengths
    DynamicTypeBuilder_ptr my_array_struct_builder = m_factory->CreateArrayBuilder(basicStruct_builder.get(), bSAlias5_lengths);
    DynamicTypeBuilder_ptr my_map_octet_short_builder = m_factory->CreateMapBuilder(octet_builder.get(), int16_builder.get());
    DynamicTypeBuilder_ptr my_map_long_struct_builder = m_factory->CreateMapBuilder(int32_builder.get(), basicStruct_builder.get());
    DynamicTypeBuilder_ptr seqOctet_builder = m_factory->CreateSequenceBuilder(octet_builder.get());
    DynamicTypeBuilder_ptr seqSeqOctet_builder = m_factory->CreateSequenceBuilder(seqOctet_builder.get());
    DynamicTypeBuilder_ptr my_map_long_seq_octet_builder = m_factory->CreateMapBuilder(int32_builder.get(), seqSeqOctet_builder.get());
    DynamicTypeBuilder_ptr my_map_long_octet_array_500_builder = m_factory->CreateMapBuilder(int32_builder.get(), myOctetArray500_builder.get());
    DynamicTypeBuilder_ptr map_octet_bsalias5_builder = m_factory->CreateMapBuilder(octet_builder.get(), bSAlias5_builder.get());
    DynamicTypeBuilder_ptr my_map_long_lol_type_builder = m_factory->CreateMapBuilder(int32_builder.get(), map_octet_bsalias5_builder.get());
    DynamicTypeBuilder_ptr my_small_string_8_builder = m_factory->CreateStringBuilder(128);
    DynamicTypeBuilder_ptr my_small_string_16_builder = m_factory->CreateWstringBuilder(64);
    DynamicTypeBuilder_ptr my_large_string_8_builder = m_factory->CreateStringBuilder(500);
    DynamicTypeBuilder_ptr my_large_string_16_builder = m_factory->CreateWstringBuilder(1024);
    DynamicTypeBuilder_ptr string75_8_builder = m_factory->CreateStringBuilder(75);
    DynamicTypeBuilder_ptr my_array_string_builder = m_factory->CreateArrayBuilder(string75_8_builder.get(), { 5, 5 });

        // MA3 is already defined.
        // bSAlias5_lengths being reused
    DynamicTypeBuilder_ptr my_array_arrays_builder = m_factory->CreateArrayBuilder(myMiniArray_builder.get(), bSAlias5_lengths);
    DynamicTypeBuilder_ptr my_sequences_array_builder = m_factory->CreateArrayBuilder(mySequenceLong_builder.get(), { 23 });
    DynamicTypeBuilder_ptr complexStruct_builder = m_factory->CreateStructBuilder();

        // Add members to the struct.
        idx = 0;
        complexStruct_builder->AddMember(idx++, "my_octet", octet_builder.get());
        complexStruct_builder->AddMember(idx++, "my_basic_struct", basicStruct_builder.get());
        complexStruct_builder->AddMember(idx++, "my_alias_enum", myAliasEnum_builder.get());
        complexStruct_builder->AddMember(idx++, "my_enum", myEnum_builder.get());
        complexStruct_builder->AddMember(idx++, "my_sequence_octet", my_sequence_octet_builder.get());
        complexStruct_builder->AddMember(idx++, "my_sequence_struct", my_sequence_struct_builder.get());
        complexStruct_builder->AddMember(idx++, "my_array_octet", my_array_octet_builder.get());
        complexStruct_builder->AddMember(idx++, "my_octet_array_500", myOctetArray500_builder.get());
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
        complexStruct_builder->AddMember(idx++, "multi_alias_array_42", mA3_builder.get());
        complexStruct_builder->AddMember(idx++, "my_array_arrays", my_array_arrays_builder.get());
        complexStruct_builder->AddMember(idx++, "my_sequences_array", my_sequences_array_builder.get());
    complexStruct_builder->SetName("ComplexStruct");

    // MyUnion
    DynamicTypeBuilder_ptr myUnion_builder = m_factory->CreateUnionBuilder(octet_builder.get());
    myUnion_builder->AddMember(0, "basic", basicStruct_builder.get(), "", { 0 }, true);
    myUnion_builder->AddMember(1, "complex", complexStruct_builder.get(), "", { 1, 2 }, false);
    myUnion_builder->SetName("MyUnion");

    // MyUnion2
    DynamicTypeBuilder_ptr myUnion2_builder = m_factory->CreateUnionBuilder(octet_builder.get());
    myUnion2_builder->AddMember(0, "uno", int32_builder.get(), "", { 0 }, true);
    myUnion2_builder->AddMember(1, "imString", string_builder.get(), "", { 1 }, false);
    myUnion2_builder->AddMember(2, "dos", int32_builder.get(), "", { 2 }, false);

    // CompleteStruct
    DynamicTypeBuilder_ptr completeStruct_builder = m_factory->CreateStructBuilder();
    // Add members to the struct.
    idx = 0;
    completeStruct_builder->AddMember(idx++, "my_union", myUnion_builder.get());
    completeStruct_builder->AddMember(idx++, "my_union_2", myUnion2_builder.get());
    completeStruct_builder->SetName("CompleteStruct");

    m_DynManualType = completeStruct_builder->Build();
}

/*

        CompleteStruct m_Static;
        CompleteStructPubSubType m_StaticType;
        // Dynamic Types
        DynamicData* m_DynAuto;
        DynamicType* m_DynAutoType;
        DynamicData* m_DynManual;
        DynamicType* m_DynManualType;
*/

TEST_F(DynamicComplexTypesTests, Static_Manual_Comparision)
{
    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(m_DynManualType);
    types::DynamicData* dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(dynData)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(dynData, &payload));

    CompleteStruct staticData;
    ASSERT_TRUE(m_StaticType.deserialize(&payload, &staticData));
    ASSERT_TRUE(m_StaticType.serialize(&staticData, &payload));

    types::DynamicData* dynData2 = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);
    ASSERT_TRUE(pubsubType.deserialize(&payload, dynData2));

    ASSERT_TRUE(dynData2->Equals(dynData));

    DynamicDataFactory::GetInstance()->DeleteData(dynData);
    DynamicDataFactory::GetInstance()->DeleteData(dynData2);
}

TEST_F(DynamicComplexTypesTests, Static_Auto_Comparision)
{
    // Serialize <-> Deserialize Test
    types::DynamicData* dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);
    uint32_t payloadSize = static_cast<uint32_t>(m_DynAutoType->getSerializedSizeProvider(dynData)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(m_DynAutoType->serialize(dynData, &payload));

    CompleteStruct staticData;
    ASSERT_TRUE(m_StaticType.deserialize(&payload, &staticData));
    ASSERT_TRUE(m_StaticType.serialize(&staticData, &payload));

    types::DynamicData* dynData2 = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);
    ASSERT_TRUE(m_DynAutoType->deserialize(&payload, dynData2));

    ASSERT_TRUE(dynData2->Equals(dynData));

    DynamicDataFactory::GetInstance()->DeleteData(dynData);
    DynamicDataFactory::GetInstance()->DeleteData(dynData2);
}
/*
TEST_F(DynamicComplexTypesTests, Manual_Auto_Comparision)
{
    types::DynamicData* dynAutoData = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);
    types::DynamicData* dynManualData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);

    ASSERT_TRUE(dynManualData->Equals(dynAutoData));

    DynamicDataFactory::GetInstance()->DeleteData(dynAutoData);
    DynamicDataFactory::GetInstance()->DeleteData(dynManualData);
}

TEST_F(DynamicComplexTypesTests, Conversions_Test)
{
    TypeObject newObject;
    DynamicTypeBuilderFactory::GetInstance()->BuildTypeObject(m_DynManualType, newObject);
    types::DynamicData* dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);

    TypeIdentifier* newIdentifier = TypeObjectFactory::GetInstance()->GetTypeIdentifier(m_DynManualType->GetName());
    DynamicType* newAutoType = TypeObjectFactory::GetInstance()->BuildDynamicType(newIdentifier, &newObject);

    ASSERT_TRUE(newAutoType->Equals(dynData));

    DynamicDataFactory::GetInstance()->DeleteData(dynData);
    DynamicDataFactory::GetInstance()->DeleteData(newAutoType);
}
*/

// TODO
/*
-> Static_Auto
-> Manual_Auto
-> Generate TypeObject from Manual and verify its correct generating another auto from it and comparing with the manual.
*/

TEST_F(DynamicComplexTypesTests, DynamicDiscoveryTest)
{
    TypeObject typeObject1, typeObject2, typeObject3;
    DynamicTypeBuilder_ptr type1, type2, type3;
    {
        type1 = DynamicTypeBuilderFactory::GetInstance()->CreateUint16Builder();
        //types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(type1);
        DynamicTypeBuilderFactory::GetInstance()->BuildTypeObject(type1->getTypeDescriptor(), typeObject1);
    }
    {
        type2 = DynamicTypeBuilderFactory::GetInstance()->CreateInt16Builder();
        //types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(type2);
        DynamicTypeBuilderFactory::GetInstance()->BuildTypeObject(type2->getTypeDescriptor(), typeObject2);
    }

    {
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateInt16Builder();
        //types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(type3);
        DynamicTypeBuilderFactory::GetInstance()->BuildTypeObject(type3->getTypeDescriptor(), typeObject3);
    }

    const TypeIdentifier* identifier1 = TypeObjectFactory::GetInstance()->GetTypeIdentifier(type1->GetName());
    const TypeIdentifier* identifier2 = TypeObjectFactory::GetInstance()->GetTypeIdentifier(type2->GetName());
    const TypeIdentifier* identifier3 = TypeObjectFactory::GetInstance()->GetTypeIdentifier(type3->GetName());
    ASSERT_FALSE(*identifier1 == *identifier2);
    ASSERT_FALSE(*identifier1 == *identifier3);
    ASSERT_TRUE(*identifier2 == *identifier3);
}

TEST_F(DynamicComplexTypesTests, StaticVsDynamicDiscovery)
{
    BasicStruct test;
    (void)test;

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
    basicStruct_builder->SetName("BasicStruct2");

    TypeObject dynamicTypeObject;
    std::map<MemberId, DynamicTypeMember*> membersMap;
    basicStruct_builder->GetAllMembers(membersMap);
    std::vector<const types::MemberDescriptor*> innerMembers;
    for (auto it : membersMap)
    {
        innerMembers.push_back(it.second->GetDescriptor());
    }
    DynamicTypeBuilderFactory::GetInstance()->BuildTypeObject(basicStruct_builder->getTypeDescriptor(),
        dynamicTypeObject, &innerMembers);

    const TypeIdentifier* static_identifier = TypeObjectFactory::GetInstance()->GetTypeIdentifier("BasicStruct");
    const TypeIdentifier* dynamic_identifier = TypeObjectFactory::GetInstance()->GetTypeIdentifier("BasicStruct2");
    ASSERT_TRUE(*static_identifier == *dynamic_identifier);
}

int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
