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
    return DynamicDataFactory::GetInstance()->CreateData(pType);
}

DynamicData* DynamicTypesHelper::CreateData()
{
    DynamicType_ptr pBaseType = DynamicTypeBuilderFactory::GetInstance()->CreateUint32Type();
    DynamicTypeBuilder_ptr pBuilder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();
    pBuilder->AddMember(0, "uint", pBaseType);
    pBuilder->SetName("Dyn_BenchMark");
    DynamicType_ptr pType = pBuilder->Build();
    return DynamicDataFactory::GetInstance()->CreateData(pType);
}

DynamicData* DynamicTypesHelper::CreateMediumData()
{
    DynamicType_ptr pType = GetComplexStructType();
    return DynamicDataFactory::GetInstance()->CreateData(pType);
}

DynamicData* DynamicTypesHelper::CreateBigData()
{
    DynamicTypeBuilder_ptr completeStruct_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();
    // Add members to the struct.
    int idx = 0;
    completeStruct_builder->AddMember(idx++, "uint", DynamicTypeBuilderFactory::GetInstance()->CreateUint32Type());
    completeStruct_builder->AddMember(idx++, "my_union", GetUnionSwitchType());
    completeStruct_builder->AddMember(idx++, "my_union_2", GetUnion2SwitchType());
    completeStruct_builder->SetName("Dyn_BenchMarkBig");
    DynamicType_ptr pType = completeStruct_builder->Build();
    return DynamicDataFactory::GetInstance()->CreateData(pType);
}

DynamicType_ptr DynamicTypesHelper::GetBasicStructType()
{
    // Members
    DynamicTypeBuilder_ptr bool_builder = DynamicTypeBuilderFactory::GetInstance()->CreateBoolBuilder();
    DynamicTypeBuilder_ptr octet_builder = DynamicTypeBuilderFactory::GetInstance()->CreateByteBuilder();
    DynamicTypeBuilder_ptr int16_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt16Builder();
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
    DynamicTypeBuilder_ptr int64_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt64Builder();
    DynamicTypeBuilder_ptr uint16_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUint16Builder();
    DynamicTypeBuilder_ptr uint32_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUint32Builder();
    DynamicTypeBuilder_ptr uint64_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUint64Builder();
    DynamicTypeBuilder_ptr float_builder = DynamicTypeBuilderFactory::GetInstance()->CreateFloat32Builder();
    DynamicTypeBuilder_ptr double_builder = DynamicTypeBuilderFactory::GetInstance()->CreateFloat64Builder();
    DynamicTypeBuilder_ptr ldouble_builder = DynamicTypeBuilderFactory::GetInstance()->CreateFloat128Builder();
    DynamicTypeBuilder_ptr char_builder = DynamicTypeBuilderFactory::GetInstance()->CreateChar8Builder();
    DynamicTypeBuilder_ptr wchar_builder = DynamicTypeBuilderFactory::GetInstance()->CreateChar16Builder();
    DynamicTypeBuilder_ptr string_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStringBuilder();
    DynamicTypeBuilder_ptr wstring_builder = DynamicTypeBuilderFactory::GetInstance()->CreateWstringBuilder();
    DynamicTypeBuilder_ptr basicStruct_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();

    // Add members to the struct.
    int idx = 0;
    basicStruct_builder->AddMember(idx++, "my_uint32", uint32_builder.get());
    basicStruct_builder->AddMember(idx++, "my_bool", bool_builder.get());
    basicStruct_builder->AddMember(idx++, "my_octet", octet_builder.get());
    basicStruct_builder->AddMember(idx++, "my_int16", int16_builder.get());
    basicStruct_builder->AddMember(idx++, "my_int32", int32_builder.get());
    basicStruct_builder->AddMember(idx++, "my_int64", int64_builder.get());
    basicStruct_builder->AddMember(idx++, "my_uint16", uint16_builder.get());
    basicStruct_builder->AddMember(idx++, "my_uint64", uint64_builder.get());
    basicStruct_builder->AddMember(idx++, "my_float32", float_builder.get());
    basicStruct_builder->AddMember(idx++, "my_float64", double_builder.get());
    basicStruct_builder->AddMember(idx++, "my_float128", ldouble_builder.get());
    basicStruct_builder->AddMember(idx++, "my_char", char_builder.get());
    basicStruct_builder->AddMember(idx++, "my_wchar", wchar_builder.get());
    basicStruct_builder->AddMember(idx++, "my_string", string_builder.get());
    basicStruct_builder->AddMember(idx++, "my_wstring", wstring_builder.get());
    basicStruct_builder->SetName("Dyn_BenchMarkSmall");

    return basicStruct_builder->Build();
}

