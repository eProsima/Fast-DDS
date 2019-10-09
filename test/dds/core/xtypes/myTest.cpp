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
// limitations under the License.#include <iostream>
    
#include <gtest/gtest.h>
#include <dds/core/xtypes/xtypes.hpp>
#include <iostream>

#include <cmath>

using namespace std; 
using namespace dds::core::xtypes;
#define DEBUG_MACRO_OFF\
    void debug(bool do_it=true){}
				

#define DEBUG_MACRO_ON \
    size_t debug_count = 0 ; \
				void debug(bool do_it=true) \
				{ \
    				if(do_it) \
								{ \
    								cout << "got up to: " << debug_count << endl ; \
												++debug_count ; \
								} \
				}




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
    EXPECT_EQ(TypeKind::FLOAT_128_TYPE, static_cast<const StructType&>(st.member("selfassign_member_name"
                                            ).type()).member("long_double").type().kind());
    EXPECT_EQ(TypeKind::UINT_64_TYPE, static_cast<const StructType&>(st.member("selfassign_member_name"
                                            ).type()).member("uint64_t").type().kind());
    EXPECT_EQ(TypeKind::UINT_8_TYPE, static_cast<const StructType&>(st.member("selfassign_member_name"
                                            ).type()).member("uint8_t").type().kind() );
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
        Member("char32_t", primitive_type<char32_t>()));
     
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
    EXPECT_EQ(TypeKind::CHAR_32_TYPE, st.member("char32_t").type().kind());
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
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("char32_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::STRUCTURE_TYPE) & uint32_t(st.kind()));

}

TEST (StructMember, base_methods_check)
{
    StructType st("struct_name");
    st.add_member(
        Member("long_double", primitive_type<long double>())).add_member(
        Member("uint64_t", primitive_type<uint64_t>())).add_member(
        Member("uint8_t", primitive_type<uint8_t>()));    
    StructMember sm("struct_member_name", st);

    EXPECT_EQ("struct_member_name", sm.name());
    EXPECT_EQ("struct_name", sm.type().name());
    EXPECT_EQ(false, sm.has_id());
    sm.id(8);
    EXPECT_EQ(true, sm.has_id());
    EXPECT_EQ(8, sm.get_id());
    EXPECT_EQ(false, sm.is_key());
    sm.key(true);
    EXPECT_EQ(true, sm.is_key());
    EXPECT_EQ(false, sm.is_optional());
    sm.optional(true);
    EXPECT_EQ(true, sm.is_optional());
    EXPECT_EQ(false, sm.is_bitset());
    sm.bitset(true);
    EXPECT_EQ(true, sm.is_bitset());
    EXPECT_EQ(0, st.member("long_double").offset());
}

TEST (StructMember, cascade_builder_with_qualifiers)
{
    StructType st("struct_name");
    st.add_member(StructMember("struct_member_name", StructType("inner_struct_name").add_member(
        Member("long_double", primitive_type<long double>())).add_member(
        Member("uint64_t", primitive_type<uint64_t>())).add_member(
        Member("uint8_t", primitive_type<uint8_t>()
            ).key(true).bitset(true).id(9))
        ).id(11).optional(true));
    
    EXPECT_EQ(true, st.member("struct_member_name").is_optional());
    EXPECT_EQ(true, st.member("struct_member_name").has_id());
    EXPECT_EQ(11, st.member("struct_member_name").get_id());

    const StructType &in = static_cast<const StructType&>(st.member("struct_member_name").type());

    EXPECT_EQ(true, in.member("uint8_t").has_id());
    EXPECT_EQ(9, in.member("uint8_t").get_id());
    EXPECT_EQ(true, in.member("uint8_t").is_key());
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
    Member("char32_t", primitive_type<char32_t>()));

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
    EXPECT_EQ(TypeKind::CHAR_32_TYPE, st.member("char32_t").type().kind());
    EXPECT_EQ(TypeKind::STRUCTURE_TYPE, st.kind());

    DynamicData d(st) ;
    
    d["bool"].value(true);
    d["uint8_t"].value(250);
    d["int16_t"].value(-32760);
    d["uint16_t"].value(65530); 
    d["int32_t"].value(-2147483640); 
    d["uint32_t"].value(4294967290);
    d["int64_t"].value(-9223372036854775800);
    d["uint64_t"].value(18446744073709551610ULL);
    d["float"].value(3.1415927410125732421875f);   
    d["double"].value(3.141592653589793115997963468544185161590576171875);
    long double a = 3.14159265358979323851280895940618620443274267017841339111328125 ;
    d["long double"] .value(a);
    d["char"].value('f');
    d["char32_t"].value(4294967290);
    
    EXPECT_EQ(true, d["bool"].value<bool>()); 
    EXPECT_EQ(250, d["uint8_t"].value<uint8_t>());
    EXPECT_EQ(-32760, d["int16_t"].value<int16_t>());
    EXPECT_EQ(65530,d["uint16_t"].value<uint16_t>()); 
    EXPECT_EQ(-2147483640, d["int32_t"].value<int32_t>()); 
    EXPECT_EQ(4294967290, d["uint32_t"].value<uint32_t>());
    EXPECT_EQ(-9223372036854775800, d["int64_t"].value<int64_t>());
    EXPECT_EQ(18446744073709551610ULL,d["uint64_t"].value<uint64_t>());
    EXPECT_EQ( float(3.1415927410125732421875) , d["float"].value<float>()) ;   
    EXPECT_EQ( double(3.141592653589793115997963468544185161590576171875) , d["double"].value<double>());
    EXPECT_EQ( a , d["long double"].value<long double>());
    EXPECT_EQ('f', d["char"].value<char>());
    EXPECT_EQ(4294967290, d["char32_t"].value<char32_t>());

}

