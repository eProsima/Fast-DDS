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

#include <fastrtps/types/idl/idl.h>

#include <iostream>

using namespace eprosima::fastrtps::types;

int main(
        int argc,
        char** argv)
{
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


    return 0;
}
