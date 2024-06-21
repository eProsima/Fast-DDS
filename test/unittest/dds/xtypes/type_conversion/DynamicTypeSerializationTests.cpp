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

#include <fstream>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include "types/all_types.hpp"

using namespace eprosima;
using namespace eprosima::fastdds::dds;


class DynamicTypeSerializationTests : public ::testing::TestWithParam<std::string>
{
public:

    void SetUp()
    {
        test::register_dynamic_types();
    }

protected:

    void get_dynamic_type(
            const std::string& type_name,
            DynamicType::_ref_type& dyn_type)
    {
        // Find TypeObjects for the type
        xtypes::TypeObjectPair type_objs;
        ASSERT_EQ(DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                        type_name, type_objs),
                  fastdds::dds::RETCODE_OK);

        // Create DynamicType from TypeObject
        dyn_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
                        type_objs.complete_type_object)->build();
    }
};

/**
 * Verify that the IDL serialization of a DynamicType generated with Fast-DDS Gen matches its IDL file.
 *
 * CASES:
 *  - Verify that the IDL file was opened successfully.
 *  - Verify that the IDL serialization finished successfully.
 *  - Verify that the two IDLs match.
 */
TEST_P(DynamicTypeSerializationTests, to_idl)
{
    const std::string type = GetParam();

    // Read the IDL file as a string
    const auto file_name = std::string("types/idls/") + type + ".idl";

    std::ifstream file(file_name);
    ASSERT_TRUE(file.is_open());

    const std::string idl_file{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    // Get Dynamic type
    DynamicType::_ref_type dyn_type;
    get_dynamic_type(type, dyn_type);

    // Serialize DynamicType to IDL
    std::string idl_serialization;
    ASSERT_EQ(idl_serialize(dyn_type, idl_serialization), RETCODE_OK);

    // Compare IDLs
    ASSERT_EQ(idl_file, idl_serialization);
}

INSTANTIATE_TEST_SUITE_P(
        DynamicTypeSerializationTests,
        DynamicTypeSerializationTests,
        ::testing::ValuesIn(test::supported_types)
);

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