DynamicType_ptr DynamicTypesHelper::GetComplexStructType()
{
    // Members (auxiliar types are tab)
    DynamicTypeBuilder_ptr octet_builder = DynamicTypeBuilderFactory::GetInstance()->CreateByteBuilder();
    DynamicTypeBuilder_ptr my_sequence_octet_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateSequenceBuilder(octet_builder.get(), 55);
    DynamicTypeBuilder_ptr my_sequence_struct_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateSequenceBuilder(GetBasicStructType());
    DynamicTypeBuilder_ptr char_builder = DynamicTypeBuilderFactory::GetInstance()->CreateChar8Builder();
    DynamicTypeBuilder_ptr byte_builder = DynamicTypeBuilderFactory::GetInstance()->CreateByteBuilder();
    DynamicTypeBuilder_ptr my_array_octet_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateArrayBuilder(byte_builder.get(), { 500, 5, 4 });
    // MyOctetArray500 is already created
    // We reuse the bounds... { 5 }
    DynamicTypeBuilder_ptr my_array_struct_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateArrayBuilder(GetBasicStructType(), { 5 });
    DynamicTypeBuilder_ptr int16_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt16Builder();
    DynamicTypeBuilder_ptr my_map_octet_short_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateMapBuilder(octet_builder.get(), int16_builder.get());
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
    DynamicTypeBuilder_ptr my_map_long_struct_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateMapBuilder(int32_builder.get()->Build(), GetBasicStructType());
    DynamicTypeBuilder_ptr seqOctet_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateSequenceBuilder(octet_builder.get());
    DynamicTypeBuilder_ptr seqSeqOctet_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateSequenceBuilder(seqOctet_builder.get());
    DynamicTypeBuilder_ptr my_map_long_seq_octet_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateMapBuilder(int32_builder.get(), seqSeqOctet_builder.get());
    DynamicTypeBuilder_ptr my_map_long_octet_array_500_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateMapBuilder(int32_builder.get()->Build(), GetMyOctetArray500Type());
    DynamicTypeBuilder_ptr map_octet_bsalias5_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateMapBuilder(octet_builder.get()->Build(), GetBSAlias5Type());
    DynamicTypeBuilder_ptr my_map_long_lol_type_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateMapBuilder(int32_builder.get(), map_octet_bsalias5_builder.get());
    DynamicTypeBuilder_ptr my_small_string_8_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateStringBuilder(128);
    DynamicTypeBuilder_ptr my_small_string_16_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateWstringBuilder(64);
    DynamicTypeBuilder_ptr my_large_string_8_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateStringBuilder(500);
    DynamicTypeBuilder_ptr my_large_string_16_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateWstringBuilder(1024);
    DynamicTypeBuilder_ptr string75_8_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateStringBuilder(75);
    DynamicTypeBuilder_ptr my_array_string_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateArrayBuilder(string75_8_builder.get(), { 5, 5 });

    // MA3 is already defined.
    // { 5 } being reused
    DynamicTypeBuilder_ptr my_array_arrays_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateArrayBuilder(GetMyMiniArrayType(), { 5 });
    DynamicTypeBuilder_ptr my_sequences_array_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateArrayBuilder(GetMySequenceLongType(), { 23 });
    DynamicTypeBuilder_ptr complexStruct_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();

    // Add members to the struct.
    int idx = 0;
    complexStruct_builder->AddMember(idx++, "uint", DynamicTypeBuilderFactory::GetInstance()->CreateUint32Type());
    complexStruct_builder->AddMember(idx++, "my_octet", octet_builder.get());
    complexStruct_builder->AddMember(idx++, "my_basic_struct", GetBasicStructType());
    complexStruct_builder->AddMember(idx++, "my_alias_enum", GetMyAliasEnumType());
    complexStruct_builder->AddMember(idx++, "my_enum", GetMyEnumType());
    complexStruct_builder->AddMember(idx++, "my_sequence_octet", my_sequence_octet_builder.get());
    complexStruct_builder->AddMember(idx++, "my_sequence_struct", my_sequence_struct_builder.get());
    complexStruct_builder->AddMember(idx++, "my_array_octet", my_array_octet_builder.get());
    complexStruct_builder->AddMember(idx++, "my_octet_array_500", GetMyOctetArray500Type());
    complexStruct_builder->AddMember(idx++, "my_array_struct", my_array_struct_builder.get());
    complexStruct_builder->AddMember(idx++, "my_map_octet_short", my_map_octet_short_builder.get());
    complexStruct_builder->AddMember(idx++, "my_map_long_struct", my_map_long_struct_builder.get());
    complexStruct_builder->AddMember(idx++, "my_map_long_seq_octet", my_map_long_seq_octet_builder.get());
    complexStruct_builder->AddMember(idx++, "my_map_long_octet_array_500", my_map_long_octet_array_500_builder.get());
    complexStruct_builder->AddMember(idx++, "my_map_long_lol_type", my_map_long_lol_type_builder.get());
    complexStruct_builder->AddMember(idx++, "my_small_string_8", my_small_string_8_builder.get());
    complexStruct_builder->AddMember(idx++, "my_small_string_16", my_small_string_16_builder.get());
    complexStruct_builder->AddMember(idx++, "my_large_string_8", my_large_string_8_builder.get());
    complexStruct_builder->AddMember(idx++, "my_large_string_16", my_large_string_16_builder.get());
    complexStruct_builder->AddMember(idx++, "my_array_string", my_array_string_builder.get());
    complexStruct_builder->AddMember(idx++, "multi_alias_array_42", GetMA3Type());
    complexStruct_builder->AddMember(idx++, "my_array_arrays", my_array_arrays_builder.get());
    complexStruct_builder->AddMember(idx++, "my_sequences_array", my_sequences_array_builder.get());
    complexStruct_builder->SetName("Dyn_BenchMarkMedium");
    return complexStruct_builder->Build();
}

