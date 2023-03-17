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

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::types;

DynamicData* DynamicTypesHelper::CreateSmallData()
{
    DynamicType_ptr pType = GetBasicStructType();
    return DynamicDataFactory::get_instance()->create_data(pType);
}

DynamicData* DynamicTypesHelper::CreateData()
{
    DynamicType_ptr pBaseType = DynamicTypeBuilderFactory::get_instance().create_uint32_type();
    DynamicTypeBuilder_ptr pBuilder = DynamicTypeBuilderFactory::get_instance().create_struct_builder();
    pBuilder->add_member(0, "uint", pBaseType);
    pBuilder->set_name("Dyn_BenchMark");
    DynamicType_ptr pType = pBuilder->build();
    return DynamicDataFactory::get_instance()->create_data(pType);
}

DynamicData* DynamicTypesHelper::CreateMediumData()
{
    DynamicType_ptr pType = GetComplexStructType();
    return DynamicDataFactory::get_instance()->create_data(pType);
}

DynamicData* DynamicTypesHelper::CreateBigData()
{
    DynamicTypeBuilder_ptr completeStruct_builder = DynamicTypeBuilderFactory::get_instance().create_struct_builder();
    // Add members to the struct.
    int idx = 0;
    completeStruct_builder->add_member(idx++, "uint", DynamicTypeBuilderFactory::get_instance().create_uint32_type());
    completeStruct_builder->add_member(idx++, "my_union", GetUnionSwitchType());
    completeStruct_builder->add_member(idx++, "my_union_2", GetUnion2SwitchType());
    completeStruct_builder->set_name("Dyn_BenchMarkBig");
    DynamicType_ptr pType = completeStruct_builder->build();
    return DynamicDataFactory::get_instance()->create_data(pType);
}

