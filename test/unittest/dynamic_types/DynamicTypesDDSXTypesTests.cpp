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
#include <fastrtps/log/Log.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "idl/BasicPubSubTypes.h"
#include <tinyxml2.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::types;

class DynamicTypesDDSXTypesTests: public ::testing::Test
{
    public:
        DynamicTypesDDSXTypesTests()
        {
        }

        ~DynamicTypesDDSXTypesTests()
        {
            Log::KillThread();
        }

        virtual void TearDown()
        {
            DynamicDataFactory::DeleteInstance();
            DynamicTypeBuilderFactory::DeleteInstance();
        }
};

TEST_F(DynamicTypesDDSXTypesTests, TypeDescriptors_unit_tests)
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

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_basic_unit_tests)
{
    // Create basic types
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
    ASSERT_TRUE(int32_builder != nullptr);
    DynamicType_ptr int32_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(int32_builder.get());
    ASSERT_TRUE(int32_type != nullptr);
    DynamicType_ptr type2 = DynamicTypeBuilderFactory::GetInstance()->CreateType(int32_builder.get());
    ASSERT_TRUE(type2 != nullptr);
    ASSERT_TRUE(type2->Equals(int32_type.get()));

    DynamicTypeBuilder_ptr struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();
    ASSERT_TRUE(struct_type_builder != nullptr);

    // Add members to the struct.
    ASSERT_TRUE(struct_type_builder->AddMember(0, "int32", int32_type) == ResponseCode::RETCODE_OK);
    auto struct_type = struct_type_builder->Build();
    ASSERT_TRUE(struct_type != nullptr);

    ASSERT_TRUE(struct_type_builder->AddMember(1, "int64", DynamicTypeBuilderFactory::GetInstance()->CreateInt64Type()) == ResponseCode::RETCODE_OK);
    auto struct_type2 = struct_type_builder->Build();
    ASSERT_TRUE(struct_type2 != nullptr);
    ASSERT_FALSE(struct_type->Equals(struct_type2.get()));
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicTypeBuilderFactory_unit_tests)
{
    // Try to create with invalid values
    ASSERT_FALSE(DynamicTypeBuilderFactory::GetInstance()->CreateCustomBuilder(nullptr));
    {
        // Create basic types
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        DynamicType_ptr type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        DynamicType_ptr type3 = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        auto data = DynamicDataFactory::GetInstance()->CreateData(created_builder.get());
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        auto data2 = DynamicDataFactory::GetInstance()->CreateData(type);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUint32Builder();
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateUint32Type();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt16Builder();
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateInt16Type();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUint16Builder();
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateUint16Type();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Builder();
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Type();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUint64Builder();
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateUint64Type();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateFloat32Builder();
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateFloat32Type();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateFloat64Builder();
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateFloat64Type();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateFloat128Builder();
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateFloat128Type();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateChar8Builder();
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateChar8Type();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateChar16Builder();
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateChar16Type();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateByteBuilder();
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateByteType();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateBoolBuilder();
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateBoolType();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStringBuilder(LENGTH_UNLIMITED);
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateStringType();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateWstringBuilder(LENGTH_UNLIMITED);
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        type3 = DynamicTypeBuilderFactory::GetInstance()->CreateWstringType();
        ASSERT_TRUE(type3 != nullptr);
        ASSERT_TRUE(type->Equals(type3.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        // Create with custom types
        TypeDescriptor pInt32Descriptor;
        pInt32Descriptor.SetKind(TK_INT32);
        pInt32Descriptor.SetName("TEST_INT32");
        created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateCustomBuilder(&pInt32Descriptor);
        ASSERT_TRUE(created_builder != nullptr);
        type = created_builder->Build();
        ASSERT_TRUE(type != nullptr);
        type2 = created_builder->Build();
        ASSERT_TRUE(type2 != nullptr);
        ASSERT_TRUE(type->Equals(type2.get()));
        data = DynamicDataFactory::GetInstance()->CreateData(type);
        data2 = DynamicDataFactory::GetInstance()->CreateCopy(data);
        ASSERT_TRUE(data2->Equals(data));
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}



TEST_F(DynamicTypesDDSXTypesTests, DynamicType_int32_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        LongStruct wlong;
        LongStructPubSubType wlongpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wlongpb.deserialize(&dynamic_payload, &wlong));

        uint32_t static_payloadSize = static_cast<uint32_t>(wlongpb.getSerializedSizeProvider(&wlong)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wlongpb.serialize(&wlong, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_uint32_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUint32Builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        ULongStruct wlong;
        ULongStructPubSubType wlongpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wlongpb.deserialize(&dynamic_payload, &wlong));

        uint32_t static_payloadSize = static_cast<uint32_t>(wlongpb.getSerializedSizeProvider(&wlong)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wlongpb.serialize(&wlong, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_int16_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt16Builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        ShortStruct wshort;
        ShortStructPubSubType wshortpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wshortpb.deserialize(&dynamic_payload, &wshort));

        uint32_t static_payloadSize = static_cast<uint32_t>(wshortpb.getSerializedSizeProvider(&wshort)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wshortpb.serialize(&wshort, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_uint16_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUint16Builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        UShortStruct wshort;
        UShortStructPubSubType wshortpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wshortpb.deserialize(&dynamic_payload, &wshort));

        uint32_t static_payloadSize = static_cast<uint32_t>(wshortpb.getSerializedSizeProvider(&wshort)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wshortpb.serialize(&wshort, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_int64_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        LongLongStruct wlonglong;
        LongLongStructPubSubType wlonglongpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wlonglongpb.deserialize(&dynamic_payload, &wlonglong));

        uint32_t static_payloadSize = static_cast<uint32_t>(wlonglongpb.getSerializedSizeProvider(&wlonglong)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wlonglongpb.serialize(&wlonglong, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_uint64_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUint64Builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        ULongLongStruct wlonglong;
        ULongLongStructPubSubType wlonglongpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wlonglongpb.deserialize(&dynamic_payload, &wlonglong));

        uint32_t static_payloadSize = static_cast<uint32_t>(wlonglongpb.getSerializedSizeProvider(&wlonglong)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wlonglongpb.serialize(&wlonglong, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_float32_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateFloat32Builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        FloatStruct wfloat;
        FloatStructPubSubType wfloatpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wfloatpb.deserialize(&dynamic_payload, &wfloat));

        uint32_t static_payloadSize = static_cast<uint32_t>(wfloatpb.getSerializedSizeProvider(&wfloat)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wfloatpb.serialize(&wfloat, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_float64_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateFloat64Builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        DoubleStruct wdouble;
        DoubleStructPubSubType wdoublepb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wdoublepb.deserialize(&dynamic_payload, &wdouble));

        uint32_t static_payloadSize = static_cast<uint32_t>(wdoublepb.getSerializedSizeProvider(&wdouble)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wdoublepb.serialize(&wdouble, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_float128_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateFloat128Builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        LongDoubleStruct wldouble;
        LongDoubleStructPubSubType wldoublepb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wldoublepb.deserialize(&dynamic_payload, &wldouble));

        uint32_t static_payloadSize = static_cast<uint32_t>(wldoublepb.getSerializedSizeProvider(&wldouble)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wldoublepb.serialize(&wldouble, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_char8_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateChar8Builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        CharStruct wchar;
        CharStructPubSubType wcharpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wcharpb.deserialize(&dynamic_payload, &wchar));

        uint32_t static_payloadSize = static_cast<uint32_t>(wcharpb.getSerializedSizeProvider(&wchar)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wcharpb.serialize(&wchar, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_char16_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateChar16Builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        WCharStruct wchar;
        WCharStructPubSubType wcharpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wcharpb.deserialize(&dynamic_payload, &wchar));

        uint32_t static_payloadSize = static_cast<uint32_t>(wcharpb.getSerializedSizeProvider(&wchar)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wcharpb.serialize(&wchar, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_byte_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateByteBuilder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        OctetStruct wchar;
        OctetStructPubSubType wcharpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wcharpb.deserialize(&dynamic_payload, &wchar));

        uint32_t static_payloadSize = static_cast<uint32_t>(wcharpb.getSerializedSizeProvider(&wchar)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wcharpb.serialize(&wchar, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_bool_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateBoolBuilder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        BoolStruct wbool;
        BoolStructPubSubType wboolpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wboolpb.deserialize(&dynamic_payload, &wbool));

        uint32_t static_payloadSize = static_cast<uint32_t>(wboolpb.getSerializedSizeProvider(&wbool)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wboolpb.serialize(&wbool, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_enum_unit_tests)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateEnumBuilder();
        ASSERT_TRUE(created_builder != nullptr);

        // Add three members to the enum.
        ASSERT_TRUE(created_builder->AddEmptyMember(0, "DEFAULT") == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(created_builder->AddEmptyMember(1, "FIRST") == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(created_builder->AddEmptyMember(2, "SECOND") == ResponseCode::RETCODE_OK);

        // Try to add a descriptor with the same name.
        ASSERT_FALSE(created_builder->AddEmptyMember(4, "DEFAULT") == ResponseCode::RETCODE_OK);

        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Work as uint32_t
        uint32_t uTest1 = 2;
        ASSERT_FALSE(data->SetEnumValue(uTest1, 1) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(data->SetEnumValue(uTest1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

        uint32_t uTest2;
        ASSERT_FALSE(data->GetInt32Value(iTest, 0) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(data->GetEnumValue(uTest2, 1) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(data->GetEnumValue(uTest2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(uTest1 == uTest2);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        EnumStruct wenum;
        EnumStructPubSubType wenumpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wenumpb.deserialize(&dynamic_payload, &wenum));

        uint32_t static_payloadSize = static_cast<uint32_t>(wenumpb.getSerializedSizeProvider(&wenum)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wenumpb.serialize(&wenum, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_string_unit_tests)
{
    uint32_t length = 15;
    {
        DynamicTypeBuilderFactory::GetInstance()->CreateStringType(length);
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStringBuilder(length);
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        StringStruct wstring;
        StringStructPubSubType wstringpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wstringpb.deserialize(&dynamic_payload, &wstring));

        uint32_t static_payloadSize = static_cast<uint32_t>(wstringpb.getSerializedSizeProvider(&wstring)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wstringpb.serialize(&wstring, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_wstring_unit_tests)
{
    uint32_t length = 15;
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateWstringBuilder(length);
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // SERIALIZATION TEST
        WStringStruct wwstring;
        WStringStructPubSubType wwstringpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wwstringpb.deserialize(&dynamic_payload, &wwstring));

        uint32_t static_payloadSize = static_cast<uint32_t>(wwstringpb.getSerializedSizeProvider(&wwstring)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wwstringpb.serialize(&wwstring, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_alias_unit_tests)
{
    {
        std::string name = "ALIAS";
        DynamicTypeBuilder_ptr base_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUint32Builder();
        ASSERT_TRUE(base_builder != nullptr);
        DynamicTypeBuilder_ptr alias_builder = DynamicTypeBuilderFactory::GetInstance()->CreateAliasBuilder(base_builder.get(), name);
        ASSERT_TRUE(alias_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(alias_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        ASSERT_TRUE(created_type->GetName() == "ALIAS");
        DynamicData* aliasData = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(aliasData != nullptr);

        ASSERT_FALSE(aliasData->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(aliasData->SetStringValue("", 1) == ResponseCode::RETCODE_OK);

        uint32_t uTest1 = 2;
        ASSERT_TRUE(aliasData->SetUint32Value(uTest1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

        uint32_t uTest2 = 0;
        ASSERT_TRUE(aliasData->GetUint32Value(uTest2, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(uTest1 == uTest2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(aliasData)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(aliasData, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(aliasData));

        // SERIALIZATION TEST
        AliasStruct walias;
        AliasStructPubSubType waliaspb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(aliasData, &dynamic_payload));
        ASSERT_TRUE(waliaspb.deserialize(&dynamic_payload, &walias));

        uint32_t static_payloadSize = static_cast<uint32_t>(waliaspb.getSerializedSizeProvider(&walias)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(waliaspb.serialize(&walias, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(aliasData));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(aliasData) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_multi_alias_unit_tests)
{
    {
        uint32_t length = 15;
        std::string name = "ALIAS";
        std::string name2 = "ALIAS2";
        DynamicTypeBuilder_ptr base_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStringBuilder(length);
        ASSERT_TRUE(base_builder != nullptr);
        DynamicTypeBuilder_ptr base_alias_builder = DynamicTypeBuilderFactory::GetInstance()->CreateAliasBuilder(base_builder.get(), name);
        ASSERT_TRUE(base_alias_builder != nullptr);
        DynamicType_ptr base_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(base_alias_builder.get());
        ASSERT_TRUE(base_type != nullptr);
        ASSERT_TRUE(base_type->GetName() == name);
        DynamicTypeBuilder_ptr alias_builder = DynamicTypeBuilderFactory::GetInstance()->CreateAliasBuilder(base_alias_builder.get(), name2);
        ASSERT_TRUE(alias_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(alias_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        ASSERT_TRUE(created_type->GetName() == name2);
        DynamicData* aliasData = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(aliasData != nullptr);

        // Try to create an alias without base type.
        DynamicTypeBuilder_ptr alias2_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateAliasBuilder(nullptr, "ALIAS2");
        ASSERT_FALSE(alias2_type_builder != nullptr);

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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(aliasData)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(aliasData, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(aliasData));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(aliasData) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
    }

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_bitset_unit_tests)
{
    uint32_t limit = 3;
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateBitsetBuilder(limit);
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

        bool test1 = true;
        ASSERT_FALSE(data->SetInt32Value(1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

        ASSERT_TRUE(data->SetBoolValue(test1, 2) == ResponseCode::RETCODE_OK);

        // Over the limit
        ASSERT_FALSE(data->SetBoolValue(test1, limit + 1) == ResponseCode::RETCODE_OK);

        bool test2 = false;
        ASSERT_TRUE(data->GetBoolValue(test2, 0) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test2 == false);
        ASSERT_TRUE(data->GetBoolValue(test2, 2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);
        test1 = false;
        ASSERT_TRUE(data->SetBoolValue(test1, 2) == ResponseCode::RETCODE_OK);
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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_bitmask_unit_tests)
{
    uint32_t limit = 3;
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::GetInstance()->CreateBitmaskBuilder(limit);
        ASSERT_TRUE(created_builder != nullptr);

        // Add two members to the bitmask
        ASSERT_TRUE(created_builder->AddEmptyMember(0, "TEST") == ResponseCode::RETCODE_OK);

        // Try to add a descriptor with the same name
        ASSERT_FALSE(created_builder->AddEmptyMember(1, "TEST") == ResponseCode::RETCODE_OK);

        ASSERT_TRUE(created_builder->AddEmptyMember(1, "TEST2") == ResponseCode::RETCODE_OK);

        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(data != nullptr);

        MemberId testId = data->GetMemberIdByName("TEST");
        ASSERT_TRUE(testId != MEMBER_ID_INVALID);
        MemberId test2Id = data->GetMemberIdByName("TEST2");
        ASSERT_TRUE(test2Id != MEMBER_ID_INVALID);

        bool test1 = true;
        ASSERT_FALSE(data->SetInt32Value(1, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(data->SetBoolValue(test1, testId) == ResponseCode::RETCODE_OK);

        // Over the limit
        ASSERT_FALSE(data->SetBoolValue(test1, limit + 1) == ResponseCode::RETCODE_OK);

        bool test2 = false;
        ASSERT_TRUE(data->GetBoolValue(test2, 2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test2 == false);
        ASSERT_TRUE(data->GetBoolValue(test2, testId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);
        ASSERT_TRUE(data->GetBoolValue(test2, testId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);
        bool test3 = data->GetBitmaskValue("TEST");
        ASSERT_TRUE(test1 == test3);

        test1 = false;
        ASSERT_TRUE(data->SetBoolValue(test1, testId) == ResponseCode::RETCODE_OK);
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

        // Serialize <-> Deserialize Test
        ASSERT_TRUE(data->SetBoolValue(true, 0) == ResponseCode::RETCODE_OK);
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_sequence_unit_tests)
{
    uint32_t length = 2;
    {
        // Then
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(base_type_builder != nullptr);
        DynamicTypeBuilder_ptr seq_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateSequenceBuilder(
            base_type_builder.get(), length);
        ASSERT_TRUE(seq_type_builder != nullptr);
        auto seq_type = seq_type_builder->Build();
        ASSERT_TRUE(seq_type != nullptr);

        auto data = DynamicDataFactory::GetInstance()->CreateData(seq_type);
        ASSERT_FALSE(data->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

        // Try to write on an empty position
        ASSERT_FALSE(data->SetInt32Value(234, 1) == ResponseCode::RETCODE_OK);

        MemberId newId;
        ASSERT_TRUE(data->InsertSequenceData(newId) == ResponseCode::RETCODE_OK);
        MemberId newId2;
        ASSERT_TRUE(data->InsertSequenceData(newId2) == ResponseCode::RETCODE_OK);

        // Try to insert more than the limit.
        MemberId newId3;
        ASSERT_FALSE(data->InsertSequenceData(newId3) == ResponseCode::RETCODE_OK);

        // Set and get a value.
        int32_t test1(234);
        ASSERT_TRUE(data->SetInt32Value(test1, newId2) == ResponseCode::RETCODE_OK);
        int32_t test2(0);
        ASSERT_TRUE(data->GetInt32Value(test2, newId2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(seq_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(seq_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // Remove the elements.
        ASSERT_TRUE(data->RemoveSequenceData(newId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(data->ClearAllValues() == ResponseCode::RETCODE_OK);

        // New Insert Methods
        ASSERT_TRUE(data->InsertInt32Value(test1, newId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(data->GetInt32Value(test2, newId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);
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


        // SERIALIZATION TEST
        SequenceStruct seq;
        SequenceStructPubSubType seqpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

        uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(seq_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_sequence_of_sequences_unit_tests)
{
    uint32_t sequence_length = 2;
    uint32_t sup_sequence_length = 3;
    {
        // Then
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(base_type_builder != nullptr);

        DynamicTypeBuilder_ptr seq_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateSequenceBuilder(
            base_type_builder.get(), sequence_length);
        ASSERT_TRUE(seq_type_builder != nullptr);
        auto seq_type = seq_type_builder->Build();
        ASSERT_TRUE(seq_type != nullptr);

        DynamicTypeBuilder_ptr seq_seq_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateSequenceBuilder(
            seq_type_builder.get(), sup_sequence_length);
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
        ASSERT_TRUE(seq_data->SetInt32Value(test1, newSeqId) == ResponseCode::RETCODE_OK);
        int32_t test2(0);
        ASSERT_TRUE(seq_data->GetInt32Value(test2, newSeqId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);

        // Return the pointer of the sequence
        ASSERT_TRUE(data->ReturnLoanedValue(seq_data) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(data->ReturnLoanedValue(seq_data) == ResponseCode::RETCODE_OK);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(seq_seq_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(seq_seq_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

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

        // SERIALIZATION TEST
        SequenceSequenceStruct seq;
        SequenceSequenceStructPubSubType seqpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

        uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(seq_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        // New Insert Methods
        ASSERT_TRUE(data->ClearAllValues() == ResponseCode::RETCODE_OK);
        seq_data = DynamicDataFactory::GetInstance()->CreateData(seq_type);
        ASSERT_TRUE(seq_data->InsertInt32Value(test1, newSeqId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(seq_data->GetInt32Value(test2, newSeqId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);
        ASSERT_TRUE(data->InsertComplexValue(seq_data, newId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(data->ClearAllValues() == ResponseCode::RETCODE_OK);


        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }

    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_array_unit_tests)
{
    std::vector<uint32_t> sequence_lengths = { 2, 2, 2 };
    {
        // Then
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(base_type_builder != nullptr);
        auto base_type = base_type_builder->Build();

        DynamicTypeBuilder_ptr array_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateArrayBuilder(
            base_type_builder.get(), sequence_lengths);
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
        ASSERT_TRUE(data->SetInt32Value(test1, testPos) == ResponseCode::RETCODE_OK);
        int32_t test2(0);
        ASSERT_TRUE(data->GetInt32Value(test2, testPos) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(array_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(array_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // Check items count before and after remove an element.
        ASSERT_TRUE(data->GetItemCount() == array_type->GetTotalBounds());
        ASSERT_TRUE(data->ClearValue(testPos) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(data->GetItemCount() == array_type->GetTotalBounds());
        ASSERT_TRUE(data->ClearArrayData(testPos) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(data->GetItemCount() == array_type->GetTotalBounds());

        // Check the clear values method
        ASSERT_TRUE(data->SetInt32Value(test1, testPos) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(data->GetItemCount() == array_type->GetTotalBounds());
        ASSERT_TRUE(data->ClearAllValues() == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(data->GetItemCount() == array_type->GetTotalBounds());

        // Try to set a value out of the array.
        ASSERT_FALSE(data->SetInt32Value(test1, 100) == ResponseCode::RETCODE_OK);

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

        // SERIALIZATION TEST
        ArraytStruct seq;
        ArraytStructPubSubType seqpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

        uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(array_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_array_of_arrays_unit_tests)
{
    std::vector<uint32_t> sequence_lengths = { 2, 2 };
    {
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(base_type_builder != nullptr);
        auto base_type = base_type_builder->Build();

        DynamicTypeBuilder_ptr array_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateArrayBuilder(
            base_type_builder.get(), sequence_lengths);
        ASSERT_TRUE(array_type_builder != nullptr);
        auto array_type = array_type_builder->Build();
        ASSERT_TRUE(array_type != nullptr);

        DynamicTypeBuilder_ptr parent_array_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateArrayBuilder(
            array_type_builder.get(), sequence_lengths);
        ASSERT_TRUE(parent_array_type_builder != nullptr);
        auto parent_array_type = parent_array_type_builder->Build();
        ASSERT_TRUE(parent_array_type != nullptr);

        auto data = DynamicDataFactory::GetInstance()->CreateData(parent_array_type);
        ASSERT_FALSE(data->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

        MemberId newId;
        ASSERT_FALSE(data->InsertSequenceData(newId) == ResponseCode::RETCODE_OK);

        // Get an index in the multidimensional array.
        std::vector<uint32_t> vPosition = { 1, 1 };
        MemberId testPos(0);
        testPos = data->GetArrayIndex(vPosition);
        ASSERT_TRUE(testPos != MEMBER_ID_INVALID);

        // Invalid input vectors.
        std::vector<uint32_t> vPosition2 = { 1, 1, 1 };
        ASSERT_FALSE(data->GetArrayIndex(vPosition2) != MEMBER_ID_INVALID);
        std::vector<uint32_t> vPosition3 = { 1, 1, 1, 1 };
        ASSERT_FALSE(data->GetArrayIndex(vPosition3) != MEMBER_ID_INVALID);

        // Loan Complex values.
        DynamicData* temp = data->LoanValue(testPos);
        ASSERT_TRUE(temp != nullptr);
        DynamicData* temp2 = data->LoanValue(testPos);
        ASSERT_FALSE(temp2 != nullptr);

        int32_t test1 = 156;
        ASSERT_TRUE(temp->SetInt32Value(test1, testPos) == ResponseCode::RETCODE_OK);
        int32_t test2(0);
        ASSERT_TRUE(temp->GetInt32Value(test2, testPos) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);

        ASSERT_TRUE(data->ReturnLoanedValue(temp) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(data->ReturnLoanedValue(temp) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(data->ReturnLoanedValue(temp2) == ResponseCode::RETCODE_OK);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(parent_array_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(parent_array_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        // Check items count before and after remove an element.
        ASSERT_TRUE(data->GetItemCount() == parent_array_type->GetTotalBounds());
        ASSERT_TRUE(data->ClearValue(testPos) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(data->GetItemCount() == parent_array_type->GetTotalBounds());
        ASSERT_TRUE(data->ClearArrayData(testPos) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(data->GetItemCount() == parent_array_type->GetTotalBounds());

        // Try to set a value out of the array.
        ASSERT_FALSE(data->SetInt32Value(test1, 100) == ResponseCode::RETCODE_OK);

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

        // SERIALIZATION TEST
        ArrayArrayStruct seq;
        ArrayArrayStructPubSubType seqpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

        uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(array_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_map_unit_tests)
{
    uint32_t map_length = 2;
    {
        // Then
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(base_type_builder != nullptr);
        auto base_type = base_type_builder->Build();

        DynamicTypeBuilder_ptr map_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateMapBuilder(
            base_type_builder.get(), base_type_builder.get(), map_length);
        ASSERT_TRUE(map_type_builder != nullptr);
        auto map_type = map_type_builder->Build();
        ASSERT_TRUE(map_type != nullptr);

        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(map_type);

        ASSERT_FALSE(data->SetInt32Value(10, MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

        // Try to write on an empty position
        ASSERT_FALSE(data->SetInt32Value(234, 0) == ResponseCode::RETCODE_OK);

        MemberId keyId;
        MemberId valueId;
        auto key_data = DynamicDataFactory::GetInstance()->CreateData(base_type);
        ASSERT_TRUE(data->InsertMapData(key_data, keyId, valueId) == ResponseCode::RETCODE_OK);

        // Try to Add the same key twice.
        ASSERT_FALSE(data->InsertMapData(key_data, keyId, valueId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(key_data) == ResponseCode::RETCODE_OK);

        MemberId keyId2;
        MemberId valueId2;
        key_data = DynamicDataFactory::GetInstance()->CreateData(base_type);
        key_data->SetInt32Value(2, MEMBER_ID_INVALID);
        ASSERT_TRUE(data->InsertMapData(key_data, keyId2, valueId2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(key_data) == ResponseCode::RETCODE_OK);

        // Try to Add one more than the limit
        auto key_data2 = DynamicDataFactory::GetInstance()->CreateData(base_type);
        key_data2->SetInt32Value(3, MEMBER_ID_INVALID);
        ASSERT_FALSE(data->InsertMapData(key_data2, keyId, valueId) == ResponseCode::RETCODE_OK);

        // Set and get a value.
        int32_t test1(234);
        ASSERT_TRUE(data->SetInt32Value(test1, valueId) == ResponseCode::RETCODE_OK);
        int32_t test2(0);
        ASSERT_TRUE(data->GetInt32Value(test2, valueId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(map_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(map_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

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

        //// SERIALIZATION TEST
        //MapStruct seq;
        //MapStructPubSubType seqpb;

        //uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        //SerializedPayload_t dynamic_payload(payloadSize3);
        //ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        //ASSERT_TRUE(dynamic_payload.length == payloadSize3);
        //ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

        //uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
        //SerializedPayload_t static_payload(static_payloadSize);
        //ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
        //ASSERT_TRUE(static_payload.length == static_payloadSize);
        //types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(map_type);
        //ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        //ASSERT_TRUE(data3->Equals(data));

        //ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        //ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        //ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);

        // Delete the map
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(key_data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_map_of_maps_unit_tests)
{
    uint32_t map_length = 2;
    {
        // Then
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(base_type_builder != nullptr);
        auto base_type = base_type_builder->Build();

        DynamicTypeBuilder_ptr map_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateMapBuilder(
            base_type_builder.get(), base_type_builder.get(), map_length);
        ASSERT_TRUE(map_type_builder != nullptr);
        auto map_type = map_type_builder->Build();
        ASSERT_TRUE(map_type != nullptr);

        DynamicTypeBuilder_ptr map_map_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateMapBuilder(
            base_type_builder.get(), map_type_builder.get(), map_length);
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
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(key_data) == ResponseCode::RETCODE_OK);

        MemberId keyId2;
        MemberId valueId2;
        key_data = DynamicDataFactory::GetInstance()->CreateData(base_type);
        key_data->SetInt32Value(2, MEMBER_ID_INVALID);
        ASSERT_TRUE(data->InsertMapData(key_data, keyId2, valueId2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(key_data) == ResponseCode::RETCODE_OK);

        // Try to Add one more than the limit
        auto key_data2 = DynamicDataFactory::GetInstance()->CreateData(base_type);
        key_data2->SetInt32Value(3, MEMBER_ID_INVALID);
        ASSERT_FALSE(data->InsertMapData(key_data2, keyId, valueId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(key_data2) == ResponseCode::RETCODE_OK);

        auto seq_data = data->LoanValue(valueId);
        ASSERT_TRUE(seq_data != nullptr);

        auto key_data3 = DynamicDataFactory::GetInstance()->CreateData(base_type);
        ASSERT_TRUE(seq_data->InsertMapData(key_data3, keyId, valueId) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(key_data3) == ResponseCode::RETCODE_OK);

        // Set and get a value.
        int32_t test1(234);
        ASSERT_TRUE(seq_data->SetInt32Value(test1, valueId) == ResponseCode::RETCODE_OK);
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

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(map_map_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(map_map_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(data));

        //// SERIALIZATION TEST
        //MapMapStruct seq;
        //MapMapStructPubSubType seqpb;

        //uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        //SerializedPayload_t dynamic_payload(payloadSize3);
        //ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        //ASSERT_TRUE(dynamic_payload.length == payloadSize3);
        //ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

        //uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
        //SerializedPayload_t static_payload(static_payloadSize);
        //ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
        //ASSERT_TRUE(static_payload.length == static_payloadSize);
        //types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(map_map_type);
        //ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        //ASSERT_TRUE(data3->Equals(data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        //ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_structure_unit_tests)
{
    {
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(base_type_builder != nullptr);
        auto base_type = base_type_builder->Build();

        DynamicTypeBuilder_ptr base_type_builder2 = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Builder();
        ASSERT_TRUE(base_type_builder2 != nullptr);
        auto base_type2 = base_type_builder2->Build();

        DynamicTypeBuilder_ptr struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();
        ASSERT_TRUE(struct_type_builder != nullptr);

        // Add members to the struct.
        ASSERT_TRUE(struct_type_builder->AddMember(0, "int32", base_type) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(struct_type_builder->AddMember(1, "int64", base_type2) == ResponseCode::RETCODE_OK);

        auto struct_type = struct_type_builder->Build();
        ASSERT_TRUE(struct_type != nullptr);
        auto struct_data = DynamicDataFactory::GetInstance()->CreateData(struct_type);
        ASSERT_TRUE(struct_data != nullptr);

        ASSERT_FALSE(struct_data->SetInt32Value(10, 1) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(struct_data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

        // Set and get the child values.
        int32_t test1(234);
        ASSERT_TRUE(struct_data->SetInt32Value(test1, 0) == ResponseCode::RETCODE_OK);
        int32_t test2(0);
        ASSERT_TRUE(struct_data->GetInt32Value(test2, 0) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);
        int64_t test3(234);
        ASSERT_TRUE(struct_data->SetInt64Value(test3, 1) == ResponseCode::RETCODE_OK);
        int64_t test4(0);
        ASSERT_TRUE(struct_data->GetInt64Value(test4, 1) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test3 == test4);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(struct_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(struct_data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(struct_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(struct_data));

        // SERIALIZATION TEST
        StructStruct seq;
        StructStructPubSubType seqpb;

        uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data)());
        SerializedPayload_t dynamic_payload(payloadSize3);
        ASSERT_TRUE(pubsubType.serialize(struct_data, &dynamic_payload));
        ASSERT_TRUE(dynamic_payload.length == payloadSize3);
        ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

        uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(struct_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(struct_data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);

        // Delete the structure
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(struct_data) == ResponseCode::RETCODE_OK);

    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_structure_inheritance_unit_tests)
{
    {
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(base_type_builder != nullptr);
        auto base_type = base_type_builder->Build();

        DynamicTypeBuilder_ptr base_type_builder2 = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Builder();
        ASSERT_TRUE(base_type_builder2 != nullptr);
        auto base_type2 = base_type_builder2->Build();

        DynamicTypeBuilder_ptr struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();
        ASSERT_TRUE(struct_type_builder != nullptr);

        // Add members to the struct.
        ASSERT_TRUE(struct_type_builder->AddMember(0, "int32", base_type) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(struct_type_builder->AddMember(1, "int64", base_type2) == ResponseCode::RETCODE_OK);

        auto struct_type = struct_type_builder->Build();
        ASSERT_TRUE(struct_type != nullptr);

        // Try to create the child struct without parent
        DynamicTypeBuilder_ptr child_struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateChildStructBuilder(nullptr);
        ASSERT_FALSE(child_struct_type_builder != nullptr);

        // Create the child struct.
        child_struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateChildStructBuilder(struct_type_builder.get());
        ASSERT_TRUE(child_struct_type_builder != nullptr);

        // Add a new member to the child struct.
        ASSERT_TRUE(child_struct_type_builder->AddMember(2, "child_int32", base_type) == ResponseCode::RETCODE_OK);

        // try to add a member to override one of the parent struct.
        ASSERT_FALSE(child_struct_type_builder->AddMember(3, "int32", base_type) == ResponseCode::RETCODE_OK);

        auto child_struct_type = child_struct_type_builder->Build();
        ASSERT_TRUE(child_struct_type != nullptr);
        auto struct_data = DynamicDataFactory::GetInstance()->CreateData(child_struct_type);
        ASSERT_TRUE(struct_data != nullptr);

        ASSERT_FALSE(struct_data->SetInt32Value(10, 1) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(struct_data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

        // Set and get the parent values.
        int32_t test1(234);
        ASSERT_TRUE(struct_data->SetInt32Value(test1, 0) == ResponseCode::RETCODE_OK);
        int32_t test2(0);
        ASSERT_TRUE(struct_data->GetInt32Value(test2, 0) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);
        int64_t test3(234);
        ASSERT_TRUE(struct_data->SetInt64Value(test3, 1) == ResponseCode::RETCODE_OK);
        int64_t test4(0);
        ASSERT_TRUE(struct_data->GetInt64Value(test4, 1) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test3 == test4);
        // Set and get the child value.
        int32_t test5(234);
        ASSERT_TRUE(struct_data->SetInt32Value(test5, 2) == ResponseCode::RETCODE_OK);
        int32_t test6(0);
        ASSERT_TRUE(struct_data->GetInt32Value(test6, 2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test5 == test6);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(child_struct_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(struct_data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(child_struct_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(struct_data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        // Delete the structure
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(struct_data) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_multi_structure_unit_tests)
{
    {
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(base_type_builder != nullptr);
        auto base_type = base_type_builder->Build();

        DynamicTypeBuilder_ptr base_type_builder2 = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Builder();
        ASSERT_TRUE(base_type_builder2 != nullptr);
        auto base_type2 = base_type_builder2->Build();

        DynamicTypeBuilder_ptr struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();
        ASSERT_TRUE(struct_type_builder != nullptr);

        // Add members to the struct.
        ASSERT_TRUE(struct_type_builder->AddMember(0, "int32", base_type) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(struct_type_builder->AddMember(1, "int64", base_type2) == ResponseCode::RETCODE_OK);

        auto struct_type = struct_type_builder->Build();
        ASSERT_TRUE(struct_type != nullptr);

        // Create the parent struct.
        DynamicTypeBuilder_ptr parent_struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();
        ASSERT_TRUE(parent_struct_type_builder != nullptr);

        // Add members to the parent struct.
        ASSERT_TRUE(parent_struct_type_builder->AddMember(0, "child_struct", struct_type) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(parent_struct_type_builder->AddMember(1, "child_int64", base_type2) == ResponseCode::RETCODE_OK);

        auto parent_struct_type = parent_struct_type_builder->Build();
        ASSERT_TRUE(parent_struct_type != nullptr);

        auto struct_data = DynamicDataFactory::GetInstance()->CreateData(parent_struct_type);
        ASSERT_TRUE(struct_data != nullptr);

        ASSERT_FALSE(struct_data->SetInt32Value(10, 1) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(struct_data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

        // Set and get the child values.
        int64_t test1(234);
        ASSERT_TRUE(struct_data->SetInt64Value(test1, 1) == ResponseCode::RETCODE_OK);
        int64_t test2(0);
        ASSERT_TRUE(struct_data->GetInt64Value(test2, 1) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);

        auto child_struct_data = struct_data->LoanValue(0);
        ASSERT_TRUE(child_struct_data != nullptr);

        // Set and get the child values.
        int32_t test3(234);
        ASSERT_TRUE(child_struct_data->SetInt32Value(test3, 0) == ResponseCode::RETCODE_OK);
        int32_t test4(0);
        ASSERT_TRUE(child_struct_data->GetInt32Value(test4, 0) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test3 == test4);
        int64_t test5(234);
        ASSERT_TRUE(child_struct_data->SetInt64Value(test5, 1) == ResponseCode::RETCODE_OK);
        int64_t test6(0);
        ASSERT_TRUE(child_struct_data->GetInt64Value(test6, 1) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test5 == test6);

        ASSERT_TRUE(struct_data->ReturnLoanedValue(child_struct_data) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(struct_data->ReturnLoanedValue(child_struct_data) == ResponseCode::RETCODE_OK);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(parent_struct_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(struct_data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(parent_struct_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(struct_data));

        // SERIALIZATION TEST
        StructStructStruct seq;
        StructStructStructPubSubType seqpb;

        uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data)());
        SerializedPayload_t dynamic_payload(payloadSize3);
        ASSERT_TRUE(pubsubType.serialize(struct_data, &dynamic_payload));
        ASSERT_TRUE(dynamic_payload.length == payloadSize3);
        ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

        uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(parent_struct_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(struct_data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);

        // Delete the map
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(struct_data) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_union_unit_tests)
{
    {
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(base_type_builder != nullptr);
        auto base_type = base_type_builder->Build();

        DynamicTypeBuilder_ptr base_type_builder2 = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Builder();
        ASSERT_TRUE(base_type_builder2 != nullptr);
        auto base_type2 = base_type_builder2->Build();

        DynamicTypeBuilder_ptr union_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUnionBuilder(
            base_type_builder.get());
        ASSERT_TRUE(union_type_builder != nullptr);

        // Add members to the union.
        ASSERT_TRUE(union_type_builder->AddMember(0, "first", base_type, "", { 0 }, true) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(union_type_builder->AddMember(1, "second", base_type2, "", { 1 }, false) == ResponseCode::RETCODE_OK);

        // Try to add a second "DEFAULT" value to the union
        ASSERT_FALSE(union_type_builder->AddMember(0, "third", base_type, "", { 0 }, true) == ResponseCode::RETCODE_OK);

        // Try to add a second value to the same case label
        ASSERT_FALSE(union_type_builder->AddMember(0, "third", base_type, "", { 1 }, false) == ResponseCode::RETCODE_OK);

        // Create a data of this union
        auto union_type = union_type_builder->Build();
        ASSERT_TRUE(union_type != nullptr);
        auto union_data = DynamicDataFactory::GetInstance()->CreateData(union_type);
        ASSERT_TRUE(union_data != nullptr);

        // Set and get the child values.
        ASSERT_FALSE(union_data->SetInt32Value(10, 1) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(union_data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

        uint64_t label;
        ASSERT_TRUE(union_data->GetUnionLabel(label) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(label == 0);

        int32_t test1(234);
        ASSERT_TRUE(union_data->SetInt32Value(test1, 0) == ResponseCode::RETCODE_OK);
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

        ASSERT_TRUE(union_data->SetInt64Value(test3, 1) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(union_data->GetInt64Value(test4, 1) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test3 == test4);
        ASSERT_TRUE(union_data->GetUnionLabel(label) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(label == 1);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(union_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(union_data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(union_data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(union_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(union_data));

        // SERIALIZATION TEST
        SimpleUnionStruct seq;
        SimpleUnionStructPubSubType seqpb;

        uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(union_data)());
        SerializedPayload_t dynamic_payload(payloadSize3);
        ASSERT_TRUE(pubsubType.serialize(union_data, &dynamic_payload));
        ASSERT_TRUE(dynamic_payload.length == payloadSize3);
        ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

        uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::GetInstance()->CreateData(union_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->Equals(union_data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data3) == ResponseCode::RETCODE_OK);

        // Delete the map
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(union_data) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_union_with_unions_unit_tests)
{
    {
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
        ASSERT_TRUE(base_type_builder != nullptr);
        auto base_type = base_type_builder->Build();

        DynamicTypeBuilder_ptr base_type_builder2 = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Builder();
        ASSERT_TRUE(base_type_builder2 != nullptr);
        auto base_type2 = base_type_builder2->Build();

        DynamicTypeBuilder_ptr union_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUnionBuilder(base_type);
        ASSERT_TRUE(union_type_builder != nullptr);

        // Add members to the union.
        ASSERT_TRUE(union_type_builder->AddMember(0, "first", base_type, "", { 0 }, true) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(union_type_builder->AddMember(1, "second", base_type2, "", { 1 }, false) == ResponseCode::RETCODE_OK);

        // Try to add a second "DEFAULT" value to the union
        ASSERT_FALSE(union_type_builder->AddMember(0, "third", base_type, "", { 0 }, true) == ResponseCode::RETCODE_OK);

        // Try to add a second value to the same case label
        ASSERT_FALSE(union_type_builder->AddMember(0, "third", base_type, "", { 1 }, false) == ResponseCode::RETCODE_OK);

        // Create a data of this union
        auto union_type = union_type_builder->Build();
        ASSERT_TRUE(union_type != nullptr);

        DynamicTypeBuilder_ptr parent_union_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUnionBuilder(base_type);
        ASSERT_TRUE(parent_union_type_builder != nullptr);

        // Add Members to the parent union
        ASSERT_TRUE(parent_union_type_builder->AddMember(0, "first", base_type, "", { 0 }, true) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(parent_union_type_builder->AddMember(1, "second", union_type, "", { 1 }, false) == ResponseCode::RETCODE_OK);

        DynamicType_ptr created_type = DynamicTypeBuilderFactory::GetInstance()->CreateType(parent_union_type_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        auto union_data = DynamicDataFactory::GetInstance()->CreateData(parent_union_type_builder.get());
        ASSERT_TRUE(union_data != nullptr);

        // Set and get the child values.
        ASSERT_FALSE(union_data->SetInt32Value(10, 1) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(union_data->SetStringValue("", MEMBER_ID_INVALID) == ResponseCode::RETCODE_OK);

        uint64_t label;
        ASSERT_TRUE(union_data->GetUnionLabel(label) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(label == 0);

        int32_t test1(234);
        ASSERT_TRUE(union_data->SetInt32Value(test1, 0) == ResponseCode::RETCODE_OK);
        int32_t test2(0);
        ASSERT_TRUE(union_data->GetInt32Value(test2, 0) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);
        ASSERT_TRUE(union_data->GetUnionLabel(label) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(label == 0);

        // Loan Value ( Activates this union id )
        DynamicData* child_data = union_data->LoanValue(1);
        ASSERT_TRUE(child_data != 0);

        int64_t test3(234);
        int64_t test4(0);

        // Try to get values from invalid indexes and from an invalid element ( not the current one )
        ASSERT_FALSE(child_data->GetInt32Value(test2, 1) == ResponseCode::RETCODE_OK);
        ASSERT_FALSE(child_data->GetInt64Value(test4, 1) == ResponseCode::RETCODE_OK);

        ASSERT_TRUE(child_data->SetInt64Value(test3, 1) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(child_data->GetInt64Value(test4, 1) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(test3 == test4);

        ASSERT_TRUE(union_data->ReturnLoanedValue(child_data) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(union_data->GetUnionLabel(label) == ResponseCode::RETCODE_OK);
        ASSERT_TRUE(label == 1);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(union_data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(union_data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::GetInstance()->CreateData(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->Equals(union_data));

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data2) == ResponseCode::RETCODE_OK);

        // Delete the map
        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(union_data) == ResponseCode::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::GetInstance()->IsEmpty());
    ASSERT_TRUE(DynamicDataFactory::GetInstance()->IsEmpty());
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_BoolStruct_test)
{
    using namespace xmlparser;
    using namespace types;
    const char* config_file = "typesDDSXTypes.xml";
    tinyxml2::XMLDocument doc;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(config_file);
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("BoolStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        BoolStruct wbool;
        BoolStructPubSubType wboolpb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(wboolpb.deserialize(&dynamic_payload, &wbool));

        uint32_t static_payloadSize = static_cast<uint32_t>(wboolpb.getSerializedSizeProvider(&wbool)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wboolpb.serialize(&wbool, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_OctetStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("OctetStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        OctetStruct refData;
        OctetStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_ShortStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("ShortStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        ShortStruct refData;
        ShortStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_LongStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("LongStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        LongStruct refData;
        LongStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_LongLongStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("LongLongStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        LongLongStruct refData;
        LongLongStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_UShortStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("UShortStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        UShortStruct refData;
        UShortStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_ULongStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("ULongStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        ULongStruct refData;
        ULongStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_ULongLongStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("ULongLongStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        ULongLongStruct refData;
        ULongLongStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_FloatStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("FloatStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        FloatStruct refData;
        FloatStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_DoubleStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("DoubleStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        DoubleStruct refData;
        DoubleStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_CharStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("CharStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        CharStruct refData;
        CharStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_WCharStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("WCharStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        WCharStruct refData;
        WCharStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_StringStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("StringStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        StringStruct refData;
        StringStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_WStringStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("WStringStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        WStringStruct refData;
        WStringStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_LargeStringStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("LargeStringStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        LargeStringStruct refData;
        LargeStringStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_ArraytStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("ArraytStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        ArraytStruct refData;
        ArraytStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_SequenceStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("SequenceStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        SequenceStruct refData;
        SequenceStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_StructStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("StructStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        StructStruct refData;
        StructStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_StructStructStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("StructStructStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        StructStructStruct refData;
        StructStructStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_SimpleUnionStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("SimpleUnionStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        SimpleUnionStruct refData;
        SimpleUnionStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_UnionUnionUnionStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("UnionUnionUnionStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        UnionUnionUnionStruct refData;
        UnionUnionUnionStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_WCharUnionStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("WCharUnionStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        WCharUnionStruct refData;
        WCharUnionStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_EnumStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("EnumStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        EnumStruct refData;
        EnumStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_AliasStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("AliasStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        AliasStruct refData;
        AliasStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_AliasAliasStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("AliasAliasStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        AliasAliasStruct refData;
        AliasAliasStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_LongDoubleStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("LongDoubleStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        LongDoubleStruct refData;
        LongDoubleStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_LargeWStringStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("LargeWStringStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        LargeWStringStruct refData;
        LargeWStringStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_ArrayArrayStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("ArrayArrayStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        ArrayArrayStruct refData;
        ArrayArrayStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_SequenceSequenceStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("SequenceSequenceStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        SequenceSequenceStruct refData;
        SequenceSequenceStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_MapStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("MapStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        MapStruct refData;
        MapStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesDDSXTypesTests, DynamicType_XML_MapMapStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile("typesDDSXTypes.xml");
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType *pbType = XMLProfileManager::CreateDynamicPubSubType("MapMapStruct");
        DynamicData* data = DynamicDataFactory::GetInstance()->CreateData(pbType->GetDynamicType());

        // SERIALIZATION TEST
        MapMapStruct refData;
        MapMapStructPubSubType refDatapb;

        uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pbType->serialize(data, &dynamic_payload));
        ASSERT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

        uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(refDatapb.serialize(&refData, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);

        ASSERT_TRUE(DynamicDataFactory::GetInstance()->DeleteData(data) == ResponseCode::RETCODE_OK);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}


int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
