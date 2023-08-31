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
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <tinyxml2.h>


#include "idl/dds-types-test/aliasesPubSubTypes.h"
#include "idl/dds-types-test/aliasesTypeObject.h"
#include "idl/dds-types-test/appendablePubSubTypes.h"
#include "idl/dds-types-test/appendableTypeObject.h"
#include "idl/dds-types-test/arraysPubSubTypes.h"
#include "idl/dds-types-test/arraysTypeObject.h"
#include "idl/dds-types-test/bitsetsPubSubTypes.h"
#include "idl/dds-types-test/bitsetsTypeObject.h"
#include "idl/dds-types-test/declarationsPubSubTypes.h"
#include "idl/dds-types-test/declarationsTypeObject.h"
#include "idl/dds-types-test/enumerationsPubSubTypes.h"
#include "idl/dds-types-test/enumerationsTypeObject.h"
#include "idl/dds-types-test/finalPubSubTypes.h"
#include "idl/dds-types-test/finalTypeObject.h"
#include "idl/dds-types-test/inheritancePubSubTypes.h"
#include "idl/dds-types-test/inheritanceTypeObject.h"
#include "idl/dds-types-test/mapsPubSubTypes.h"
#include "idl/dds-types-test/mapsTypeObject.h"
#include "idl/dds-types-test/primitivesPubSubTypes.h"
#include "idl/dds-types-test/primitivesTypeObject.h"
#include "idl/dds-types-test/sequencesPubSubTypes.h"
#include "idl/dds-types-test/sequencesTypeObject.h"
#include "idl/dds-types-test/stringsPubSubTypes.h"
#include "idl/dds-types-test/stringsTypeObject.h"
#include "idl/dds-types-test/structuresPubSubTypes.h"
#include "idl/dds-types-test/structuresTypeObject.h"
#include "idl/dds-types-test/unionsPubSubTypes.h"
#include "idl/dds-types-test/unionsTypeObject.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;
#include <unordered_map>
#include <functional>
#include <vector>
#include <algorithm>

class DynamicTypesDDSTypesTest : public ::testing::Test
{
    const std::string config_file_ = "types_profile.xml";

public:

    DynamicTypesDDSTypesTest()
    {
    }

    ~DynamicTypesDDSTypesTest()
    {
        eprosima::fastdds::dds::Log::KillThread();
    }

    virtual void TearDown()
    {
        DynamicDataFactory::delete_instance();
        DynamicTypeBuilderFactory::delete_instance();
    }

    const std::string& config_file()
    {
        return config_file_;
    }

};

enum class ExpectedType {
    Short,
    UShort,
    Long,
    ULong,
    LongLong,
    ULongLong,
    Float,
    Double,
    LongDouble,
    Boolean,
    Octet,
    Char,
    WChar,
    String,
    WString,
    Enum,
    Bitmask,
    Array
};

using SetMethod = std::function<ReturnCode_t(DynamicData*, const void*, MemberId)>;
using GetMethod = std::function<ReturnCode_t(DynamicData*, void*, MemberId)>;

std::map<ExpectedType, SetMethod> setTypeToMethod = {
    {ExpectedType::Short, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_int16_value(*static_cast<const int16_t*>(value), member_id);
    }},
    {ExpectedType::UShort, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_uint16_value(*static_cast<const uint16_t*>(value), member_id);
    }},
    {ExpectedType::Long, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_int32_value(*static_cast<const int32_t*>(value), member_id);
    }},
    {ExpectedType::ULong, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_uint32_value(*static_cast<const uint32_t*>(value), member_id);
    }},
    {ExpectedType::LongLong, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_int64_value(*static_cast<const int64_t*>(value), member_id);
    }},
    {ExpectedType::ULongLong, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_uint64_value(*static_cast<const uint64_t*>(value), member_id);
    }},
    {ExpectedType::Float, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_float32_value(*static_cast<const float*>(value), member_id);
    }},
    {ExpectedType::Double, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_float64_value(*static_cast<const double*>(value), member_id);
    }},
    {ExpectedType::LongDouble, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_float128_value(*static_cast<const long double*>(value), member_id);
    }},
    {ExpectedType::Boolean, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_bool_value(*static_cast<const bool*>(value), member_id);
    }},
    {ExpectedType::Octet, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_byte_value(*static_cast<const octet*>(value), member_id);
    }},
    {ExpectedType::Char, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_char8_value(*static_cast<const char*>(value), member_id);
    }},
    {ExpectedType::WChar, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_char16_value(*static_cast<const wchar_t*>(value), member_id);
    }},
    {ExpectedType::String, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_string_value(*static_cast<const std::string*>(value), member_id);
    }},
    {ExpectedType::WString, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_wstring_value(*static_cast<const std::wstring*>(value), member_id);
    }},
    {ExpectedType::Enum, [](DynamicData* data, const void* value, MemberId member_id) {
        return data->set_enum_value(*static_cast<const std::string*>(value), member_id);
    }},
    {ExpectedType::Bitmask, [](DynamicData* data, const void* value, MemberId member_id) {
        if(member_id == 0){
            return data->set_int16_value(*static_cast<const int16_t*>(value), 0);
        }        
        return data->set_bitmask_value(*static_cast<const uint64_t*>(value));
    }}
};

