// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
    
#include <gtest/gtest.h>
#include <dds/core/xtypes/xtypes.hpp>
#include <iostream>

#include <cmath>
#include <bitset>

using namespace std; 
using namespace dds::core::xtypes;


#define UINT8 250
#define INT16 -32760
#define UINT16 65530 
#define INT32 -2147483640
#define UINT32 4294967290
#define INT64 -9223372036854775800
#define UINT64 18446744073709551610ULL
#define FLOAT 3.1415927410125732421875f
#define DOUBLE 3.1415926535897931159979631875
#define LDOUBLE 3.14159265358979321159979631875
#define CHAR 'f'
#define WCHAR 34590

#define INNER_STRING_VALUE "lay_down_and_cry" 
#define INNER_SEQUENCE_STRING "another_prick_in_the_wall"
#define SECOND_INNER_STRING "paint_it_black"

#define STRUCTS_SIZE 1E1
#define CHECKS_NUMBER 1E2

/********************************
 *        DynamicType Tests        *
 ********************************/

TEST (StructType , verify_constructor_and_size) 
{ 
    StructType st("struct_name"); 
    EXPECT_EQ("struct_name", st.name());
    st.add_member(Member("int", primitive_type<int>()));
    EXPECT_EQ(4, st.memory_size());
}

TEST (StructType, build_one_of_each_primitive_type) 
{ 
    StructType st("struct_name"); 
    EXPECT_EQ("struct_name", st.name());
    EXPECT_EQ(TypeKind::STRUCTURE_TYPE, st.kind());
    size_t mem_size = 0;
    st.add_member(Member("bool", primitive_type<bool>()));
    mem_size+=sizeof(bool);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("uint8_t", primitive_type<uint8_t>()));
    mem_size+=sizeof(uint8_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("int16_t", primitive_type<int16_t>()));
    mem_size+=sizeof(int16_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("uint16_t", primitive_type<uint16_t>()));
    mem_size+=sizeof(uint16_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("int32_t", primitive_type<int32_t>()));
    mem_size+=sizeof(int32_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("uint32_t", primitive_type<uint32_t>()));
    mem_size+=sizeof(uint32_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("int64_t", primitive_type<int64_t>()));
    mem_size+=sizeof(int64_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("uint64_t", primitive_type<uint64_t>()));
    mem_size+=sizeof(uint64_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("float", primitive_type<float>()));
    mem_size+=sizeof(float);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("double", primitive_type<double>()));
    mem_size+=sizeof(double);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("long_double", primitive_type<long double>()));
    mem_size+=sizeof(long double);
    EXPECT_EQ(mem_size, st.memory_size());
}

TEST (StructType, cascade_add_member_and_copy)
{
    StructType st("struct_name");
    st.add_member( 
        Member("bool", primitive_type<bool>())).add_member(
        Member("uint8_t", primitive_type<uint8_t>())).add_member(
        Member("int16_t", primitive_type<int16_t>())).add_member(
        Member("uint16_t", primitive_type<uint16_t>())).add_member(
        Member("int32_t", primitive_type<int32_t>())).add_member(
        Member("uint32_t", primitive_type<uint32_t>())).add_member(
        Member("int64_t", primitive_type<int64_t>())).add_member(
        Member("uint64_t", primitive_type<uint64_t>())).add_member(
        Member("float", primitive_type<float>())).add_member(
        Member("double", primitive_type<double>())).add_member(
        Member("long_double", primitive_type<long double>())); 
    
    size_t mem_size = 0;
    mem_size+=sizeof(bool);
    mem_size+=sizeof(uint8_t);
    mem_size+=sizeof(int16_t);
    mem_size+=sizeof(uint16_t);
    mem_size+=sizeof(int32_t);
    mem_size+=sizeof(uint32_t);
    mem_size+=sizeof(int64_t);
    mem_size+=sizeof(uint64_t);
    mem_size+=sizeof(float);
    mem_size+=sizeof(double);
    mem_size+=sizeof(long double);
    
    StructType cp = st;
    EXPECT_EQ("struct_name", cp.name());
    EXPECT_EQ(mem_size, cp.memory_size());
}

TEST (StructType, self_assign)
{
    StructType st("struct_name");
    st.add_member(
        Member("long_double", primitive_type<long double>())).add_member(
        Member("uint64_t", primitive_type<uint64_t>())).add_member(
        Member("uint8_t", primitive_type<uint8_t>()));    
    
    StructType in("struct_name");
    in.add_member(
        Member("long_double", primitive_type<long double>())).add_member(
        Member("uint64_t", primitive_type<uint64_t>())).add_member(
        Member("uint8_t", primitive_type<uint32_t>()));    
    
    st.add_member(Member("in_member_name", in));
    st.add_member(Member("selfassign_member_name", st));
    EXPECT_EQ(TypeKind::STRUCTURE_TYPE, st.member("in_member_name").type().kind());
    EXPECT_EQ(TypeKind::STRUCTURE_TYPE, st.member("selfassign_member_name").type().kind());
    EXPECT_EQ(TypeKind::FLOAT_128_TYPE, 
        static_cast<const StructType&>(st.member("selfassign_member_name").type()).member("long_double").type().kind());
    EXPECT_EQ(TypeKind::UINT_64_TYPE, 
        static_cast<const StructType&>(st.member("selfassign_member_name").type()).member("uint64_t").type().kind());
    EXPECT_EQ(TypeKind::UINT_8_TYPE, 
        static_cast<const StructType&>(st.member("selfassign_member_name").type()).member("uint8_t").type().kind() );
    size_t mem_size_in = 0;
    mem_size_in+=sizeof(long double);
    mem_size_in+=sizeof(uint64_t);
    mem_size_in+=sizeof(uint32_t);
    EXPECT_EQ(mem_size_in, st.member("in_member_name").type().memory_size());
    
    mem_size_in+=sizeof(long double);
    mem_size_in+=sizeof(uint64_t);
    mem_size_in+=sizeof(uint8_t);
    EXPECT_EQ(mem_size_in, st.member("selfassign_member_name").type().memory_size());
}

