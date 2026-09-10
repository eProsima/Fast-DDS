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

#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

#include <utils/UnitsParser.hpp>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include "types/all_types.hpp"

using namespace eprosima;
using namespace eprosima::fastdds::dds;


class DynTypeIDLTests : public ::testing::TestWithParam<std::string>
{
protected:

    void get_dynamic_type(
            const std::string& type_name,
            DynamicType::_ref_type& dyn_type)
    {
        // Find TypeObjects for the type
        xtypes::TypeObjectPair type_objs;
        ASSERT_EQ(DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(type_name,
                type_objs),
                fastdds::dds::RETCODE_OK);

        // Create DynamicType from TypeObject
        dyn_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
            type_objs.complete_type_object)->build();
    }

    std::string snake_to_camel(
            const std::string& snake_case)
    {
        std::string camel_case;
        bool to_upper = true;

        for (const auto& ch : snake_case)
        {
            if (ch == '_')
            {
                to_upper = true;
            }
            else if (to_upper)
            {
                std::string ch_str(1, ch);
                utils::to_uppercase(ch_str);
                camel_case += ch_str;
                to_upper = false;
            }
            else
            {
                camel_case += ch;
            }
        }

        return camel_case;
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
TEST_P(DynTypeIDLTests, to_idl)
{
    const std::string type = GetParam();

    test::register_type_object_representation(type);

    // Read the IDL file as a string
    const auto file_name = std::string("types/") + type + "/" + type + ".idl";

    std::ifstream file(file_name);
    ASSERT_TRUE(file.is_open());

    const std::string idl_file{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    // Get Dynamic type
    DynamicType::_ref_type dyn_type;
    get_dynamic_type(snake_to_camel(type), dyn_type);
    ASSERT_NE(dyn_type, nullptr);

    // Serialize DynamicType to IDL
    std::stringstream idl_serialization;
    ASSERT_EQ(idl_serialize(dyn_type, idl_serialization), RETCODE_OK);

    // Compare IDLs
    ASSERT_EQ(idl_file, idl_serialization.str());
}

INSTANTIATE_TEST_SUITE_P(
    DynTypeIDLTests,
    DynTypeIDLTests,
    ::testing::ValuesIn(test::supported_types)
    );

/**
 * Verify that array aliases keep bounds after their names,
 * and that the serialized IDL can be parsed back into the same type.
 */
TEST(DynTypeIDLRegressionTests, array_aliases)
{
    const std::string input_file = "types/array_declarations/array_declarations.idl";
    std::ifstream file(input_file);
    ASSERT_TRUE(file.is_open());
    const std::string expected{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    auto factory = DynamicTypeBuilderFactory::get_instance();
    auto builder = factory->create_type_w_uri(input_file, "ArrayDeclarations", {});
    ASSERT_NE(builder, nullptr);
    auto type = builder->build();
    ASSERT_NE(type, nullptr);

    std::stringstream serialized;
    ASSERT_EQ(idl_serialize(type, serialized), RETCODE_OK);
    EXPECT_EQ(expected, serialized.str());

    const std::string output_file = "array_declarations_roundtrip.idl";
    {
        std::ofstream output(output_file);
        ASSERT_TRUE(output.is_open());
        output << serialized.str();
    }
    auto reparsed = factory->create_type_w_uri(output_file, "ArrayDeclarations", {});
    std::remove(output_file.c_str());
    ASSERT_NE(reparsed, nullptr);
    EXPECT_TRUE(type->equals(reparsed->build()));
}

/**
 * Verify array declarators in both labelled and default union members.
 */
TEST(DynTypeIDLRegressionTests, array_union_members)
{
    auto factory = DynamicTypeBuilderFactory::get_instance();
    auto float64_type = factory->get_primitive_type(TK_FLOAT64);
    auto array_builder = factory->create_array_type(float64_type, {9});
    auto matrix_builder = factory->create_array_type(float64_type, {2, 3});
    ASSERT_NE(array_builder, nullptr);
    ASSERT_NE(matrix_builder, nullptr);

    auto descriptor = traits<TypeDescriptor>::make_shared();
    descriptor->kind(TK_UNION);
    descriptor->name("ArrayUnion");
    descriptor->discriminator_type(factory->get_primitive_type(TK_INT32));
    auto union_builder = factory->create_type(descriptor);
    ASSERT_NE(union_builder, nullptr);

    auto member = traits<MemberDescriptor>::make_shared();
    member->name("values");
    member->type(array_builder->build());
    member->label({0, 1});
    ASSERT_EQ(union_builder->add_member(member), RETCODE_OK);

    member = traits<MemberDescriptor>::make_shared();
    member->name("matrix");
    member->type(matrix_builder->build());
    member->is_default_label(true);
    ASSERT_EQ(union_builder->add_member(member), RETCODE_OK);

    descriptor = traits<TypeDescriptor>::make_shared();
    descriptor->kind(TK_STRUCTURE);
    descriptor->name("ArrayUnionStruct");
    auto struct_builder = factory->create_type(descriptor);
    ASSERT_NE(struct_builder, nullptr);
    member = traits<MemberDescriptor>::make_shared();
    member->name("selection");
    member->type(union_builder->build());
    ASSERT_EQ(struct_builder->add_member(member), RETCODE_OK);

    std::stringstream serialized;
    ASSERT_EQ(idl_serialize(struct_builder->build(), serialized), RETCODE_OK);
    EXPECT_EQ(serialized.str(),
            "union ArrayUnion switch (long)\n"
            "{\n"
            "    case 0:\n"
            "    case 1:\n"
            "        double values[9];\n"
            "    default:\n"
            "        double matrix[2][3];\n"
            "};\n\n"
            "@extensibility(APPENDABLE)\n"
            "struct ArrayUnionStruct\n"
            "{\n"
            "    ArrayUnion selection;\n"
            "};\n");
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