DynamicType_ptr DynamicTypesHelper::GetMyOctetArray500Type()
{
    DynamicTypeBuilder_ptr octet_builder = DynamicTypeBuilderFactory::GetInstance()->CreateByteBuilder();
    DynamicTypeBuilder_ptr myOctetArray500_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateArrayBuilder(octet_builder.get(), { 500 });
    myOctetArray500_builder->SetName("MyOctetArray500");
    return myOctetArray500_builder->Build();
}

DynamicType_ptr DynamicTypesHelper::GetBSAlias5Type()
{
    DynamicTypeBuilder_ptr bSAlias5_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateArrayBuilder(GetBasicStructType(), { 5 });
    bSAlias5_builder->SetName("BSAlias5");
    return bSAlias5_builder->Build();
}

DynamicType_ptr DynamicTypesHelper::GetMyMiniArrayType()
{
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
    DynamicTypeBuilder_ptr myMiniArray_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateArrayBuilder(int32_builder.get(), { 2 });
    myMiniArray_builder->SetName("MyMiniArray");
    return myMiniArray_builder->Build();
}

DynamicType_ptr DynamicTypesHelper::GetMySequenceLongType()
{
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
    DynamicTypeBuilder_ptr seqLong_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateSequenceBuilder(int32_builder.get());
    DynamicTypeBuilder_ptr mySequenceLong_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateAliasBuilder(seqLong_builder.get(), "MySequenceLong");
    return mySequenceLong_builder->Build();
}

DynamicType_ptr DynamicTypesHelper::GetMyEnumType()
{
    DynamicTypeBuilder_ptr myEnum_builder = DynamicTypeBuilderFactory::GetInstance()->CreateEnumBuilder();
    myEnum_builder->SetName("MyEnum");
    myEnum_builder->AddEmptyMember(0, "A");
    myEnum_builder->AddEmptyMember(1, "B");
    myEnum_builder->AddEmptyMember(2, "C");
    return myEnum_builder->Build();
}

DynamicType_ptr DynamicTypesHelper::GetMyAliasEnumType()
{
    DynamicTypeBuilder_ptr myAliasEnum_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateAliasBuilder(GetMyEnumType(), "MyAliasEnum");
    return myAliasEnum_builder->Build();
}

DynamicType_ptr DynamicTypesHelper::GetMA3Type()
{
    DynamicTypeBuilder_ptr mA3_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateArrayBuilder(GetMyAliasEnum3Type(), { 42 });
    mA3_builder->SetName("MA3");
    return mA3_builder->Build();
}

DynamicType_ptr DynamicTypesHelper::GetMyAliasEnum3Type()
{
    DynamicTypeBuilder_ptr myAliasEnum3_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateAliasBuilder(GetMyAliasEnum2Type(), "MyAliasEnum3");
    return myAliasEnum3_builder->Build();
}

DynamicType_ptr DynamicTypesHelper::GetMyAliasEnum2Type()
{
    DynamicTypeBuilder_ptr myAliasEnum2_builder =
        DynamicTypeBuilderFactory::GetInstance()->CreateAliasBuilder(GetMyAliasEnumType(), "MyAliasEnum2");
    return myAliasEnum2_builder->Build();
}

DynamicType_ptr DynamicTypesHelper::GetUnionSwitchType()
{
    DynamicTypeBuilder_ptr myUnion_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUnionBuilder(GetMyEnumType());
    myUnion_builder->AddMember(0, "basic", GetBasicStructType(), "A", { 0 }, false);
    myUnion_builder->AddMember(1, "complex", GetComplexStructType(), "B", { 1, 2 }, false);
    myUnion_builder->SetName("MyUnion");
    return myUnion_builder->Build();
}

DynamicType_ptr DynamicTypesHelper::GetUnion2SwitchType()
{
    DynamicTypeBuilder_ptr octet_builder = DynamicTypeBuilderFactory::GetInstance()->CreateByteBuilder();
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Builder();
    DynamicTypeBuilder_ptr string_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStringBuilder();
    DynamicTypeBuilder_ptr myUnion2_builder = DynamicTypeBuilderFactory::GetInstance()->CreateUnionBuilder(octet_builder.get());
    myUnion2_builder->AddMember(0, "uno", int32_builder.get(), "0", { 0 }, false);
    myUnion2_builder->AddMember(1, "imString", string_builder.get(), "1", { 1 }, false);
    myUnion2_builder->AddMember(2, "tres", int32_builder.get(), "2", { 2 }, false);
    myUnion2_builder->SetName("MyUnion2");
    return myUnion2_builder->Build();
}