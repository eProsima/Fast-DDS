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
    // Given
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
}

TEST_F(DynamicTypesTests, DynamicType_basic_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);
    // Create basic types
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
    ASSERT_TRUE(created_type != nullptr);
    auto type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    auto type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));

    auto struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructType();
    ASSERT_TRUE(struct_type_builder != nullptr);

    // Add members to the struct.
    types::MemberDescriptor descriptor;
    descriptor.SetId(0);
    descriptor.SetName("int32");
    descriptor.SetType(created_type);
    ASSERT_TRUE(struct_type_builder->AddMember(&descriptor) == ResponseCode::RETCODE_OK);

    auto struct_type = struct_type_builder->Build();
    ASSERT_TRUE(struct_type != nullptr);

    types::MemberDescriptor descriptor2;
    descriptor2.SetId(1);
    descriptor2.SetName("int64");
    descriptor2.SetType(created_type);
    ASSERT_TRUE(struct_type_builder->AddMember(&descriptor2) == ResponseCode::RETCODE_OK);
    auto struct_type2 = struct_type_builder->Build();
    ASSERT_TRUE(struct_type2 != nullptr);
    ASSERT_FALSE(struct_type->Equals(struct_type2));

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(struct_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(struct_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(struct_type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(struct_type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(struct_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(struct_type_builder) == ResponseCode::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicTypeBuilderFactory_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Try to create with invalid values
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->CreateCustomType(nullptr));

    // Create basic types
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
    ASSERT_TRUE(created_type != nullptr);
    auto type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    auto type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    auto data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateUint32Type();
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateInt16Type();
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateUint16Type();
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Type();
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateUint64Type();
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateFloat32Type();
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateFloat64Type();
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateFloat128Type();
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateChar8Type();
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateChar16Type();
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateByteType();
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateBoolType();
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateStringType(LENGTH_UNLIMITED);
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateWstringType(LENGTH_UNLIMITED);
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    // Create with custom types
    TypeDescriptor pInt32Descriptor;
    pInt32Descriptor.SetKind(TK_INT32);
    pInt32Descriptor.SetName("TEST_INT32");
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateCustomType(&pInt32Descriptor);
    ASSERT_TRUE(created_type != nullptr);
    type = created_type->Build();
    ASSERT_TRUE(type != nullptr);
    type2 = DynamicTypeBuilderFactory::GetInstance()->CreateTypeCopy(type);
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type->Equals(type2));
    data = DynamicDataFactory::GetInstance()->CreateData(type);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(type2) == ResponseCode::RETCODE_OK);
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
    types::DynamicType* new_type = created_type->Build();
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    int32_t test1 = 123;
    int32_t test2 = 0;
    ASSERT_TRUE(data->SetInt32Value(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetInt32Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetInt32Value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    //ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    //int32_t iTest32;
    //ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}


TEST_F(DynamicTypesTests, DynamicType_uint32_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateUint32Type();
    ASSERT_TRUE(created_type != nullptr);
    types::DynamicType* new_type = created_type->Build();
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    uint32_t test1 = 123;
    uint32_t test2 = 0;
    ASSERT_TRUE(data->SetUint32Value(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetUint32Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetUint32Value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //uint32_t uTest32;
    //ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_int16_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateInt16Type();
    ASSERT_TRUE(created_type != nullptr);
    types::DynamicType* new_type = created_type->Build();
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    int16_t test1 = 123;
    int16_t test2 = 0;
    ASSERT_TRUE(data->SetInt16Value(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetInt16Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetInt16Value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //int16_t iTest16;
    //ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_uint16_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateUint16Type();
    ASSERT_TRUE(created_type != nullptr);
    types::DynamicType* new_type = created_type->Build();
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    uint16_t test1 = 123;
    uint16_t test2 = 0;
    ASSERT_TRUE(data->SetUint16Value(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetUint16Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetUint16Value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //uint16_t uTest16;
    //ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_int64_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Type();
    ASSERT_TRUE(created_type != nullptr);
    types::DynamicType* new_type = created_type->Build();
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    int64_t test1 = 123;
    int64_t test2 = 0;
    ASSERT_TRUE(data->SetInt64Value(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetInt64Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetInt64Value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //int64_t iTest64;
    //ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_uint64_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateUint64Type();
    ASSERT_TRUE(created_type != nullptr);
    types::DynamicType* new_type = created_type->Build();
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    uint64_t test1 = 123;
    uint64_t test2 = 0;
    ASSERT_TRUE(data->SetUint64Value(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetUint64Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetUint64Value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //uint64_t uTest64;
    //ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_float32_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateFloat32Type();
    ASSERT_TRUE(created_type != nullptr);
    types::DynamicType* new_type = created_type->Build();
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    float test1 = 123.0f;
    float test2 = 0.0f;
    ASSERT_TRUE(data->SetFloat32Value(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetFloat32Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetFloat32Value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //float fTest32;
    //ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_float64_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateFloat64Type();
    ASSERT_TRUE(created_type != nullptr);
    types::DynamicType* new_type = created_type->Build();
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    double test1 = 123.0;
    double test2 = 0.0;
    ASSERT_TRUE(data->SetFloat64Value(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetFloat64Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetFloat64Value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //double fTest64;
    //ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_float128_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateFloat128Type();
    ASSERT_TRUE(created_type != nullptr);
    types::DynamicType* new_type = created_type->Build();
    ASSERT_TRUE(new_type != nullptr);
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);
    ASSERT_TRUE(data != nullptr);

    long double test1 = 123.0;
    long double test2 = 0.0;
    ASSERT_TRUE(data->SetFloat128Value(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetFloat128Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetFloat128Value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //long double fTest128;
    //ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_char8_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateChar8Type();
    ASSERT_TRUE(created_type != nullptr);
    types::DynamicType* new_type = created_type->Build();
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    char test1 = 'a';
    char test2 = 'b';
    ASSERT_TRUE(data->SetChar8Value(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetChar8Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetChar8Value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //char cTest8;
    //ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_char16_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateChar16Type();
    ASSERT_TRUE(created_type != nullptr);
    types::DynamicType* new_type = created_type->Build();
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    wchar_t test1 = L'a';
    wchar_t test2 = L'b';
    ASSERT_TRUE(data->SetChar16Value(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetChar16Value(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetChar16Value(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //wchar_t cTest16;
    //ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_byte_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateByteType();
    ASSERT_TRUE(created_type != nullptr);
    types::DynamicType* new_type = created_type->Build();
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    octet test1 = 255;
    octet test2 = 0;
    ASSERT_TRUE(data->SetByteValue(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetByteValue(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetByteValue(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //octet oTest;
    //ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_bool_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateBoolType();
    ASSERT_TRUE(created_type != nullptr);
    types::DynamicType* new_type = created_type->Build();
    types::DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);

    bool test1 = true;
    bool test2 = false;
    ASSERT_TRUE(data->SetBoolValue(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetBoolValue(test2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetBoolValue(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //bool bTest;
    //ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

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

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    // Try to set an invalid value.
    ASSERT_FALSE(data->SetEnumValue("BAD", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    std::string test1 = "SECOND";
    ASSERT_FALSE(data->SetEnumValue(test1, 1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->SetEnumValue(test1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    std::string test2;
    int iTest;
    ASSERT_FALSE(data->GetInt32Value(iTest, 0) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->GetEnumValue(test2, 1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetEnumValue(test2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //std::string sEnumTest;
    //ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

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
    ASSERT_FALSE(data->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", 1) == ResponseCode::RETCODE_OK);
    std::string sTest1 = "STRING_TEST";
    ASSERT_TRUE(data->SetStringValue(sTest1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    int test = 0;
    ASSERT_FALSE(data->GetInt32Value(test, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest2 = "";
    ASSERT_FALSE(data->GetStringValue(sTest2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetStringValue(sTest2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(sTest1 == sTest2);

    ASSERT_FALSE(data->SetStringValue("TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //std::string sTest;
    //ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(new_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_wstring_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* created_type(nullptr);
    uint32_t string_length = 15;

    // Then
    created_type = DynamicTypeBuilderFactory::GetInstance()->CreateWstringType(string_length);
    ASSERT_TRUE(created_type != nullptr);

    types::DynamicType* new_type = created_type->Build();
    DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(new_type);
    ASSERT_FALSE(data->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", 1) == ResponseCode::RETCODE_OK);
    std::wstring sTest1 = L"STRING_TEST";
    ASSERT_TRUE(data->SetWstringValue(sTest1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    int test = 0;
    ASSERT_FALSE(data->GetInt32Value(test, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring sTest2 = L"";
    ASSERT_FALSE(data->GetWstringValue(sTest2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetWstringValue(sTest2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(sTest1 == sTest2);

    ASSERT_FALSE(data->SetWstringValue(L"TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //std::wstring wsTest;
    //ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

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
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* alias_type_builder(nullptr);
    uint32_t string_length = 15;

    // Then
    base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStringType(string_length);
    ASSERT_TRUE(base_type_builder != nullptr);
    auto base_type = base_type_builder->Build();
    alias_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateAliasType(base_type, "ALIAS");
    ASSERT_TRUE(alias_type_builder != nullptr);
    auto alias_type = alias_type_builder->Build();
    ASSERT_TRUE(alias_type != nullptr);
    ASSERT_TRUE(alias_type->GetName() == "ALIAS");

    auto aliasData = DynamicDataFactory::GetInstance()->CreateData(alias_type);
    ASSERT_FALSE(aliasData->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(aliasData->SetStringValue("", 1) == ResponseCode::RETCODE_OK);
    std::string sTest1 = "STRING_TEST";
    ASSERT_TRUE(aliasData->SetStringValue(sTest1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    int test = 0;
    ASSERT_FALSE(aliasData->GetInt32Value(test, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest2 = "";
    ASSERT_FALSE(aliasData->GetStringValue(sTest2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(aliasData->GetStringValue(sTest2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(sTest1 == sTest2);

    ASSERT_FALSE(aliasData->SetStringValue("TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(aliasData) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(alias_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(alias_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(alias_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(alias_type_builder) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_multi_alias_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* alias_type_builder(nullptr);
    DynamicTypeBuilder* alias2_type_builder(nullptr);
    uint32_t string_length = 15;

    // Then
    base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStringType(string_length);
    ASSERT_TRUE(base_type_builder != nullptr);
    auto base_type = base_type_builder->Build();
    alias_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateAliasType(base_type, "ALIAS");
    ASSERT_TRUE(alias_type_builder != nullptr);
    auto alias_type = alias_type_builder->Build();
    ASSERT_TRUE(alias_type != nullptr);
    ASSERT_TRUE(alias_type->GetName() == "ALIAS");

    // Try to create an alias without base type.
    alias2_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateAliasType(nullptr, "ALIAS2");
    ASSERT_FALSE(alias2_type_builder != nullptr);

    alias2_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateAliasType(alias_type, "ALIAS2");
    ASSERT_TRUE(alias2_type_builder != nullptr);
    auto alias2_type = alias2_type_builder->Build();
    ASSERT_TRUE(alias2_type != nullptr);
    ASSERT_TRUE(alias2_type->GetName() == "ALIAS2");

    auto aliasData = DynamicDataFactory::GetInstance()->CreateData(alias2_type);
    ASSERT_FALSE(aliasData->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(aliasData->SetStringValue("", 1) == ResponseCode::RETCODE_OK);
    std::string sTest1 = "STRING_TEST";
    ASSERT_TRUE(aliasData->SetStringValue(sTest1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    int test = 0;
    ASSERT_FALSE(aliasData->GetInt32Value(test, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest2 = "";
    ASSERT_FALSE(aliasData->GetStringValue(sTest2, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(aliasData->GetStringValue(sTest2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(sTest1 == sTest2);

    ASSERT_FALSE(aliasData->SetStringValue("TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(aliasData) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(alias_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(alias_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(alias_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(alias_type_builder) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(alias2_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(alias2_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(alias2_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(alias2_type_builder) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
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
    ASSERT_FALSE(data->SetInt32Value(1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

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

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

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
    ASSERT_FALSE(data->SetInt32Value(1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
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

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    //ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

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
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* seq_type_builder(nullptr);
    uint32_t sequence_length = 2;

    // Then
    base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
    ASSERT_TRUE(base_type_builder != nullptr);
    auto base_type = base_type_builder->Build();

    seq_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateSequenceType(base_type, sequence_length);
    ASSERT_TRUE(seq_type_builder != nullptr);
    auto seq_type = seq_type_builder->Build();
    ASSERT_TRUE(seq_type != nullptr);

    auto data = DynamicDataFactory::GetInstance()->CreateData(seq_type);
    ASSERT_FALSE(data->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    // Try to write on an empty position
    ASSERT_FALSE(data->SetInt32Value(1, 234) == ResponseCode::RETCODE_OK);

    MemberId newId;
    ASSERT_TRUE(data->InsertSequenceData(newId) == ResponseCode::RETCODE_OK);
    MemberId newId2;
    ASSERT_TRUE(data->InsertSequenceData(newId2) == ResponseCode::RETCODE_OK);

    // Try to insert more than the limit.
    MemberId newId3;
    ASSERT_FALSE(data->InsertSequenceData(newId3) == ResponseCode::RETCODE_OK);

    // Set and get a value.
    int32_t test1(234);
    ASSERT_TRUE(data->SetInt32Value(newId2, test1) == ResponseCode::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(data->GetInt32Value(test2, newId2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Remove the elements.
    ASSERT_TRUE(data->RemoveSequenceData(newId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->ClearAllValues() == ResponseCode::RETCODE_OK);

    // Check that the sequence is empty.
    ASSERT_FALSE(data->GetInt32Value(test2, 0) == ResponseCode::RETCODE_OK);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    // Delete the sequence
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

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

TEST_F(DynamicTypesTests, DynamicType_sequence_of_sequences_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* seq_type_builder(nullptr);
    DynamicTypeBuilder* seq_seq_type_builder(nullptr);
    uint32_t sequence_length = 2;
    uint32_t sup_sequence_length = 3;

    // Then
    base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
    ASSERT_TRUE(base_type_builder != nullptr);
    auto base_type = base_type_builder->Build();

    seq_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateSequenceType(base_type, sequence_length);
    ASSERT_TRUE(seq_type_builder != nullptr);
    auto seq_type = seq_type_builder->Build();
    ASSERT_TRUE(seq_type != nullptr);

    seq_seq_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateSequenceType(seq_type, sup_sequence_length);
    ASSERT_TRUE(seq_seq_type_builder != nullptr);
    auto seq_seq_type = seq_seq_type_builder->Build();
    ASSERT_TRUE(seq_seq_type != nullptr);

    auto data = DynamicDataFactory::GetInstance()->CreateData(seq_seq_type);
    ASSERT_FALSE(data->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    MemberId newId;
    ASSERT_TRUE(data->InsertSequenceData(newId) == ResponseCode::RETCODE_OK);
    MemberId newId2;
    ASSERT_TRUE(data->InsertSequenceData(newId2) == ResponseCode::RETCODE_OK);

    // Loan Value to modify the first sequence
    auto seq_data = data->LoanValue(newId);
    ASSERT_TRUE(seq_data != nullptr);

    MemberId newSeqId;
    ASSERT_TRUE(seq_data->InsertSequenceData(newSeqId) == ResponseCode::RETCODE_OK);

    // Set and get a value.
    int32_t test1(234);
    ASSERT_TRUE(seq_data->SetInt32Value(newSeqId, test1) == ResponseCode::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(seq_data->GetInt32Value(test2, newSeqId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Return the pointer of the sequence
    ASSERT_TRUE(data->ReturnLoanedValue(seq_data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->ReturnLoanedValue(seq_data) == ResponseCode::RETCODE_OK);

    // Remove the elements.
    ASSERT_TRUE(data->RemoveSequenceData(newId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->ClearAllValues() == ResponseCode::RETCODE_OK);

    // Check that the sequence is empty.
    ASSERT_FALSE(data->GetInt32Value(test2, 0) == ResponseCode::RETCODE_OK);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    // Delete the sequence
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    // Clean the types Factory.
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(seq_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(seq_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(seq_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(seq_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(seq_seq_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(seq_seq_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(seq_seq_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(seq_seq_type) == ResponseCode::RETCODE_OK);

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

    // Then
    base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
    ASSERT_TRUE(base_type_builder != nullptr);
    auto base_type = base_type_builder->Build();

    array_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateArrayType(base_type, sequence_lengths);
    ASSERT_TRUE(array_type_builder != nullptr);
    auto array_type = array_type_builder->Build();
    ASSERT_TRUE(array_type != nullptr);

    auto data = DynamicDataFactory::GetInstance()->CreateData(array_type);
    ASSERT_FALSE(data->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    MemberId newId;
    ASSERT_FALSE(data->InsertSequenceData(newId) == ResponseCode::RETCODE_OK);

    // Get an index in the multidimensional array.
    std::vector<uint32_t> vPosition = { 1, 1, 1 };
    MemberId testPos(0);
    testPos = data->GetArrayIndex(vPosition);
    ASSERT_TRUE(testPos != MEMBER_ID_INVALID);

    // Invalid input vectors.
    std::vector<uint32_t> vPosition2 = { 1, 1 };
    ASSERT_FALSE(data->GetArrayIndex(vPosition2) != MEMBER_ID_INVALID);
    std::vector<uint32_t> vPosition3 = { 1, 1, 1, 1 };
    ASSERT_FALSE(data->GetArrayIndex(vPosition3) != MEMBER_ID_INVALID);

    // Set and get a value.
    int32_t test1 = 156;
    ASSERT_TRUE(data->SetInt32Value(testPos, test1) == ResponseCode::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(data->GetInt32Value(test2, testPos) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Check items count before and after remove an element.
    ASSERT_TRUE(data->GetItemCount() == 1);
    ASSERT_TRUE(data->ClearValue(testPos) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetItemCount() == 1);
    ASSERT_TRUE(data->RemoveArrayData(testPos) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetItemCount() == 0);

    // Check the clear values method
    ASSERT_TRUE(data->SetInt32Value(testPos, test1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetItemCount() == 1);
    ASSERT_TRUE(data->ClearAllValues() == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetItemCount() == 0);

    // Try to set a value out of the array.
    ASSERT_FALSE(data->SetInt32Value(100, test1) == ResponseCode::RETCODE_OK);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    // Delete the array
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

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

    DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(map_type);

    ASSERT_FALSE(data->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    // Try to write on an empty position
    ASSERT_FALSE(data->SetInt32Value(0, 234) == ResponseCode::RETCODE_OK);

    MemberId keyId;
    MemberId valueId;
    auto key_data = DynamicDataFactory::GetInstance()->CreateData(base_type);
    ASSERT_TRUE(data->InsertMapData(key_data, keyId, valueId) == ResponseCode::RETCODE_OK);

    // Try to Add the same key twice.
    ASSERT_FALSE(data->InsertMapData(key_data, keyId, valueId) == ResponseCode::RETCODE_OK);

    MemberId keyId2;
    MemberId valueId2;
    key_data = DynamicDataFactory::GetInstance()->CreateData(base_type);
    key_data->SetInt32Value(MEMBER_ID_INVALID, 2);
    ASSERT_TRUE(data->InsertMapData(key_data, keyId2, valueId2) == ResponseCode::RETCODE_OK);

    // Try to Add one more than the limit
    auto key_data2 = DynamicDataFactory::GetInstance()->CreateData(base_type);
    key_data2->SetInt32Value(MEMBER_ID_INVALID, 3);
    ASSERT_FALSE(data->InsertMapData(key_data2, keyId, valueId) == ResponseCode::RETCODE_OK);

    // Set and get a value.
    int32_t test1(234);
    ASSERT_TRUE(data->SetInt32Value(valueId, test1) == ResponseCode::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(data->GetInt32Value(test2, valueId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Check items count with removes
    ASSERT_TRUE(data->GetItemCount() == 2);
    ASSERT_FALSE(data->RemoveMapData(valueId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetItemCount() == 2);
    ASSERT_TRUE(data->RemoveMapData(keyId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetItemCount() == 1);
    ASSERT_TRUE(data->ClearAllValues() == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(data->GetItemCount() == 0);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    // Delete the map
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(key_data2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(key_data2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

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

TEST_F(DynamicTypesTests, DynamicType_map_of_maps_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* map_type_builder(nullptr);
    DynamicTypeBuilder* map_map_type_builder(nullptr);
    uint32_t map_length = 2;

    // Then
    base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
    ASSERT_TRUE(base_type_builder != nullptr);
    auto base_type = base_type_builder->Build();

    map_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateMapType(base_type, base_type, map_length);
    ASSERT_TRUE(map_type_builder != nullptr);
    auto map_type = map_type_builder->Build();
    ASSERT_TRUE(map_type != nullptr);

    map_map_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateMapType(base_type, map_type, map_length);
    ASSERT_TRUE(map_map_type_builder != nullptr);
    auto map_map_type = map_map_type_builder->Build();
    ASSERT_TRUE(map_map_type != nullptr);

    DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(map_map_type);

    ASSERT_FALSE(data->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    MemberId keyId;
    MemberId valueId;
    auto key_data = DynamicDataFactory::GetInstance()->CreateData(base_type);
    ASSERT_TRUE(data->InsertMapData(key_data, keyId, valueId) == ResponseCode::RETCODE_OK);

    // Try to Add the same key twice.
    ASSERT_FALSE(data->InsertMapData(key_data, keyId, valueId) == ResponseCode::RETCODE_OK);

    MemberId keyId2;
    MemberId valueId2;
    key_data = DynamicDataFactory::GetInstance()->CreateData(base_type);
    key_data->SetInt32Value(MEMBER_ID_INVALID, 2);
    ASSERT_TRUE(data->InsertMapData(key_data, keyId2, valueId2) == ResponseCode::RETCODE_OK);

    // Try to Add one more than the limit
    auto key_data2 = DynamicDataFactory::GetInstance()->CreateData(base_type);
    key_data2->SetInt32Value(MEMBER_ID_INVALID, 3);
    ASSERT_FALSE(data->InsertMapData(key_data2, keyId, valueId) == ResponseCode::RETCODE_OK);

    auto seq_data = data->LoanValue(valueId);
    ASSERT_TRUE(seq_data != nullptr);

    auto key_data3 = DynamicDataFactory::GetInstance()->CreateData(base_type);
    ASSERT_TRUE(seq_data->InsertMapData(key_data3, keyId, valueId) == ResponseCode::RETCODE_OK);

    // Set and get a value.
    int32_t test1(234);
    ASSERT_TRUE(seq_data->SetInt32Value(valueId, test1) == ResponseCode::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(seq_data->GetInt32Value(test2, valueId) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_TRUE(data->ReturnLoanedValue(seq_data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->ReturnLoanedValue(seq_data) == ResponseCode::RETCODE_OK);

    ASSERT_FALSE(data->SetInt32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint16Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetInt64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUint64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat32Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat64Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetFloat128Value(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar8Value('a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetChar16Value(L'a', MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetByteValue(0, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetBoolValue(false, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetWstringValue(L"", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetEnumValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(data->SetUnionId(0) == ResponseCode::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->GetInt32Value(iTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->GetUint32Value(uTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->GetInt16Value(iTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->GetUint16Value(uTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->GetInt64Value(iTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->GetUint64Value(uTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->GetFloat32Value(fTest32, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->GetFloat64Value(fTest64, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->GetFloat128Value(fTest128, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->GetChar8Value(cTest8, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->GetChar16Value(cTest16, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->GetByteValue(oTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->GetBoolValue(bTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->GetStringValue(sTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->GetWstringValue(wsTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->GetEnumValue(sEnumTest, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    // Delete the map
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(key_data2) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(key_data2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(key_data3) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(key_data3) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

    // Clean the types Factory.
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(base_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(map_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(map_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(map_map_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(map_map_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(map_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(map_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(map_map_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(map_map_type) == ResponseCode::RETCODE_OK);
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
    ASSERT_FALSE(struct_data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

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

TEST_F(DynamicTypesTests, DynamicType_structure_inheritance_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* base_type_builder2(nullptr);
    DynamicTypeBuilder* struct_type_builder(nullptr);
    DynamicTypeBuilder* child_struct_type_builder(nullptr);

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

    // Try to create the child struct without parent
    child_struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateChildStructType(nullptr);
    ASSERT_FALSE(child_struct_type_builder != nullptr);

    // Create the child struct.
    child_struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateChildStructType(struct_type);
    ASSERT_TRUE(child_struct_type_builder != nullptr);

    // Add a new member to the child struct.
    types::MemberDescriptor descriptor3;
    descriptor.SetId(2);
    descriptor.SetName("child_int32");
    descriptor.SetType(base_type);
    ASSERT_TRUE(child_struct_type_builder->AddMember(&descriptor) == ResponseCode::RETCODE_OK);

    // try to add a member to override one of the parent struct.
    types::MemberDescriptor descriptor4;
    descriptor.SetId(3);
    descriptor.SetName("int32");
    descriptor.SetType(base_type);
    ASSERT_FALSE(child_struct_type_builder->AddMember(&descriptor) == ResponseCode::RETCODE_OK);

    auto child_struct_type = child_struct_type_builder->Build();
    ASSERT_TRUE(child_struct_type != nullptr);
    auto struct_data = DynamicDataFactory::GetInstance()->CreateData(child_struct_type);
    ASSERT_TRUE(struct_data != nullptr);

    ASSERT_FALSE(struct_data->SetInt32Value(1, 10) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(struct_data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    // Set and get the parent values.
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
    // Set and get the child value.
    int32_t test5(234);
    ASSERT_TRUE(struct_data->SetInt32Value(2, test5) == ResponseCode::RETCODE_OK);
    int32_t test6(0);
    ASSERT_TRUE(struct_data->GetInt32Value(test6, 2) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test5 == test6);

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

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(child_struct_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(child_struct_type) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(child_struct_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(child_struct_type_builder) == ResponseCode::RETCODE_OK);

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesTests, DynamicType_multi_structure_unit_tests)
{
    // Given
    DynamicTypeBuilderFactory::GetInstance();
    DynamicTypeBuilder* base_type_builder(nullptr);
    DynamicTypeBuilder* base_type_builder2(nullptr);
    DynamicTypeBuilder* struct_type_builder(nullptr);
    DynamicTypeBuilder* parent_struct_type_builder(nullptr);

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

    // Create the parent struct.
    parent_struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructType();
    ASSERT_TRUE(parent_struct_type_builder != nullptr);

    // Add members to the parent struct.
    types::MemberDescriptor descriptor3;
    descriptor3.SetId(0);
    descriptor3.SetName("child_struct");
    descriptor3.SetType(struct_type);
    ASSERT_TRUE(parent_struct_type_builder->AddMember(&descriptor3) == ResponseCode::RETCODE_OK);

    types::MemberDescriptor descriptor4;
    descriptor4.SetId(1);
    descriptor4.SetName("child_int64");
    descriptor4.SetType(base_type2);
    ASSERT_TRUE(parent_struct_type_builder->AddMember(&descriptor4) == ResponseCode::RETCODE_OK);

    auto parent_struct_type = parent_struct_type_builder->Build();
    ASSERT_TRUE(parent_struct_type != nullptr);

    auto struct_data = DynamicDataFactory::GetInstance()->CreateData(parent_struct_type);
    ASSERT_TRUE(struct_data != nullptr);

    ASSERT_FALSE(struct_data->SetInt32Value(1, 10) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(struct_data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

    // Set and get the child values.
    int64_t test1(234);
    ASSERT_TRUE(struct_data->SetInt64Value(1, test1) == ResponseCode::RETCODE_OK);
    int64_t test2(0);
    ASSERT_TRUE(struct_data->GetInt64Value(test2, 1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    auto child_struct_data = struct_data->LoanValue(0);
    ASSERT_TRUE(child_struct_data != nullptr);

    // Set and get the child values.
    int32_t test3(234);
    ASSERT_TRUE(child_struct_data->SetInt32Value(0, test3) == ResponseCode::RETCODE_OK);
    int32_t test4(0);
    ASSERT_TRUE(child_struct_data->GetInt32Value(test4, 0) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test3 == test4);
    int64_t test5(234);
    ASSERT_TRUE(child_struct_data->SetInt64Value(1, test5) == ResponseCode::RETCODE_OK);
    int64_t test6(0);
    ASSERT_TRUE(child_struct_data->GetInt64Value(test6, 1) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(test5 == test6);

    ASSERT_TRUE(struct_data->ReturnLoanedValue(child_struct_data) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(struct_data->ReturnLoanedValue(child_struct_data) == ResponseCode::RETCODE_OK);

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

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(parent_struct_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(parent_struct_type_builder) == ResponseCode::RETCODE_OK);
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(parent_struct_type) == ResponseCode::RETCODE_OK);
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->DeleteType(parent_struct_type) == ResponseCode::RETCODE_OK);

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
    ASSERT_FALSE(union_data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

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

int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
