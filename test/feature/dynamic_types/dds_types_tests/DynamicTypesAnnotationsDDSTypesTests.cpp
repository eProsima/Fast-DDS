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

#include <gtest/gtest.h>

#include "../DynamicTypesDDSTypesTest.hpp"
#include "../../../dds-types-test/helpers/basic_inner_types.hpp"
#include "../../../dds-types-test/annotationsPubSubTypes.hpp"
#include "../../../dds-types-test/annotationsTypeObjectSupport.hpp"
#include <fastdds/dds/xtypes/dynamic_types/AnnotationDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

constexpr const char* var_basic_short_name = "var_basic_short";
constexpr const char* basic_annotations_member_name = "basic_annotations_member";

namespace eprosima {
namespace fastdds {
namespace dds {

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AnnotatedStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("AnnotatedStruct");
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    AnnotationDescriptor::_ref_type annotation_descriptor {traits<AnnotationDescriptor>::make_shared()};
    TypeDescriptor::_ref_type annotation_type {traits<TypeDescriptor>::make_shared()};
    annotation_type->kind(TK_ANNOTATION);
    annotation_type->name("AnnotationTest");
    DynamicTypeBuilder::_ref_type annotation_builder {factory->create_type(annotation_type)};

    MemberDescriptor::_ref_type annotation_parameter {traits<MemberDescriptor>::make_shared()};
    annotation_parameter->name("var_short");
    annotation_parameter->type(factory->get_primitive_type(TK_INT16));
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_ushort");
    annotation_parameter->type(factory->get_primitive_type(TK_UINT16));
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_long");
    annotation_parameter->type(factory->get_primitive_type(TK_INT32));
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_ulong");
    annotation_parameter->type(factory->get_primitive_type(TK_UINT32));
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_longlong");
    annotation_parameter->type(factory->get_primitive_type(TK_INT64));
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_ulonglong");
    annotation_parameter->type(factory->get_primitive_type(TK_UINT64));
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_float");
    annotation_parameter->type(factory->get_primitive_type(TK_FLOAT32));
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_double");
    annotation_parameter->type(factory->get_primitive_type(TK_FLOAT64));
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_boolean");
    annotation_parameter->type(factory->get_primitive_type(TK_BOOLEAN));
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_octet");
    annotation_parameter->type(factory->get_primitive_type(TK_BYTE));
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_char8");
    annotation_parameter->type(factory->get_primitive_type(TK_CHAR8));
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_char16");
    annotation_parameter->type(factory->get_primitive_type(TK_CHAR16));
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_string");
    annotation_parameter->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_wstring");
    annotation_parameter->type(factory->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("enum_value");
    TypeDescriptor::_ref_type enum_type_descriptor {traits<TypeDescriptor>::make_shared()};
    enum_type_descriptor->name("InnerEnumHelper");
    enum_type_descriptor->kind(TK_ENUM);
    DynamicTypeBuilder::_ref_type enum_builder {factory->create_type(enum_type_descriptor)};
    MemberDescriptor::_ref_type enum_literal {traits<MemberDescriptor>::make_shared()};
    enum_literal->type(factory->get_primitive_type(TK_UINT32));
    enum_literal->name("ONE");
    enum_builder->add_member(enum_literal);
    enum_literal = traits<MemberDescriptor>::make_shared();
    enum_literal->type(factory->get_primitive_type(TK_UINT32));
    enum_literal->name("TWO");
    enum_builder->add_member(enum_literal);
    DynamicType::_ref_type enum_type {enum_builder->build()};
    annotation_parameter->type(enum_type);
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("enum_default_value");
    annotation_parameter->type(enum_type);
    annotation_parameter->default_value("TWO");
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_string_10");
    TypeDescriptor::_ref_type alias_type_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_type_descriptor->kind(TK_ALIAS);
    alias_type_descriptor->name("Inner_alias_bounded_string_helper");
    alias_type_descriptor->base_type(factory->create_string_type(10)->build());
    DynamicType::_ref_type alias_type {factory->create_type(alias_type_descriptor)->build()};
    annotation_parameter->type(alias_type);
    annotation_builder->add_member(annotation_parameter);
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    annotation_parameter->name("var_default_string_10");
    annotation_parameter->type(alias_type);
    annotation_parameter->default_value("Hello");
    annotation_parameter = traits<MemberDescriptor>::make_shared();
    // There is no official support in the STL to convert from wstring to string.
    /*
       annotation_parameter->name("var_wstring_alias");
       alias_type_descriptor = traits<TypeDescriptor>::make_shared();
       alias_type_descriptor->kind(TK_ALIAS);
       alias_type_descriptor->name("Inner_alias_bounded_wstring_helper");
       alias_type_descriptor->base_type(factory->create_wstring_type(10)->build());
       annotation_parameter->type(factory->create_type(alias_type_descriptor)->build());
       annotation_builder->add_member(annotation_parameter);
     */

    annotation_descriptor->type(annotation_builder->build());
    annotation_descriptor->set_value("var_short", std::to_string(1));
    annotation_descriptor->set_value("var_ushort", std::to_string(1));
    annotation_descriptor->set_value("var_long", std::to_string(1));
    annotation_descriptor->set_value("var_ulong", std::to_string(1));
    annotation_descriptor->set_value("var_longlong", std::to_string(1));
    annotation_descriptor->set_value("var_ulonglong", std::to_string(1));
    annotation_descriptor->set_value("var_float", std::to_string(1));
    annotation_descriptor->set_value("var_double", std::to_string(1));
    annotation_descriptor->set_value("var_boolean", std::to_string(true));
    annotation_descriptor->set_value("var_octet", std::to_string(0));
    annotation_descriptor->set_value("var_char8", std::to_string('a'));
    annotation_descriptor->set_value("var_char16", std::to_string(L'a'));
    annotation_descriptor->set_value("var_string", "a");
    // There is no official support in the STL to convert from wstring to string.
    // annotation_descriptor->set_value("var_wstring", L"a");
    annotation_descriptor->set_value("enum_value", "ONE");

    type_builder->apply_annotation(annotation_descriptor);
    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    for (auto encoding : encodings)
    {
        AnnotatedStruct struct_data;
        TypeSupport static_pubsubType {new AnnotatedStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
    }

    // Dynamic Language Binding does not currently support wchar and wstring parameters.

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_EmptyAnnotatedStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("EmptyAnnotatedStruct");
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    AnnotationDescriptor::_ref_type annotation_descriptor {traits<AnnotationDescriptor>::make_shared()};
    TypeDescriptor::_ref_type annotation_type {traits<TypeDescriptor>::make_shared()};
    annotation_type->kind(TK_ANNOTATION);
    annotation_type->name("EmptyAnnotationTest");
    DynamicTypeBuilder::_ref_type annotation_builder {factory->create_type(annotation_type)};
    annotation_descriptor->type(annotation_builder->build());

    type_builder->apply_annotation(annotation_descriptor);
    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    for (auto encoding : encodings)
    {
        EmptyAnnotatedStruct struct_data;
        TypeSupport static_pubsubType {new EmptyAnnotatedStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_EmptyAnnotatedStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_BasicAnnotationsTest)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("BasicAnnotationsStruct");
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    AnnotationDescriptor::_ref_type annotation_descriptor {traits<AnnotationDescriptor>::make_shared()};
    TypeDescriptor::_ref_type annotation_type {traits<TypeDescriptor>::make_shared()};
    annotation_type->kind(TK_ANNOTATION);
    annotation_type->name("BasicAnnotationsTest");
    DynamicTypeBuilder::_ref_type annotation_builder {factory->create_type(annotation_type)};

    MemberDescriptor::_ref_type annotation_parameter {traits<MemberDescriptor>::make_shared()};
    annotation_parameter->name(var_basic_short_name);
    annotation_parameter->type(factory->get_primitive_type(TK_INT16));
    annotation_builder->add_member(annotation_parameter);

    annotation_descriptor->type(annotation_builder->build());
    annotation_descriptor->set_value(var_basic_short_name, std::to_string(1));

    type_builder->apply_annotation(annotation_descriptor);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(basic_annotations_member_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    type_builder->add_member(member_descriptor);
    type_builder->apply_annotation_to_member(0, annotation_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    for (auto encoding : encodings)
    {
        BasicAnnotationsStruct struct_data;
        TypeSupport static_pubsubType {new BasicAnnotationsStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_BasicAnnotationsStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

} // dds
} // fastdds
} // eprosima