TEST (DynamicData, long_random_sequence)
{
    SequenceType st(primitive_type<double>()) ;

    DynamicData d(st) ;
    srand48(time(0)) ;
    
    for(int i = 0 ; i < 65000 ; ++i)
    {
        double r = lrand48()/double(RAND_MAX) ;
        d.push(r);
        EXPECT_EQ(r, d[i].value<double>());
    }
}

TEST (DynamicData, sequence)
{
    StructType s("struct") ;
				s.add_member(Member("sequence_1", SequenceType(primitive_type<char>()))) ;
				s.add_member(Member("sequence_2", SequenceType(primitive_type<char>()))) ;
				{
				    DynamicData d(s) ;
				    d["sequence_1"].push('a') ;
				    d["sequence_1"].push('b') ;
								d["sequence_1"].push('c') ;
								d["sequence_1"].push('d') ;
								d["sequence_1"].push('e') ;
								d["sequence_2"].push(d["sequence_1"]) ;
    }				
}

TEST (DynamicData, test_just_for_luis)
{
    
				StructType temp_type("Temp");
				temp_type.add_member(Member("number", primitive_type<uint32_t>()));
				temp_type.add_member(Member("string", StringType()));

				StructType fixed_type("Fixed");
				fixed_type.add_member(Member("inner", SequenceType(temp_type)));

				DynamicData fixed(fixed_type);
				
				try{
								for(int i = 0; i < 2; i++)
								{
												DynamicData temp(temp_type);
												temp["number"].value(i);
												temp["string"].value(std::to_string(i));
												fixed["inner"].push(temp);
 								}
     }catch(exception &e)
 				{
 				    cout << "dove: " << e.what() ;
 								exit(0xff) ; 
 				}
}

DEBUG_MACRO_ON
DynamicData cdd(StructType &st)
{	 
    StructType ist("ist");
    ist.add_member(
        Member("int", ArrayType(primitive_type<uint32_t>(), 10)));
    st.add_member(Member("str", ist));

    DynamicData dd(st) ;
				for(int i = 0 ; i < 1E1; ++i)
        dd["str"]["int"][i].value<uint32_t>(32);
				return dd ;
}

TEST (DynamicData, cp_type)
{
    StructType st("st");
    DynamicData dd = cdd(st) ;
				cout << dd["str"]["int"][7].value<int32_t>() << endl ;
}


DynamicData ret_dyn_data(StructType &st, int i )
{
				switch(i){
				    case 1:
				        st.add_member(
				            Member("int", primitive_type<int32_t>()));
				        break ;
								case 2:
            st.add_member(
				            Member("array", ArrayType(primitive_type<uint32_t>(), 100000000)));
            break ;
        default:
								    break ;
				}
				DynamicData dt(st) ;
				switch(i){
				    case 1:
        				dt["int"].value<int32_t>(32);
				        break ;
								case 2:
								    for(int j = 0 ; j < 10 ; ++j)
        				    dt["array"][i].value<int32_t>(32);
            break ;
        default:
								    break ;
				}
				
				return dt;
}

TEST (DynamicData, cp)
{
    StructType ist("int_st");
				DynamicData idt = ret_dyn_data(ist, 1) ;

    StructType ast("arr_st");
				DynamicData adt = ret_dyn_data(ast, 2) ;

}


