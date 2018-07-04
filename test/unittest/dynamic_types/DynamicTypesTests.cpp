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

        void HELPER_SetDescriptorDefaults();
};

TEST_F(DynamicTypesTests, TypeDescriptors_unit_tests)
{
    //// Given
    TypeDescriptor pInt32Descriptor;
    pInt32Descriptor.setKind(TK_INT32);
    pInt32Descriptor.setName("TEST_INT32");
    TypeDescriptor pInt32Descriptor2;

    // Then
    ASSERT_FALSE(pInt32Descriptor.equals(&pInt32Descriptor2));
    ASSERT_FALSE(pInt32Descriptor2.copy_from(nullptr) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(pInt32Descriptor2.copy_from(&pInt32Descriptor) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(pInt32Descriptor.equals(&pInt32Descriptor2));
    pInt32Descriptor2.setName("TEST_2");
    ASSERT_FALSE(pInt32Descriptor.equals(&pInt32Descriptor2));
    pInt32Descriptor2.setName(pInt32Descriptor.getName());
    ASSERT_TRUE(pInt32Descriptor.equals(&pInt32Descriptor2));
    pInt32Descriptor2.setKind(TK_NONE);
    ASSERT_FALSE(pInt32Descriptor.equals(&pInt32Descriptor2));

    //TODO: CONSISTENCY TESTS.
}

TEST_F(DynamicTypesTests, DynamicTypeBuilderFactory_unit_tests)
{
    //// Given
    DynamicTypeBuilderFactory::get_instance();
    TypeDescriptor pInt32Descriptor;
    pInt32Descriptor.setKind(TK_INT32);
    pInt32Descriptor.setName("TEST_INT32");
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    ASSERT_FALSE(DynamicTypeBuilderFactory::get_instance()->create_type(nullptr));
    created_type = DynamicTypeBuilderFactory::get_instance()->create_type(&pInt32Descriptor);
    ASSERT_TRUE(created_type != nullptr);
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->delete_type(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::get_instance()->delete_type(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::DeleteInstance() == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::DeleteInstance() == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->IsEmpty());
    /*
    auto builder = DynamicTypeBuilderFactory::get_instance()->create_int32_type();
    auto type = builder->build();
    //auto seqBuilder = DynamicTypeBuilderFactory::get_instance()->create_sequence_type(type, 10);
    //auto seqType = seqBuilder->build();

    auto data = DynamicDataFactory::get_instance()->create_data(type);
    data->set_int32_value(MEMBER_ID_INVALID, 10);

    int test = 0;
    data->get_int32_value(test, MEMBER_ID_INVALID);
    */
}

TEST_F(DynamicTypesTests, DynamicType_int32_unit_tests)
{
    //// Given
    DynamicTypeBuilderFactory::get_instance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::get_instance()->create_int32_type();
    ASSERT_TRUE(created_type != nullptr);
    auto new_type = created_type->build();
    auto data = DynamicDataFactory::get_instance()->create_data(new_type);

    int test1 = 123;
    ASSERT_TRUE(data->set_int32_value(MEMBER_ID_INVALID, test1) == ResponseCode::RETCODE_OK);

    int test2 = 0;
    ASSERT_FALSE(data->get_int32_value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->get_int32_value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->delete_type(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::get_instance()->delete_type(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->delete_type(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::get_instance()->delete_type(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->IsEmpty());

    ASSERT_TRUE(DynamicTypeBuilderFactory::DeleteInstance() == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::DeleteInstance() == ResponseCode::RETCODE_OK);


    /*


    //auto seqBuilder = DynamicTypeBuilderFactory::get_instance()->create_sequence_type(type, 10);
    //auto seqType = seqBuilder->build();

    auto data = DynamicDataFactory::get_instance()->create_data(type);
    data->set_int32_value(MEMBER_ID_INVALID, 10);

    int test = 0;
    data->get_int32_value(test, MEMBER_ID_INVALID);
    */
}

TEST_F(DynamicTypesTests, DynamicType_string_unit_tests)
{
    //// Given
    DynamicTypeBuilderFactory::get_instance();
    DynamicTypeBuilder* created_type(nullptr);
    uint32_t string_length = 15;

    // Then
    created_type = DynamicTypeBuilderFactory::get_instance()->create_string_type(string_length);
    ASSERT_TRUE(created_type != nullptr);

    auto new_type = created_type->build();
    auto data = DynamicDataFactory::get_instance()->create_data(new_type);
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

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->delete_type(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::get_instance()->delete_type(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->delete_type(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::get_instance()->delete_type(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_alias_unit_tests)
{
    // CREATE ALIAS FROM BASIC TYPE
    // CREATE ALIAS FROM COMPLEX TYPE
    // CHECK NAMES OF THE TYPES AFTER THE CREATION OF THE DATA
    /*
    // Given
    DynamicTypeBuilderFactory::get_instance();
    DynamicTypeBuilder* created_type(nullptr);
    uint32_t string_length = 15;

    // Then
    created_type = DynamicTypeBuilderFactory::get_instance()->create_string_type(string_length);
    ASSERT_TRUE(created_type != nullptr);

    auto new_type = created_type->build();
    auto data = DynamicDataFactory::get_instance()->create_data(new_type);
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

    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->delete_type(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::get_instance()->delete_type(created_type) == ResponseCode::RETCODE_OK);
    */
}

TEST_F(DynamicTypesTests, DynamicType_sequence_unit_tests)
{
    // CREATE SEQUENCE OF SEQUENCES

    // Given
    DynamicTypeBuilderFactory::get_instance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* seq_type_builder(nullptr);
    uint32_t sequence_length = 2;
    // CREATE SEQUENCE OF BASIC TYPES

    // Then
    base_type_builder = DynamicTypeBuilderFactory::get_instance()->create_int32_type();
    ASSERT_TRUE(base_type_builder != nullptr);
    auto base_type = base_type_builder->build();

    seq_type_builder = DynamicTypeBuilderFactory::get_instance()->create_sequence_type(base_type, sequence_length);
    ASSERT_TRUE(seq_type_builder != nullptr);
    auto seq_type = seq_type_builder->build();
    ASSERT_TRUE(seq_type != nullptr);

    auto seq_data = DynamicDataFactory::get_instance()->create_data(seq_type);
    ASSERT_FALSE(seq_data->set_int32_value(MEMBER_ID_INVALID, 10) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(seq_data->set_string_value(MEMBER_ID_INVALID, "") == ResponseCode::RETCODE_OK);

    MemberId newId;
    ASSERT_TRUE(seq_data->insert_new_data(newId) == ResponseCode::RETCODE_OK);
    MemberId newId2;
    ASSERT_TRUE(seq_data->insert_new_data(newId2) == ResponseCode::RETCODE_OK);

    // Try to insert more than the limit.
    MemberId newId3;
    ASSERT_FALSE(seq_data->insert_new_data(newId3) == ResponseCode::RETCODE_OK);

    // Set and get a value.
    int32_t test1(234);
    ASSERT_TRUE(seq_data->set_int32_value(newId2, test1) == ResponseCode::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(seq_data->get_int32_value(test2, newId2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Remove the elements.
    ASSERT_TRUE(seq_data->remove_data(newId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(seq_data->clear_all_values() == ResponseCode::RETCODE_OK);

    // Check that the sequence is empty.
    ASSERT_FALSE(seq_data->get_int32_value(test2, 0) == ResponseCode::RETCODE_OK);

    // Delete the sequence
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(seq_data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::get_instance()->delete_data(seq_data) == ResponseCode::RETCODE_OK);

    // Clean the types Factory.
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->delete_type(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::get_instance()->delete_type(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->delete_type(seq_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::get_instance()->delete_type(seq_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->delete_type(seq_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::get_instance()->delete_type(seq_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->delete_type(base_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::get_instance()->delete_type(base_type_builder) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->IsEmpty());
}

void DynamicTypesTests::HELPER_SetDescriptorDefaults()
{
}

//TODO: //ARCE:

/*
PENDING TESTS

TypeBuilderFactory: -> creates with invalid values, create a type of each basic kind, create a builder of each type.
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
