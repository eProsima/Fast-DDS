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

        virtual void TearDown()
        {
            DynamicDataFactory::DeleteInstance();
            DynamicTypeBuilderFactory::DeleteInstance();
        }

        void HELPER_SetDescriptorDefaults();
};

TEST_F(DynamicTypesTests, TypeDescriptors_unit_tests)
{
    //// Given
    TypeDescriptor pInt32Descriptor;
    pInt32Descriptor.SetKind(TK_INT32);
    pInt32Descriptor.SetName("TEST_INT32");
    TypeDescriptor pInt32Descriptor2;

    // Then
    ASSERT_FALSE(pInt32Descriptor.Equals(&pInt32Descriptor2));
    ASSERT_FALSE(pInt32Descriptor2.CopyFrom(nullptr) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(pInt32Descriptor2.CopyFrom(&pInt32Descriptor) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(pInt32Descriptor.Equals(&pInt32Descriptor2));
    pInt32Descriptor2.SetName("TEST_2");
    ASSERT_FALSE(pInt32Descriptor.Equals(&pInt32Descriptor2));
    pInt32Descriptor2.SetName(pInt32Descriptor.GetName());
    ASSERT_TRUE(pInt32Descriptor.Equals(&pInt32Descriptor2));
    pInt32Descriptor2.SetKind(TK_NONE);
    ASSERT_FALSE(pInt32Descriptor.Equals(&pInt32Descriptor2));

    //TODO: CONSISTENCY TESTS.
}

TEST_F(DynamicTypesTests, DynamicTypeBuilderFactory_unit_tests)
{
    //// Given
    DynamicTypeBuilderFactory::GetInstance();
    TypeDescriptor pInt32Descriptor;
    pInt32Descriptor.SetKind(TK_INT32);
    pInt32Descriptor.SetName("TEST_INT32");
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->CreateType(nullptr));
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(&pInt32Descriptor);
    ASSERT_TRUE(created_type != nullptr);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_int32_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
    ASSERT_TRUE(created_type != nullptr);
    auto new_type = created_type->Build();
    auto data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    int test1 = 123;
    ASSERT_TRUE(data->SetInt32Value(MEMBER_ID_INVALID, test1) == ResponseCode::RETCODE_OK);

    int test2 = 0;
    ASSERT_FALSE(data->GetInt32Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetInt32Value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_enum_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateEnumType();
    ASSERT_TRUE(created_type != nullptr);

    // Add three members to the enum.
    types::MemberDescriptor descriptor;
    descriptor.SetIndex(0);
    descriptor.SetName("DEFAULT");
    ASSERT_TRUE(created_type->AddMember(&descriptor) == ResponseCode::RETCODE_OK);

    descriptor.SetIndex(1);
    descriptor.SetName("FIRST");
    ASSERT_TRUE(created_type->AddMember(&descriptor) == ResponseCode::RETCODE_OK);

    descriptor.SetIndex(2);
    descriptor.SetName("SECOND");
    ASSERT_TRUE(created_type->AddMember(&descriptor) == ResponseCode::RETCODE_OK);

    // Try to add a descriptor with the same name.
    descriptor.SetIndex(4);
    descriptor.SetName("DEFAULT");
    ASSERT_FALSE(created_type->AddMember(&descriptor) == ResponseCode::RETCODE_OK);

    auto new_type = created_type->Build();
    auto data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    ASSERT_FALSE(data->SetInt32Value(MEMBER_ID_INVALID, 0) == ResponseCode::RETCODE_OK);

    std::string test1 = "SECOND";
    ASSERT_FALSE(data->SetEnumValue(1, test1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->SetEnumValue(MEMBER_ID_INVALID, test1) == ResponseCode::RETCODE_OK);

    std::string test2;
    int iTest;
    ASSERT_FALSE(data->GetInt32Value(iTest, 0) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetEnumValue(test2, 1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetEnumValue(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_string_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);
    uint32_t string_length = 15;

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateStringType(string_length);
    ASSERT_TRUE(created_type != nullptr);

    auto new_type = created_type->Build();
    auto data = DynamicDataFactory::GetInstance()->CreateData(new_type);
    ASSERT_FALSE(data->SetInt32Value(MEMBER_ID_INVALID, 10) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue(1, "") == ResponseCode::RETCODE_OK);
    std::string sTest1 = "STRING_TEST";
    ASSERT_TRUE(data->SetStringValue(MEMBER_ID_INVALID, sTest1) == ResponseCode::RETCODE_OK);

    int test = 0;
    ASSERT_FALSE(data->GetInt32Value(test, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest2 = "";
    ASSERT_FALSE(data->GetStringValue(sTest2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetStringValue(sTest2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(sTest1 == sTest2);

    ASSERT_FALSE(data->SetStringValue(MEMBER_ID_INVALID, "TEST_OVER_LENGTH_LIMITS") == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_alias_unit_tests)
{
    // CREATE ALIAS FROM BASIC TYPE
    // CREATE ALIAS FROM COMPLEX TYPE
    // CHECK NAMES OF THE TYPES AFTER THE CREATION OF THE DATA
    /*
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);
    uint32_t string_length = 15;

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->create_string_type(string_length);
    ASSERT_TRUE(created_type != nullptr);

    auto new_type = created_type->build();
    auto data = DynamicDataFactory::GetInstance()->create_data(new_type);
    ASSERT_FALSE(data->set_int32_value(MEMBER_ID_INVALID, 10) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value(1, "") == ResponseCode::RETCODE_OK);
    std::string sTest1 = "STRING_TEST";
    ASSERT_TRUE(data->set_string_value(MEMBER_ID_INVALID, sTest1) == ResponseCode::RETCODE_OK);

    int test = 0;
    ASSERT_FALSE(data->get_int32_value(test, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest2 = "";
    ASSERT_FALSE(data->get_string_value(sTest2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->get_string_value(sTest2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(sTest1 == sTest2);

    ASSERT_FALSE(data->set_string_value(MEMBER_ID_INVALID, "TEST_OVER_LENGTH_LIMITS") == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    */
}

TEST_F(DynamicTypesTests, DynamicType_bitset_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);
    uint32_t limit = 3;

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateBitsetType(limit);
    ASSERT_TRUE(created_type != nullptr);
    auto new_type = created_type->Build();
    auto data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    bool test1 = true;
    ASSERT_FALSE(data->SetInt32Value(MEMBER_ID_INVALID, 1) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(data->SetBoolValue(2, test1) == ResponseCode::RETCODE_OK);

    // Over the limit
    ASSERT_FALSE(data->SetBoolValue(limit + 1, test1) == ResponseCode::RETCODE_OK);

    bool test2 = false;
    ASSERT_TRUE(data->GetBoolValue(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test2 == false);
    ASSERT_TRUE(data->GetBoolValue(test2, 2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);
    test1 = false;
    ASSERT_TRUE(data->SetBoolValue(2, test1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetBoolValue(test2, 2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_bitmask_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);
    uint32_t limit = 3;

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateBitmaskType(limit);
    ASSERT_TRUE(created_type != nullptr);

    // Add two members to the bitmask
    types::MemberDescriptor descriptor;
    descriptor.SetIndex(0);
    descriptor.SetName("TEST");
    ASSERT_TRUE(created_type->AddMember(&descriptor) == ResponseCode::RETCODE_OK);

    // Try to add a descriptor with the same name
    descriptor.SetIndex(1);
    ASSERT_FALSE(created_type->AddMember(&descriptor) == ResponseCode::RETCODE_OK);

    descriptor.SetName("TEST2");
    ASSERT_TRUE(created_type->AddMember(&descriptor) == ResponseCode::RETCODE_OK);

    auto new_type = created_type->Build();
    auto data = DynamicDataFactory::GetInstance()->CreateData(new_type);
    MemberId testId = data->GetMemberIdByName("TEST");
    ASSERT_TRUE(testId != MEMBER_ID_INVALID);
    MemberId test2Id = data->GetMemberIdByName("TEST2");
    ASSERT_TRUE(test2Id != MEMBER_ID_INVALID);

    bool test1 = true;
    ASSERT_FALSE(data->SetInt32Value(MEMBER_ID_INVALID, 1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->SetBoolValue(testId, test1) == ResponseCode::RETCODE_OK);

    // Over the limit
    ASSERT_FALSE(data->SetBoolValue(limit + 1, test1) == ResponseCode::RETCODE_OK);

    bool test2 = false;
    ASSERT_TRUE(data->GetBoolValue(test2, 2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test2 == false);
    ASSERT_TRUE(data->GetBoolValue(test2, testId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);
    test1 = false;
    ASSERT_TRUE(data->SetBoolValue(testId, test1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetBoolValue(test2, test2Id) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetBoolValue(test2, testId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_sequence_unit_tests)
{
    //TODO:  CREATE SEQUENCE OF SEQUENCES

    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* seq_type_builder(nullptr);
    uint32_t sequence_length = 2;
    // CREATE SEQUENCE OF BASIC TYPES

    // Then
    base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
    ASSERT_TRUE(base_type_builder != nullptr);
    auto base_type = base_type_builder->Build();

    seq_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateSequenceType(base_type, sequence_length);
    ASSERT_TRUE(seq_type_builder != nullptr);
    auto seq_type = seq_type_builder->Build();
    ASSERT_TRUE(seq_type != nullptr);

    auto seq_data = DynamicDataFactory::GetInstance()->CreateData(seq_type);
    ASSERT_FALSE(seq_data->SetInt32Value(MEMBER_ID_INVALID, 10) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(seq_data->SetStringValue(MEMBER_ID_INVALID, "") == ResponseCode::RETCODE_OK);

    MemberId newId;
    ASSERT_TRUE(seq_data->InsertSequenceData(newId) == ResponseCode::RETCODE_OK);
    MemberId newId2;
    ASSERT_TRUE(seq_data->InsertSequenceData(newId2) == ResponseCode::RETCODE_OK);

    // Try to insert more than the limit.
    MemberId newId3;
    ASSERT_FALSE(seq_data->InsertSequenceData(newId3) == ResponseCode::RETCODE_OK);

    // Set and get a value.
    int32_t test1(234);
    ASSERT_TRUE(seq_data->SetInt32Value(newId2, test1) == ResponseCode::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(seq_data->GetInt32Value(test2, newId2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Remove the elements.
    ASSERT_TRUE(seq_data->RemoveSequenceData(newId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(seq_data->ClearAllValues() == ResponseCode::RETCODE_OK);

    // Check that the sequence is empty.
    ASSERT_FALSE(seq_data->GetInt32Value(test2, 0) == ResponseCode::RETCODE_OK);

    // Delete the sequence
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(seq_data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(seq_data) == ResponseCode::RETCODE_OK);

    // Clean the types Factory.
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(seq_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(seq_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(seq_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(seq_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_array_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* array_type_builder(nullptr);
    std::vector<uint32_t> sequence_lengths = { 2, 2, 2 };
    // CREATE SEQUENCE OF BASIC TYPES

    // Then
    base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
    ASSERT_TRUE(base_type_builder != nullptr);
    auto base_type = base_type_builder->Build();

    array_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateArrayType(base_type, sequence_lengths);
    ASSERT_TRUE(array_type_builder != nullptr);
    auto array_type = array_type_builder->Build();
    ASSERT_TRUE(array_type != nullptr);

    auto array_data = DynamicDataFactory::GetInstance()->CreateData(array_type);
    ASSERT_FALSE(array_data->SetInt32Value(MEMBER_ID_INVALID, 10) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(array_data->SetStringValue(MEMBER_ID_INVALID, "") == ResponseCode::RETCODE_OK);

    MemberId newId;
    ASSERT_FALSE(array_data->InsertSequenceData(newId) == ResponseCode::RETCODE_OK);

    // Get an index in the multidimensional array.
    std::vector<uint32_t> vPosition = { 1, 1, 1 };
    MemberId testPos(0);
    testPos = array_data->GetArrayIndex(vPosition);
    ASSERT_TRUE(testPos != MEMBER_ID_INVALID);

    // Invalid input vectors.
    std::vector<uint32_t> vPosition2 = { 1, 1 };
    ASSERT_FALSE(array_data->GetArrayIndex(vPosition2) != MEMBER_ID_INVALID);
    std::vector<uint32_t> vPosition3 = { 1, 1, 1, 1 };
    ASSERT_FALSE(array_data->GetArrayIndex(vPosition3) != MEMBER_ID_INVALID);

    // Set and get a value.
    int32_t test1 = 156;
    ASSERT_TRUE(array_data->SetInt32Value(testPos, test1) == ResponseCode::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(array_data->GetInt32Value(test2, testPos) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Check items count before and after remove an element.
    ASSERT_TRUE(array_data->GetItemCount() == 1);
    ASSERT_TRUE(array_data->ClearValue(testPos) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(array_data->GetItemCount() == 1);
    ASSERT_TRUE(array_data->RemoveArrayData(testPos) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(array_data->GetItemCount() == 0);

    // Check the clear values method
    ASSERT_TRUE(array_data->SetInt32Value(testPos, test1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(array_data->GetItemCount() == 1);
    ASSERT_TRUE(array_data->ClearAllValues() == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(array_data->GetItemCount() == 0);

    // Try to set a value out of the array.
    ASSERT_FALSE(array_data->SetInt32Value(100, test1) == ResponseCode::RETCODE_OK);

    // Delete the array
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(array_data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(array_data) == ResponseCode::RETCODE_OK);

    // Clean the types Factory.
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(array_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(array_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(array_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(array_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_map_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* map_type_builder(nullptr);
    uint32_t map_length = 2;

    // Then
    base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
    ASSERT_TRUE(base_type_builder != nullptr);
    auto base_type = base_type_builder->Build();

    map_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateMapType(base_type, base_type, map_length);
    ASSERT_TRUE(map_type_builder != nullptr);
    auto map_type = map_type_builder->Build();
    ASSERT_TRUE(map_type != nullptr);

    auto map_data = DynamicDataFactory::GetInstance()->CreateData(map_type);

    ASSERT_FALSE(map_data->SetInt32Value(MEMBER_ID_INVALID, 10) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(map_data->SetStringValue(MEMBER_ID_INVALID, "") == ResponseCode::RETCODE_OK);

    MemberId keyId;
    MemberId valueId;
    auto key_data = DynamicDataFactory::GetInstance()->CreateData(base_type);
    ASSERT_TRUE(map_data->InsertMapData(key_data, keyId, valueId) == ResponseCode::RETCODE_OK);

    // Try to Add the same key twice.
    ASSERT_FALSE(map_data->InsertMapData(key_data, keyId, valueId) == ResponseCode::RETCODE_OK);

    MemberId keyId2;
    MemberId valueId2;
    key_data = DynamicDataFactory::GetInstance()->CreateData(base_type);
    key_data->SetInt32Value(MEMBER_ID_INVALID, 2);
    ASSERT_TRUE(map_data->InsertMapData(key_data, keyId2, valueId2) == ResponseCode::RETCODE_OK);

    // Try to Add one more than the limit
    auto key_data2 = DynamicDataFactory::GetInstance()->CreateData(base_type);
    key_data2->SetInt32Value(MEMBER_ID_INVALID, 3);
    ASSERT_FALSE(map_data->InsertMapData(key_data2, keyId, valueId) == ResponseCode::RETCODE_OK);

    // Set and get a value.
    int32_t test1(234);
    ASSERT_TRUE(map_data->SetInt32Value(valueId, test1) == ResponseCode::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(map_data->GetInt32Value(test2, valueId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Check items count with removes
    ASSERT_TRUE(map_data->GetItemCount() == 2);
    ASSERT_FALSE(map_data->RemoveMapData(valueId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(map_data->GetItemCount() == 2);
    ASSERT_TRUE(map_data->RemoveMapData(keyId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(map_data->GetItemCount() == 1);
    ASSERT_TRUE(map_data->ClearAllValues() == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(map_data->GetItemCount() == 0);

    // Delete the map
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(key_data2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(key_data2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(map_data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(map_data) == ResponseCode::RETCODE_OK);

    // Clean the types Factory.
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(map_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(map_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(map_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(map_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_structure_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* base_type_builder2(nullptr);
    DynamicTypeBuilder* struct_type_builder(nullptr);

    // Then
    base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
    ASSERT_TRUE(base_type_builder != nullptr);
    auto base_type = base_type_builder->Build();

    base_type_builder2 = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Type();
    ASSERT_TRUE(base_type_builder2 != nullptr);
    auto base_type2 = base_type_builder2->Build();

    struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructType();
    ASSERT_TRUE(struct_type_builder != nullptr);

    // Add members to the struct.
    types::MemberDescriptor descriptor;
    descriptor.SetId(0);
    descriptor.SetName("int32");
    descriptor.SetType(base_type);
    ASSERT_TRUE(struct_type_builder->AddMember(&descriptor) == ResponseCode::RETCODE_OK);

    types::MemberDescriptor descriptor2;
    descriptor2.SetId(1);
    descriptor2.SetName("int64");
    descriptor2.SetType(base_type2);
    ASSERT_TRUE(struct_type_builder->AddMember(&descriptor2) == ResponseCode::RETCODE_OK);

    auto struct_type = struct_type_builder->Build();
    ASSERT_TRUE(struct_type != nullptr);
    auto struct_data = DynamicDataFactory::GetInstance()->CreateData(struct_type);
    ASSERT_TRUE(struct_data != nullptr);

    ASSERT_FALSE(struct_data->SetInt32Value(1, 10) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(struct_data->SetStringValue(MEMBER_ID_INVALID, "") == ResponseCode::RETCODE_OK);

    // Set and get the child values.
    int32_t test1(234);
    ASSERT_TRUE(struct_data->SetInt32Value(0, test1) == ResponseCode::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(struct_data->GetInt32Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);
    int64_t test3(234);
    ASSERT_TRUE(struct_data->SetInt64Value(1, test3) == ResponseCode::RETCODE_OK);
    int64_t test4(0);
    ASSERT_TRUE(struct_data->GetInt64Value(test4, 1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test3 == test4);

    // Delete the map
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(struct_data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(struct_data) == ResponseCode::RETCODE_OK);

    // Clean the types Factory.
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(struct_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(struct_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(struct_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(struct_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_union_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* base_type_builder2(nullptr);
    DynamicTypeBuilder* union_type_builder(nullptr);

    // Then
    base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
    ASSERT_TRUE(base_type_builder != nullptr);
    auto base_type = base_type_builder->Build();

    base_type_builder2 = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Type();
    ASSERT_TRUE(base_type_builder2 != nullptr);
    auto base_type2 = base_type_builder2->Build();

    union_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUnionType(base_type);
    ASSERT_TRUE(union_type_builder != nullptr);

    // Add members to the struct.
    types::MemberDescriptor descriptor;
    descriptor.SetId(0);
    descriptor.SetName("first");
    descriptor.SetType(base_type);
    descriptor.SetDefaultUnionValue(true);
    descriptor.AddUnionCaseIndex(0);
    ASSERT_TRUE(union_type_builder->AddMember(&descriptor) == ResponseCode::RETCODE_OK);

    types::MemberDescriptor descriptor2;
    descriptor2.SetId(1);
    descriptor2.SetName("second");
    descriptor2.SetType(base_type2);
    descriptor2.AddUnionCaseIndex(1);
    ASSERT_TRUE(union_type_builder->AddMember(&descriptor2) == ResponseCode::RETCODE_OK);

    // Try to add a second "DEFAULT" value to the union
    types::MemberDescriptor descriptor3;
    descriptor3.SetId(0);
    descriptor3.SetName("third");
    descriptor3.SetType(base_type);
    descriptor3.SetDefaultUnionValue(true);
    descriptor3.AddUnionCaseIndex(0);
    ASSERT_FALSE(union_type_builder->AddMember(&descriptor3) == ResponseCode::RETCODE_OK);

    // Try to add a second value to the same case
    descriptor3.SetDefaultUnionValue(false);
    descriptor3.AddUnionCaseIndex(1);
    ASSERT_FALSE(union_type_builder->AddMember(&descriptor3) == ResponseCode::RETCODE_OK);

    // Create a data of this union
    auto union_type = union_type_builder->Build();
    ASSERT_TRUE(union_type != nullptr);
    auto union_data = DynamicDataFactory::GetInstance()->CreateData(union_type);
    ASSERT_TRUE(union_data != nullptr);

    // Set and get the child values.
    ASSERT_FALSE(union_data->SetInt32Value(1, 10) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(union_data->SetStringValue(MEMBER_ID_INVALID, "") == ResponseCode::RETCODE_OK);

    uint64_t label;
    ASSERT_TRUE(union_data->GetUnionLabel(label) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(label == 0);

    int32_t test1(234);
    ASSERT_TRUE(union_data->SetInt32Value(0, test1) == ResponseCode::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(union_data->GetInt32Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);
    ASSERT_TRUE(union_data->GetUnionLabel(label) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(label == 0);

    int64_t test3(234);
    int64_t test4(0);

    // Try to get values from invalid indexes and from an invalid element ( not the current one )
    ASSERT_FALSE(union_data->GetInt32Value(test2, 1) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(union_data->GetInt64Value(test4, 1) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(union_data->SetInt64Value(1, test3) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(union_data->GetInt64Value(test4, 1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test3 == test4);
    ASSERT_TRUE(union_data->GetUnionLabel(label) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(label == 1);

    // Delete the map
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(union_data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(union_data) == ResponseCode::RETCODE_OK);

    // Clean the types Factory.
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(union_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(union_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(union_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(union_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

void DynamicTypesTests::HELPER_SetDescriptorDefaults()
{
}

//TODO: //ARCE:

/*
PENDING TESTS

TypeBuilderFactory: -> creates with invalid values, create a type of each basic kind, create a builder of each type.
MODIFY A BUILDER TO CHECK THAT THE PREVIOUSLY CREATED TYPE DOESN'T CHANGE
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
