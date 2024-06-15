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

/**
 * @file IdlParser_main.cpp
 *
 */

#define FASTDDS_ENFORCE_LOG_INFO

#include <DynamicTypeBuilderFactoryImpl.hpp>
#include <fastrtps/log/Log.h>

#include <iostream>

using namespace eprosima::fastdds::dds;
using eprosima::fastdds::dds::Log;

int main(
        int argc,
        char** argv)
{
    Log::SetVerbosity(Log::Kind::Info);
    Log::SetCategoryFilter(std::regex("IDLPARSER"));

     std::cout << "Processing forward declarations:" << std::endl;
     std::string test00 =
             R"(
         struct StructDcl;
         union UnionDcl;
     )";
     DynamicType::_ref_type fwd_dcl_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_idl(test00)->build();
     if (nullptr != fwd_dcl_type)
     {
         EPROSIMA_LOG_INFO(IDLPARSER, "Succeeded to create DynamicType from IDL: " << test00);
     }
     else
     {
         EPROSIMA_LOG_ERROR(IDLPARSER, "Failed to create DynamicType from IDL: " << test00);
     }

     std::cout << "Processing literals, expressions, and consts:" << std::endl;
     std::string test01 =
             R"(
         // boolean literals in IDL are case sensitive and should be exactly TRUE or FALSE
         const boolean C_BOOL = TRUE;
         const string<100> C_STR = "hello";
         const int32 C_LITERALS1 = (0x5 | (4 + 3)) + (4 << 1) * 2; // 23
         const int32 C_LITERALS2 = (02 + 0x1) * 2; // 6
         const int32 C_LITERALS3 = 4 / ( 07 & 0x2 ); // 2
         const int32 C_LITERALS4 = -(5 * ( 03 + 0xF )); // -90
         const int32 C_LITERALS5 = 0xf0 & ~(4 * ( 0x7 - 02 )); // 224
         const int32 C_LITERALS6 = (0x7 | 0x9) & ~(6 * ( 024 - 5 )); // 5
         const float C_LITERALS7 = (2 + 4) / 3.0d; /* 2.0 */
         const float C_LITERALS8 = (2e0 + 4) / 3.0d; /* 2.0 */
     )";
     DynamicTypeBuilder::_ref_type literal_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_idl(test01);

    // std::cout << "Processing IDL string:" << std::endl;
    // std::string test02 =
    //         R"(
    //     struct InnerType
    //     {
    //         uint32 im1;
    //         float im2;
    //     };
    // )";
    // idl::Context context02 = idl::parse(test02);

    // std::cout << "Processing IDL file:" << std::endl;
    // idl::Context context03 = idl::parse_file("idl/test02.idl");

    // std::string test04 =
    //         R"(
    //     enum e_test { a, b, c };
    // )";
    // idl::Context context04 = idl::parse(test04);

    // std::string test05 =
    //         R"(
    //     struct Test05
    //     {
    //         long my_long;
    //         short my_short;
    //         long long my_llong;
    //         int32 my_int;
    //         unsigned short my_ushort;
    //         unsigned long my_ulong;
    //         long double my_ldouble;
    //         float my_float;
    //         double my_double;
    //     };
    // )";
    // idl::Context context05 = idl::parse(test05);

    // std::string test06 =
    //         R"(
    //     struct Point {
    //         float x;
    //         float y;
    //         float z;
    //     };
    //     typedef Point PointAlias;
    //     typedef Point PointArrayOf10[ 10 /**/];
    //     typedef PointAlias TwoDimPointArray[5][3];
    // )";
    // idl::Context context06 = idl::parse(test06);

    // std::string test07 =
    //         R"(
    //     union MyUnion switch (int32) {
    //         case 0:
    //         case 1:
    //             long myLong;
    //         case 2:
    //             double myDouble;
    //         default:
    //             char myChar;
    //     };
    // )";
    // idl::Context context07 = idl::parse(test07);

    std::string idl_spec =
            R"(
        struct InnerType
        {
            uint32 im1;
            float im2;
        };
    )";

    DynamicType::_ref_type inner_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_idl(idl_spec)->build();
    if (nullptr != inner_type)
    {
        EPROSIMA_LOG_INFO(IDLPARSER, "Succeeded to create DynamicType from IDL: " << idl_spec);
    }
    else
    {
        EPROSIMA_LOG_ERROR(IDLPARSER, "Failed to create DynamicType from IDL: " << idl_spec);
    }

    // idl::Context context = idl::parse(idl_spec);
    // v1_3::DynamicType inner = context.module().structure("InnerType");

    // v1_3::DynamicTypeBuilderFactory& factory = v1_3::DynamicTypeBuilderFactory::get_instance();
    // v1_3::DynamicTypeBuilder_ptr builder = factory.create_struct_type();
    // builder->set_name("OuterType");
    // builder->add_member(0, "om1", factory.get_string_type());
    // auto outer = builder->build();

    // idl::Module root;
    // idl::Module& submod_a = root.create_submodule("a");
    // idl::Module& submod_b = root.create_submodule("b");
    // idl::Module& submod_aa = submod_a.create_submodule("a");
    // submod_aa.structure(std::move(const_cast<v1_3::DynamicType&>(*outer)));
    // submod_b.structure(inner);
    // /*
    //    root
    //        \_a
    //        |  \_ a _ OuterType
    //        |
    //        \_ b _ InnerType
    // */
    // std::cout << std::boolalpha;
    // std::cout << "Does a::a::OuterType exists?: " << root.has_structure("a::a::OuterType") << std::endl;
    // std::cout << "Does ::InnerType exists?: " << root.has_structure("::InnerType") << std::endl;
    // std::cout << "Does InnerType exists?: " << root.has_structure("InnerType") << std::endl;
    // std::cout << "Does OuterType exists?: " << root.has_structure("OuterType") << std::endl;
    // std::cout << "Does ::b::InnerType exists?: " << root.has_structure("::b::InnerType") << std::endl;
    // std::cout << "Does b::InnerType exists?: " << root.has_structure("b::InnerType") << std::endl;

    // v1_3::DynamicData* outer_data = v1_3::DynamicDataFactory::get_instance()->create_data(
    //     std::make_shared<v1_3::DynamicType>(root["a"]["a"].structure("OuterType"))
    // ); // ::a::a::OuterType
    // v1_3::MemberId member_id = outer_data->get_member_id_by_name("om1");
    // outer_data->set_string_value("This is a string.", member_id);

    // std::string scope_inner_type = inner.get_name(); // b::InnerType
    // v1_3::DynamicData* inner_data = v1_3::DynamicDataFactory::get_instance()->create_data(
    //     std::make_shared<v1_3::DynamicType>(root.structure(scope_inner_type))
    // );
    // inner_data->set_uint32_value(32u, inner_data->get_member_id_by_name("im1"));
    // inner_data->set_float32_value(3.14159265f, inner_data->get_member_id_by_name("im2"));

    Log::Flush();
    Log::Reset();

    return 0;
}
