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
#include <fastrtps/types/DynamicTypeBuilderPtr.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::types;

DynamicData* DynamicTypesHelper::CreateSmallData()
{
    DynamicType_ptr pType = GetBasicStructType();
    return DynamicDataFactory::get_instance()->create_data(pType);
}

DynamicData* DynamicTypesHelper::CreateData()
{
    DynamicType_ptr pBaseType = DynamicTypeBuilderFactory::get_instance()->create_uint32_type();
    DynamicTypeBuilder_ptr pBuilder = DynamicTypeBuilderFactory::get_instance()->create_struct_builder();
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
    DynamicTypeBuilder_ptr completeStruct_builder = DynamicTypeBuilderFactory::get_instance()->create_struct_builder();
    // Add members to the struct.
    int idx = 0;
    completeStruct_builder->add_member(idx++, "uint", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
    completeStruct_builder->add_member(idx++, "my_union", GetUnionSwitchType());
    completeStruct_builder->add_member(idx++, "my_union_2", GetUnion2SwitchType());
    completeStruct_builder->set_name("Dyn_BenchMarkBig");
    DynamicType_ptr pType = completeStruct_builder->build();
    return DynamicDataFactory::get_instance()->create_data(pType);
}

DynamicType_ptr DynamicTypesHelper::GetBasicStructType()
{
    // Members
    DynamicTypeBuilder_ptr bool_builder = DynamicTypeBuilderFactory::get_instance()->create_bool_builder();
    DynamicTypeBuilder_ptr octet_builder = DynamicTypeBuilderFactory::get_instance()->create_byte_builder();
    DynamicTypeBuilder_ptr int16_builder = DynamicTypeBuilderFactory::get_instance()->create_int16_builder();
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::get_instance()->create_int32_builder();
    DynamicTypeBuilder_ptr int64_builder = DynamicTypeBuilderFactory::get_instance()->create_int64_builder();
    DynamicTypeBuilder_ptr uint16_builder = DynamicTypeBuilderFactory::get_instance()->create_uint16_builder();
    DynamicTypeBuilder_ptr uint32_builder = DynamicTypeBuilderFactory::get_instance()->create_uint32_builder();
    DynamicTypeBuilder_ptr uint64_builder = DynamicTypeBuilderFactory::get_instance()->create_uint64_builder();
    DynamicTypeBuilder_ptr float_builder = DynamicTypeBuilderFactory::get_instance()->create_float32_builder();
    DynamicTypeBuilder_ptr double_builder = DynamicTypeBuilderFactory::get_instance()->create_float64_builder();
    DynamicTypeBuilder_ptr ldouble_builder = DynamicTypeBuilderFactory::get_instance()->create_float128_builder();
    DynamicTypeBuilder_ptr char_builder = DynamicTypeBuilderFactory::get_instance()->create_char8_builder();
    DynamicTypeBuilder_ptr wchar_builder = DynamicTypeBuilderFactory::get_instance()->create_char16_builder();
    DynamicTypeBuilder_ptr string_builder = DynamicTypeBuilderFactory::get_instance()->create_string_builder();
    DynamicTypeBuilder_ptr wstring_builder = DynamicTypeBuilderFactory::get_instance()->create_wstring_builder();
    DynamicTypeBuilder_ptr basicStruct_builder = DynamicTypeBuilderFactory::get_instance()->create_struct_builder();

    // Add members to the struct.
    int idx = 0;
    basicStruct_builder->add_member(idx++, "my_uint32", uint32_builder.get());
    basicStruct_builder->add_member(idx++, "my_bool", bool_builder.get());
    basicStruct_builder->add_member(idx++, "my_octet", octet_builder.get());
    basicStruct_builder->add_member(idx++, "my_int16", int16_builder.get());
    basicStruct_builder->add_member(idx++, "my_int32", int32_builder.get());
    basicStruct_builder->add_member(idx++, "my_int64", int64_builder.get());
    basicStruct_builder->add_member(idx++, "my_uint16", uint16_builder.get());
    basicStruct_builder->add_member(idx++, "my_uint64", uint64_builder.get());
    basicStruct_builder->add_member(idx++, "my_float32", float_builder.get());
    basicStruct_builder->add_member(idx++, "my_float64", double_builder.get());
    basicStruct_builder->add_member(idx++, "my_float128", ldouble_builder.get());
    basicStruct_builder->add_member(idx++, "my_char", char_builder.get());
    basicStruct_builder->add_member(idx++, "my_wchar", wchar_builder.get());
    basicStruct_builder->add_member(idx++, "my_string", string_builder.get());
    basicStruct_builder->add_member(idx++, "my_wstring", wstring_builder.get());
    basicStruct_builder->set_name("Dyn_BenchMarkSmall");

    return basicStruct_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetComplexStructType()
{
    // Members (auxiliar types are tab)
    DynamicTypeBuilder_ptr octet_builder = DynamicTypeBuilderFactory::get_instance()->create_byte_builder();
    DynamicTypeBuilder_ptr my_sequence_octet_builder =
        DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(octet_builder.get(), 55);
    DynamicTypeBuilder_ptr my_sequence_struct_builder =
        DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(GetBasicStructType());
    DynamicTypeBuilder_ptr char_builder = DynamicTypeBuilderFactory::get_instance()->create_char8_builder();
    DynamicTypeBuilder_ptr byte_builder = DynamicTypeBuilderFactory::get_instance()->create_byte_builder();
    DynamicTypeBuilder_ptr my_array_octet_builder =
        DynamicTypeBuilderFactory::get_instance()->create_array_builder(byte_builder.get(), { 500, 5, 4 });
    // MyOctetArray500 is already created
    // We reuse the bounds... { 5 }
    DynamicTypeBuilder_ptr my_array_struct_builder =
        DynamicTypeBuilderFactory::get_instance()->create_array_builder(GetBasicStructType(), { 5 });
    DynamicTypeBuilder_ptr int16_builder = DynamicTypeBuilderFactory::get_instance()->create_int16_builder();
    DynamicTypeBuilder_ptr my_map_octet_short_builder =
        DynamicTypeBuilderFactory::get_instance()->create_map_builder(octet_builder.get(), int16_builder.get());
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::get_instance()->create_int32_builder();
    DynamicTypeBuilder_ptr my_map_long_struct_builder =
        DynamicTypeBuilderFactory::get_instance()->create_map_builder(int32_builder.get()->build(), GetBasicStructType());
    DynamicTypeBuilder_ptr seqOctet_builder =
        DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(octet_builder.get());
    DynamicTypeBuilder_ptr seqSeqOctet_builder =
        DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(seqOctet_builder.get());
    DynamicTypeBuilder_ptr my_map_long_seq_octet_builder =
        DynamicTypeBuilderFactory::get_instance()->create_map_builder(int32_builder.get(), seqSeqOctet_builder.get());
    DynamicTypeBuilder_ptr my_map_long_octet_array_500_builder =
        DynamicTypeBuilderFactory::get_instance()->create_map_builder(int32_builder.get()->build(), GetMyOctetArray500Type());
    DynamicTypeBuilder_ptr map_octet_bsalias5_builder =
        DynamicTypeBuilderFactory::get_instance()->create_map_builder(octet_builder.get()->build(), GetBSAlias5Type());
    DynamicTypeBuilder_ptr my_map_long_lol_type_builder =
        DynamicTypeBuilderFactory::get_instance()->create_map_builder(int32_builder.get(), map_octet_bsalias5_builder.get());
    DynamicTypeBuilder_ptr my_small_string_8_builder =
        DynamicTypeBuilderFactory::get_instance()->create_string_builder(128);
    DynamicTypeBuilder_ptr my_small_string_16_builder =
        DynamicTypeBuilderFactory::get_instance()->create_wstring_builder(64);
    DynamicTypeBuilder_ptr my_large_string_8_builder =
        DynamicTypeBuilderFactory::get_instance()->create_string_builder(500);
    DynamicTypeBuilder_ptr my_large_string_16_builder =
        DynamicTypeBuilderFactory::get_instance()->create_wstring_builder(1024);
    DynamicTypeBuilder_ptr string75_8_builder =
        DynamicTypeBuilderFactory::get_instance()->create_string_builder(75);
    DynamicTypeBuilder_ptr my_array_string_builder =
        DynamicTypeBuilderFactory::get_instance()->create_array_builder(string75_8_builder.get(), { 5, 5 });

    // MA3 is already defined.
    // { 5 } being reused
    DynamicTypeBuilder_ptr my_array_arrays_builder =
        DynamicTypeBuilderFactory::get_instance()->create_array_builder(GetMyMiniArrayType(), { 5 });
    DynamicTypeBuilder_ptr my_sequences_array_builder =
        DynamicTypeBuilderFactory::get_instance()->create_array_builder(GetMySequenceLongType(), { 23 });
    DynamicTypeBuilder_ptr complexStruct_builder =
        DynamicTypeBuilderFactory::get_instance()->create_struct_builder();

    // Add members to the struct.
    int idx = 0;
    complexStruct_builder->add_member(idx++, "uint", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
    complexStruct_builder->add_member(idx++, "my_octet", octet_builder.get());
    complexStruct_builder->add_member(idx++, "my_basic_struct", GetBasicStructType());
    complexStruct_builder->add_member(idx++, "my_alias_enum", GetMyAliasEnumType());
    complexStruct_builder->add_member(idx++, "my_enum", GetMyEnumType());
    complexStruct_builder->add_member(idx++, "my_sequence_octet", my_sequence_octet_builder.get());
    complexStruct_builder->add_member(idx++, "my_sequence_struct", my_sequence_struct_builder.get());
    complexStruct_builder->add_member(idx++, "my_array_octet", my_array_octet_builder.get());
    complexStruct_builder->add_member(idx++, "my_octet_array_500", GetMyOctetArray500Type());
    complexStruct_builder->add_member(idx++, "my_array_struct", my_array_struct_builder.get());
    complexStruct_builder->add_member(idx++, "my_map_octet_short", my_map_octet_short_builder.get());
    complexStruct_builder->add_member(idx++, "my_map_long_struct", my_map_long_struct_builder.get());
    complexStruct_builder->add_member(idx++, "my_map_long_seq_octet", my_map_long_seq_octet_builder.get());
    complexStruct_builder->add_member(idx++, "my_map_long_octet_array_500", my_map_long_octet_array_500_builder.get());
    complexStruct_builder->add_member(idx++, "my_map_long_lol_type", my_map_long_lol_type_builder.get());
    complexStruct_builder->add_member(idx++, "my_small_string_8", my_small_string_8_builder.get());
    complexStruct_builder->add_member(idx++, "my_small_string_16", my_small_string_16_builder.get());
    complexStruct_builder->add_member(idx++, "my_large_string_8", my_large_string_8_builder.get());
    complexStruct_builder->add_member(idx++, "my_large_string_16", my_large_string_16_builder.get());
    complexStruct_builder->add_member(idx++, "my_array_string", my_array_string_builder.get());
    complexStruct_builder->add_member(idx++, "multi_alias_array_42", GetMA3Type());
    complexStruct_builder->add_member(idx++, "my_array_arrays", my_array_arrays_builder.get());
    complexStruct_builder->add_member(idx++, "my_sequences_array", my_sequences_array_builder.get());
    complexStruct_builder->set_name("Dyn_BenchMarkMedium");
    return complexStruct_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMyOctetArray500Type()
{
    DynamicTypeBuilder_ptr octet_builder = DynamicTypeBuilderFactory::get_instance()->create_byte_builder();
    DynamicTypeBuilder_ptr myOctetArray500_builder =
        DynamicTypeBuilderFactory::get_instance()->create_array_builder(octet_builder.get(), { 500 });
    myOctetArray500_builder->set_name("MyOctetArray500");
    return myOctetArray500_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetBSAlias5Type()
{
    DynamicTypeBuilder_ptr bSAlias5_builder =
        DynamicTypeBuilderFactory::get_instance()->create_array_builder(GetBasicStructType(), { 5 });
    bSAlias5_builder->set_name("BSAlias5");
    return bSAlias5_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMyMiniArrayType()
{
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::get_instance()->create_int32_builder();
    DynamicTypeBuilder_ptr myMiniArray_builder =
        DynamicTypeBuilderFactory::get_instance()->create_array_builder(int32_builder.get(), { 2 });
    myMiniArray_builder->set_name("MyMiniArray");
    return myMiniArray_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMySequenceLongType()
{
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::get_instance()->create_int32_builder();
    DynamicTypeBuilder_ptr seqLong_builder =
        DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(int32_builder.get());
    DynamicTypeBuilder_ptr mySequenceLong_builder =
        DynamicTypeBuilderFactory::get_instance()->create_alias_builder(seqLong_builder.get(), "MySequenceLong");
    return mySequenceLong_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMyEnumType()
{
    DynamicTypeBuilder_ptr myEnum_builder = DynamicTypeBuilderFactory::get_instance()->create_enum_builder();
    myEnum_builder->set_name("MyEnum");
    myEnum_builder->add_empty_member(0, "A");
    myEnum_builder->add_empty_member(1, "B");
    myEnum_builder->add_empty_member(2, "C");
    return myEnum_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMyAliasEnumType()
{
    DynamicTypeBuilder_ptr myAliasEnum_builder =
        DynamicTypeBuilderFactory::get_instance()->create_alias_builder(GetMyEnumType(), "MyAliasEnum");
    return myAliasEnum_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMA3Type()
{
    DynamicTypeBuilder_ptr mA3_builder =
        DynamicTypeBuilderFactory::get_instance()->create_array_builder(GetMyAliasEnum3Type(), { 42 });
    mA3_builder->set_name("MA3");
    return mA3_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMyAliasEnum3Type()
{
    DynamicTypeBuilder_ptr myAliasEnum3_builder =
        DynamicTypeBuilderFactory::get_instance()->create_alias_builder(GetMyAliasEnum2Type(), "MyAliasEnum3");
    return myAliasEnum3_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetMyAliasEnum2Type()
{
    DynamicTypeBuilder_ptr myAliasEnum2_builder =
        DynamicTypeBuilderFactory::get_instance()->create_alias_builder(GetMyAliasEnumType(), "MyAliasEnum2");
    return myAliasEnum2_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetUnionSwitchType()
{
    DynamicTypeBuilder_ptr myUnion_builder = DynamicTypeBuilderFactory::get_instance()->create_union_builder(GetMyEnumType());
    myUnion_builder->add_member(0, "basic", GetBasicStructType(), "A", { 0 }, false);
    myUnion_builder->add_member(1, "complex", GetComplexStructType(), "B", { 1, 2 }, false);
    myUnion_builder->set_name("MyUnion");
    return myUnion_builder->build();
}

DynamicType_ptr DynamicTypesHelper::GetUnion2SwitchType()
{
    DynamicTypeBuilder_ptr octet_builder = DynamicTypeBuilderFactory::get_instance()->create_byte_builder();
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::get_instance()->create_int32_builder();
    DynamicTypeBuilder_ptr string_builder = DynamicTypeBuilderFactory::get_instance()->create_string_builder();
    DynamicTypeBuilder_ptr myUnion2_builder = DynamicTypeBuilderFactory::get_instance()->create_union_builder(octet_builder.get());
    myUnion2_builder->add_member(0, "uno", int32_builder.get(), "0", { 0 }, false);
    myUnion2_builder->add_member(1, "imString", string_builder.get(), "1", { 1 }, false);
    myUnion2_builder->add_member(2, "tres", int32_builder.get(), "2", { 2 }, false);
    myUnion2_builder->set_name("MyUnion2");
    return myUnion2_builder->build();
}