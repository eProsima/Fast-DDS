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

/**
 * @file BenchMarkPublisher.cpp
 *
 */

#include "DynamicTypesHelper.h"
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>

#include <array>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::types::v1_3;

DynamicData* DynamicTypesHelper::CreateSmallData()
{
    DynamicType_ptr pType {GetBasicStructType()};
    return DynamicDataFactory::get_instance().create_data(*pType);
}

DynamicData* DynamicTypesHelper::CreateData()
{
    DynamicType_ptr pBaseType {DynamicTypeBuilderFactory::get_instance().get_uint32_type()};
    DynamicTypeBuilder_ptr pBuilder {DynamicTypeBuilderFactory::get_instance().create_struct_type()};

    MemberDescriptor md;
    md.set_id(0_id);
    md.set_name("uint");
    md.set_type(*pBaseType);

    pBuilder->add_member(md);
    pBuilder->set_name("Dyn_BenchMark");

    DynamicType_ptr pType {pBuilder->build()};
    return DynamicDataFactory::get_instance().create_data(*pType);
}

DynamicData* DynamicTypesHelper::CreateMediumData()
{
    DynamicType_ptr pType {GetComplexStructType()};
    return DynamicDataFactory::get_instance().create_data(*pType);
}

DynamicData* DynamicTypesHelper::CreateBigData()
{
    DynamicTypeBuilder_ptr completeStruct_builder {DynamicTypeBuilderFactory::get_instance().create_struct_type()};
    completeStruct_builder->set_name("Dyn_BenchMarkBig");

    // Add members to the struct.
    MemberId idx {0};

    MemberDescriptor md;
    md.set_id(idx++);
    md.set_name("uint");
    md.set_type(DynamicTypeBuilderFactory::get_instance().get_uint32_type());
    completeStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_union");
    md.set_type(*GetUnionSwitchType());
    completeStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_union_2");
    md.set_type(*GetUnion2SwitchType());
    completeStruct_builder->add_member(md);

    DynamicType_ptr pType {completeStruct_builder->build()};
    return DynamicDataFactory::get_instance().create_data(*pType);
}