std::map<ExpectedType, GetMethod> getTypeToMethod = {
    {ExpectedType::Short, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_int16_value(*static_cast<int16_t*>(value), member_id);
    }},
    {ExpectedType::UShort, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_uint16_value(*static_cast<uint16_t*>(value), member_id);
    }},
    {ExpectedType::Long, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_int32_value(*static_cast<int32_t*>(value), member_id);
    }},
    {ExpectedType::ULong, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_uint32_value(*static_cast<uint32_t*>(value), member_id);
    }},
    {ExpectedType::LongLong, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_int64_value(*static_cast<int64_t*>(value), member_id);
    }},
    {ExpectedType::ULongLong, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_uint64_value(*static_cast<uint64_t*>(value), member_id);
    }},
    {ExpectedType::Float, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_float32_value(*static_cast<float*>(value), member_id);
    }},
    {ExpectedType::Double, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_float64_value(*static_cast<double*>(value), member_id);
    }},
    {ExpectedType::LongDouble, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_float128_value(*static_cast<long double*>(value), member_id);
    }},
    {ExpectedType::Boolean, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_bool_value(*static_cast<bool*>(value), member_id);
    }},
    {ExpectedType::Octet, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_byte_value(*static_cast<octet*>(value), member_id);
    }},
    {ExpectedType::Char, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_char8_value(*static_cast<char*>(value), member_id);
    }},
    {ExpectedType::WChar, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_char16_value(*static_cast<wchar_t*>(value), member_id);
    }},
    {ExpectedType::String, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_string_value(*static_cast<std::string*>(value), member_id);
    }},
    {ExpectedType::WString, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_wstring_value(*static_cast<std::wstring*>(value), member_id);
    }},
    {ExpectedType::Enum, [](DynamicData* data, void* value, MemberId member_id) {
        return data->get_enum_value(*static_cast<std::string*>(value), member_id);
    }},
    {ExpectedType::Bitmask, [](DynamicData* data, void* value, MemberId member_id) {
        if(member_id == 0){
            return data->get_int16_value(*static_cast<int16_t*>(value), 0);
        }
        return data->get_bitmask_value(*static_cast<uint64_t*>(value));
    }}
};


void check_set_values(DynamicData* data, const std::vector<ExpectedType>& expected_types, void* type_value, MemberId member_id = MEMBER_ID_INVALID)
{
    for (const auto& typeMethodPair : setTypeToMethod) {
        ExpectedType currentType = typeMethodPair.first;
        SetMethod setMethod = typeMethodPair.second;

        if (std::find(expected_types.begin(), expected_types.end(), currentType) != expected_types.end()) {
            ASSERT_FALSE(setMethod(data, type_value, 0) == ReturnCode_t::RETCODE_OK);
            ASSERT_TRUE(setMethod(data, type_value, member_id) == ReturnCode_t::RETCODE_OK);
        } else {
            if ((std::find(expected_types.begin(), expected_types.end(), ExpectedType::Bitmask) != expected_types.end()) &&
            currentType == ExpectedType::Boolean) {
                //Bitmasks can also use set_boolean_value
            }else{
                ASSERT_FALSE(setMethod(data, type_value, member_id) == ReturnCode_t::RETCODE_OK);
            }
        }
    }
}