DynamicType_ptr DynamicTypesHelper::GetBasicStructType()
{
    // Members
    DynamicType_ptr bool_type = DynamicTypeBuilderFactory::get_instance().create_bool_type();
    DynamicType_ptr octet_type = DynamicTypeBuilderFactory::get_instance().create_byte_type();
    DynamicType_ptr int16_type = DynamicTypeBuilderFactory::get_instance().create_int16_type();
    DynamicType_ptr int32_type = DynamicTypeBuilderFactory::get_instance().create_int32_type();
    DynamicType_ptr int64_type = DynamicTypeBuilderFactory::get_instance().create_int64_type();
    DynamicType_ptr uint16_type = DynamicTypeBuilderFactory::get_instance().create_uint16_type();
    DynamicType_ptr uint32_type = DynamicTypeBuilderFactory::get_instance().create_uint32_type();
    DynamicType_ptr uint64_type = DynamicTypeBuilderFactory::get_instance().create_uint64_type();
    DynamicType_ptr float_type = DynamicTypeBuilderFactory::get_instance().create_float32_type();
    DynamicType_ptr double_type = DynamicTypeBuilderFactory::get_instance().create_float64_type();
    DynamicType_ptr ldouble_type = DynamicTypeBuilderFactory::get_instance().create_float128_type();
    DynamicType_ptr char_type = DynamicTypeBuilderFactory::get_instance().create_char8_type();
    DynamicType_ptr wchar_type = DynamicTypeBuilderFactory::get_instance().create_char16_type();
    DynamicType_ptr string_type = DynamicTypeBuilderFactory::get_instance().create_string_type();
    DynamicType_ptr wstring_type = DynamicTypeBuilderFactory::get_instance().create_wstring_type();
    DynamicTypeBuilder_ptr basicStruct_builder = DynamicTypeBuilderFactory::get_instance().create_struct_builder();

    // Add members to the struct.
    int idx = 0;
    basicStruct_builder->add_member(idx++, "my_uint32", uint32_type);
    basicStruct_builder->add_member(idx++, "my_bool", bool_type);
    basicStruct_builder->add_member(idx++, "my_octet", octet_type);
    basicStruct_builder->add_member(idx++, "my_int16", int16_type);
    basicStruct_builder->add_member(idx++, "my_int32", int32_type);
    basicStruct_builder->add_member(idx++, "my_int64", int64_type);
    basicStruct_builder->add_member(idx++, "my_uint16", uint16_type);
    basicStruct_builder->add_member(idx++, "my_uint64", uint64_type);
    basicStruct_builder->add_member(idx++, "my_float32", float_type);
    basicStruct_builder->add_member(idx++, "my_float64", double_type);
    basicStruct_builder->add_member(idx++, "my_float128", ldouble_type);
    basicStruct_builder->add_member(idx++, "my_char", char_type);
    basicStruct_builder->add_member(idx++, "my_wchar", wchar_type);
    basicStruct_builder->add_member(idx++, "my_string", string_type);
    basicStruct_builder->add_member(idx++, "my_wstring", wstring_type);
    basicStruct_builder->set_name("Dyn_BenchMarkSmall");

    return basicStruct_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetComplexStructType()
{
    // Members (auxiliar types are tab)
    DynamicType_ptr octet_type = DynamicTypeBuilderFactory::get_instance().create_byte_type();
    DynamicTypeBuilder_ptr my_sequence_octet_builder =
            DynamicTypeBuilderFactory::get_instance().create_sequence_builder(*octet_type, 55);
    DynamicTypeBuilder_ptr my_sequence_struct_builder =
            DynamicTypeBuilderFactory::get_instance().create_sequence_builder(*GetBasicStructType());

    DynamicType_ptr char_type = DynamicTypeBuilderFactory::get_instance().create_char8_type();
    DynamicType_ptr byte_type = DynamicTypeBuilderFactory::get_instance().create_byte_type();
    DynamicTypeBuilder_ptr my_array_octet_builder =
            DynamicTypeBuilderFactory::get_instance().create_array_builder(*byte_type, { 500, 5, 4 });

    // MyOctetArray500 is already created
    // We reuse the bounds... { 5 }
    DynamicTypeBuilder_ptr my_array_struct_builder =
            DynamicTypeBuilderFactory::get_instance().create_array_builder(*GetBasicStructType(), { 5 });
    DynamicType_ptr int16_type = DynamicTypeBuilderFactory::get_instance().create_int16_type();
    DynamicTypeBuilder_ptr my_map_octet_short_builder =
            DynamicTypeBuilderFactory::get_instance().create_map_builder(*octet_type, *int16_type);
    DynamicType_ptr int32_type = DynamicTypeBuilderFactory::get_instance().create_int32_type();
    DynamicTypeBuilder_ptr my_map_long_struct_builder =
            DynamicTypeBuilderFactory::get_instance().create_map_builder(*int32_type, *GetBasicStructType());
    DynamicTypeBuilder_ptr seqOctet_builder =
            DynamicTypeBuilderFactory::get_instance().create_sequence_builder(*octet_type);
    DynamicTypeBuilder_ptr seqSeqOctet_builder =
            DynamicTypeBuilderFactory::get_instance().create_sequence_builder(*seqOctet_builder->build());
    DynamicTypeBuilder_ptr my_map_long_seq_octet_builder =
            DynamicTypeBuilderFactory::get_instance().create_map_builder(*int32_type, *seqSeqOctet_builder->build());
    DynamicTypeBuilder_ptr my_map_long_octet_array_500_builder =
            DynamicTypeBuilderFactory::get_instance().create_map_builder(*int32_type, *GetMyOctetArray500Type());
    DynamicTypeBuilder_ptr map_octet_bsalias5_builder =
            DynamicTypeBuilderFactory::get_instance().create_map_builder(*octet_type, *GetBSAlias5Type());
    DynamicTypeBuilder_ptr my_map_long_lol_type_builder =
            DynamicTypeBuilderFactory::get_instance().create_map_builder(*int32_type, *map_octet_bsalias5_builder->build());
    DynamicType_ptr my_small_string_8_type =
            DynamicTypeBuilderFactory::get_instance().create_string_type(128);
    DynamicType_ptr my_small_string_16_type =
            DynamicTypeBuilderFactory::get_instance().create_wstring_type(64);
    DynamicType_ptr my_large_string_8_type =
            DynamicTypeBuilderFactory::get_instance().create_string_type(500);
    DynamicType_ptr my_large_string_16_type =
            DynamicTypeBuilderFactory::get_instance().create_wstring_type(1024);
    DynamicType_ptr string75_8_type =
            DynamicTypeBuilderFactory::get_instance().create_string_type(75);
    DynamicTypeBuilder_ptr my_array_string_builder =
            DynamicTypeBuilderFactory::get_instance().create_array_builder(*string75_8_type, { 5, 5 });

    // MA3 is already defined.
    // { 5 } being reused
    DynamicTypeBuilder_ptr my_array_arrays_builder =
            DynamicTypeBuilderFactory::get_instance().create_array_builder(*GetMyMiniArrayType(), { 5 });
    DynamicTypeBuilder_ptr my_sequences_array_builder =
            DynamicTypeBuilderFactory::get_instance().create_array_builder(*GetMySequenceLongType(), { 23 });
    DynamicTypeBuilder_ptr complexStruct_builder =
            DynamicTypeBuilderFactory::get_instance().create_struct_builder();

    // Add members to the struct.
    int idx = 0;
    complexStruct_builder->add_member(idx++, "uint", DynamicTypeBuilderFactory::get_instance().create_uint32_type());
    complexStruct_builder->add_member(idx++, "my_octet", octet_type);
    complexStruct_builder->add_member(idx++, "my_basic_struct", GetBasicStructType());
    complexStruct_builder->add_member(idx++, "my_alias_enum", GetMyAliasEnumType());
    complexStruct_builder->add_member(idx++, "my_enum", GetMyEnumType());
    complexStruct_builder->add_member(idx++, "my_sequence_octet", my_sequence_octet_builder->build());
    complexStruct_builder->add_member(idx++, "my_sequence_struct", my_sequence_struct_builder->build());
    complexStruct_builder->add_member(idx++, "my_array_octet", my_array_octet_builder->build());
    complexStruct_builder->add_member(idx++, "my_octet_array_500", GetMyOctetArray500Type());
    complexStruct_builder->add_member(idx++, "my_array_struct", my_array_struct_builder->build());
    complexStruct_builder->add_member(idx++, "my_map_octet_short", my_map_octet_short_builder->build());
    complexStruct_builder->add_member(idx++, "my_map_long_struct", my_map_long_struct_builder->build());
    complexStruct_builder->add_member(idx++, "my_map_long_seq_octet", my_map_long_seq_octet_builder->build());
    complexStruct_builder->add_member(idx++, "my_map_long_octet_array_500", my_map_long_octet_array_500_builder->build());
    complexStruct_builder->add_member(idx++, "my_map_long_lol_type", my_map_long_lol_type_builder->build());
    complexStruct_builder->add_member(idx++, "my_small_string_8", my_small_string_8_type);
    complexStruct_builder->add_member(idx++, "my_small_string_16", my_small_string_16_type);
    complexStruct_builder->add_member(idx++, "my_large_string_8", my_large_string_8_type);
    complexStruct_builder->add_member(idx++, "my_large_string_16", my_large_string_16_type);
    complexStruct_builder->add_member(idx++, "my_array_string", my_array_string_builder->build());
    complexStruct_builder->add_member(idx++, "multi_alias_array_42", GetMA3Type());
    complexStruct_builder->add_member(idx++, "my_array_arrays", my_array_arrays_builder->build());
    complexStruct_builder->add_member(idx++, "my_sequences_array", my_sequences_array_builder->build());
    complexStruct_builder->set_name("Dyn_BenchMarkMedium");
    return complexStruct_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMyOctetArray500Type()
{
    DynamicType_ptr octet_type = DynamicTypeBuilderFactory::get_instance().create_byte_type();
    DynamicTypeBuilder_ptr myOctetArray500_builder =
            DynamicTypeBuilderFactory::get_instance().create_array_builder(*octet_type, { 500 });
    myOctetArray500_builder->set_name("MyOctetArray500");
    return myOctetArray500_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetBSAlias5Type()
{
    DynamicTypeBuilder_ptr bSAlias5_builder =
            DynamicTypeBuilderFactory::get_instance().create_array_builder(*GetBasicStructType(), { 5 });
    bSAlias5_builder->set_name("BSAlias5");
    return bSAlias5_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMyMiniArrayType()
{
    DynamicType_ptr int32_type = DynamicTypeBuilderFactory::get_instance().create_int32_type();
    DynamicTypeBuilder_ptr myMiniArray_builder =
            DynamicTypeBuilderFactory::get_instance().create_array_builder(*int32_type, { 2 });
    myMiniArray_builder->set_name("MyMiniArray");
    return myMiniArray_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMySequenceLongType()
{
    DynamicType_ptr int32_type = DynamicTypeBuilderFactory::get_instance().create_int32_type();
    DynamicTypeBuilder_ptr seqLong_builder =
            DynamicTypeBuilderFactory::get_instance().create_sequence_builder(*int32_type);
    DynamicTypeBuilder_ptr mySequenceLong_builder =
            DynamicTypeBuilderFactory::get_instance().create_alias_builder(*seqLong_builder->build(), "MySequenceLong");
    return mySequenceLong_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMyEnumType()
{
    DynamicTypeBuilder_ptr myEnum_builder = DynamicTypeBuilderFactory::get_instance().create_enum_builder();
    myEnum_builder->set_name("MyEnum");
    myEnum_builder->add_member(0, "A");
    myEnum_builder->add_member(1, "B");
    myEnum_builder->add_member(2, "C");
    return myEnum_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMyAliasEnumType()
{
    DynamicTypeBuilder_ptr myAliasEnum_builder =
            DynamicTypeBuilderFactory::get_instance().create_alias_builder(*GetMyEnumType(), "MyAliasEnum");
    return myAliasEnum_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMA3Type()
{
    DynamicTypeBuilder_ptr mA3_builder =
            DynamicTypeBuilderFactory::get_instance().create_array_builder(*GetMyAliasEnum3Type(), { 42 });
    mA3_builder->set_name("MA3");
    return mA3_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMyAliasEnum3Type()
{
    DynamicTypeBuilder_ptr myAliasEnum3_builder =
            DynamicTypeBuilderFactory::get_instance().create_alias_builder(*GetMyAliasEnum2Type(), "MyAliasEnum3");
    return myAliasEnum3_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMyAliasEnum2Type()
{
    DynamicTypeBuilder_ptr myAliasEnum2_builder =
            DynamicTypeBuilderFactory::get_instance().create_alias_builder(*GetMyAliasEnumType(), "MyAliasEnum2");
    return myAliasEnum2_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetUnionSwitchType()
{
    DynamicTypeBuilder_ptr myUnion_builder = DynamicTypeBuilderFactory::get_instance().create_union_builder(
        *GetMyEnumType());
    myUnion_builder->add_member(0, "basic", GetBasicStructType(), "A", std::vector<uint64_t>{ 0 }, false);
    myUnion_builder->add_member(1, "complex", GetComplexStructType(), "B", std::vector<uint64_t>{ 1, 2 }, false);
    myUnion_builder->set_name("MyUnion");
    return myUnion_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetUnion2SwitchType()
{
    DynamicType_ptr octet_type = DynamicTypeBuilderFactory::get_instance().create_byte_type();
    DynamicType_ptr int32_type = DynamicTypeBuilderFactory::get_instance().create_int32_type();
    DynamicType_ptr string_type = DynamicTypeBuilderFactory::get_instance().create_string_type();
    DynamicTypeBuilder_ptr myUnion2_builder = DynamicTypeBuilderFactory::get_instance().create_union_builder(
        *octet_type);
    myUnion2_builder->add_member(0, "uno", int32_type, "0", std::vector<uint64_t>{ 0 }, false);
    myUnion2_builder->add_member(1, "imString", string_type, "1", std::vector<uint64_t>{ 1 }, false);
    myUnion2_builder->add_member(2, "tres", int32_type, "2", std::vector<uint64_t>{ 2 }, false);
    myUnion2_builder->set_name("MyUnion2");
    return myUnion2_builder->build();
}
