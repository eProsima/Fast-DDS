// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file
 * This file contains unit tests related to the TypeObjectRegistry API.
 */

#include <gtest/gtest.h>

#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/xtypes/type_representation/TypeAssignability.hpp>

#include "assignability.hpp"
#include "assignabilityPubSubTypes.hpp"
#include "assignabilityTypeObjectSupport.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

TEST(TypeAssignabilityTests, DifferentNumberElements_Final)
{
    TypeIdentifierPair type_identifiers_1;
    DifferentNumberElements::Final::register_OneElement_type_identifier(type_identifiers_1);

    TypeIdentifierPair type_identifiers_2;
    DifferentNumberElements::Final::register_TwoElements_type_identifier(type_identifiers_2);

    TypeIdentifierPair type_identifiers_3;
    DifferentNumberElements::Final::register_ElementsTwo_type_identifier(type_identifiers_3);

    TypeIdentifierPair type_identifiers_4;
    DifferentNumberElements::Final::register_ThreeElements_type_identifier(type_identifiers_4);

    TypeIdentifierPair type_identifiers_5;
    DifferentNumberElements::Final::register_ElementsThree_type_identifier(type_identifiers_5);

    // Check OneElement assignable-from TwoElements
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    // Check TwoElements assignable-from OneElement
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    // Check OneElement assignable-from ElementsTwo
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    // Check ElementsTwo assignable-from OneElement
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    // Check OneElement assignable-from ThreeElements
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_4.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_4.type_identifier2()));

    // Check ThreeElements assignable-from OneElement
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    // Check OneElement assignable-from ElementsThree
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_5.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_5.type_identifier2()));

    // Check ElementsThree assignable-from OneElement
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    // Check TwoElements assignable-from ElementsTwo
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    // Check ElementsTwo assignable-from TwoElements
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    // Check TwoElements assignable-from ThreeElements
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_4.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_4.type_identifier2()));

    // Check ThreeElements assignable-from TwoElements
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    // Check TwoElements assignable-from ElementsThree
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_5.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_5.type_identifier2()));

    // Check ElementsThree assignable-from TwoElements
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    // Check ElementsTwo assignable-from ThreeElements
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_4.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_4.type_identifier2()));

    // Check ThreeElements assignable-from ElementsTwo
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    // Check ElementsTwo assignable-from ElementsThree
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_5.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_5.type_identifier2()));

    // Check ElementsThree assignable-from ElementsTwo
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    // Check ThreeElements assignable-from ElementsThree
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier1(),
            type_identifiers_5.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier2(),
            type_identifiers_5.type_identifier2()));

    // Check ElementsThree assignable-from ThreeElements
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier1(),
            type_identifiers_4.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier2(),
            type_identifiers_4.type_identifier2()));
}

