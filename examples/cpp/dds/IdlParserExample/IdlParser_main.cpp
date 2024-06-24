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
    DynamicTypeBuilder::_ref_type null_builder = DynamicTypeBuilderFactory::get_instance()->create_type_w_idl(test01);

    std::string test02 =
            R"(
        struct Test02
        {
            long my_long;
            short my_short;
            long long my_llong;
            int32 my_int;
            unsigned short my_ushort;
            unsigned long my_ulong;
            long double my_ldouble;
            float my_float;
            double my_double;
        };
    )";
    DynamicType::_ref_type struct_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_idl(test02)->build();
    if (nullptr != struct_type)
    {
        EPROSIMA_LOG_INFO(IDLPARSER, "Succeeded to create DynamicType from IDL: " << test02);
    }
    else
    {
        EPROSIMA_LOG_ERROR(IDLPARSER, "Failed to create DynamicType from IDL: " << test02);
    }

    std::string test03 =
            R"(
        enum e_test { a, b, c };
    )";
    DynamicType::_ref_type enum_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_idl(test03)->build();
    if (nullptr != enum_type)
    {
        EPROSIMA_LOG_INFO(IDLPARSER, "Succeeded to create DynamicType from IDL: " << test03);
    }
    else
    {
        EPROSIMA_LOG_ERROR(IDLPARSER, "Failed to create DynamicType from IDL: " << test03);
    }

    std::string test04 =
            R"(
        struct Point {
            float x;
            float y;
            float z;
        };
        typedef Point PointAlias;
        typedef Point PointArrayOf10[ 10 /**/];
        typedef PointAlias TwoDimPointArray[5][3];
    )";
    DynamicType::_ref_type alias_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_idl(test04)->build();
    if (nullptr != alias_type)
    {
        EPROSIMA_LOG_INFO(IDLPARSER, "Succeeded to create DynamicType from IDL: " << test04);
    }
    else
    {
        EPROSIMA_LOG_ERROR(IDLPARSER, "Failed to create DynamicType from IDL: " << test04);
    }

    std::string test05 =
            R"(
        union MyUnion switch (int32) {
            case 0:
            case 1:
                long myLong;
            case 2:
                double myDouble;
            default:
                char myChar;
        };
    )";
    DynamicType::_ref_type union_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_idl(test05)->build();
    if (nullptr != union_type)
    {
        EPROSIMA_LOG_INFO(IDLPARSER, "Succeeded to create DynamicType from IDL: " << test05);
    }
    else
    {
        EPROSIMA_LOG_ERROR(IDLPARSER, "Failed to create DynamicType from IDL: " << test05);
    }

    Log::Flush();
    Log::Reset();

    return 0;
}
