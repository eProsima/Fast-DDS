// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _TEST_UNITTEST_DYNAMIC_TYPES_DYNAMIC_TYPES_DDS_TYPES_TEST_HPP
#define _TEST_UNITTEST_DYNAMIC_TYPES_DYNAMIC_TYPES_DDS_TYPES_TEST_HPP

#include <gtest/gtest.h>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

constexpr const char* enum_name = "InnerEnumHelper";
constexpr const char* enum_value_1_name = "ENUM_VALUE_1";
constexpr const char* enum_value_2_name = "ENUM_VALUE_2";
constexpr const char* enum_value_3_name = "ENUM_VALUE_3";

constexpr const char* bitmask_name = "InnerBitMaskHelper";
constexpr const char* bitmask_flag_0_name = "flag0";
constexpr const char* bitmask_flag_1_name = "flag1";
constexpr const char* bitmask_flag_4_name = "flag4";
constexpr const char* bitmask_flag_6_name = "flag6";

constexpr const char* bounded_bitmask_name = "InnerBoundedBitMaskHelper";
constexpr const char* bounded_bitmask_bflag_0_name = "bflag0";
constexpr const char* bounded_bitmask_bflag_1_name = "bflag1";
constexpr const char* bounded_bitmask_bflag_4_name = "bflag4";
constexpr const char* bounded_bitmask_bflag_6_name = "bflag6";

constexpr const char* alias_name = "InnerAliasHelper";

constexpr const char* union_name = "InnerUnionHelper";
constexpr const char* union_long_member_name = "longValue";
constexpr const char* union_float_member_name = "floatValue";
constexpr const char* union_short_member_name = "shortValue";

constexpr const char* struct_name = "InnerStructureHelper";
constexpr const char* struct_long_member_name = "field1";
constexpr const char* struct_float_member_name = "field2";

constexpr const char* empty_struct_name = "InnerEmptyStructureHelper";

constexpr const char* bitset_name = "InnerBitsetHelper";
constexpr const char* bitfield_a = "a";
constexpr const char* bitfield_b = "b";
constexpr const char* bitfield_c = "c";
constexpr const char* bitfield_d = "d";

constexpr const char* bounded_string_alias = "Inner_alias_bounded_string_helper";
constexpr const char* bounded_wstring_alias = "Inner_alias_bounded_wstring_helper";
constexpr const char* array_alias = "Inner_alias_array_helper";
constexpr const char* seq_alias = "Inner_alias_sequence_helper";
constexpr const char* map_alias = "Inner_alias_map_helper";
constexpr const char* inner_struct_helper_alias_struct_name = "inner_structure_helper_alias";
constexpr const char* inner_bitset_helper_alias_struct_name = "inner_bitset_helper_alias";

constexpr const char* var_short_name = "var_short";
constexpr const char* var_ushort_name = "var_ushort";
constexpr const char* var_long_name = "var_long";
constexpr const char* var_ulong_name = "var_ulong";
constexpr const char* var_long_long_name = "var_longlong";
constexpr const char* var_ulong_long_name = "var_ulonglong";
constexpr const char* var_float_name = "var_float";
constexpr const char* var_double_name = "var_double";
constexpr const char* var_long_double_name = "var_longdouble";
constexpr const char* var_bool_name = "var_boolean";
constexpr const char* var_byte_name = "var_octet";
constexpr const char* var_char_name = "var_char8";
constexpr const char* var_wchar_name = "var_char16";
constexpr const char* var_union_name = "var_union";
constexpr const char* var_str_name = "var_str";

extern std::array<DataRepresentationId_t, 2> encodings;

class DynamicTypesDDSTypesTest : public ::testing::Test
{
public:

    DynamicTypesDDSTypesTest() = default;

    ~DynamicTypesDDSTypesTest();

    void TearDown() override;