TEST(TypeAssignabilityTests, DifferentNumberElements_Appendable)
{
    TypeIdentifierPair type_identifiers_1;
    DifferentNumberElements::Appendable::register_OneElement_type_identifier(type_identifiers_1);

    TypeIdentifierPair type_identifiers_2;
    DifferentNumberElements::Appendable::register_TwoElements_type_identifier(type_identifiers_2);

    TypeIdentifierPair type_identifiers_3;
    DifferentNumberElements::Appendable::register_ElementsTwo_type_identifier(type_identifiers_3);

    TypeIdentifierPair type_identifiers_4;
    DifferentNumberElements::Appendable::register_ThreeElements_type_identifier(type_identifiers_4);

    TypeIdentifierPair type_identifiers_5;
    DifferentNumberElements::Appendable::register_ElementsThree_type_identifier(type_identifiers_5);

    DifferentNumberElements::Appendable::OneElementPubSubType one_element_type;
    DifferentNumberElements::Appendable::TwoElementsPubSubType two_elements_type;
    DifferentNumberElements::Appendable::ThreeElementsPubSubType three_elements_type;
    DifferentNumberElements::Appendable::ElementsThreePubSubType elements_three_type;

    // Check OneElement assignable-from TwoElements
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    DifferentNumberElements::Appendable::OneElement to_encode_1;
    to_encode_1.a(69);
    DifferentNumberElements::Appendable::TwoElements to_decode_1;
    to_decode_1.a(1);
    to_decode_1.b(1);
    rtps::SerializedPayload_t payload_1(one_element_type.calculate_serialized_size(&to_encode_1,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.serialize(&to_encode_1, payload_1, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(two_elements_type.deserialize(payload_1, &to_decode_1));
    EXPECT_EQ(to_decode_1.a(), to_encode_1.a());
    EXPECT_EQ(to_decode_1.b(), 0);

    // Check TwoElements assignable-from OneElement
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    DifferentNumberElements::Appendable::TwoElements to_encode_2;
    to_encode_2.a(70);
    to_encode_2.b(60);
    DifferentNumberElements::Appendable::OneElement to_decode_2;
    to_decode_2.a(1);
    rtps::SerializedPayload_t payload_2(two_elements_type.calculate_serialized_size(&to_encode_2,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(two_elements_type.serialize(&to_encode_2, payload_2, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.deserialize(payload_2, &to_decode_2));
    EXPECT_EQ(to_decode_2.a(), to_encode_2.a());

    // Check OneElement assignable-from ElementsTwo
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    // Check ElementsTwo assignable-from OneElement
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    // Check OneElement assignable-from ThreeElements
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_4.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_4.type_identifier2()));

    DifferentNumberElements::Appendable::OneElement to_encode_3;
    to_encode_3.a(-100);
    DifferentNumberElements::Appendable::ThreeElements to_decode_3;
    to_decode_3.a(1);
    to_decode_3.b(1);
    to_decode_3.s("test");
    rtps::SerializedPayload_t payload_3(one_element_type.calculate_serialized_size(&to_encode_3,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.serialize(&to_encode_3, payload_3, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(three_elements_type.deserialize(payload_3, &to_decode_3));
    EXPECT_EQ(to_decode_3.a(), to_encode_3.a());
    EXPECT_EQ(to_decode_3.b(), 0);
    EXPECT_EQ(to_decode_3.s(), "");

    // Check ThreeElements assignable-from OneElement
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    DifferentNumberElements::Appendable::ThreeElements to_encode_4;
    to_encode_4.a(-1);
    to_encode_4.b(1);
    to_encode_4.s("test");
    DifferentNumberElements::Appendable::OneElement to_decode_4;
    to_decode_4.a(-100);
    rtps::SerializedPayload_t payload_4(three_elements_type.calculate_serialized_size(&to_encode_4,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(three_elements_type.serialize(&to_encode_4, payload_4,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.deserialize(payload_4, &to_decode_4));
    EXPECT_EQ(to_decode_4.a(), to_encode_4.a());

    // Check OneElement assignable-from ElementsThree
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_5.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_5.type_identifier2()));

    DifferentNumberElements::Appendable::OneElement to_encode_5;
    to_encode_5.a(-100);
    DifferentNumberElements::Appendable::ElementsThree to_decode_5;
    to_decode_5.a(1);
    to_decode_5.b(1);
    to_decode_5.s("test");
    rtps::SerializedPayload_t payload_5(one_element_type.calculate_serialized_size(&to_encode_5,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.serialize(&to_encode_5, payload_5, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_three_type.deserialize(payload_5, &to_decode_5));
    EXPECT_EQ(to_decode_5.a(), to_encode_5.a());
    EXPECT_EQ(to_decode_5.b(), 0);
    EXPECT_EQ(to_decode_5.s(), "");

    // Check ElementsThree assignable-from OneElement
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    DifferentNumberElements::Appendable::ElementsThree to_encode_6;
    to_encode_6.a(-1);
    to_encode_6.b(1);
    to_encode_6.s("test");
    DifferentNumberElements::Appendable::OneElement to_decode_6;
    to_decode_6.a(-100);
    rtps::SerializedPayload_t payload_6(elements_three_type.calculate_serialized_size(&to_encode_6,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_three_type.serialize(&to_encode_6, payload_6,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.deserialize(payload_6, &to_decode_6));
    EXPECT_EQ(to_decode_6.a(), to_encode_6.a());

    // Check TwoElements assignable-from ElementsTwo
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    // Check ElementsTwo assignable-from TwoElements
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    // Check TwoElements assignable-from ThreeElements
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_4.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_4.type_identifier2()));

    DifferentNumberElements::Appendable::TwoElements to_encode_7;
    to_encode_7.a(-100);
    to_encode_7.b(100);
    DifferentNumberElements::Appendable::ThreeElements to_decode_7;
    to_decode_7.a(1);
    to_decode_7.b(1);
    to_decode_7.s("test");
    rtps::SerializedPayload_t payload_7(two_elements_type.calculate_serialized_size(&to_encode_7,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(two_elements_type.serialize(&to_encode_7, payload_7, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(three_elements_type.deserialize(payload_7, &to_decode_7));
    EXPECT_EQ(to_decode_7.a(), to_encode_7.a());
    EXPECT_EQ(to_decode_7.b(), to_encode_7.b());
    EXPECT_EQ(to_decode_7.s(), "");

    // Check ThreeElements assignable-from TwoElements
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    DifferentNumberElements::Appendable::ThreeElements to_encode_8;
    to_encode_8.a(-1);
    to_encode_8.b(1);
    to_encode_8.s("test");
    DifferentNumberElements::Appendable::TwoElements to_decode_8;
    to_decode_8.a(-10);
    to_decode_8.b(10);
    rtps::SerializedPayload_t payload_8(three_elements_type.calculate_serialized_size(&to_encode_8,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(three_elements_type.serialize(&to_encode_8, payload_8,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(two_elements_type.deserialize(payload_8, &to_decode_8));
    EXPECT_EQ(to_decode_8.a(), to_encode_8.a());
    EXPECT_EQ(to_decode_8.b(), to_encode_8.b());

    // Check TwoElements assignable-from ElementsThree
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_5.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_5.type_identifier2()));

    // Check ElementsThree assignable-from TwoElements
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    // Check ElementsTwo assignable-from ThreeElements
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_4.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_4.type_identifier2()));

    // Check ThreeElements assignable-from ElementsTwo
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    // Check ElementsTwo assignable-from ElementsThree
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_5.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_5.type_identifier2()));

    // Check ElementsThree assignable-from ElementsTwo
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    // Check ThreeElements assignable-from ElementsThree
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier1(),
            type_identifiers_5.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier2(),
            type_identifiers_5.type_identifier2()));

    // Check ElementsThree assignable-from ThreeElements
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier1(),
            type_identifiers_4.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier2(),
            type_identifiers_4.type_identifier2()));
}

TEST(TypeAssignabilityTests, DifferentNumberElements_Mutable)
{
    TypeIdentifierPair type_identifiers_1;
    DifferentNumberElements::Mutable::register_OneElement_type_identifier(type_identifiers_1);

    TypeIdentifierPair type_identifiers_2;
    DifferentNumberElements::Mutable::register_TwoElements_type_identifier(type_identifiers_2);

    TypeIdentifierPair type_identifiers_3;
    DifferentNumberElements::Mutable::register_ElementsTwo_type_identifier(type_identifiers_3);

    TypeIdentifierPair type_identifiers_4;
    DifferentNumberElements::Mutable::register_ThreeElements_type_identifier(type_identifiers_4);

    TypeIdentifierPair type_identifiers_5;
    DifferentNumberElements::Mutable::register_ElementsThree_type_identifier(type_identifiers_5);

    DifferentNumberElements::Mutable::OneElementPubSubType one_element_type;
    DifferentNumberElements::Mutable::TwoElementsPubSubType two_elements_type;
    DifferentNumberElements::Mutable::ElementsTwoPubSubType elements_two_type;
    DifferentNumberElements::Mutable::ThreeElementsPubSubType three_elements_type;
    DifferentNumberElements::Mutable::ElementsThreePubSubType elements_three_type;

    // Check OneElement assignable-from TwoElements
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    DifferentNumberElements::Mutable::OneElement to_encode_1;
    to_encode_1.a(69);
    DifferentNumberElements::Mutable::TwoElements to_decode_1;
    to_decode_1.a(1);
    to_decode_1.b(1);
    rtps::SerializedPayload_t payload_1(one_element_type.calculate_serialized_size(&to_encode_1,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.serialize(&to_encode_1, payload_1, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(two_elements_type.deserialize(payload_1, &to_decode_1));
    EXPECT_EQ(to_decode_1.a(), to_encode_1.a());
    EXPECT_EQ(to_decode_1.b(), 0);

    // Check TwoElements assignable-from OneElement
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    DifferentNumberElements::Mutable::TwoElements to_encode_2;
    to_encode_2.a(70);
    to_encode_2.b(60);
    DifferentNumberElements::Mutable::OneElement to_decode_2;
    to_decode_2.a(1);
    rtps::SerializedPayload_t payload_2(two_elements_type.calculate_serialized_size(&to_encode_2,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(two_elements_type.serialize(&to_encode_2, payload_2, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.deserialize(payload_2, &to_decode_2));
    EXPECT_EQ(to_decode_2.a(), to_encode_2.a());

    // Check OneElement assignable-from ElementsTwo
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    DifferentNumberElements::Mutable::OneElement to_encode_3;
    to_encode_3.a(69);
    DifferentNumberElements::Mutable::ElementsTwo to_decode_3;
    to_decode_3.a(1);
    to_decode_3.b(1);
    rtps::SerializedPayload_t payload_3(one_element_type.calculate_serialized_size(&to_encode_3,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.serialize(&to_encode_3, payload_3, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_two_type.deserialize(payload_3, &to_decode_3));
    EXPECT_EQ(to_decode_3.a(), to_encode_3.a());
    EXPECT_EQ(to_decode_3.b(), 0);

    // Check ElementsTwo assignable-from OneElement
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    DifferentNumberElements::Mutable::ElementsTwo to_encode_4;
    to_encode_4.a(70);
    to_encode_4.b(60);
    DifferentNumberElements::Mutable::OneElement to_decode_4;
    to_decode_4.a(1);
    rtps::SerializedPayload_t payload_4(elements_two_type.calculate_serialized_size(&to_encode_4,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_two_type.serialize(&to_encode_4, payload_4, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.deserialize(payload_4, &to_decode_4));
    EXPECT_EQ(to_decode_4.a(), to_encode_4.a());

    // Check OneElement assignable-from ThreeElements
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_4.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_4.type_identifier2()));

    DifferentNumberElements::Mutable::OneElement to_encode_5;
    to_encode_5.a(60);
    DifferentNumberElements::Mutable::ThreeElements to_decode_5;
    to_decode_5.a(1);
    to_decode_5.b(1);
    to_decode_5.s("test");
    rtps::SerializedPayload_t payload_5(one_element_type.calculate_serialized_size(&to_encode_5,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.serialize(&to_encode_5, payload_5, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(three_elements_type.deserialize(payload_5, &to_decode_5));
    EXPECT_EQ(to_decode_5.a(), to_encode_5.a());
    EXPECT_EQ(to_decode_5.b(), 0);
    EXPECT_EQ(to_decode_5.s(), "");

    // Check ThreeElements assignable-from OneElement
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    DifferentNumberElements::Mutable::ThreeElements to_encode_6;
    to_encode_6.a(50);
    to_encode_6.b(1);
    to_encode_6.s("test");
    DifferentNumberElements::Mutable::OneElement to_decode_6;
    to_decode_6.a(1);
    rtps::SerializedPayload_t payload_6(three_elements_type.calculate_serialized_size(&to_encode_6,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(three_elements_type.serialize(&to_encode_6, payload_6,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.deserialize(payload_6, &to_decode_6));
    EXPECT_EQ(to_decode_6.a(), to_encode_6.a());

    // Check OneElement assignable-from ElementsThree
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_5.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_5.type_identifier2()));

    DifferentNumberElements::Mutable::OneElement to_encode_7;
    to_encode_7.a(59);
    DifferentNumberElements::Mutable::ElementsThree to_decode_7;
    to_decode_7.a(1);
    to_decode_7.b(1);
    to_decode_7.s("test");
    rtps::SerializedPayload_t payload_7(one_element_type.calculate_serialized_size(&to_encode_7,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.serialize(&to_encode_7, payload_7, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_three_type.deserialize(payload_7, &to_decode_7));
    EXPECT_EQ(to_decode_7.a(), to_encode_7.a());
    EXPECT_EQ(to_decode_7.b(), 0);
    EXPECT_EQ(to_decode_7.s(), "");

    // Check ElementsThree assignable-from OneElement
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    DifferentNumberElements::Mutable::ElementsThree to_encode_8;
    to_encode_8.a(50);
    to_encode_8.b(1);
    to_encode_8.s("test");
    DifferentNumberElements::Mutable::OneElement to_decode_8;
    to_decode_8.a(1);
    rtps::SerializedPayload_t payload_8(elements_three_type.calculate_serialized_size(&to_encode_8,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_three_type.serialize(&to_encode_8, payload_8,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(one_element_type.deserialize(payload_8, &to_decode_8));
    EXPECT_EQ(to_decode_8.a(), to_encode_8.a());

    // Check TwoElements assignable-from ElementsTwo
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    DifferentNumberElements::Mutable::TwoElements to_encode_9;
    to_encode_9.a(-1);
    to_encode_9.b(10);
    DifferentNumberElements::Mutable::ElementsTwo to_decode_9;
    to_decode_9.a(1);
    to_decode_9.b(1);
    rtps::SerializedPayload_t payload_9(two_elements_type.calculate_serialized_size(&to_encode_9,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(two_elements_type.serialize(&to_encode_9, payload_9, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_two_type.deserialize(payload_9, &to_decode_9));
    EXPECT_EQ(to_decode_9.a(), to_encode_9.a());
    EXPECT_EQ(to_decode_9.b(), to_encode_9.b());

    // Check ElementsTwo assignable-from TwoElements
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    DifferentNumberElements::Mutable::ElementsTwo to_encode_10;
    to_encode_10.a(49);
    to_encode_10.b(-10);
    DifferentNumberElements::Mutable::TwoElements to_decode_10;
    to_decode_10.a(1);
    to_decode_10.b(1);
    rtps::SerializedPayload_t payload_10(elements_two_type.calculate_serialized_size(&to_encode_10,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_two_type.serialize(&to_encode_10, payload_10,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(two_elements_type.deserialize(payload_10, &to_decode_10));
    EXPECT_EQ(to_decode_10.a(), to_encode_10.a());
    EXPECT_EQ(to_decode_10.b(), to_encode_10.b());

    // Check TwoElements assignable-from ThreeElements
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_4.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_4.type_identifier2()));

    DifferentNumberElements::Mutable::TwoElements to_encode_11;
    to_encode_11.a(-10);
    to_encode_11.b(20);
    DifferentNumberElements::Mutable::ThreeElements to_decode_11;
    to_decode_11.a(1);
    to_decode_11.b(1);
    to_decode_11.s("test");
    rtps::SerializedPayload_t payload_11(two_elements_type.calculate_serialized_size(&to_encode_11,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(two_elements_type.serialize(&to_encode_11, payload_11,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(three_elements_type.deserialize(payload_11, &to_decode_11));
    EXPECT_EQ(to_decode_11.a(), to_encode_11.a());
    EXPECT_EQ(to_decode_11.b(), to_encode_11.b());

    // Check ThreeElements assignable-from TwoElements
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    DifferentNumberElements::Mutable::ThreeElements to_encode_12;
    to_encode_12.a(49);
    to_encode_12.b(-10);
    to_encode_12.s("test");
    DifferentNumberElements::Mutable::TwoElements to_decode_12;
    to_decode_12.a(1);
    to_decode_12.b(1);
    rtps::SerializedPayload_t payload_12(three_elements_type.calculate_serialized_size(&to_encode_12,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(three_elements_type.serialize(&to_encode_12, payload_12,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(two_elements_type.deserialize(payload_12, &to_decode_12));
    EXPECT_EQ(to_decode_12.a(), to_encode_12.a());
    EXPECT_EQ(to_decode_12.b(), to_encode_12.b());

    // Check TwoElements assignable-from ElementsThree
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_5.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_5.type_identifier2()));

    DifferentNumberElements::Mutable::TwoElements to_encode_13;
    to_encode_13.a(-10);
    to_encode_13.b(20);
    DifferentNumberElements::Mutable::ElementsThree to_decode_13;
    to_decode_13.a(1);
    to_decode_13.b(1);
    to_decode_13.s("test");
    rtps::SerializedPayload_t payload_13(two_elements_type.calculate_serialized_size(&to_encode_13,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(two_elements_type.serialize(&to_encode_13, payload_13,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_three_type.deserialize(payload_13, &to_decode_13));
    EXPECT_EQ(to_decode_13.a(), to_encode_13.a());
    EXPECT_EQ(to_decode_13.b(), to_encode_13.b());

    // Check ElementsThree assignable-from TwoElements
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    DifferentNumberElements::Mutable::ElementsThree to_encode_14;
    to_encode_14.a(40);
    to_encode_14.b(-10);
    to_encode_14.s("test");
    DifferentNumberElements::Mutable::TwoElements to_decode_14;
    to_decode_14.a(1);
    to_decode_14.b(1);
    rtps::SerializedPayload_t payload_14(elements_three_type.calculate_serialized_size(&to_encode_14,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_three_type.serialize(&to_encode_14, payload_14,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(two_elements_type.deserialize(payload_14, &to_decode_14));
    EXPECT_EQ(to_decode_14.a(), to_encode_14.a());
    EXPECT_EQ(to_decode_14.b(), to_encode_14.b());

    // Check ElementsTwo assignable-from ThreeElements
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_4.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_4.type_identifier2()));

    DifferentNumberElements::Mutable::ElementsTwo to_encode_15;
    to_encode_15.a(-10);
    to_encode_15.b(20);
    DifferentNumberElements::Mutable::ThreeElements to_decode_15;
    to_decode_15.a(1);
    to_decode_15.b(1);
    to_decode_15.s("test");
    rtps::SerializedPayload_t payload_15(elements_two_type.calculate_serialized_size(&to_encode_15,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_two_type.serialize(&to_encode_15, payload_15,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(three_elements_type.deserialize(payload_15, &to_decode_15));
    EXPECT_EQ(to_decode_15.a(), to_encode_15.a());
    EXPECT_EQ(to_decode_15.b(), to_encode_15.b());

    // Check ThreeElements assignable-from ElementsTwo
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    DifferentNumberElements::Mutable::ThreeElements to_encode_16;
    to_encode_16.a(40);
    to_encode_16.b(-11);
    to_encode_16.s("test");
    DifferentNumberElements::Mutable::ElementsTwo to_decode_16;
    to_decode_16.a(1);
    to_decode_16.b(1);
    rtps::SerializedPayload_t payload_16(three_elements_type.calculate_serialized_size(&to_encode_16,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(three_elements_type.serialize(&to_encode_16, payload_16,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_two_type.deserialize(payload_16, &to_decode_16));
    EXPECT_EQ(to_decode_16.a(), to_encode_16.a());
    EXPECT_EQ(to_decode_16.b(), to_encode_16.b());

    // Check ElementsTwo assignable-from ElementsThree
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_5.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_5.type_identifier2()));

    DifferentNumberElements::Mutable::ElementsTwo to_encode_17;
    to_encode_17.a(-10);
    to_encode_17.b(30);
    DifferentNumberElements::Mutable::ElementsThree to_decode_17;
    to_decode_17.a(1);
    to_decode_17.b(1);
    to_decode_17.s("test");
    rtps::SerializedPayload_t payload_17(elements_two_type.calculate_serialized_size(&to_encode_17,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_two_type.serialize(&to_encode_17, payload_17,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_three_type.deserialize(payload_17, &to_decode_17));
    EXPECT_EQ(to_decode_17.a(), to_encode_17.a());
    EXPECT_EQ(to_decode_17.b(), to_encode_17.b());

    // Check ElementsThree assignable-from ElementsTwo
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    DifferentNumberElements::Mutable::ElementsThree to_encode_18;
    to_encode_18.a(40);
    to_encode_18.b(-11);
    to_encode_18.s("test");
    DifferentNumberElements::Mutable::ElementsTwo to_decode_18;
    to_decode_18.a(1);
    to_decode_18.b(1);
    rtps::SerializedPayload_t payload_18(elements_three_type.calculate_serialized_size(&to_encode_18,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_three_type.serialize(&to_encode_18, payload_18,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_two_type.deserialize(payload_18, &to_decode_18));
    EXPECT_EQ(to_decode_18.a(), to_encode_18.a());
    EXPECT_EQ(to_decode_18.b(), to_encode_18.b());

    // Check ThreeElements assignable-from ElementsThree
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier1(),
            type_identifiers_5.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_4.type_identifier2(),
            type_identifiers_5.type_identifier2()));

    DifferentNumberElements::Mutable::ThreeElements to_encode_19;
    to_encode_19.a(-10);
    to_encode_19.b(30);
    to_encode_19.s("unittest");
    DifferentNumberElements::Mutable::ElementsThree to_decode_19;
    to_decode_19.a(1);
    to_decode_19.b(1);
    to_decode_19.s("test");
    rtps::SerializedPayload_t payload_19(three_elements_type.calculate_serialized_size(&to_encode_19,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(three_elements_type.serialize(&to_encode_19, payload_19,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_three_type.deserialize(payload_19, &to_decode_19));
    EXPECT_EQ(to_decode_19.a(), to_encode_19.a());
    EXPECT_EQ(to_decode_19.b(), to_encode_19.b());
    EXPECT_EQ(to_decode_19.s(), to_encode_19.s());

    // Check ElementsThree assignable-from ThreeElements
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier1(),
            type_identifiers_4.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_5.type_identifier2(),
            type_identifiers_4.type_identifier2()));

    DifferentNumberElements::Mutable::ElementsThree to_encode_20;
    to_encode_20.a(40);
    to_encode_20.b(-11);
    to_encode_20.s("test");
    DifferentNumberElements::Mutable::ThreeElements to_decode_20;
    to_decode_20.a(1);
    to_decode_20.b(1);
    to_decode_20.s("unittest");
    rtps::SerializedPayload_t payload_20(elements_three_type.calculate_serialized_size(&to_encode_20,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(elements_three_type.serialize(&to_encode_20, payload_20,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(three_elements_type.deserialize(payload_20, &to_decode_20));
    EXPECT_EQ(to_decode_20.a(), to_encode_20.a());
    EXPECT_EQ(to_decode_20.b(), to_encode_20.b());
    EXPECT_EQ(to_decode_20.s(), to_encode_20.s());
}

TEST(TypeAssignabilityTests, Primitive_Final)
{
    TypeIdentifierPair type_identifiers_1;
    Primitives::Final::register_Int8Struct_type_identifier(type_identifiers_1);

    TypeIdentifierPair type_identifiers_2;
    Primitives::Final::register_Int8Struct_eq_type_identifier(type_identifiers_2);

    TypeIdentifierPair type_identifiers_3;
    Primitives::Final::register_Int8Struct_ne_type_identifier(type_identifiers_3);

    Primitives::Final::Int8StructPubSubType int8struct_type;
    Primitives::Final::Int8Struct_eqPubSubType int8struct_eq_type;

    // Check Int8Struct assignable-from Int8Struct_eq
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_2.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_2.type_identifier2()));

    Primitives::Final::Int8Struct to_encode_1;
    to_encode_1.a(69);
    Primitives::Final::Int8Struct_eq to_decode_1;
    to_decode_1.a(1);
    rtps::SerializedPayload_t payload_1(int8struct_type.calculate_serialized_size(&to_encode_1,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(int8struct_type.serialize(&to_encode_1, payload_1, DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(int8struct_eq_type.deserialize(payload_1, &to_decode_1));
    EXPECT_EQ(to_decode_1.a(), to_encode_1.a());

    // Check Int8Struct_eq assignable-from Int8Struct
    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_TRUE(TypeAssignability::type_assignable_from(type_identifiers_2.type_identifier2(),
            type_identifiers_1.type_identifier2()));

    Primitives::Final::Int8Struct_eq to_encode_2;
    to_encode_2.a(69);
    Primitives::Final::Int8Struct to_decode_2;
    to_decode_2.a(1);
    rtps::SerializedPayload_t payload_2(int8struct_eq_type.calculate_serialized_size(&to_encode_2,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(int8struct_eq_type.serialize(&to_encode_2, payload_2,
            DataRepresentationId_t::XCDR_DATA_REPRESENTATION));
    ASSERT_TRUE(int8struct_type.deserialize(payload_2, &to_decode_2));
    EXPECT_EQ(to_decode_2.a(), to_encode_2.a());

    // Check Int8Struct assignable-from Int8Struct_ne
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier1(),
            type_identifiers_3.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_1.type_identifier2(),
            type_identifiers_3.type_identifier2()));

    // Check Int8Struct_ne assignable-from Int8Struct
    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier1(),
            type_identifiers_1.type_identifier1()));

    EXPECT_FALSE(TypeAssignability::type_assignable_from(type_identifiers_3.type_identifier2(),
            type_identifiers_1.type_identifier2()));
}

} // xtypes
} // dds
} // fastdds
} // eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