DynamicData create_dynamic_data(long double pi, StructType& the_struct/*, StructType& inner_struct, StructType& second_inner_struct*/)
{
				StructType inner_struct("inner_struct") ;
    StructType second_inner_struct("second_inner_struct");
				second_inner_struct.add_member(
				    Member("second_inner_string", StringType())).add_member(
								Member("second_inner_uint32_t", primitive_type<uint32_t>())).add_member(
								Member("second_inner_array", ArrayType(primitive_type<uint8_t>(), 10)));
				
				inner_struct.add_member(
				    Member("inner_string", StringType())).add_member(
								Member("inner_float", primitive_type<float>())).add_member(
								Member("inner_sequence_string", SequenceType(StringType()))).add_member(
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
				
				DynamicData the_data(the_struct) ;
				the_data["bool"].value(true) ;
    the_data["uint8_t"].value(uint8_t(230)) ;
				the_data["int16_t"].value<>(int16_t(-784));
				the_data["uint16_t"].value<>(uint16_t(784));
				the_data["int32_t"].value<>(int32_t(-5469372));
				the_data["uint32_t"].value<>(uint32_t(45350234)) ;
				the_data["int64_t"].value<>(int64_t(-1234523556));
				the_data["uint64_t"].value<>(uint64_t(1234523556));
				the_data["float"].value<>(float(3.1415926)) ;
				the_data["double"].value<>(double(-3.14159264));
				
				the_data["long_double"].value<>(pi);

    for(int i = 0 ; i < 1E4 ; ++i) // creating "sequence"
				{
				    DynamicData tmp_data(inner_struct);
								tmp_data["inner_string"].string("lay_down_and_cry") ;
								tmp_data["inner_float"].value(float(3.1415)) ;
        for (int j = 0 ; j < 1E3 ; ++j) // creating "sequence.inner_sequence_string"
								{
								    StringType s ;
								    DynamicData dst(s) ;
												dst.string("another_prick_in_the_world");
								    tmp_data["inner_sequence_string"].push(dst) ;
								}
								
								for (int j = 0 ; j < 1E2 ; ++j) // creating "sequence.inner_sequence_struct"
								{
								    DynamicData tmp_inner_data(second_inner_struct);
												tmp_inner_data["second_inner_string"].string("paint_it_black");
												tmp_inner_data["second_inner_uint32_t"].value<>(uint32_t(38)) ;
												for(int k = 0 ; k < 1E1 ; ++k) //creating "sequence.inner_sequence_struct.second_inner_array"
												{
												    tmp_inner_data["second_inner_array"][k].value<uint8_t>(56) ;
												}
								    tmp_data["inner_sequence_struct"].push(tmp_inner_data);
								}
								for(int j = 0 ; j < 1E1 ; ++j)
								{
								    for(int k = 0 ; k < 1E1 ; ++k)
												{
                the_data["array"][j][k].value<long double>(3.1415) ;
												}
								}
        the_data["sequence"].push(tmp_data);
    }

    return the_data ;
}

TEST (DynamicData, cascade_construction)
{
    long double pi = 3.14159265358979323846 ;
    StructType the_struct("the_struct") ;
    DynamicData the_data = create_dynamic_data(pi, the_struct/*, inner_struct, second_inner_struct*/) ;


    debug(false);
    EXPECT_EQ(45350234, the_data["uint32_t"].value<uint32_t>());
    debug(false);
    EXPECT_EQ(-5469372, the_data["int32_t"].value<int32_t>()); 
    debug(false);
				EXPECT_EQ(784, the_data["uint16_t"].value<uint16_t>());
    debug(false);
    EXPECT_EQ(-784, the_data["int16_t"].value<int16_t>()); 
    debug(false);
    EXPECT_EQ(true, the_data["bool"].value<bool>());
    debug(false);
				EXPECT_EQ(230, the_data["uint8_t"].value<uint8_t>());
    debug(false);
    EXPECT_EQ(-1234523556, the_data["int64_t"].value<int64_t>()); 
    debug(false);
				EXPECT_EQ(1234523556, the_data["uint64_t"].value<uint64_t>());
    debug(false);
    EXPECT_EQ(3.1415926f, the_data["float"].value<float>()); 
    debug(false);
    EXPECT_EQ(-3.14159264, the_data["double"].value<double>()); 
    debug(false);
    EXPECT_EQ(pi, the_data["long_double"].value<long double>());
    debug(false);

    srand48(time(0)) ;
    
    for (int i = 0 ; i < 1E2 ; ++i)
				{
				    size_t idx_4 = lrand48()%int(1E4) ;
        EXPECT_EQ("lay_down_and_cry", the_data["sequence"][idx_4]["inner_string"].string()) ;
        EXPECT_EQ(3.1415f, the_data["sequence"][idx_4]["inner_float"].value<float>());
        size_t idx_3 = lrand48()%int(1E3) ;
        EXPECT_EQ("another_prick_in_the_world", the_data["sequence"][idx_4]["inner_sequence_string"][idx_3].string()) ;
        size_t idx_2 = lrand48()%int(1E2) ;
        EXPECT_EQ("paint_it_black", the_data["sequence"][idx_4]["inner_sequence_struct"][idx_2]["second_inner_string"].string()) ;
        EXPECT_EQ(38, the_data["sequence"][idx_4]["inner_sequence_struct"][idx_2]["second_inner_uint32_t"].value<uint32_t>()) ;
				
				    size_t arr_idx_3 = lrand48()%int(1E1);
				    size_t arr_idx_2 = lrand48()%int(1E1);
				    size_t arr_idx_1 = lrand48()%int(1E1);
								
								long double check_over = 3.1415 ;
								EXPECT_EQ(check_over, the_data["array"][arr_idx_3][arr_idx_2].value<long double>());
								EXPECT_EQ(uint8_t(56), the_data["sequence"][idx_4]["inner_sequence_struct"][idx_2]["second_inner_array"][arr_idx_1].value<uint8_t>());
        debug();
				}	
    debug(); //100
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
//				EXPECT_EQ("all_this_stuff", the_data["seq"].string());  <<======= this call fails. It is kept here as a memorandum
}

int main() 
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