    template<typename static_data, typename static_pubsub>
    void check_serialization_deserialization(
            const DynamicType::_ref_type& type,
            DynamicData::_ref_type& data,
            DataRepresentationId data_representation,
            static_data& data_static,
            static_pubsub& static_pubsubType,
            uint8_t ihandle_bytes_to_compare = rtps::RTPS_KEY_HASH_SIZE)
    {
        TypeSupport dyn_pubsubType {new DynamicPubSubType(type)};

        uint32_t dyn_payloadSize = static_cast<uint32_t>(dyn_pubsubType.calculate_serialized_size(&data,
                data_representation));

        // Dynamic Serialization <-> Dynamic Deserialization
        eprosima::fastdds::rtps::SerializedPayload_t dyn_payload(dyn_payloadSize);
        ASSERT_TRUE(dyn_pubsubType.serialize(&data, dyn_payload, data_representation));
        DynamicData::_ref_type data1 {DynamicDataFactory::get_instance()->create_data(type)};
        ASSERT_TRUE(dyn_pubsubType.deserialize(dyn_payload, &data1));
        EXPECT_TRUE(data1->equals(data));

        // Dynamic Serialization <-> Static Deserialization
        ASSERT_TRUE(static_pubsubType.deserialize(dyn_payload, &data_static));

        // Static Serialization <-> Dynamic Deserialization
        uint32_t static_payloadSize = static_pubsubType.calculate_serialized_size(&data_static,
                        data_representation);
        EXPECT_EQ(static_payloadSize, dyn_payloadSize);
        eprosima::fastdds::rtps::SerializedPayload_t static_payload(static_payloadSize);
        ASSERT_TRUE(static_pubsubType.serialize(&data_static, static_payload, data_representation));
        EXPECT_EQ(static_payload.length, static_payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(type)};
        ASSERT_TRUE(dyn_pubsubType.deserialize(static_payload, &data2));
        EXPECT_TRUE(data2->equals(data));

        // Check key hash calculation
        if (static_pubsubType.get()->is_compute_key_provided)
        {
            eprosima::fastdds::rtps::InstanceHandle_t static_ih;
            eprosima::fastdds::rtps::InstanceHandle_t dyn_ih;
            ASSERT_TRUE(static_pubsubType.compute_key(&data_static, static_ih));
            ASSERT_TRUE(dyn_pubsubType.compute_key(&data, dyn_ih));
            EXPECT_NE(static_ih, eprosima::fastdds::dds::InstanceHandle_t());
            // Big endian target
            const rtps::octet* static_ih_start = static_ih.value + rtps::RTPS_KEY_HASH_SIZE - ihandle_bytes_to_compare;
            const rtps::octet* dyn_ih_start = dyn_ih.value + rtps::RTPS_KEY_HASH_SIZE - ihandle_bytes_to_compare;
            ASSERT_EQ(memcmp(static_ih_start, dyn_ih_start, ihandle_bytes_to_compare), 0);
        }

        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data1), RETCODE_OK);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    static void check_typeobject_registry(
            const DynamicType::_ref_type& dyn_type,
            const xtypes::TypeIdentifierPair& static_type_ids);

    /**
     * Auxiliary methods to create helper types
     */
    static DynamicType::_ref_type create_inner_enum_helper();
    static DynamicType::_ref_type create_inner_bitmask_helper();
    static DynamicType::_ref_type create_inner_bounded_bitmask_helper();
    static DynamicType::_ref_type create_inner_alias_helper();
    static DynamicType::_ref_type create_inner_struct_helper();
    static DynamicType::_ref_type create_inner_empty_struct_helper();
    static DynamicType::_ref_type create_inner_struct_helper_alias();
    static DynamicType::_ref_type create_inner_union_helper();
    static DynamicType::_ref_type create_inner_bitset_helper();
    static DynamicType::_ref_type create_inner_alias_bounded_string_helper();
    static DynamicType::_ref_type create_inner_alias_bounded_wstring_helper();
    static DynamicType::_ref_type create_inner_alias_array_helper();
    static DynamicType::_ref_type create_inner_alias_sequence_helper();
    static DynamicType::_ref_type create_inner_alias_map_helper();
    static DynamicType::_ref_type create_inner_bitset_helper_alias();

};

} // dds
} // fastdds
} // eprosima

#endif // _TEST_UNITTEST_DYNAMIC_TYPES_DYNAMIC_TYPES_DDS_TYPES_TEST_HPP