DynamicType_ptr DynamicTypesHelper::GetBasicStructType()
{
    // Members
    DynamicType_ptr bool_type {DynamicTypeBuilderFactory::get_instance().get_bool_type()};
    DynamicType_ptr octet_type {DynamicTypeBuilderFactory::get_instance().get_byte_type()};
    DynamicType_ptr int16_type {DynamicTypeBuilderFactory::get_instance().get_int16_type()};
    DynamicType_ptr int32_type {DynamicTypeBuilderFactory::get_instance().get_int32_type()};
    DynamicType_ptr int64_type {DynamicTypeBuilderFactory::get_instance().get_int64_type()};
    DynamicType_ptr uint16_type {DynamicTypeBuilderFactory::get_instance().get_uint16_type()};
    DynamicType_ptr uint32_type {DynamicTypeBuilderFactory::get_instance().get_uint32_type()};
    DynamicType_ptr uint64_type {DynamicTypeBuilderFactory::get_instance().get_uint64_type()};
    DynamicType_ptr float_type {DynamicTypeBuilderFactory::get_instance().get_float32_type()};
    DynamicType_ptr double_type {DynamicTypeBuilderFactory::get_instance().get_float64_type()};
    DynamicType_ptr ldouble_type {DynamicTypeBuilderFactory::get_instance().get_float128_type()};
    DynamicType_ptr char_type {DynamicTypeBuilderFactory::get_instance().get_char8_type()};
    DynamicType_ptr wchar_type {DynamicTypeBuilderFactory::get_instance().get_char16_type()};
    DynamicType_ptr string_type {DynamicTypeBuilderFactory::get_instance().get_string_type()};
    DynamicType_ptr wstring_type {DynamicTypeBuilderFactory::get_instance().get_wstring_type()};
    DynamicTypeBuilder_ptr basicStruct_builder {DynamicTypeBuilderFactory::get_instance().create_struct_type()};

    // Add members to the struct.
    MemberId idx {0};
    MemberDescriptor md;

    md.set_id(idx++);
    md.set_name("my_uint32");
    md.set_type(*uint32_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_bool");
    md.set_type(*bool_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_octet");
    md.set_type(*octet_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_int16");
    md.set_type(*int16_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_int32");
    md.set_type(*int32_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_int64");
    md.set_type(*int64_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_uint16");
    md.set_type(*uint16_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_uint64");
    md.set_type(*uint64_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_float32");
    md.set_type(*float_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_float64");
    md.set_type(*double_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_float128");
    md.set_type(*ldouble_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_char");
    md.set_type(*char_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_wchar");
    md.set_type(*wchar_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_string");
    md.set_type(*string_type);
    basicStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_wstring");
    md.set_type(*wstring_type);
    basicStruct_builder->add_member(md);

    basicStruct_builder->set_name("Dyn_BenchMarkSmall");

    return DynamicType_ptr {basicStruct_builder->build()};
}

DynamicType_ptr DynamicTypesHelper::GetComplexStructType()
{
    // Members (auxiliar types are tab)
    DynamicType_ptr octet_type {DynamicTypeBuilderFactory::get_instance().get_byte_type()};
    DynamicTypeBuilder_ptr my_sequence_octet_builder
            {DynamicTypeBuilderFactory::get_instance().create_sequence_type(*octet_type, 55)};
    DynamicTypeBuilder_ptr my_sequence_struct_builder
            {DynamicTypeBuilderFactory::get_instance().create_sequence_type(*GetBasicStructType())};

    DynamicType_ptr char_type {DynamicTypeBuilderFactory::get_instance().get_char8_type()};
    DynamicType_ptr byte_type {DynamicTypeBuilderFactory::get_instance().get_byte_type()};

    std::array<uint32_t,3> bounds {500, 5, 4};
    DynamicTypeBuilder_ptr my_array_octet_builder {
            DynamicTypeBuilderFactory::get_instance().create_array_type(
                    *byte_type,
                    bounds.data(),
                    static_cast<uint32_t>(bounds.size()))};

    // MyOctetArray500 is already created
    // We reuse the bounds... { 5 }
    uint32_t len = 5;
    DynamicTypeBuilder_ptr my_array_struct_builder
            {DynamicTypeBuilderFactory::get_instance().create_array_type(*GetBasicStructType(), &len, 1u)};
    DynamicType_ptr int16_type {DynamicTypeBuilderFactory::get_instance().get_int16_type()};
    DynamicTypeBuilder_ptr my_map_octet_short_builder
            {DynamicTypeBuilderFactory::get_instance().create_map_type(*octet_type, *int16_type)};
    DynamicType_ptr int32_type {DynamicTypeBuilderFactory::get_instance().get_int32_type()};
    DynamicTypeBuilder_ptr my_map_long_struct_builder
            {DynamicTypeBuilderFactory::get_instance().create_map_type(*int32_type, *GetBasicStructType())};
    DynamicTypeBuilder_ptr seqOctet_builder
            {DynamicTypeBuilderFactory::get_instance().create_sequence_type(*octet_type)};
    DynamicTypeBuilder_ptr seqSeqOctet_builder
            {DynamicTypeBuilderFactory::get_instance().create_sequence_type(*seqOctet_builder->build())};
    DynamicTypeBuilder_ptr my_map_long_seq_octet_builder
            {DynamicTypeBuilderFactory::get_instance().create_map_type(*int32_type, *seqSeqOctet_builder->build())};
    DynamicTypeBuilder_ptr my_map_long_octet_array_500_builder
            {DynamicTypeBuilderFactory::get_instance().create_map_type(*int32_type, *GetMyOctetArray500Type())};
    DynamicTypeBuilder_ptr map_octet_bsalias5_builder
            {DynamicTypeBuilderFactory::get_instance().create_map_type(*octet_type, *GetBSAlias5Type())};
    DynamicTypeBuilder_ptr my_map_long_lol_type_builder
            {DynamicTypeBuilderFactory::get_instance().create_map_type(*int32_type,
                    *map_octet_bsalias5_builder->build())};
    DynamicType_ptr my_small_string_8_type
            {DynamicTypeBuilderFactory::get_instance().get_string_type(128)};
    DynamicType_ptr my_small_string_16_type
            {DynamicTypeBuilderFactory::get_instance().get_wstring_type(64)};
    DynamicType_ptr my_large_string_8_type
            {DynamicTypeBuilderFactory::get_instance().get_string_type(500)};
    DynamicType_ptr my_large_string_16_type
            {DynamicTypeBuilderFactory::get_instance().get_wstring_type(1024)};
    DynamicType_ptr string75_8_type
            {DynamicTypeBuilderFactory::get_instance().get_string_type(75)};

    std::array<uint32_t,3> sbounds {5, 5};
    DynamicTypeBuilder_ptr my_array_string_builder {
            DynamicTypeBuilderFactory::get_instance().create_array_type(
                    *string75_8_type,
                    sbounds.data(),
                    static_cast<uint32_t>(sbounds.size()))};

    // MA3 is already defined.
    // { 5 } being reused
    DynamicTypeBuilder_ptr my_array_arrays_builder
            {DynamicTypeBuilderFactory::get_instance().create_array_type(*GetMyMiniArrayType(), &len, 1)};

    len = 23u;
    DynamicTypeBuilder_ptr my_sequences_array_builder
            {DynamicTypeBuilderFactory::get_instance().create_array_type(*GetMySequenceLongType(), &len, 1)};
    DynamicTypeBuilder_ptr complexStruct_builder
            {DynamicTypeBuilderFactory::get_instance().create_struct_type()};

    // Add members to the struct.
    MemberId idx {0};
    MemberDescriptor md;

    md.set_id(idx++);
    md.set_name("uint");
    md.set_type(DynamicTypeBuilderFactory::get_instance().get_uint32_type());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_octet");
    md.set_type(*octet_type);
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_basic_struct");
    md.set_type(*GetBasicStructType());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_alias_enum");
    md.set_type(*GetMyAliasEnumType());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_enum");
    md.set_type(*GetMyEnumType());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_sequence_octet");
    md.set_type(my_sequence_octet_builder->build());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_sequence_struct");
    md.set_type(my_sequence_struct_builder->build());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_array_octet");
    md.set_type(my_array_octet_builder->build());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_octet_array_500");
    md.set_type(*GetMyOctetArray500Type());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_array_struct");
    md.set_type(my_array_struct_builder->build());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_map_octet_short");
    md.set_type(my_map_octet_short_builder->build());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_map_long_struct");
    md.set_type(my_map_long_struct_builder->build());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_map_long_seq_octet");
    md.set_type(my_map_long_seq_octet_builder->build());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_map_long_octet_array_500");
    md.set_type(my_map_long_octet_array_500_builder->build());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_map_long_lol_type");
    md.set_type(my_map_long_lol_type_builder->build());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_small_string_8");
    md.set_type(*my_small_string_8_type);
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_small_string_16");
    md.set_type(*my_small_string_16_type);
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_large_string_8");
    md.set_type(*my_large_string_8_type);
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_large_string_16");
    md.set_type(*my_large_string_16_type);
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_array_string");
    md.set_type(my_array_string_builder->build());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("multi_alias_array_42");
    md.set_type(*GetMA3Type());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_array_arrays");
    md.set_type(my_array_arrays_builder->build());
    complexStruct_builder->add_member(md);

    md.set_id(idx++);
    md.set_name("my_sequences_array");
    md.set_type(my_sequences_array_builder->build());
    complexStruct_builder->add_member(md);

    complexStruct_builder->set_name("Dyn_BenchMarkMedium");

    return DynamicType_ptr {complexStruct_builder->build()};
}

DynamicType_ptr DynamicTypesHelper::GetMyOctetArray500Type()
{
    uint32_t len = 500;
    DynamicType_ptr octet_type {DynamicTypeBuilderFactory::get_instance().get_byte_type()};
    DynamicTypeBuilder_ptr myOctetArray500_builder
            {DynamicTypeBuilderFactory::get_instance().create_array_type(*octet_type, &len, 1)};
    myOctetArray500_builder->set_name("MyOctetArray500");
    return DynamicType_ptr {myOctetArray500_builder->build()};
}

DynamicType_ptr DynamicTypesHelper::GetBSAlias5Type()
{
    uint32_t len = 5;
    DynamicTypeBuilder_ptr bSAlias5_builder
            {DynamicTypeBuilderFactory::get_instance().create_array_type(*GetBasicStructType(), &len, 1)};
    bSAlias5_builder->set_name("BSAlias5");
    return DynamicType_ptr {bSAlias5_builder->build()};
}

DynamicType_ptr DynamicTypesHelper::GetMyMiniArrayType()
{
    uint32_t len = 2;
    DynamicType_ptr int32_type {DynamicTypeBuilderFactory::get_instance().get_int32_type()};
    DynamicTypeBuilder_ptr myMiniArray_builder
            {DynamicTypeBuilderFactory::get_instance().create_array_type(*int32_type, &len, 1)};
    myMiniArray_builder->set_name("MyMiniArray");
    return DynamicType_ptr {myMiniArray_builder->build()};
}

DynamicType_ptr DynamicTypesHelper::GetMySequenceLongType()
{
    DynamicType_ptr int32_type {DynamicTypeBuilderFactory::get_instance().get_int32_type()};
    DynamicTypeBuilder_ptr seqLong_builder
            {DynamicTypeBuilderFactory::get_instance().create_sequence_type(*int32_type)};
    DynamicTypeBuilder_ptr mySequenceLong_builder {
            DynamicTypeBuilderFactory::get_instance().create_alias_type(
                    *DynamicType_ptr {seqLong_builder->build()},
                    "MySequenceLong")};
    return DynamicType_ptr {mySequenceLong_builder->build()};
}

DynamicType_ptr DynamicTypesHelper::GetMyEnumType()
{
    DynamicTypeBuilder_ptr myEnum_builder {DynamicTypeBuilderFactory::get_instance().create_enum_type()};
    myEnum_builder->set_name("MyEnum");

    MemberDescriptor md;
    md.set_id(0_id);
    md.set_name("A");
    myEnum_builder->add_member(md);

    md.set_id(1_id);
    md.set_name("B");
    myEnum_builder->add_member(md);

    md.set_id(2_id);
    md.set_name("C");
    myEnum_builder->add_member(md);

    return DynamicType_ptr {myEnum_builder->build()};
}

DynamicType_ptr DynamicTypesHelper::GetMyAliasEnumType()
{
    DynamicTypeBuilder_ptr myAliasEnum_builder
            {DynamicTypeBuilderFactory::get_instance().create_alias_type(*GetMyEnumType(), "MyAliasEnum")};
    return DynamicType_ptr {myAliasEnum_builder->build()};
}

DynamicType_ptr DynamicTypesHelper::GetMA3Type()
{
    uint32_t len = 42;
    DynamicTypeBuilder_ptr mA3_builder
            {DynamicTypeBuilderFactory::get_instance().create_array_type(*GetMyAliasEnum3Type(), &len, 1)};
    mA3_builder->set_name("MA3");
    return DynamicType_ptr {mA3_builder->build()};
}

DynamicType_ptr DynamicTypesHelper::GetMyAliasEnum3Type()
{
    DynamicTypeBuilder_ptr myAliasEnum3_builder
            {DynamicTypeBuilderFactory::get_instance().create_alias_type(*GetMyAliasEnum2Type(), "MyAliasEnum3")};
    return DynamicType_ptr {myAliasEnum3_builder->build()};
}

DynamicType_ptr DynamicTypesHelper::GetMyAliasEnum2Type()
{
    DynamicTypeBuilder_ptr myAliasEnum2_builder
            {DynamicTypeBuilderFactory::get_instance().create_alias_type(*GetMyAliasEnumType(), "MyAliasEnum2")};
    return DynamicType_ptr {myAliasEnum2_builder->build()};
}

DynamicType_ptr DynamicTypesHelper::GetUnionSwitchType()
{
    DynamicTypeBuilder_ptr myUnion_builder {
            DynamicTypeBuilderFactory::get_instance().create_union_type(*GetMyEnumType())};

    MemberDescriptor md;
    md.set_id(0_id);
    md.set_name("basic");
    md.set_type(*GetBasicStructType());
    md.set_default_value("A");
    std::array<uint32_t, 1> ul{ 0 };
    md.set_labels(ul.data(), static_cast<uint32_t>(ul.size()));
    md.set_default_label(false);
    myUnion_builder->add_member(md);

    md.set_id(1_id);
    md.set_name("complex");
    md.set_type(*GetComplexStructType());
    md.set_default_value("B");
    std::array<uint32_t, 2> uls{1, 2};
    md.set_labels(uls.data(), static_cast<uint32_t>(uls.size()));
    md.set_default_label(false);
    myUnion_builder->add_member(md);

    myUnion_builder->set_name("MyUnion");
    return DynamicType_ptr {myUnion_builder->build()};
}

DynamicType_ptr DynamicTypesHelper::GetUnion2SwitchType()
{
    DynamicType_ptr octet_type {DynamicTypeBuilderFactory::get_instance().get_byte_type()};
    DynamicType_ptr int32_type {DynamicTypeBuilderFactory::get_instance().get_int32_type()};
    DynamicType_ptr string_type {DynamicTypeBuilderFactory::get_instance().get_string_type()};
    DynamicTypeBuilder_ptr myUnion2_builder {
            DynamicTypeBuilderFactory::get_instance().create_union_type(*octet_type)};

    MemberDescriptor md;
    md.set_id(0_id);
    md.set_name("uno");
    md.set_type(*int32_type);
    md.set_default_value("0");
    std::array<uint32_t, 1> ul0{ 0 };
    md.set_labels(ul0.data(), static_cast<uint32_t>(ul0.size()));
    md.set_default_label(false);
    myUnion2_builder->add_member(md);

    md.set_id(1_id);
    md.set_name("imString");
    md.set_type(*string_type);
    md.set_default_value("1");
    std::array<uint32_t, 1> ul1{ 1 };
    md.set_labels(ul1.data(), static_cast<uint32_t>(ul1.size()));
    md.set_default_label(false);
    myUnion2_builder->add_member(md);

    md.set_id(2_id);
    md.set_name("tres");
    md.set_type(*int32_type);
    md.set_default_value("2");
    std::array<uint32_t, 1> ul2{ 2 };
    md.set_labels(ul2.data(), static_cast<uint32_t>(ul2.size()));
    md.set_default_label(false);
    myUnion2_builder->add_member(md);

    myUnion2_builder->set_name("MyUnion2");

    return DynamicType_ptr {myUnion2_builder->build()};
}
