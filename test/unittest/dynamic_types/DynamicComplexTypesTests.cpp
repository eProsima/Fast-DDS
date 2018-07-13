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
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
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
            init();
        }

        ~DynamicComplexTypesTests()
        {
            if (m_DynAutoType != nullptr)
            {
                m_factory->DeleteType(m_DynAutoType);
            }
            //DynamicDataFactory::GetInstance()->DeleteData(m_DynAuto);

            if (m_DynManualType != nullptr)
            {
                m_factory->DeleteType(m_DynManualType);
            }
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
        types::DynamicType* m_DynAutoType;
        //DynamicData* m_DynManual;
        types::DynamicType* m_DynManualType;
        DynamicTypeBuilderFactory* m_factory;
};

void DynamicComplexTypesTests::init()
{
    m_factory = DynamicTypeBuilderFactory::GetInstance();

    const TypeIdentifier *id = TypeObjectFactory::GetInstance()->GetTypeIdentifier("CompleteStruct");
    const TypeObject *obj = TypeObjectFactory::GetInstance()->GetTypeObject(id);
    m_DynAutoType = TypeObjectFactory::GetInstance()->BuildDynamicType(id, obj);

    // Manual creation
    // MyEnum
    DynamicTypeBuilder* myEnum_builder = m_factory->CreateUint32Type();
    myEnum_builder->SetName("MyEnum");

    // MyAliasEnum
    DynamicTypeBuilder* myAliasEnum_builder = m_factory->CreateAliasType(myEnum_builder, "MyAliasEnum");

    // MyAliasEnum2
    DynamicTypeBuilder* myAliasEnum2_builder = m_factory->CreateAliasType(myAliasEnum_builder, "MyAliasEnum2");

    // MyAliasEnum3
    DynamicTypeBuilder* myAliasEnum3_builder = m_factory->CreateAliasType(myAliasEnum2_builder, "MyAliasEnum3");

    // BasicStruct

    // Members
    DynamicTypeBuilder* bool_builder = m_factory->CreateBoolType();
    DynamicTypeBuilder* octet_builder = m_factory->CreateByteType();
    DynamicTypeBuilder* int16_builder = m_factory->CreateInt16Type();
    DynamicTypeBuilder* int32_builder = m_factory->CreateInt32Type();
    DynamicTypeBuilder* int64_builder = m_factory->CreateInt64Type();
    DynamicTypeBuilder* uint16_builder = m_factory->CreateUint16Type();
    DynamicTypeBuilder* uint32_builder = m_factory->CreateUint32Type();
    DynamicTypeBuilder* uint64_builder = m_factory->CreateUint64Type();
    DynamicTypeBuilder* float_builder = m_factory->CreateFloat32Type();
    DynamicTypeBuilder* double_builder = m_factory->CreateFloat64Type();
    DynamicTypeBuilder* ldouble_builder = m_factory->CreateFloat128Type();
    DynamicTypeBuilder* char_builder = m_factory->CreateChar8Type();
    DynamicTypeBuilder* wchar_builder = m_factory->CreateChar16Type();
    DynamicTypeBuilder* string_builder = m_factory->CreateStringType();
    DynamicTypeBuilder* wstring_builder = m_factory->CreateWstringType();
    DynamicTypeBuilder* basicStruct_builder = m_factory->CreateStructType();

    // Add members to the struct.
    int idx = 0;
    basicStruct_builder->AddMember(idx++, "my_bool", bool_builder);
    basicStruct_builder->AddMember(idx++, "my_octet", octet_builder);
    basicStruct_builder->AddMember(idx++, "my_int16", int16_builder);
    basicStruct_builder->AddMember(idx++, "my_int32", int32_builder);
    basicStruct_builder->AddMember(idx++, "my_int64", int64_builder);
    basicStruct_builder->AddMember(idx++, "my_uint16", uint16_builder);
    basicStruct_builder->AddMember(idx++, "my_uint32", uint32_builder);
    basicStruct_builder->AddMember(idx++, "my_uint64", uint64_builder);
    basicStruct_builder->AddMember(idx++, "my_float32", float_builder);
    basicStruct_builder->AddMember(idx++, "my_float64", double_builder);
    basicStruct_builder->AddMember(idx++, "my_float128", ldouble_builder);
    basicStruct_builder->AddMember(idx++, "my_char", char_builder);
    basicStruct_builder->AddMember(idx++, "my_wchar", wchar_builder);
    basicStruct_builder->AddMember(idx++, "my_string", string_builder);
    basicStruct_builder->AddMember(idx++, "my_wstring", wstring_builder);
    basicStruct_builder->SetName("BasicStruct");

    // MyOctetArray500
    std::vector<uint32_t> myOctetArray500_lengths = { 500 };
    DynamicTypeBuilder* myOctetArray500_builder = m_factory->CreateArrayType(octet_builder, myOctetArray500_lengths);
    myOctetArray500_builder->SetName("MyOctetArray500");

    // BSAlias5
    std::vector<uint32_t> bSAlias5_lengths = { 5 };
    DynamicTypeBuilder* bSAlias5_builder = m_factory->CreateArrayType(basicStruct_builder, bSAlias5_lengths);
    bSAlias5_builder->SetName("BSAlias5");

    // MA3
    std::vector<uint32_t> mA3_lengths = { 42 };
    DynamicTypeBuilder* mA3_builder = m_factory->CreateArrayType(myAliasEnum3_builder, mA3_lengths);
    mA3_builder->SetName("MA3");

    // MyMiniArray
    std::vector<uint32_t> myMiniArray_lengths = { 2 };
    DynamicTypeBuilder* myMiniArray_builder = m_factory->CreateArrayType(int32_builder, myMiniArray_lengths);
    myMiniArray_builder->SetName("MyMiniArray");

    // MySequenceLong
    DynamicTypeBuilder* seqLong_builder = m_factory->CreateSequenceType(int32_builder);
    DynamicTypeBuilder* mySequenceLong_builder = m_factory->CreateAliasType(seqLong_builder, "MySequenceLong");

    // ComplexStruct
        // Members (auxiliar types are tab)
        // octet, BasicStruct, MyAliasEnum and MyEnum are already created.
        DynamicTypeBuilder* my_sequence_octet_builder = m_factory->CreateSequenceType(octet_builder, 55);
        DynamicTypeBuilder* my_sequence_struct_builder = m_factory->CreateSequenceType(basicStruct_builder);
        DynamicTypeBuilder* my_array_octet_builder = m_factory->CreateArrayType(char_builder, { 500, 5, 4 });
        // MyOctetArray500 is already created
            // We reuse the bounds... bSAlias5_lengths
        DynamicTypeBuilder* my_array_struct_builder = m_factory->CreateArrayType(basicStruct_builder, bSAlias5_lengths);
        DynamicTypeBuilder* my_map_octet_short_builder = m_factory->CreateMapType(octet_builder, int16_builder);
        DynamicTypeBuilder* my_map_long_struct_builder = m_factory->CreateMapType(int32_builder, basicStruct_builder);
            DynamicTypeBuilder* seqOctet_builder = m_factory->CreateSequenceType(octet_builder);
            DynamicTypeBuilder* seqSeqOctet_builder = m_factory->CreateSequenceType(seqOctet_builder);
        DynamicTypeBuilder* my_map_long_seq_octet_builder = m_factory->CreateMapType(int32_builder, seqSeqOctet_builder);
        DynamicTypeBuilder* my_map_long_octet_array_500_builder = m_factory->CreateMapType(int32_builder, myOctetArray500_builder);
            DynamicTypeBuilder* map_octet_bsalias5_builder = m_factory->CreateMapType(octet_builder, bSAlias5_builder);
        DynamicTypeBuilder* my_map_long_lol_type_builder = m_factory->CreateMapType(int32_builder, map_octet_bsalias5_builder);
        DynamicTypeBuilder* my_small_string_8_builder = m_factory->CreateStringType(128);
        DynamicTypeBuilder* my_small_string_16_builder = m_factory->CreateWstringType(64);
        DynamicTypeBuilder* my_large_string_8_builder = m_factory->CreateStringType(500);
        DynamicTypeBuilder* my_large_string_16_builder = m_factory->CreateWstringType(1024);
            DynamicTypeBuilder* string75_8_builder = m_factory->CreateStringType(75);
        DynamicTypeBuilder* my_array_string_builder = m_factory->CreateArrayType(string75_8_builder, { 5, 5 });

        // MA3 is already defined.
        // bSAlias5_lengths being reused
        DynamicTypeBuilder* my_array_arrays_builder = m_factory->CreateArrayType(myMiniArray_builder, bSAlias5_lengths);
        DynamicTypeBuilder* my_sequences_array_builder = m_factory->CreateArrayType(mySequenceLong_builder, { 23 });
        DynamicTypeBuilder* complexStruct_builder = m_factory->CreateStructType();

        // Add members to the struct.
        idx = 0;
        complexStruct_builder->AddMember(idx++, "my_octet", octet_builder);
        complexStruct_builder->AddMember(idx++, "my_basic_struct", basicStruct_builder);
        complexStruct_builder->AddMember(idx++, "my_alias_enum", myAliasEnum_builder);
        complexStruct_builder->AddMember(idx++, "my_enum", myEnum_builder);
        complexStruct_builder->AddMember(idx++, "my_sequence_octet", my_sequence_octet_builder);
        complexStruct_builder->AddMember(idx++, "my_sequence_struct", my_sequence_struct_builder);
        complexStruct_builder->AddMember(idx++, "my_array_octet", my_array_octet_builder);
        complexStruct_builder->AddMember(idx++, "my_octet_array_500", myOctetArray500_builder);
        complexStruct_builder->AddMember(idx++, "my_array_struct", my_array_struct_builder);
        complexStruct_builder->AddMember(idx++, "my_map_octet_short", my_map_octet_short_builder);
        complexStruct_builder->AddMember(idx++, "my_map_long_struct", my_map_long_struct_builder);
        complexStruct_builder->AddMember(idx++, "my_map_long_seq_octet", my_map_long_seq_octet_builder);
        complexStruct_builder->AddMember(idx++, "my_map_long_octet_array_500", my_map_long_octet_array_500_builder);
        complexStruct_builder->AddMember(idx++, "my_map_long_lol_type", my_map_long_lol_type_builder);
        complexStruct_builder->AddMember(idx++, "my_small_string_8", my_small_string_8_builder);
        complexStruct_builder->AddMember(idx++, "my_small_string_16", my_small_string_16_builder);
        complexStruct_builder->AddMember(idx++, "my_large_string_8", my_large_string_8_builder);
        complexStruct_builder->AddMember(idx++, "my_large_string_16", my_large_string_16_builder);
        complexStruct_builder->AddMember(idx++, "my_array_string", my_array_string_builder);
        complexStruct_builder->AddMember(idx++, "multi_alias_array_42", mA3_builder);
        complexStruct_builder->AddMember(idx++, "my_array_arrays", my_array_arrays_builder);
        complexStruct_builder->AddMember(idx++, "my_sequences_array", my_sequences_array_builder);
    complexStruct_builder->SetName("ComplexStruct");

    // MyUnion
    DynamicTypeBuilder* myUnion_builder = m_factory->CreateUnionType(octet_builder);
    myUnion_builder->AddMember(0, "basic", basicStruct_builder, "", { 0 }, true);
    myUnion_builder->AddMember(1, "complex", complexStruct_builder, "", { 1, 2 }, false);
    myUnion_builder->SetName("MyUnion");

    // MyUnion2
    DynamicTypeBuilder* myUnion2_builder = m_factory->CreateUnionType(octet_builder);
    myUnion2_builder->AddMember(0, "uno", int32_builder, "", { 0 }, true);
    myUnion2_builder->AddMember(1, "imString", string_builder, "", { 1 }, false);
    myUnion2_builder->AddMember(2, "dos", int32_builder, "", { 2 }, false);

    // CompleteStruct
    DynamicTypeBuilder* completeStruct_builder = m_factory->CreateStructType();
    // Add members to the struct.
    idx = 0;
    completeStruct_builder->AddMember(idx++, "my_union", myUnion_builder);
    completeStruct_builder->AddMember(idx++, "my_union_2", myUnion2_builder);
    completeStruct_builder->SetName("CompleteStruct");

    m_DynManualType = completeStruct_builder->Build();

    // Clear the builders
    m_factory->DeleteType(myEnum_builder);
    m_factory->DeleteType(myAliasEnum_builder);
    m_factory->DeleteType(myAliasEnum2_builder);
    m_factory->DeleteType(myAliasEnum3_builder);

    m_factory->DeleteType(bool_builder);
    m_factory->DeleteType(octet_builder);
    m_factory->DeleteType(int16_builder);
    m_factory->DeleteType(int32_builder);
    m_factory->DeleteType(int64_builder);
    m_factory->DeleteType(uint16_builder);
    m_factory->DeleteType(uint32_builder);
    m_factory->DeleteType(uint64_builder);
    m_factory->DeleteType(float_builder);
    m_factory->DeleteType(double_builder);
    m_factory->DeleteType(ldouble_builder);
    m_factory->DeleteType(char_builder);
    m_factory->DeleteType(wchar_builder);
    m_factory->DeleteType(string_builder);
    m_factory->DeleteType(wstring_builder);
    m_factory->DeleteType(basicStruct_builder);

    m_factory->DeleteType(myOctetArray500_builder);
    m_factory->DeleteType(bSAlias5_builder);
    m_factory->DeleteType(mA3_builder);
    m_factory->DeleteType(myMiniArray_builder);
    m_factory->DeleteType(seqLong_builder);
    m_factory->DeleteType(mySequenceLong_builder);

    m_factory->DeleteType(my_array_arrays_builder);
    m_factory->DeleteType(my_sequences_array_builder);
    m_factory->DeleteType(complexStruct_builder);
    m_factory->DeleteType(my_sequence_octet_builder);
    m_factory->DeleteType(my_sequence_struct_builder);
    m_factory->DeleteType(my_array_octet_builder);
    m_factory->DeleteType(my_array_struct_builder);
    m_factory->DeleteType(my_map_octet_short_builder);
    m_factory->DeleteType(my_map_long_struct_builder);
    m_factory->DeleteType(seqOctet_builder);
    m_factory->DeleteType(seqSeqOctet_builder);
    m_factory->DeleteType(my_map_long_seq_octet_builder);
    m_factory->DeleteType(my_map_long_octet_array_500_builder);
    m_factory->DeleteType(map_octet_bsalias5_builder);
    m_factory->DeleteType(my_map_long_lol_type_builder);
    m_factory->DeleteType(my_small_string_8_builder);
    m_factory->DeleteType(my_small_string_16_builder);
    m_factory->DeleteType(my_large_string_8_builder);
    m_factory->DeleteType(my_large_string_16_builder);
    m_factory->DeleteType(string75_8_builder);
    m_factory->DeleteType(my_array_string_builder);

    m_factory->DeleteType(myUnion_builder);
    m_factory->DeleteType(myUnion2_builder);
    m_factory->DeleteType(completeStruct_builder);
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
    types::DynamicData* dynData = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);
    uint32_t payloadSize = static_cast<uint32_t>(m_DynManualType->getSerializedSizeProvider(dynData)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(m_DynManualType->serialize(dynData, &payload));

    CompleteStruct staticData;
    ASSERT_TRUE(m_StaticType.deserialize(&payload, &staticData));
    ASSERT_TRUE(m_StaticType.serialize(&staticData, &payload));

    types::DynamicData* dynData2 = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);
    ASSERT_TRUE(m_DynManualType->deserialize(&payload, dynData2));

    ASSERT_TRUE(dynData2->Equals(dynData));

    DynamicDataFactory::GetInstance()->DeleteData(dynData);
    DynamicDataFactory::GetInstance()->DeleteData(dynData2);
}

// TODO
/*
-> Static_Auto
-> Manual_Auto
*/

int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