TEST (StructType, type_verify_test)
{
    StructType st("struct_name");
    st.add_member(
        Member("bool", primitive_type<bool>())).add_member(
        Member("uint8_t", primitive_type<uint8_t>())).add_member(
        Member("int16_t",  primitive_type<int16_t>())).add_member(
        Member("uint16_t",  primitive_type<uint16_t>())).add_member(
        Member("int32_t",  primitive_type<int32_t>())).add_member(
        Member("uint32_t", primitive_type<uint32_t>())).add_member(
        Member("int64_t", primitive_type<int64_t>())).add_member(
        Member("uint64_t", primitive_type<uint64_t>())).add_member(
        Member("float",  primitive_type<float>())).add_member(
        Member("double", primitive_type<double>())).add_member(
        Member("long double", primitive_type<long double>())).add_member(
        Member("char", primitive_type<char>())).add_member(
        Member("char16_t", primitive_type<wchar_t>()));

     
    EXPECT_EQ(TypeKind::BOOLEAN_TYPE, st.member("bool").type().kind());
    EXPECT_EQ(TypeKind::UINT_8_TYPE, st.member("uint8_t").type().kind());
    EXPECT_EQ(TypeKind::INT_16_TYPE , st.member("int16_t").type().kind());
    EXPECT_EQ(TypeKind::UINT_16_TYPE, st.member("uint16_t").type().kind());
    EXPECT_EQ(TypeKind::INT_32_TYPE, st.member("int32_t").type().kind());
    EXPECT_EQ(TypeKind::UINT_32_TYPE, st.member("uint32_t").type().kind());
    EXPECT_EQ(TypeKind::INT_64_TYPE, st.member("int64_t").type().kind());
    EXPECT_EQ(TypeKind::UINT_64_TYPE, st.member("uint64_t").type().kind());
    EXPECT_EQ(TypeKind::FLOAT_32_TYPE, st.member("float").type().kind());
    EXPECT_EQ(TypeKind::FLOAT_64_TYPE, st.member("double").type().kind());
    EXPECT_EQ(TypeKind::FLOAT_128_TYPE, st.member("long double").type().kind());
    EXPECT_EQ(TypeKind::CHAR_8_TYPE, st.member("char").type().kind());
    EXPECT_EQ(TypeKind::CHAR_16_TYPE, st.member("char16_t").type().kind());
    EXPECT_EQ(TypeKind::STRUCTURE_TYPE, st.kind());

    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("bool").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("uint8_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("int16_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("uint16_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("int32_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("uint32_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("int64_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("uint64_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("float").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("double").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("long double").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("char").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("char16_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::STRUCTURE_TYPE) & uint32_t(st.kind()));

}

TEST (DynamicData, primitive_types)
{
    StructType st("struct_name");
    st.add_member(
    Member("bool", primitive_type<bool>())).add_member(
    Member("uint8_t", primitive_type<uint8_t>())).add_member(
    Member("int16_t",  primitive_type<int16_t>())).add_member(
    Member("uint16_t",  primitive_type<uint16_t>())).add_member(
    Member("int32_t",  primitive_type<int32_t>())).add_member(
    Member("uint32_t", primitive_type<uint32_t>())).add_member(
    Member("int64_t", primitive_type<int64_t>())).add_member(
    Member("uint64_t", primitive_type<uint64_t>())).add_member(
    Member("float",  primitive_type<float>())).add_member(
    Member("double", primitive_type<double>())).add_member(
    Member("long double", primitive_type<long double>())).add_member(
    Member("char", primitive_type<char>())).add_member(
    Member("char16_t", primitive_type<wchar_t>()));

    EXPECT_EQ(TypeKind::BOOLEAN_TYPE, st.member("bool").type().kind());
    EXPECT_EQ(TypeKind::UINT_8_TYPE, st.member("uint8_t").type().kind());
    EXPECT_EQ(TypeKind::INT_16_TYPE , st.member("int16_t").type().kind());
    EXPECT_EQ(TypeKind::UINT_16_TYPE, st.member("uint16_t").type().kind());
    EXPECT_EQ(TypeKind::INT_32_TYPE, st.member("int32_t").type().kind());
    EXPECT_EQ(TypeKind::UINT_32_TYPE, st.member("uint32_t").type().kind());
    EXPECT_EQ(TypeKind::INT_64_TYPE, st.member("int64_t").type().kind());
    EXPECT_EQ(TypeKind::UINT_64_TYPE, st.member("uint64_t").type().kind());
    EXPECT_EQ(TypeKind::FLOAT_32_TYPE, st.member("float").type().kind());
    EXPECT_EQ(TypeKind::FLOAT_64_TYPE, st.member("double").type().kind());
    EXPECT_EQ(TypeKind::FLOAT_128_TYPE, st.member("long double").type().kind());
    EXPECT_EQ(TypeKind::CHAR_8_TYPE, st.member("char").type().kind());
    EXPECT_EQ(TypeKind::CHAR_16_TYPE, st.member("char16_t").type().kind());
    EXPECT_EQ(TypeKind::STRUCTURE_TYPE, st.kind());

    DynamicData d(st);


    d["bool"].value<bool>(true);
    d["uint8_t"].value<uint8_t>(UINT8);
    d["int16_t"].value<int16_t>(INT16);
    d["uint16_t"].value<uint16_t>(UINT16); 
    d["int32_t"].value<int32_t>(INT32); 
    d["uint32_t"].value<uint32_t>(UINT32);
    d["int64_t"].value<int64_t>(INT64);
    d["uint64_t"].value<uint64_t>(UINT64);
    d["float"].value<float>(FLOAT);   
    d["double"].value<double>(DOUBLE);
    d["long double"].value<long double>(LDOUBLE);
    d["char"].value<char>(CHAR);
    d["char16_t"].value<wchar_t>(WCHAR);
    
    EXPECT_EQ(true, d["bool"].value<bool>()); 
    EXPECT_EQ(UINT8, d["uint8_t"].value<uint8_t>());
    EXPECT_EQ(INT16, d["int16_t"].value<int16_t>());
    EXPECT_EQ(UINT16,d["uint16_t"].value<uint16_t>()); 
    EXPECT_EQ(INT32, d["int32_t"].value<int32_t>()); 
    EXPECT_EQ(UINT32, d["uint32_t"].value<uint32_t>());
    EXPECT_EQ(INT64, d["int64_t"].value<int64_t>());
    EXPECT_EQ(UINT64,d["uint64_t"].value<uint64_t>());
    EXPECT_EQ( float(FLOAT) , d["float"].value<float>());   
    EXPECT_EQ( double(DOUBLE) , d["double"].value<double>());
    long double ld = LDOUBLE;
    EXPECT_EQ( ld , d["long double"].value<long double>());
    EXPECT_EQ( CHAR , d["char"].value<char>());
    EXPECT_EQ( WCHAR , d["char16_t"].value<wchar_t>());

}

TEST (DynamicData, long_random_sequence)
{
    SequenceType st(primitive_type<double>());

    DynamicData d(st);
    srand48(time(0));
    
    for(int i = 0; i < 65000; ++i)
    {
        double r = lrand48()/double(RAND_MAX);
        d.push(r);
        EXPECT_EQ(r, d[i].value<double>());
    }
}

TEST (DynamicData, sequence)
{
    StructType s("struct");
    s.add_member(Member("sequence_1", SequenceType(primitive_type<char>())));
    s.add_member(Member("sequence_2", SequenceType(primitive_type<char>())));
    {
        DynamicData d(s);
        d["sequence_1"].push('a');
        d["sequence_1"].push('b');
        d["sequence_1"].push('c');
        d["sequence_1"].push('d');
        d["sequence_1"].push('e');
        d["sequence_2"].push(d["sequence_1"]);
    }                
}

DynamicData cdd(StructType &st)
{     
    StringType stri_t;
    SequenceType sequ_t(stri_t);
    StructType stru_t("just_a_struct");
    stru_t.add_member(
    Member("sequence", sequ_t));
                
    StructType sec_stru("another_struct");
    sec_stru.add_member(
        Member("int", primitive_type<uint32_t>()));
                
    st.add_member("struct", stru_t).add_member("other_struct", sec_stru);
    
    DynamicData dd(st);
    for(int i = 0; i < 1E1; ++i)
        dd["struct"]["sequence"].push<string>("checking");
    return dd;
}

DynamicData ret_dyn_data(StructType &st, int i )
{
    switch(i){
        case 1:
            st.add_member(
                Member("int", primitive_type<int32_t>()));
            break;
        case 2:
            st.add_member(
                Member("array", ArrayType(primitive_type<uint32_t>(), 10)));
            break;
        default:
            break;
    }
    DynamicData dt(st);
    switch(i){
        case 1:
            dt["int"].value<int32_t>(32);
            break;
        case 2:
            for(size_t j = 0; j < 10; ++j)
                dt["array"][i].value<uint32_t>(32);
            break;
        default:
            break;
    }
    return dt;
}

TEST (DynamicData, cp)
{
    StructType ist("int_st");
    DynamicData idt = ret_dyn_data(ist, 1);

    StructType ast("arr_st");
    DynamicData adt = ret_dyn_data(ast, 2);
}


DynamicData create_dynamic_data(long double pi, StructType& the_struct, StructType& inner_struct, StructType& second_inner_struct)
{
//    StructType inner_struct("inner_struct");
//    StructType second_inner_struct("second_inner_struct");
    second_inner_struct.add_member(
        Member("second_inner_string", StringType())).add_member(
        Member("second_inner_uint32_t", primitive_type<uint32_t>())).add_member(
        Member("second_inner_array", ArrayType(primitive_type<uint8_t>(), 10)));
    StringType st;     
    inner_struct.add_member(
        Member("inner_string", st)).add_member(
        Member("inner_float", primitive_type<float>())).add_member(
        Member("inner_sequence_string", SequenceType(st))).add_member(
        Member("inner_sequence_struct", SequenceType(second_inner_struct)));
                
    the_struct.add_member( 
        Member("bool", primitive_type<bool>())).add_member(
        Member("uint8_t", primitive_type<uint8_t>())).add_member(
        Member("int16_t", primitive_type<int16_t>())).add_member(
        Member("uint16_t", primitive_type<uint16_t>())).add_member(
        Member("int32_t", primitive_type<int32_t>())).add_member(
        Member("uint32_t", primitive_type<uint32_t>())).add_member(
        Member("int64_t", primitive_type<int64_t>())).add_member(
        Member("uint64_t", primitive_type<uint64_t>())).add_member(
        Member("float", primitive_type<float>())).add_member(
        Member("double", primitive_type<double>())).add_member(
        Member("long_double", primitive_type<long double>())).add_member(
        Member("array", ArrayType(ArrayType(primitive_type<long double>(), 10), 10))).add_member(
        Member("sequence", SequenceType(inner_struct)));
                
    DynamicData the_data(the_struct);
    the_data["bool"].value(true);
    the_data["uint8_t"].value<uint8_t>(UINT8);
    the_data["int16_t"].value<int16_t>(INT16);
    the_data["uint16_t"].value<uint16_t>(UINT16);
    the_data["int32_t"].value<int32_t>(INT32);
    the_data["uint32_t"].value<uint32_t>(UINT32);
    the_data["int64_t"].value<int64_t>(INT64);
    the_data["uint64_t"].value<uint64_t>(UINT64);
    the_data["float"].value<float>(FLOAT);
    the_data["double"].value<double>(DOUBLE);
                
    the_data["long_double"].value<>(pi);

    for(int i = 0; i < STRUCTS_SIZE; ++i) // creating "sequence"
    {
        DynamicData tmp_data(inner_struct);
        tmp_data["inner_string"].string(INNER_STRING_VALUE);
        tmp_data["inner_float"].value<float>(FLOAT);
        for (int j = 0; j < STRUCTS_SIZE; ++j) // creating "sequence.inner_sequence_string"
        {
            tmp_data["inner_sequence_string"].push<string>(INNER_SEQUENCE_STRING);
        }
                                
        for (int j = 0; j < STRUCTS_SIZE; ++j) // creating "sequence.inner_sequence_struct"
        {
            DynamicData tmp_inner_data(second_inner_struct);
            tmp_inner_data["second_inner_string"].string(SECOND_INNER_STRING);
            tmp_inner_data["second_inner_uint32_t"].value<uint32_t>(UINT32);
            for(int k = 0; k < STRUCTS_SIZE; ++k) //creating "sequence.inner_sequence_struct.second_inner_array"
            {
                tmp_inner_data["second_inner_array"][k].value<uint8_t>(UINT8);
            }
            tmp_data["inner_sequence_struct"].push(tmp_inner_data);
        }
        for(int j = 0; j < STRUCTS_SIZE; ++j)
        {
            for(int k = 0; k < STRUCTS_SIZE; ++k)
            {
                the_data["array"][j][k].value<long double>(LDOUBLE);
            }
        }
        the_data["sequence"].push(tmp_data);
    }

    return the_data;
}

TEST (DynamicData, cascade_construction)
{
    long double pi = LDOUBLE;
    StructType the_struct("the_struct");
    StructType inner_struct("inner_struct");
    StructType second_inner_struct("second_inner_struct");
    
    DynamicData the_data = create_dynamic_data(pi, the_struct, inner_struct, second_inner_struct);

    EXPECT_EQ(UINT32, the_data["uint32_t"].value<uint32_t>());
    EXPECT_EQ(INT32, the_data["int32_t"].value<int32_t>()); 
    EXPECT_EQ(UINT16, the_data["uint16_t"].value<uint16_t>());
    EXPECT_EQ(INT16, the_data["int16_t"].value<int16_t>()); 
    EXPECT_EQ(true, the_data["bool"].value<bool>());
    EXPECT_EQ(UINT8, the_data["uint8_t"].value<uint8_t>());
    EXPECT_EQ(INT64, the_data["int64_t"].value<int64_t>()); 
    EXPECT_EQ(UINT64, the_data["uint64_t"].value<uint64_t>());
    EXPECT_EQ(FLOAT, the_data["float"].value<float>()); 
    EXPECT_EQ(DOUBLE, the_data["double"].value<double>()); 
    EXPECT_EQ(pi, the_data["long_double"].value<long double>());

    srand48(time(0));

    for (int i = 0; i < CHECKS_NUMBER; ++i)
    {
        size_t idx_4 = lrand48()%int(STRUCTS_SIZE);
        EXPECT_EQ(INNER_STRING_VALUE, the_data["sequence"][idx_4]["inner_string"].string());
        EXPECT_EQ(FLOAT, the_data["sequence"][idx_4]["inner_float"].value<float>());
        size_t idx_3 = lrand48()%int(STRUCTS_SIZE);
        EXPECT_EQ(INNER_SEQUENCE_STRING, the_data["sequence"][idx_4]["inner_sequence_string"][idx_3].string());
        size_t idx_2 = lrand48()%int(STRUCTS_SIZE);
        EXPECT_EQ(SECOND_INNER_STRING, the_data["sequence"][idx_4]["inner_sequence_struct"][idx_2]["second_inner_string"].string());
        EXPECT_EQ(UINT32, the_data["sequence"][idx_4]["inner_sequence_struct"][idx_2]["second_inner_uint32_t"].value<uint32_t>());
                
        size_t arr_idx_3 = lrand48()%int(STRUCTS_SIZE);
        size_t arr_idx_2 = lrand48()%int(STRUCTS_SIZE);
        size_t arr_idx_1 = lrand48()%int(STRUCTS_SIZE);
                                
        long double check_over = LDOUBLE;
        EXPECT_EQ(check_over, the_data["array"][arr_idx_3][arr_idx_2].value<long double>());
        EXPECT_EQ(UINT8, the_data["sequence"][idx_4]["inner_sequence_struct"][idx_2]["second_inner_array"][arr_idx_1].value<uint8_t>());
    }    
}

TEST (DynamicData, curious_interactions)
{
    StructType st("st");
    st.add_member(
    Member("seq", SequenceType(StringType())));

    DynamicData the_data(st);

    StringType str;
    DynamicData dstr(str);
    dstr.string("all_this_stuff");
                
    the_data["seq"].push(dstr);
    EXPECT_EQ("all_this_stuff", the_data["seq"][0].string());
//  EXPECT_EQ("all_this_stuff", the_data["seq"].string());<<=this call fails. It is kept here as a memorandum
}

TEST (DynamicType, testing_is_compatible_string_no_bound)
{
    StringType s;
    StringType r;
    
    EXPECT_EQ(TypeConsistency::EQUALS, r.is_compatible(s) );
    EXPECT_EQ(TypeConsistency::EQUALS, s.is_compatible(r) );
}

TEST (DynamicType, testing_is_compatible_string_same_bound)
{
    srand48(time(0));
    size_t b = lrand48()%1000;
    StringType s(b);
    StringType r(b);
    
    EXPECT_EQ(TypeConsistency::EQUALS, r.is_compatible(s) );
    EXPECT_EQ(TypeConsistency::EQUALS, s.is_compatible(r) );
}


TEST (DynamicType, testing_is_compatible_string_different_bound)
{
    StringType s(15);
    StringType r(30);
    
    EXPECT_NE(0, uint32_t(TypeConsistency::IGNORE_STRING_BOUNDS) & uint32_t(s.is_compatible(r)) );
    EXPECT_NE(0, uint32_t(TypeConsistency::IGNORE_STRING_BOUNDS) & uint32_t(r.is_compatible(s)) );
}


TEST (DynamicType, testing_is_compatible_structure_of_string)
{
    StringType st;
    StructType r("check");
    r.add_member(Member("string", st));
    StructType s("other_check");
    s.add_member(Member("string", st));

    EXPECT_EQ(TypeConsistency::EQUALS , r.is_compatible(s) );
    EXPECT_EQ(TypeConsistency::EQUALS , s.is_compatible(r) );
}

TEST (DynamicType, testing_is_compatible_structure_of_sequence_no_bound)
{
    SequenceType s(primitive_type<uint32_t>());
    StructType the_str("check");
    the_str.add_member(Member("int", s));
    StructType other_str("other_check");
    other_str.add_member(Member("int", s));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_sequence_different_bound)
{
    SequenceType s(primitive_type<uint32_t>(),15);
    SequenceType r(primitive_type<uint32_t>(),19);
    StructType the_str("check");
    the_str.add_member(Member("int", s));
    StructType other_str("other_check");
    other_str.add_member(Member("int", r));
    EXPECT_EQ(TypeConsistency::IGNORE_SEQUENCE_BOUNDS, the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::IGNORE_SEQUENCE_BOUNDS , other_str.is_compatible(the_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_sequence_same_bound)
{
    SequenceType s(primitive_type<uint32_t>(),15);
    StructType the_str("check");
    the_str.add_member(Member("int", s));
    StructType other_str("other_check");
    other_str.add_member(Member("int", s));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_primitive_type_int)
{
    StructType the_str("check");
    the_str.add_member(Member("int", primitive_type<uint32_t>()));
    StructType other_str("other_check");
    other_str.add_member(Member("int", primitive_type<uint32_t>()));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_primitive_type_float)
{
    StructType the_str("check");
    the_str.add_member(Member("int", primitive_type<long double>()));
    StructType other_str("other_check");
    other_str.add_member(Member("int", primitive_type<long double>()));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_primitive_type_char)
{
    StructType the_str("check");
    the_str.add_member(Member("int", primitive_type<wchar_t>()));
    StructType other_str("other_check");
    other_str.add_member(Member("int", primitive_type<wchar_t>()));
    StructType another_str("another_check");
    another_str.add_member(Member("int", primitive_type<char>()));
    
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, the_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, other_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, another_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, another_str.is_compatible(other_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_primitive_type_int32_t)
{
    StructType the_str("check");
    the_str.add_member(Member("int", primitive_type<uint32_t>()));
    StructType other_str("other_check");
    other_str.add_member(Member("int", primitive_type<uint32_t>()));
    StructType another_str("another_check");
    another_str.add_member(Member("int", primitive_type<int32_t>()));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN, the_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN, other_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN, another_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN, another_str.is_compatible(other_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_primitive_type_mixed_int)
{
    StructType the_str("check");
    the_str.add_member(Member("int", primitive_type<uint16_t>()));
    StructType other_str("other_check");
    other_str.add_member(Member("int", primitive_type<uint32_t>()));
    StructType another_str("another_check");
    another_str.add_member(Member("int", primitive_type<int64_t>()));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_TYPE_SIGN, the_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_TYPE_SIGN, other_str.is_compatible(another_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_array_same_bound)
{
    StructType the_str("check");
    ArrayType the_array(primitive_type<uint32_t>(), 10);
    the_str.add_member(Member("arr", the_array));
    StructType other_str("other_check");
    other_str.add_member(Member("arr", the_array));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_array_different_bound_and_type)
{
    StructType the_str("check");
    ArrayType the_array(primitive_type<uint32_t>(), 10);
    the_str.add_member(Member("arr", the_array));

    ArrayType other_array(primitive_type<uint32_t>(), 11);
    StructType other_str("other_check");
    other_str.add_member(Member("arr", other_array));

    ArrayType another_array(primitive_type<int32_t>(), 10);
    StructType another_str("other_check");
    another_str.add_member(Member("arr", another_array));

    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN, the_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS, other_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS | TypeConsistency::IGNORE_TYPE_SIGN, other_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN, another_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS | TypeConsistency::IGNORE_TYPE_SIGN, another_str.is_compatible(other_str));
}

template<typename A, typename B>
void singleCheck(void)
{
    DynamicData dd1(primitive_type<A>());
    DynamicData dd2(primitive_type<A>());
    dd1.value(A(15));
    dd2.value(A(15));
    EXPECT_EQ(dd1, dd2);
    dd2.value(A(16.3));
    EXPECT_NE(dd1, dd2);
}

TEST (DynamicData, testing_equality_check_primitive_type_uint8)
{
    singleCheck<uint8_t, uint8_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_uint16)
{
    singleCheck<uint16_t, uint16_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_int16)
{
    singleCheck<int16_t, int16_t>();
}
TEST (DynamicData, testing_equality_check_primitive_typeu_int32)
{
    singleCheck<uint32_t, uint32_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_int32)
{
    singleCheck<int32_t, int32_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_uint64)
{
    singleCheck<uint64_t, uint64_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_int64)
{
    singleCheck<int64_t, int64_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_float)
{
    singleCheck<float, float>();
}
TEST (DynamicData, testing_equality_check_primitive_type_double)
{
    singleCheck<double, double>();
}
TEST (DynamicData, testing_equality_check_primitive_type_char)
{
    singleCheck<char, char>();
}
TEST (DynamicData, testing_equality_check_primitive_type_wchar)
{
    singleCheck<wchar_t, wchar_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_double_longdouble)
{
    singleCheck<double, long double>();
}
TEST (DynamicData, testing_equality_check_primitive_type_int16_uint32)
{
    singleCheck<int16_t, uint32_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_double_uint8)
{
    singleCheck<double, uint8_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_int32_uint16)
{
    singleCheck<int32_t, uint16_t>();
}

TEST (DynamicData, testing_equality_check_string)
{
    SequenceType s(primitive_type<uint64_t>());
    DynamicData d1(s);
    DynamicData d2(s);
    d1.push<uint64_t>(3456); 
    d2.push<uint64_t>(3456); 
    EXPECT_EQ(true , d1 == d2);
    d2[0].value<uint64_t>(3457);
    EXPECT_NE(d1, d2);
    d2[0].value<uint64_t>(3456);
    d2.push<uint64_t>(435);
    EXPECT_NE(d1, d2);
}

TEST (DynamicData, test_equality_check_struct)
{
    StructType stru("the_struct");
    stru.add_member(Member("lld", primitive_type<long double>()));

    DynamicData d1(stru);
    d1["lld"].value<long double>(3.1415926);
    DynamicData d2(stru);
    d2["lld"].value<long double>(3.1415926);
    EXPECT_EQ(d1, d2);
    d2["lld"].value<long double>(3.1415925);
    EXPECT_NE(d1, d2);

    stru.add_member(Member("c", primitive_type<char>()));
    DynamicData d3(stru);
    d3["lld"].value<long double>(3.1415926);
    EXPECT_NE(d3, d1);
    d3["c"].value<char>(3.1415926); //just an :"out of the ordinady
    EXPECT_NE(d3, d1);

}

namespace{
    template<typename T>
    ostream& operator << (ostream& o, const vector<T>& v)
    {
        for(auto it = v.begin(); it != v.end(); ++it)
        {
            o << '(' << *it << ')';
        }
        return o;
    }
}

TEST (DynamicData, test_equality_complex_struct)
{
    ArrayType ar(primitive_type<uint32_t>(), 15);
    SequenceType se(primitive_type<uint32_t>(), 15);
    StringType str;
    SequenceType seq(str, 15);
    StructType st("the_type");
    st.add_member(Member("array", ar));
    st.add_member(Member("sequence", se));
    st.add_member(Member("seqstring", seq));
    st.add_member(Member("string", str));
    DynamicData d1(st);
    d1["string"].string("sono io che sono qui");

    for(int i=0; i < 15; ++i)
    {
        d1["array"][i].value<uint32_t>(42);
        d1["sequence"].push<uint32_t>(42);
        d1["seqstring"].push<string>("ci sono anche io");
    }
    vector<char> v = d1["string"].as_vector<char>();
    vector<string> vv = d1["seqstring"].as_vector<string>();
    vector<uint32_t> t = d1["array"].as_vector<uint32_t>();
    vector<uint32_t> s = d1["sequence"].as_vector<uint32_t>();
    

}

TEST (DynamicType , wstring_and_wstring_struct)
{
    WStringType wst;
    StringType st;
    DynamicData d(wst);
    DynamicData dd(st);

    d.wstring(L"sadfsfdasdf");
    dd.string("sadfsfdasdf");
   
    EXPECT_EQ( TypeConsistency::NONE, wst.is_compatible(st) );
    EXPECT_EQ( TypeConsistency::NONE, st.is_compatible(wst) );

    StructType struc1("the_struct");
    struc1.add_member(Member("theString", wst ));

    
    StructType struc2("the_struct");
    struc2.add_member(Member("theString", st ));

    EXPECT_EQ( TypeConsistency::NONE, struc1.is_compatible(struc2) );
    EXPECT_EQ( TypeConsistency::NONE, struc2.is_compatible(struc1) );
}

TEST (QoS, sequence)
{
    SequenceType s1(primitive_type<uint16_t>(), 10);
    SequenceType s2(primitive_type<uint16_t>(), 20);

    DynamicData d(s1);

    for(int i = 0; i < 10; ++i)
    {
        d.push(uint16_t());
    }
    d[9].value<uint16_t>(256);
    
    DynamicData dd(d, s2);

    EXPECT_EQ(10, dd.size());
    EXPECT_NE(dd , d);
    for (size_t i = 0; i < d.size(); ++i)
    {
        EXPECT_EQ( d[i].value<uint16_t>() , dd[i].value<uint16_t>() );
    }
}

TEST (QoS, other_sequence)
{
    SequenceType s1(primitive_type<uint16_t>(), 20);
    SequenceType s2(primitive_type<uint16_t>(), 10);

    DynamicData d(s1);

    for(size_t i = 0; i < 20; ++i)
    {
        d.push(uint16_t());
    }
    d[9].value<uint16_t>(256);
    
    DynamicData dd(d, s2);

    EXPECT_EQ(10, dd.size());
    EXPECT_NE(dd , d);
    for (size_t i = 0; i < dd.size(); ++i)
    {
        EXPECT_EQ( d[i].value<uint16_t>() , dd[i].value<uint16_t>() );
    }
}

TEST (QoS, string)
{
    StringType s(20);
    StringType t(10);
    
    EXPECT_EQ(s.is_compatible(t) , t.is_compatible(s));
    EXPECT_EQ(TypeConsistency::IGNORE_STRING_BOUNDS, t.is_compatible(s));
    
    DynamicData d(s);
    d.string("12345678901234567890");
    
    DynamicData e(d,t);
    EXPECT_EQ(10, e.size()); //still pending investigation
    EXPECT_NE( e , d);
    for (size_t i = 0; i < e.size(); ++i)
    {
        EXPECT_EQ( d[i].value<char>() , e[i].value<char>() );
    }
}

TEST (QoS, other_string)
{
    StringType s(10);
    StringType t(20);
    
    EXPECT_EQ(s.is_compatible(t) , t.is_compatible(s));
    EXPECT_EQ(TypeConsistency::IGNORE_STRING_BOUNDS, t.is_compatible(s));
    
    DynamicData d(s);
    d.string("1234567890");
    
    DynamicData e(d,t);
    EXPECT_EQ(10, e.size()); 
    EXPECT_NE( e , d);
    for (size_t i = 0; i < d.size(); ++i)
    {
        EXPECT_EQ( d[i].value<char>() , e[i].value<char>() );
    }
}

TEST (QoS, wstring)
{
    WStringType s(20);
    WStringType t(10);
    
    EXPECT_EQ(s.is_compatible(t) , t.is_compatible(s));
    EXPECT_EQ(TypeConsistency::IGNORE_STRING_BOUNDS, t.is_compatible(s));
    
    DynamicData d(s);
    d.wstring(L"12345678901234567890");
    
    DynamicData e(d,t);
    EXPECT_EQ(10, e.size()); //still pending investigation
    EXPECT_NE( e , d);
    for (size_t i = 0; i < e.size(); ++i)
    {
        EXPECT_EQ( d[i].value<wchar_t>() , e[i].value<wchar_t>() );
    }
}

TEST (QoS, other_wstring)
{
    WStringType s(10);
    WStringType t(20);
    
    EXPECT_EQ(s.is_compatible(t) , t.is_compatible(s));
    EXPECT_EQ(TypeConsistency::IGNORE_STRING_BOUNDS, t.is_compatible(s));
    
    DynamicData d(s);
    d.wstring(L"1234567890");
    
    DynamicData e(d,t);
    EXPECT_EQ(10, e.size()); 
    EXPECT_NE(e , d);
    for (size_t i = 0; i < d.size(); ++i)
    {
        EXPECT_EQ( d[i].value<wchar_t>() , e[i].value<wchar_t>() );
    }
}

TEST (QoS, array)
{
    ArrayType a(primitive_type<uint16_t>(), 10);
    ArrayType b(primitive_type<uint16_t>(), 20);
    
    DynamicData d(a);
    d[8].value(10);
    DynamicData e(d,b);
    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS, b.is_compatible(a));
    EXPECT_NE(e , d);
    for (size_t i = 0; i < d.size(); ++i)
    {
        EXPECT_EQ( d[i].value<uint16_t>() , e[i].value<uint16_t>() );
    }
}

TEST (QoS, other_array)
{
    ArrayType a(primitive_type<uint16_t>(), 20);
    ArrayType b(primitive_type<uint16_t>(), 10);
    
    DynamicData d(a);
    d[8].value(10);
    d[13].value(10);
    DynamicData e(d,b);
    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS, b.is_compatible(a));
    EXPECT_NE(e , d);
    for (size_t i = 0; i < e.size(); ++i)
    {
        EXPECT_EQ( d[i].value<uint16_t>() , e[i].value<uint16_t>() );
    }
}

TEST (QoS, Array_qos)
{

    ArrayType a_arr(primitive_type<uint16_t>(), 10);
    ArrayType b_arr(primitive_type<int32_t>(), 11);

    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS |
              TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_TYPE_SIGN , a_arr.is_compatible(b_arr));
    
}

TEST (QoS, mixed_types)
{
    StructType a("composition");
    StructType b("composition");
    StringType a_string(10);
    WStringType b_wstring(10);
    SequenceType a_seq(primitive_type<uint32_t>(), 10);
    SequenceType b_seq(primitive_type<int32_t>(), 11);
    ArrayType a_arr(primitive_type<uint16_t>(), 10);
    ArrayType b_arr(primitive_type<int32_t>(), 11);


    
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, a_string.is_compatible(b_wstring));

    EXPECT_EQ(TypeConsistency::IGNORE_SEQUENCE_BOUNDS |
              TypeConsistency::IGNORE_TYPE_SIGN, a_seq.is_compatible(b_seq));

    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS |
              TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_TYPE_SIGN , a_arr.is_compatible(b_arr));
    
    a.add_member(
            Member("a_string", a_string)).add_member(
            Member("a_seq", a_seq)).add_member(
            Member("a_arr", a_arr)).add_member(
            Member("a_primitive", primitive_type<wchar_t>()));

    b.add_member(
            Member("b_wstring", b_wstring)).add_member(
            Member("b_seq", b_seq)).add_member(
            Member("b_arr", b_arr));

    EXPECT_EQ(TypeConsistency::IGNORE_MEMBER_NAMES|
              TypeConsistency::IGNORE_TYPE_SIGN |
              TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_STRING_BOUNDS |
              TypeConsistency::IGNORE_SEQUENCE_BOUNDS |
              TypeConsistency::IGNORE_ARRAY_BOUNDS |
              TypeConsistency::IGNORE_MEMBERS , a.is_compatible(b));
    

}

template <typename A, typename B, typename C>
void dynamicDataChecker()
{

    StructType a("composition");
    StructType b("composition");
    
    StringType string(10);
    SequenceType a_seq(primitive_type<A>(), 10);
    SequenceType b_seq(primitive_type<B>(), 10);
    ArrayType a_arr(primitive_type<A>(), 10);
    ArrayType b_arr(primitive_type<B>(), 10);
    
    a.add_member(
            Member("string", string)).add_member(
            Member("seq", a_seq)).add_member(
            Member("arr", a_arr)).add_member(
            Member("flying_element", primitive_type<C>()));
    
    b.add_member(
            Member("string", string)).add_member(
            Member("seq", b_seq)).add_member(
            Member("arr", b_arr));

    EXPECT_EQ(TypeConsistency::IGNORE_MEMBERS , a.is_compatible(b));

    DynamicData d(a);
    DynamicData o(b);
    
    d["string"].string("the_string");
    o["string"].string("the_string");
    
    for (size_t i = 0; i < 10; i-=-1 )
    {
        d["seq"].push<A>(UINT32);
        d["arr"][i] = UINT32;
        o["seq"].push<B>(UINT32);
        o["arr"][i] = UINT32;
    }

    DynamicData dd(d,b);
   
    EXPECT_EQ(o,dd);
}

TEST (QoS, ignore_member)
{
    dynamicDataChecker<int32_t, uint16_t, char>();
    dynamicDataChecker<int64_t, uint16_t, char>();
    dynamicDataChecker<int64_t, uint8_t, char>();
    dynamicDataChecker<int16_t, uint8_t, char>();
    dynamicDataChecker<float, float, char>();
    dynamicDataChecker<double, float, char>();
    dynamicDataChecker<long double, double, char>();
}

TEST (QoS, ignore_member_simple_primitive)
{
    StructType a("composition");
    StructType b("composition");
    a.add_member(
            Member("x", primitive_type<char>())).add_member(
            Member("y", primitive_type<char>()));
    
    b.add_member(
            Member("x", primitive_type<char>()));

    EXPECT_EQ(TypeConsistency::IGNORE_MEMBERS , a.is_compatible(b));
    
}

int main(int argc, char** argv) 
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
