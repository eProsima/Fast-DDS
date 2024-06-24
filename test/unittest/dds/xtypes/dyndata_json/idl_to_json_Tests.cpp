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
#include <nlohmann/json.hpp>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobject.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include "types/comprehensive_type/gen/ComprehensiveTypePubSubTypes.h"
#include "types/comprehensive_type/gen/ComprehensiveTypeTypeObjectSupport.hpp"
#include "types/comprehensive_type/json/ComprehensiveType_OMG.hpp"
#include "types/comprehensive_type/json/ComprehensiveType_Filled_OMG.hpp"
#include "types/comprehensive_type/json/ComprehensiveType_EPROSIMA.hpp"
#include "types/comprehensive_type/json/ComprehensiveType_Filled_EPROSIMA.hpp"

#include "types/types.hpp"

#include <gtest/gtest.h>

/**
 * These tests are designed to validate the serialization process from DynamicData to JSON format using
 * the eProsima Fast DDS library.
 *
 * CASES:
 *  1. Test for serializing raw DynamicData to JSON using the EPROSIMA format.
 *  2. Test for serializing filled DynamicData to JSON using the EPROSIMA format.
 *  3. Test for serializing raw DynamicData to JSON using the OMG format.
 *  4. Test for serializing filled DynamicData to JSON using the OMG format.
 *
 *  Each test verifies that the generated JSON matches the expected JSON string.
 */

using namespace eprosima::fastdds::dds;


TEST(idl_to_json_Tests, ComprehensiveType_EPROSIMA)
{
    auto dyn_type = create_dynamic_type<DataTypeKind::COMPREHENSIVE_TYPE>();
    auto dyn_data = create_dynamic_data<DataTypeKind::COMPREHENSIVE_TYPE>(dyn_type, false);

    std::stringstream generated_json;
    generated_json << std::setw(4);
    const auto ret = json_serialize(
                dyn_data,
                generated_json,
                DynamicDataJsonFormat::EPROSIMA);

    ASSERT_EQ(ret, RETCODE_OK);
    ASSERT_EQ(generated_json.str(), expected_json_comprehensive_eprosima);
}

TEST(idl_to_json_Tests, ComprehensiveTypeFilled_EPROSIMA)
{
    auto dyn_type = create_dynamic_type<DataTypeKind::COMPREHENSIVE_TYPE>();

    for (unsigned int i = 1; i < 3; i++)
    {
        auto dyn_data = create_dynamic_data<DataTypeKind::COMPREHENSIVE_TYPE>(dyn_type, true, i);
        std::stringstream generated_json;
        generated_json << std::setw(4);

        const auto ret = json_serialize(
                    dyn_data,
                    generated_json,
                    DynamicDataJsonFormat::EPROSIMA);

        ASSERT_EQ(ret, RETCODE_OK);
        ASSERT_EQ(generated_json.str(), expected_json_comprehensive_filled_eprosima[i]);
    }
}

TEST(idl_to_json_Tests, ComprehensiveType_OMG)
{
    auto dyn_type = create_dynamic_type<DataTypeKind::COMPREHENSIVE_TYPE>();
    auto dyn_data = create_dynamic_data<DataTypeKind::COMPREHENSIVE_TYPE>(dyn_type, false);

    std::stringstream generated_json;
    generated_json << std::setw(4);
    const auto ret = json_serialize(
                dyn_data,
                generated_json,
                DynamicDataJsonFormat::OMG);

    ASSERT_EQ(ret, RETCODE_OK);
    ASSERT_EQ(generated_json.str(), expected_json_comprehensive_omg);
}

TEST(idl_to_json_Tests, ComprehensiveTypeFilled_OMG)
{
    auto dyn_type = create_dynamic_type<DataTypeKind::COMPREHENSIVE_TYPE>();

    for (unsigned int i = 1; i < 3; i++)
    {
        auto dyn_data = create_dynamic_data<DataTypeKind::COMPREHENSIVE_TYPE>(dyn_type, true, i);
        std::stringstream generated_json;
        generated_json << std::setw(4);
        const auto ret = json_serialize(
                    dyn_data,
                    generated_json,
                    DynamicDataJsonFormat::OMG);

        ASSERT_EQ(ret, RETCODE_OK);
        ASSERT_EQ(generated_json.str(), expected_json_comprehensive_filled_omg[i]);
    }
}


int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