void check_get_values(DynamicData* data, const std::vector<ExpectedType>& expected_types, void* type_value, MemberId member_id =  MEMBER_ID_INVALID)
{
    for (const auto& typeMethodPair : getTypeToMethod) {
        ExpectedType currentType = typeMethodPair.first;
        GetMethod getMethod = typeMethodPair.second;

        if (std::find(expected_types.begin(), expected_types.end(), currentType) != expected_types.end()) {
            ASSERT_FALSE(getMethod(data, type_value, 0) == ReturnCode_t::RETCODE_OK);
            ASSERT_TRUE(getMethod(data, type_value, member_id) == ReturnCode_t::RETCODE_OK);
        } else {
            ASSERT_FALSE(getMethod(data, type_value, member_id) == ReturnCode_t::RETCODE_OK);
        }
    }
}



TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ShortStruct)
{        
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_int16_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        int16_t test_value_1 = 123;
        int16_t test_value_2 = 0;

        std::vector<ExpectedType> expected_types = {ExpectedType::Short};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UShortStruct)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_uint16_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        uint16_t test_value_1 = 123;
        uint16_t test_value_2 = 0;

        std::vector<ExpectedType> expected_types = {ExpectedType::UShort};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_LongStruct)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_int32_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        int32_t test_value_1 = 123;
        int32_t test_value_2 = 0;

        std::vector<ExpectedType> expected_types = {ExpectedType::Long};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ULongStruct)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_uint32_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        uint32_t test_value_1 = 123;
        uint32_t test_value_2 = 0;

        std::vector<ExpectedType> expected_types = {ExpectedType::ULong};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_LongLongStruct)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_int64_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        int64_t test_value_1 = 123;
        int64_t test_value_2 = 0;

        std::vector<ExpectedType> expected_types = {ExpectedType::LongLong};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ULongLongStruct)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_uint64_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        uint64_t test_value_1 = 123;
        uint64_t test_value_2 = 0;

        std::vector<ExpectedType> expected_types = {ExpectedType::ULongLong};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_FloatStruct)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_float32_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        float test_value_1 = 123.0f;
        float test_value_2 = 0.0f;

        std::vector<ExpectedType> expected_types = {ExpectedType::Float};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }

    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_DoubleStruct)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_float64_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        double test_value_1 = 123.0;
        double test_value_2 = 0.0;

        std::vector<ExpectedType> expected_types = {ExpectedType::Double};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }

    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_LongDoubleStruct)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_float128_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        long double test_value_1 = 123.0;
        long double test_value_2 = 0.0;

        std::vector<ExpectedType> expected_types = {ExpectedType::LongDouble};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_BooleanStruct)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_bool_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        bool test_value_1 = true;
        bool test_value_2 = false;

        std::vector<ExpectedType> expected_types = {ExpectedType::Boolean};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

        // SERIALIZATION TEST
        BooleanStruct wbool;
        BooleanStructPubSubType wboolpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wboolpb.deserialize(&dynamic_payload, &wbool));

        uint32_t static_payloadSize = static_cast<uint32_t>(wboolpb.getSerializedSizeProvider(&wbool)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wboolpb.serialize(&wbool, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_OctetStruct)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_byte_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        octet test_value_1 = 255;
        octet test_value_2 = 0;

        std::vector<ExpectedType> expected_types = {ExpectedType::Octet};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_CharStruct)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_char8_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        char test_value_1 = 'a';
        char test_value_2 = 'b';

        std::vector<ExpectedType> expected_types = {ExpectedType::Char};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);


        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }

    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_WCharStruct)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_char16_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        wchar_t test_value_1 = L'a';
        wchar_t test_value_2 = L'b';

        std::vector<ExpectedType> expected_types = {ExpectedType::WChar};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_String)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_string_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        std::string test_value_1 = "STRING_TEST";
        std::string test_value_2 = "";

        std::vector<ExpectedType> expected_types = {ExpectedType::String};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_WString)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_wstring_builder();
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        std::wstring test_value_1 = L"STRING_TEST";
        std::wstring test_value_2 = L"";

        std::vector<ExpectedType> expected_types = {ExpectedType::WString};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ShortBoundedString)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_string_builder(1);
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        std::string test_value_1 = "A";
        std::string test_value_2 = "";

        ASSERT_FALSE(data->set_string_value("TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

        std::vector<ExpectedType> expected_types = {ExpectedType::String};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ShortBoundedWString)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_wstring_builder(1);
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        std::wstring test_value_1 = L"A";
        std::wstring test_value_2 = L"";

        ASSERT_FALSE(data->set_wstring_value(L"TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

        std::vector<ExpectedType> expected_types = {ExpectedType::WString};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_LargeBoundedString)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_string_builder(41925);
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        std::string test_value_1;
        for (int i = 0; i < 41925; ++i) {
            test_value_1 += "A";
        }

        std::string test_value_2 = "";
        std::string over_length_string;
        for (int i = 0; i < 41926; ++i) {
            over_length_string += "A";
        }

        ASSERT_FALSE(data->set_string_value(over_length_string, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

        std::vector<ExpectedType> expected_types = {ExpectedType::String};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_LargeBoundedWString)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_wstring_builder(41925);
        ASSERT_TRUE(created_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        std::wstring test_value_1;
        for (int i = 0; i < 41925; ++i) {
            test_value_1 += L"A";
        }

        std::wstring test_value_2 = L"";
        std::wstring over_length_string;
        for (int i = 0; i < 41926; ++i) {
            over_length_string += L"A";
        }

        ASSERT_FALSE(data->set_wstring_value(over_length_string, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

        std::vector<ExpectedType> expected_types = {ExpectedType::WString};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

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
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}


TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_Enum)
{
    {
        DynamicTypeBuilder_ptr created_builder = DynamicTypeBuilderFactory::get_instance()->create_enum_builder();
        ASSERT_TRUE(created_builder != nullptr);

        // Add three members to the enum.
        ASSERT_TRUE(created_builder->add_empty_member(0, "DEFAULT") == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(created_builder->add_empty_member(1, "FIRST") == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(created_builder->add_empty_member(2, "SECOND") == ReturnCode_t::RETCODE_OK);

        // Try to add a descriptor with the same name.
        ASSERT_FALSE(created_builder->add_empty_member(4, "DEFAULT") == ReturnCode_t::RETCODE_OK);

        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

        // Try to set an invalid value.
        ASSERT_FALSE(data->set_enum_value("BAD", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

        std::string test_value_1 = "SECOND";
        std::string test_value_2;

        std::vector<ExpectedType> expected_types = {ExpectedType::Enum};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

        // SERIALIZATION TEST
        EnumStructure wenum;
        EnumStructurePubSubType wenumpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wenumpb.deserialize(&dynamic_payload, &wenum));

        uint32_t static_payloadSize = static_cast<uint32_t>(wenumpb.getSerializedSizeProvider(&wenum)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wenumpb.serialize(&wenum, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}


TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_Bitmask)
{
    uint32_t limit = 5;
    {
        DynamicTypeBuilder_ptr created_builder =
                DynamicTypeBuilderFactory::get_instance()->create_bitmask_builder(limit);
        ASSERT_TRUE(created_builder != nullptr);

        // Add two members to the bitmask
        ASSERT_TRUE(created_builder->add_empty_member(0, "FLAG0") == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(created_builder->add_empty_member(1, "FLAG1") == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(created_builder->add_empty_member(4, "FLAG4") == ReturnCode_t::RETCODE_OK);
        
        // Try to add a descriptor with the same name
        ASSERT_FALSE(created_builder->add_empty_member(1, "FLAG0") == ReturnCode_t::RETCODE_OK);
        // Out of bounds
        ASSERT_FALSE(created_builder->add_empty_member(5, "FLAG5") == ReturnCode_t::RETCODE_OK); 

        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(created_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        MemberId test_member_id;
        test_member_id = data->get_member_id_by_name("FLAG0");
        ASSERT_TRUE(test_member_id != MEMBER_ID_INVALID);
        test_member_id = data->get_member_id_by_name("FLAG1");
        ASSERT_TRUE(test_member_id != MEMBER_ID_INVALID);
        test_member_id = data->get_member_id_by_name("FLAG4");
        ASSERT_TRUE(test_member_id != MEMBER_ID_INVALID);
        test_member_id= data->get_member_id_by_name("FLAG5");
        ASSERT_FALSE(test_member_id != MEMBER_ID_INVALID);

        uint64_t test_bitmask_value_1 = 55;// 00110111
        uint64_t test_bitmask_value_2;

        std::vector<ExpectedType> expected_types = {ExpectedType::Bitmask};
        expected_types.push_back(ExpectedType::ULongLong);
        check_set_values(data, expected_types, &test_bitmask_value_1);
        check_get_values(data, expected_types, &test_bitmask_value_2);

        ASSERT_TRUE(test_bitmask_value_1 == test_bitmask_value_2);
        ASSERT_TRUE(data->get_bool_value("FLAG0"));
        ASSERT_TRUE(data->get_bool_value("FLAG1"));
        ASSERT_TRUE(data->get_bool_value("FLAG4"));

        // Over the limit
        ASSERT_FALSE(data->set_bool_value(true, limit + 1) == ReturnCode_t::RETCODE_OK);

        bool test_value_1 = true;
        bool test_value_2 = false;

        ASSERT_TRUE(data->get_bool_value(test_value_2, 2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(test_value_2 == true);
        ASSERT_TRUE(data->get_bool_value(test_value_2, data->get_member_id_by_name("FLAG0")) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(test_value_2 == data->get_bool_value("FLAG0"));

        bool test3 = data->get_bool_value("FLAG0");
        ASSERT_TRUE(test_value_1 == test3);
        ASSERT_TRUE(data->set_bool_value(true, "FLAG4") == ReturnCode_t::RETCODE_OK);
        bool test4 = data->get_bool_value("FLAG4");
        ASSERT_TRUE(test4 == true);

        test_value_1 = false;
        ASSERT_TRUE(data->set_bool_value(test_value_1, data->get_member_id_by_name("FLAG0")) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(data->get_bool_value(test_value_2, data->get_member_id_by_name("FLAG0")) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

        // SERIALIZATION TEST
        BitMaskStructure wbitmask;
        BitMaskStructurePubSubType wbitmaskpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(wbitmaskpb.deserialize(&dynamic_payload, &wbitmask));

        uint32_t static_payloadSize = static_cast<uint32_t>(wbitmaskpb.getSerializedSizeProvider(&wbitmask)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(wbitmaskpb.serialize(&wbitmask, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}


TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasUInt32)
{
    {
        std::string name = "AliasUInt32";
        DynamicTypeBuilder_ptr base_builder = DynamicTypeBuilderFactory::get_instance()->create_uint32_builder();
        ASSERT_TRUE(base_builder != nullptr);
        DynamicTypeBuilder_ptr alias_builder = DynamicTypeBuilderFactory::get_instance()->create_alias_builder(base_builder.get(), name);
        ASSERT_TRUE(alias_builder != nullptr);
        DynamicType_ptr created_type = DynamicTypeBuilderFactory::get_instance()->create_type(alias_builder.get());
        ASSERT_TRUE(created_type != nullptr);
        ASSERT_TRUE(created_type->get_name() == "AliasUInt32");
        DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(data != nullptr);

        uint32_t test_value_1 = 2;
        uint32_t test_value_2 = 0;

        std::vector<ExpectedType> expected_types = {ExpectedType::ULong};
        check_set_values(data, expected_types, &test_value_1);
        check_get_values(data, expected_types, &test_value_2);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

        // SERIALIZATION TEST
        AliasUInt32 walias;
        AliasUInt32PubSubType waliaspb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(waliaspb.deserialize(&dynamic_payload, &walias));

        uint32_t static_payloadSize = static_cast<uint32_t>(waliaspb.getSerializedSizeProvider(&walias)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(waliaspb.serialize(&walias, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }

    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}


TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MultidimensionalArray)
{
    std::vector<uint32_t> arrays_lengths = { 2, 2, 2 };
    {
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::get_instance()->create_int32_builder();
        DynamicType_ptr base_type = base_type_builder->build();
        ASSERT_TRUE(base_type_builder != nullptr);
        ASSERT_TRUE(base_type != nullptr);

        DynamicTypeBuilder_ptr array_type_builder = DynamicTypeBuilderFactory::get_instance()->create_array_builder(base_type_builder.get(), arrays_lengths);
        DynamicType_ptr array_type = array_type_builder->build();
        ASSERT_TRUE(array_type_builder != nullptr);
        ASSERT_TRUE(array_type != nullptr);

        DynamicData* data = DynamicDataFactory::get_instance()->create_data(array_type);
        ASSERT_TRUE(data != nullptr);

        int32_t test_value_1 = 156;
        int32_t test_value_2 = 0;

        std::vector<ExpectedType> array_expected_types;
        check_set_values(data, array_expected_types, &test_value_1);
        check_get_values(data, array_expected_types, &test_value_2);
        
        // Get an index in the multidimensional array.
        std::vector<uint32_t> vPosition = { 1, 1, 1 };
        MemberId testPos(0);
        testPos = data->get_array_index(vPosition);
        ASSERT_TRUE(data->get_array_index(vPosition) != MEMBER_ID_INVALID);

        // Invalid input vectors.
        std::vector<uint32_t> vPosition2 = { 1, 1 };
        ASSERT_FALSE(data->get_array_index(vPosition2) != MEMBER_ID_INVALID);
        std::vector<uint32_t> vPosition3 = { 1, 1, 1, 1 };
        ASSERT_FALSE(data->get_array_index(vPosition3) != MEMBER_ID_INVALID);

        std::vector<ExpectedType> expected_types= {ExpectedType::Long};
        check_set_values(data, expected_types, &test_value_1, testPos);
        check_get_values(data, expected_types, &test_value_2, testPos);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Check items count before and after remove an element.
        ASSERT_TRUE(data->get_item_count() == array_type->get_total_bounds());
        ASSERT_TRUE(data->clear_value(testPos) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(data->get_item_count() == array_type->get_total_bounds());
        ASSERT_TRUE(data->clear_array_data(testPos) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(data->get_item_count() == array_type->get_total_bounds());

        // Check the clear values method
        ASSERT_TRUE(data->set_int32_value(test_value_1, testPos) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(data->get_item_count() == array_type->get_total_bounds());
        ASSERT_TRUE(data->clear_all_values() == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(data->get_item_count() == array_type->get_total_bounds());

        // Try to set a value out of the array.
        ASSERT_FALSE(data->set_int32_value(test_value_1, 100) == ReturnCode_t::RETCODE_OK);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(array_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(array_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

        // SERIALIZATION TEST
        ArrayLong warray;
        ArrayLongPubSubType warraypb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));        
        ASSERT_TRUE(warraypb.deserialize(&dynamic_payload, &warray));

        uint32_t static_payloadSize = static_cast<uint32_t>(warraypb.getSerializedSizeProvider(&warray)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(warraypb.serialize(&warray, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(array_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}


TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_Sequence)
{
    uint32_t length = 2;
    {
        // Then
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::get_instance()->create_int32_builder();
        ASSERT_TRUE(base_type_builder != nullptr);
        DynamicTypeBuilder_ptr seq_type_builder = DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
            base_type_builder.get(), length);
        ASSERT_TRUE(seq_type_builder != nullptr);
        auto seq_type = seq_type_builder->build();
        ASSERT_TRUE(seq_type != nullptr);

        auto data = DynamicDataFactory::get_instance()->create_data(seq_type);
        ASSERT_FALSE(data->set_int32_value(10, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

        // Try to write on an empty position
        ASSERT_FALSE(data->set_int32_value(234, 1) == ReturnCode_t::RETCODE_OK);

        MemberId newId;
        ASSERT_TRUE(data->insert_sequence_data(newId) == ReturnCode_t::RETCODE_OK);
        MemberId newId2;
        ASSERT_TRUE(data->insert_sequence_data(newId2) == ReturnCode_t::RETCODE_OK);

        // Try to insert more than the limit.
        MemberId newId3;
        ASSERT_FALSE(data->insert_sequence_data(newId3) == ReturnCode_t::RETCODE_OK);

        // Set and get a value.
        int32_t test_value_1(234);
        int32_t test_value_2 = 0;

        ASSERT_TRUE(data->set_int32_value(test_value_1, newId2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(data->get_int32_value(test_value_2, newId2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(test_value_1 == test_value_2);


        std::vector<ExpectedType> sequence_expected_types= {ExpectedType::Long};
        check_set_values(data, sequence_expected_types, &test_value_1);
        check_get_values(data, sequence_expected_types, &test_value_2);


        std::vector<ExpectedType> expected_types= {ExpectedType::Long};
        check_set_values(data, expected_types, &test_value_1, newId);
        check_get_values(data, expected_types, &test_value_2, newId);

        ASSERT_TRUE(test_value_1 == test_value_2);

        // Remove the elements.
        ASSERT_TRUE(data->remove_sequence_data(newId) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(data->clear_all_values() == ReturnCode_t::RETCODE_OK);

        // New Insert Methods
        ASSERT_TRUE(data->insert_int32_value(test_value_1, newId) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(data->get_int32_value(test_value_2, newId) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(test_value_1 == test_value_2);
        ASSERT_TRUE(data->clear_all_values() == ReturnCode_t::RETCODE_OK);

        // Check that the sequence is empty.
        ASSERT_FALSE(data->get_int32_value(test_value_2, 0) == ReturnCode_t::RETCODE_OK);

        ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

        int32_t iTest32;
        ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        uint32_t uTest32;
        ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        int16_t iTest16;
        ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        uint16_t uTest16;
        ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        int64_t iTest64;
        ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        uint64_t uTest64;
        ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        float fTest32;
        ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        double fTest64;
        ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        long double fTest128;
        ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        char cTest8;
        ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        wchar_t cTest16;
        ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        octet oTest;
        ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        bool bTest;
        ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        std::string sTest;
        ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        std::wstring wsTest;
        ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
        std::string sEnumTest;
        ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

        
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(seq_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(seq_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(data));

        // SERIALIZATION TEST
        SequenceLong seq;
        SequenceLongPubSubType seqpb;

        SerializedPayload_t dynamic_payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
        ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

        uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(seq_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}


TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructLong)
{
    {
        DynamicTypeBuilder_ptr base_type_builder = DynamicTypeBuilderFactory::get_instance()->create_int32_builder();
        ASSERT_TRUE(base_type_builder != nullptr);
        auto base_type = base_type_builder->build();

        DynamicTypeBuilder_ptr struct_type_builder = DynamicTypeBuilderFactory::get_instance()->create_struct_builder();
        ASSERT_TRUE(struct_type_builder != nullptr);

        // Add members to the struct.
        ASSERT_TRUE(struct_type_builder->add_member(0, "int32", base_type) == ReturnCode_t::RETCODE_OK);

        auto struct_type = struct_type_builder->build();
        ASSERT_TRUE(struct_type != nullptr);

        auto struct_data = DynamicDataFactory::get_instance()->create_data(struct_type);
        ASSERT_TRUE(struct_data != nullptr);

        ASSERT_FALSE(struct_data->set_int64_value(10, 1) == ReturnCode_t::RETCODE_OK);
        ASSERT_FALSE(struct_data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

        // Set and get the child values.
        int32_t test1(234);
        ASSERT_TRUE(struct_data->set_int32_value(test1, 0) == ReturnCode_t::RETCODE_OK);
        int32_t test2(0);
        ASSERT_TRUE(struct_data->get_int32_value(test2, 0) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(test1 == test2);

        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(struct_type);
        uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(struct_data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);

        types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(struct_type);
        ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
        ASSERT_TRUE(data2->equals(struct_data));

        // SERIALIZATION TEST
        StructLong seq;
        StructLongPubSubType seqpb;

        uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data)());
        SerializedPayload_t dynamic_payload(payloadSize3);
        ASSERT_TRUE(pubsubType.serialize(struct_data, &dynamic_payload));
        ASSERT_TRUE(dynamic_payload.length == payloadSize3);
        ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

        uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
        SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
        ASSERT_TRUE(static_payload.length == static_payloadSize);
        types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(struct_type);
        ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
        ASSERT_TRUE(data3->equals(struct_data));

        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);

        // Delete the structure
        ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(struct_data) == ReturnCode_t::RETCODE_OK);

    }
    ASSERT_TRUE(DynamicTypeBuilderFactory::get_instance()->is_empty());
    ASSERT_TRUE(DynamicDataFactory::get_instance()->is_empty());
}


int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
