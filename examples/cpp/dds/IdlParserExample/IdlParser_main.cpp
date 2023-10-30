// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/types/idl/idl.h>
#include <fastrtps/log/Log.h>

#include <iostream>

using namespace eprosima::fastrtps::types;
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
    idl::Context context00 = idl::parse(test00);

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
    idl::Context context01 = idl::parse(test01);

    std::cout << "Processing IDL string:" << std::endl;
    std::string test02 =
            R"(
        struct InnerType
        {
            uint32 im1;
            float im2;
        };
    )";
    idl::Context context02 = idl::parse(test02);

    std::cout << "Processing IDL file:" << std::endl;
    idl::Context context03 = idl::parse_file("idl/test02.idl");

    std::string test04 =
            R"(
        enum e_test { a, b, c };
    )";
    idl::Context context04 = idl::parse(test04);

    std::string test05 =
            R"(
        struct Test05
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
    idl::Context context05 = idl::parse(test05);

    std::string test06 =
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
    idl::Context context06 = idl::parse(test06);

    std::string test07 =
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
    idl::Context context07 = idl::parse(test07);

    Log::Flush();
    Log::Reset();

    return 0;
}
