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

#include <string>
#include <iostream>
#include <fstream>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobject.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include "types/types.hpp"

#include <gtest/gtest.h>

/**
 * This test is designed to validate the serialization process from DynamicData to JSON format using
 * the eProsima Fast DDS library.
 *
 * CASES:
 *  1. Serialization of an unfilled DynamicData to JSON using the EPROSIMA format.
 *  2. Serialization of a filled DynamicData to JSON using the EPROSIMA format.
 *  3. Serialization of an unfilled DynamicData to JSON using the OMG format.
 *  4. Serialization of a filled DynamicData to JSON using the OMG format.
 *
 *  Each case verifies that the generated JSON matches the expected JSON string.
 */

using namespace eprosima::fastdds::dds;

template <DataTypeKind Data>
void test_generic()
{
    std::vector<DynamicDataJsonFormat> format_options = {DynamicDataJsonFormat::EPROSIMA, DynamicDataJsonFormat::OMG};
    auto dyn_type = create_dynamic_type<Data>();

    ASSERT_NE(dyn_type, nullptr);

    for (const auto& format_kind : format_options)
    {
        for (unsigned int fill_index = 0; fill_index <= 3; fill_index++)
        {
            bool filled = fill_index != 0;

            auto dyn_data = create_dynamic_data<Data>(dyn_type, filled, fill_index);

            ASSERT_NE(dyn_data, nullptr);

            std::stringstream generated_json;
            generated_json << std::setw(4);
            const auto ret = json_serialize(
                dyn_data,
                format_kind,
                generated_json);

            ASSERT_EQ(ret, RETCODE_OK);
            ASSERT_EQ(generated_json.str(), get_expected_json<Data>(format_kind, filled, fill_index));
        }
    }
}

TEST(DynDataJSONTests, ComprehensiveType)
{
    test_generic<DataTypeKind::COMPREHENSIVE_TYPE>();
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
