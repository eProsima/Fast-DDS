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

    std::cout << "Processing struct forward declaration:" << std::endl;
    std::string test00 =
            R"(
        struct StructDcl;
        union UnionDcl;
        const boolean C_BOOL = true;
    )";
    idl::Context context00 = idl::parse(test00);

    std::cout << "Processing IDL string:" << std::endl;
    std::string idl_spec =
            R"(
        struct InnerType
        {
            uint32 im1;
            float im2;
        };
    )";
    idl::Context context = idl::parse(idl_spec);

    std::cout << "Processing IDL file:" << std::endl;
    idl::Context context_file = idl::parse_file("idl/test02.idl");

    Log::Flush();
    Log::Reset();

    return 0;
}
