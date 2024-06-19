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

#include <algorithm>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include "types/all_types.hpp"

using namespace eprosima;

class DynTypeToIdlTests : public ::testing::TestWithParam<test::SupportedType>
{
};

/**
 * Verify that the IDL serialization of a DynamicType matches the IDL file that defined the type.
 *
 * CASES:
 *  - Verify that the IDL file was opened successfully.
 *  - Verify that the IDL serialization finished successfully.
 *  - Verify that the two IDLs match.
 */
TEST_P(DynTypeToIdlTests, msg_schema_generation)
{
    const test::SupportedType type = GetParam();

    // Read the IDL file as a string
    const std::string full_path{"/home/daniel/Documents/eprosima/dynamic_type_idl/src/fastdds/test/unittest/dds/xtypes/type_conversion/types/idls/"};
    const auto file_name = full_path + to_string(type) + ".idl";

    std::ifstream file(file_name);

    ASSERT_TRUE(file.is_open());

    const std::string idl_file{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    // Get Dynamic type
    const fastdds::dds::DynamicType::_ref_type dyn_type = test::get_dynamic_type(type);

    // Serialize DynamicType to IDL
    std::string idl_serialization;
    ASSERT_EQ(fastdds::dds::idl_serialize(dyn_type, idl_serialization), fastdds::dds::RETCODE_OK);

    // Compare IDLs
    ASSERT_EQ(idl_file, idl_serialization);
}

INSTANTIATE_TEST_SUITE_P(DynTypeToIdlTests, DynTypeToIdlTests, ::testing::Values(
            test::SupportedType::hello_world,
            test::SupportedType::numeric_array,
            test::SupportedType::char_sequence,
            test::SupportedType::basic_struct,
            test::SupportedType::basic_array_struct,
            test::SupportedType::float_bounded_sequence,
            test::SupportedType::arrays_and_sequences,
            test::SupportedType::complex_nested_arrays,
            test::SupportedType::enum_struct,
            test::SupportedType::union_struct,
            test::SupportedType::map_struct
            ));

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
