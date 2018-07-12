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
            factory->DeleteType(m_DynAutoType);
            //DynamicDataFactory::GetInstance()->DeleteData(m_DynAuto);
            factory->DeleteType(m_DynManualType);
            //DynamicDataFactory::GetInstance()->DeleteData(m_DynManual);

            if (!DynamicTypeBuilderFactory::GetInstance()->IsEmpty())
            {
                logError(DYN_TEST, "DynamicTypeBuilderFactory is not empty.");
            }

            if (!DynamicDataFactory::GetInstance()->IsEmpty())
            {
                logError(DYN_TEST, "DynamicDataFactory is not empty.");
            }

            Log::KillThread();
        }

        virtual void TearDown()
        {
            DynamicDataFactory::DeleteInstance();
            DynamicTypeBuilderFactory::DeleteInstance();
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
        DynamicTypeBuilderFactory* factory;
};

void DynamicComplexTypesTests::init()
{
    factory = DynamicTypeBuilderFactory::GetInstance();

    const TypeIdentifier *id = TypeObjectFactory::GetInstance()->GetTypeIdentifier("CompleteStruct");
    const TypeObject *obj = TypeObjectFactory::GetInstance()->GetTypeObject(id);
    m_DynAutoType = TypeObjectFactory::GetInstance()->BuildDynamicType(id, obj);
    //m_DynAuto = DynamicDataFactory::GetInstance()->CreateData(m_DynAutoType);

    // Manual creation
    // MyEnum
    DynamicTypeBuilder* myEnum_builder(nullptr);
    myEnum_builder = factory->CreateUint32Type();
    myEnum_builder->SetName("MyEnum");
    auto myEnum_type = myEnum_builder->Build();

    // MyAliasEnum
    DynamicTypeBuilder* myAliasEnum_builder(nullptr);
    myAliasEnum_builder = factory->CreateAliasType(myEnum_type, "MyAliasEnum");
    auto myAliasEnum_type = myAliasEnum_builder->Build();

    // MyAliasEnum2
    DynamicTypeBuilder* myAliasEnum2_builder(nullptr);
    myAliasEnum2_builder = factory->CreateAliasType(myAliasEnum_type, "MyAliasEnum2");
    auto myAliasEnum2_type = myAliasEnum2_builder->Build();

    // MyAliasEnum3
    DynamicTypeBuilder* myAliasEnum3_builder(nullptr);
    myAliasEnum3_builder = factory->CreateAliasType(myAliasEnum2_type, "MyAliasEnum3");
    auto myAliasEnum3_type = myAliasEnum3_builder->Build();

    // BasicStruct
        // Members
        DynamicTypeBuilder* bool_builder = factory->CreateBoolType();
        DynamicTypeBuilder* octet_builder = factory->CreateByteType();
        //DynamicTypeBuilder* short_builder = factory->CreateInt16Type();
        DynamicTypeBuilder* int16_builder = factory->CreateInt16Type();
        DynamicTypeBuilder* int32_builder = factory->CreateInt32Type();
        DynamicTypeBuilder* int64_builder = factory->CreateInt64Type();
        DynamicTypeBuilder* uint16_builder = factory->CreateUint16Type();
        DynamicTypeBuilder* uint32_builder = factory->CreateUint32Type();
        DynamicTypeBuilder* uint64_builder = factory->CreateUint64Type();
        DynamicTypeBuilder* float_builder = factory->CreateFloat32Type();
        DynamicTypeBuilder* double_builder = factory->CreateFloat64Type();
        DynamicTypeBuilder* ldouble_builder = factory->CreateFloat128Type();
        DynamicTypeBuilder* char_builder = factory->CreateChar8Type();
        DynamicTypeBuilder* wchar_builder = factory->CreateChar16Type();
        DynamicTypeBuilder* string_builder = factory->CreateStringType();
        DynamicTypeBuilder* wstring_builder = factory->CreateWstringType();
        auto boolType = bool_builder->Build();
        auto octetType = octet_builder->Build();
        //auto shortType = short_builder->Build();
        auto int16Type = int16_builder->Build();
        auto int32Type = int32_builder->Build();
        auto int64Type = int64_builder->Build();
        auto uint16Type = uint16_builder->Build();
        auto uint32Type = uint32_builder->Build();
        auto uint64Type = uint64_builder->Build();
        auto floatType = float_builder->Build();
        auto doubleType = double_builder->Build();
        auto ldoubleType = ldouble_builder->Build();
        auto charType = char_builder->Build();
        auto wcharType = wchar_builder->Build();
        auto stringType = string_builder->Build();
        auto wstringType = wstring_builder->Build();
        auto basicStruct_builder = factory->CreateStructType();
        // Add members to the struct.
        int idx = 0;
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_bool"); descriptor.SetType(boolType);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_octet"); descriptor.SetType(octetType);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_int16"); descriptor.SetType(int16Type);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_int32"); descriptor.SetType(int32Type);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_int64"); descriptor.SetType(int64Type);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_uint16"); descriptor.SetType(uint16Type);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_uint32"); descriptor.SetType(uint32Type);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_uint64"); descriptor.SetType(uint64Type);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_float32"); descriptor.SetType(floatType);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_float64"); descriptor.SetType(doubleType);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_float128"); descriptor.SetType(ldoubleType);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_char"); descriptor.SetType(charType);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_wchar"); descriptor.SetType(wcharType);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_string"); descriptor.SetType(stringType);
            basicStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_wstring"); descriptor.SetType(wstringType);
            basicStruct_builder->AddMember(&descriptor);
        }
    basicStruct_builder->SetName("BasicStruct");
    auto basicStruct_type = basicStruct_builder->Build();

    // MyOctetArray500
    std::vector<uint32_t> myOctetArray500_lengths = { 500 };
    DynamicTypeBuilder* myOctetArray500_builder = factory->CreateArrayType(octetType, myOctetArray500_lengths);
    myOctetArray500_builder->SetName("MyOctetArray500");
    auto myOctetArray500_type = myOctetArray500_builder->Build();

    // BSAlias5
    std::vector<uint32_t> bSAlias5_lengths = { 5 };
    DynamicTypeBuilder* bSAlias5_builder = factory->CreateArrayType(basicStruct_type, bSAlias5_lengths);
    bSAlias5_builder->SetName("BSAlias5");
    auto bSAlias5_type = bSAlias5_builder->Build();

    // MA3
    std::vector<uint32_t> mA3_lengths = { 42 };
    DynamicTypeBuilder* mA3_builder = factory->CreateArrayType(myAliasEnum3_type, mA3_lengths);
    mA3_builder->SetName("MA3");
    auto mA3_type = mA3_builder->Build();

    // MyMiniArray
    std::vector<uint32_t> myMiniArray_lengths = { 2 };
    DynamicTypeBuilder* myMiniArray_builder = factory->CreateArrayType(int32Type, myMiniArray_lengths);
    myMiniArray_builder->SetName("MyMiniArray");
    auto myMiniArray_type = myMiniArray_builder->Build();

    // MySequenceLong
    DynamicTypeBuilder* seqLong_builder = factory->CreateSequenceType(int32Type);
    auto seqLongType = seqLong_builder->Build();
    DynamicTypeBuilder* mySequenceLong_builder(nullptr);
    mySequenceLong_builder = factory->CreateAliasType(seqLongType, "MySequenceLong");
    auto mySequenceLong_type = mySequenceLong_builder->Build();

    // ComplexStruct
        // Members (auxiliar types are tab)
        // octet, BasicStruct, MyAliasEnum and MyEnum are already created.
        DynamicTypeBuilder* my_sequence_octet_builder = factory->CreateSequenceType(octetType, 55);
        DynamicTypeBuilder* my_sequence_struct_builder = factory->CreateSequenceType(basicStruct_type);
            std::vector<uint32_t> my_array_octet_lengths = { 500, 5, 4 };
        DynamicTypeBuilder* my_array_octet_builder = factory->CreateArrayType(charType, my_array_octet_lengths);
        // MyOctetArray500 is already created
            // We reuse the bounds... bSAlias5_lengths
        DynamicTypeBuilder* my_array_struct_builder = factory->CreateArrayType(basicStruct_type, bSAlias5_lengths);
        DynamicTypeBuilder* my_map_octet_short_builder = factory->CreateMapType(octetType, int16Type);
        DynamicTypeBuilder* my_map_long_struct_builder = factory->CreateMapType(int32Type, basicStruct_type);
            DynamicTypeBuilder* seqOctet_builder = factory->CreateSequenceType(octetType);
            auto seqOctet_type = seqOctet_builder->Build();
            DynamicTypeBuilder* seqSeqOctet_builder = factory->CreateSequenceType(seqOctet_type);
            auto seqSeqOctet_type = seqSeqOctet_builder->Build();
        DynamicTypeBuilder* my_map_long_seq_octet_builder = factory->CreateMapType(int32Type, seqSeqOctet_type);
        DynamicTypeBuilder* my_map_long_octet_array_500_builder =
                factory->CreateMapType(int32Type, myOctetArray500_type);
            DynamicTypeBuilder* map_octet_bsalias5_builder = factory->CreateMapType(octetType, bSAlias5_type);
            auto map_octet_bsalias5_type = map_octet_bsalias5_builder->Build();
        DynamicTypeBuilder* my_map_long_lol_type_builder = factory->CreateMapType(int32Type, map_octet_bsalias5_type);
        DynamicTypeBuilder* my_small_string_8_builder = factory->CreateStringType(128);
        DynamicTypeBuilder* my_small_string_16_builder = factory->CreateWstringType(64);
        DynamicTypeBuilder* my_large_string_8_builder = factory->CreateStringType(500);
        DynamicTypeBuilder* my_large_string_16_builder = factory->CreateWstringType(1024);
            DynamicTypeBuilder* string75_8_builder = factory->CreateStringType(75);
            auto string75_8_type = string75_8_builder->Build();
            std::vector<uint32_t> my_array_string_lengths = { 5, 5 };
        DynamicTypeBuilder* my_array_string_builder =
                factory->CreateArrayType(string75_8_type, my_array_string_lengths);
        // MA3 is already defined.
            // bSAlias5_lengths being reused
        DynamicTypeBuilder* my_array_arrays_builder = factory->CreateArrayType(myMiniArray_type, bSAlias5_lengths);
            std::vector<uint32_t> my_sequences_array_lengths = { 23 };
        DynamicTypeBuilder* my_sequences_array_builder =
                factory->CreateArrayType(mySequenceLong_type, my_sequences_array_lengths);

        auto my_sequence_octetType = my_sequence_octet_builder->Build();
        auto my_sequence_structType = my_sequence_struct_builder->Build();
        auto my_array_octetType = my_array_octet_builder->Build();
        auto my_array_structType = my_array_struct_builder->Build();
        auto my_map_octet_shortType = my_map_octet_short_builder->Build();
        auto my_map_long_structType = my_map_long_struct_builder->Build();
        auto my_map_long_seq_octetType = my_map_long_seq_octet_builder->Build();
        auto my_map_long_octet_array_500Type = my_map_long_octet_array_500_builder->Build();
        auto my_map_long_lol_typeType = my_map_long_lol_type_builder->Build();
        auto my_small_string_8Type = my_small_string_8_builder->Build();
        auto my_small_string_16Type = my_small_string_16_builder->Build();
        auto my_large_string_8Type = my_large_string_8_builder->Build();
        auto my_large_string_16Type = my_large_string_16_builder->Build();
        auto my_array_stringType = my_array_string_builder->Build();
        auto my_array_arraysType = my_array_arrays_builder->Build();
        auto my_sequences_arrayType = my_sequences_array_builder->Build();
        auto complexStruct_builder = factory->CreateStructType();
        // Add members to the struct.
        idx = 0;
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_octet"); descriptor.SetType(octetType);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_basic_struct"); descriptor.SetType(basicStruct_type);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_alias_enum"); descriptor.SetType(myAliasEnum_type);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_enum"); descriptor.SetType(myEnum_type);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_sequence_octet"); descriptor.SetType(my_sequence_octetType);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_sequence_struct"); descriptor.SetType(my_sequence_structType);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_array_octet"); descriptor.SetType(my_array_octetType);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_octet_array_500"); descriptor.SetType(myOctetArray500_type);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_array_struct"); descriptor.SetType(my_array_structType);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_map_octet_short"); descriptor.SetType(my_map_octet_shortType);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_map_long_struct"); descriptor.SetType(my_map_long_structType);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_map_long_seq_octet"); descriptor.SetType(my_map_long_seq_octetType);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_map_long_octet_array_500"); descriptor.SetType(my_map_long_octet_array_500Type);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_map_long_lol_type"); descriptor.SetType(my_map_long_lol_typeType);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_small_string_8"); descriptor.SetType(my_small_string_8Type);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_small_string_16"); descriptor.SetType(my_small_string_16Type);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_large_string_8"); descriptor.SetType(my_large_string_8Type);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_large_string_16"); descriptor.SetType(my_large_string_16Type);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_array_string"); descriptor.SetType(my_array_stringType);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("multi_alias_array_42"); descriptor.SetType(mA3_type);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_array_arrays"); descriptor.SetType(my_array_arraysType);
            complexStruct_builder->AddMember(&descriptor);
        }
        {
            types::MemberDescriptor descriptor;
            descriptor.SetId(idx++);
            descriptor.SetName("my_sequences_array"); descriptor.SetType(my_sequences_arrayType);
            complexStruct_builder->AddMember(&descriptor);
        }
    complexStruct_builder->SetName("ComplexStruct");
    auto complexStruct_type = complexStruct_builder->Build();

    // MyUnion
    DynamicTypeBuilder* myUnion_builder = factory->CreateUnionType(octetType);
    types::MemberDescriptor mud;
    mud.SetId(0);
    mud.SetName("basic");
    mud.SetType(basicStruct_type);
    mud.SetDefaultUnionValue(true);
    mud.AddUnionCaseIndex(0);
    myUnion_builder->AddMember(&mud);
    mud.SetId(1);
    mud.SetName("complex");
    mud.SetType(complexStruct_type);
    mud.SetDefaultUnionValue(false);
    mud.AddUnionCaseIndex(1);
    mud.AddUnionCaseIndex(2);
    myUnion_builder->AddMember(&mud);
    myUnion_builder->SetName("MyUnion");
    auto myUnion_type = myUnion_builder->Build();

    // MyUnion2
    DynamicTypeBuilder* myUnion2_builder = factory->CreateUnionType(octetType);
    mud.SetId(0);
    mud.SetName("uno");
    mud.SetType(int32Type);
    mud.SetDefaultUnionValue(true);
    mud.AddUnionCaseIndex(0);
    myUnion2_builder->AddMember(&mud);
    mud.SetId(1);
    mud.SetName("imString");
    mud.SetType(stringType);
    mud.SetDefaultUnionValue(false);
    mud.AddUnionCaseIndex(1);
    myUnion2_builder->AddMember(&mud);
    mud.SetId(2);
    mud.SetName("dos");
    mud.SetType(int32Type);
    mud.AddUnionCaseIndex(2);
    myUnion2_builder->AddMember(&mud);
    myUnion2_builder->SetName("MyUnion2");
    auto myUnion2_type = myUnion2_builder->Build();

    // CompleteStruct
    auto completeStruct_builder = factory->CreateStructType();
    // Add members to the struct.
    idx = 0;
    {
        types::MemberDescriptor descriptor;
        descriptor.SetId(idx++);
        descriptor.SetName("my_union");
        descriptor.SetType(myUnion_type);
        completeStruct_builder->AddMember(&descriptor);
        descriptor.SetId(idx++);
        descriptor.SetName("my_union_2");
        descriptor.SetType(myUnion2_type);
        completeStruct_builder->AddMember(&descriptor);
        completeStruct_builder->SetName("CompleteStruct");
    }
    m_DynManualType = completeStruct_builder->Build();
    //m_DynManual = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);

    factory->DeleteType(completeStruct_builder);
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
    ASSERT_TRUE(m_DynManualType->serialize(&dynData, &payload));

    CompleteStruct staticData;
    ASSERT_TRUE(m_StaticType.deserialize(&payload, &staticData));
    ASSERT_TRUE(m_StaticType.serialize(&staticData, &payload));

    types::DynamicData* dynData2 = DynamicDataFactory::GetInstance()->CreateData(m_DynManualType);
    ASSERT_TRUE(m_DynManualType->deserialize(&payload, dynData2));

    ASSERT_TRUE(dynData2->Equals(dynData));

    DynamicDataFactory::GetInstance()->DeleteData(dynData);
    DynamicDataFactory::GetInstance()->DeleteData(dynData2);
}

int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
