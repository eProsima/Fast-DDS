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

#include <gtest/gtest.h>

#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <ScopedLogs.hpp>

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::dds;


class IdlParserTests : public ::testing::Test
{
public:

    IdlParserTests() = default;

    ~IdlParserTests()
    {
        Log::Flush();
    }

    virtual void TearDown()
    {
        DynamicDataFactory::delete_instance();
        DynamicTypeBuilderFactory::delete_instance();
    }

};

TEST_F(IdlParserTests, primitives)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> empty_include_paths;

    DynamicTypeBuilder::_ref_type builder1 = factory->create_type_w_document("IDL/primitives.idl", "ShortStruct", empty_include_paths);
    EXPECT_TRUE(builder1);
    DynamicType::_ref_type type1 = builder1->build();
    ASSERT_TRUE(type1);
    DynamicData::_ref_type data1 {DynamicDataFactory::get_instance()->create_data(type1)};
    ASSERT_TRUE(data1);
    int16_t test1 {0};
    EXPECT_EQ(data1->set_int16_value(0, 100), RETCODE_OK);
    EXPECT_EQ(data1->get_int16_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(test1, 100);

    DynamicTypeBuilder::_ref_type builder2 = factory->create_type_w_document("IDL/primitives.idl", "UShortStruct", empty_include_paths);
    EXPECT_TRUE(builder2);
    DynamicType::_ref_type type2 = builder2->build();
    ASSERT_TRUE(type2);
    DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(type2)};
    ASSERT_TRUE(data2);

    DynamicTypeBuilder::_ref_type builder3 = factory->create_type_w_document("IDL/primitives.idl", "LongStruct", empty_include_paths);
    EXPECT_TRUE(builder3);
    DynamicType::_ref_type type3 = builder3->build();
    ASSERT_TRUE(type3);

    DynamicTypeBuilder::_ref_type builder4 = factory->create_type_w_document("IDL/primitives.idl", "ULongStruct", empty_include_paths);
    EXPECT_TRUE(builder4);
    DynamicType::_ref_type type4 = builder4->build();
    ASSERT_TRUE(type4);

    DynamicTypeBuilder::_ref_type builder5 = factory->create_type_w_document("IDL/primitives.idl", "LongLongStruct", empty_include_paths);
    EXPECT_TRUE(builder5);
    DynamicType::_ref_type type5 = builder5->build();
    ASSERT_TRUE(type5);

    DynamicTypeBuilder::_ref_type builder6 = factory->create_type_w_document("IDL/primitives.idl", "ULongLongStruct", empty_include_paths);
    EXPECT_TRUE(builder6);
    DynamicType::_ref_type type6 = builder6->build();
    ASSERT_TRUE(type6);

    DynamicTypeBuilder::_ref_type builder7 = factory->create_type_w_document("IDL/primitives.idl", "FloatStruct", empty_include_paths);
    EXPECT_TRUE(builder7);
    DynamicType::_ref_type type7 = builder7->build();
    ASSERT_TRUE(type7);

    DynamicTypeBuilder::_ref_type builder8 = factory->create_type_w_document("IDL/primitives.idl", "DoubleStruct", empty_include_paths);
    EXPECT_TRUE(builder8);
    DynamicType::_ref_type type8 = builder8->build();
    ASSERT_TRUE(type8);

    DynamicTypeBuilder::_ref_type builder9 = factory->create_type_w_document("IDL/primitives.idl", "LongDoubleStruct", empty_include_paths);
    EXPECT_TRUE(builder9);
    DynamicType::_ref_type type9 = builder9->build();
    ASSERT_TRUE(type9);

    DynamicTypeBuilder::_ref_type builder10 = factory->create_type_w_document("IDL/primitives.idl", "BooleanStruct", empty_include_paths);
    EXPECT_TRUE(builder10);
    DynamicType::_ref_type type10 = builder10->build();
    ASSERT_TRUE(type10);

    DynamicTypeBuilder::_ref_type builder11 = factory->create_type_w_document("IDL/primitives.idl", "OctetStruct", empty_include_paths);
    EXPECT_TRUE(builder11);
    DynamicType::_ref_type type11 = builder11->build();
    ASSERT_TRUE(type11);

    DynamicTypeBuilder::_ref_type builder12 = factory->create_type_w_document("IDL/primitives.idl", "CharStruct", empty_include_paths);
    EXPECT_TRUE(builder12);
    DynamicType::_ref_type type12 = builder12->build();
    ASSERT_TRUE(type12);

    DynamicTypeBuilder::_ref_type builder13 = factory->create_type_w_document("IDL/primitives.idl", "WCharStruct", empty_include_paths);
    EXPECT_TRUE(builder13);
    DynamicType::_ref_type type13 = builder13->build();
    ASSERT_TRUE(type13);

    DynamicTypeBuilder::_ref_type builder14 = factory->create_type_w_document("IDL/primitives.idl", "Int8Struct", empty_include_paths);
    EXPECT_TRUE(builder14);
    DynamicType::_ref_type type14 = builder14->build();
    ASSERT_TRUE(type14);

    DynamicTypeBuilder::_ref_type builder15 = factory->create_type_w_document("IDL/primitives.idl", "Uint8Struct", empty_include_paths);
    EXPECT_TRUE(builder15);
    DynamicType::_ref_type type15 = builder15->build();
    ASSERT_TRUE(type15);

    DynamicTypeBuilder::_ref_type builder16 = factory->create_type_w_document("IDL/primitives.idl", "Int16Struct", empty_include_paths);
    EXPECT_TRUE(builder16);
    DynamicType::_ref_type type16 = builder16->build();
    ASSERT_TRUE(type16);

    DynamicTypeBuilder::_ref_type builder17 = factory->create_type_w_document("IDL/primitives.idl", "Uint16Struct", empty_include_paths);
    EXPECT_TRUE(builder17);
    DynamicType::_ref_type type17 = builder17->build();
    ASSERT_TRUE(type17);

    DynamicTypeBuilder::_ref_type builder18 = factory->create_type_w_document("IDL/primitives.idl", "Int32Struct", empty_include_paths);
    EXPECT_TRUE(builder18);
    DynamicType::_ref_type type18 = builder18->build();
    ASSERT_TRUE(type18);

    DynamicTypeBuilder::_ref_type builder19 = factory->create_type_w_document("IDL/primitives.idl", "Uint32Struct", empty_include_paths);
    EXPECT_TRUE(builder19);
    DynamicType::_ref_type type19 = builder19->build();
    ASSERT_TRUE(type19);

    DynamicTypeBuilder::_ref_type builder20 = factory->create_type_w_document("IDL/primitives.idl", "Int64Struct", empty_include_paths);
    EXPECT_TRUE(builder20);
    DynamicType::_ref_type type20 = builder20->build();
    ASSERT_TRUE(type20);

    DynamicTypeBuilder::_ref_type builder21 = factory->create_type_w_document("IDL/primitives.idl", "Uint64Struct", empty_include_paths);
    EXPECT_TRUE(builder21);
    DynamicType::_ref_type type21 = builder21->build();
    ASSERT_TRUE(type21);
}

TEST_F(IdlParserTests, strings)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> empty_include_paths;

    DynamicTypeBuilder::_ref_type builder1 = factory->create_type_w_document("IDL/strings.idl", "StringStruct", empty_include_paths);
    EXPECT_TRUE(builder1);
    DynamicType::_ref_type type1 = builder1->build();
    ASSERT_TRUE(type1);
    DynamicData::_ref_type data1 {DynamicDataFactory::get_instance()->create_data(type1)};
    ASSERT_TRUE(data1);
    std::string test1;
    EXPECT_EQ(data1->set_string_value(0, "hello"), RETCODE_OK);
    EXPECT_EQ(data1->get_string_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(test1, "hello");

    DynamicTypeBuilder::_ref_type builder2 = factory->create_type_w_document("IDL/strings.idl", "WStringStruct", empty_include_paths);
    EXPECT_TRUE(builder2);
    DynamicType::_ref_type type2 = builder2->build();
    ASSERT_TRUE(type2);

    DynamicTypeBuilder::_ref_type builder3 = factory->create_type_w_document("IDL/strings.idl", "SmallStringStruct", empty_include_paths);
    EXPECT_TRUE(builder3);
    DynamicType::_ref_type type3 = builder3->build();
    ASSERT_TRUE(type3);

    DynamicTypeBuilder::_ref_type builder4 = factory->create_type_w_document("IDL/strings.idl", "SmallWStringStruct", empty_include_paths);
    EXPECT_TRUE(builder4);
    DynamicType::_ref_type type4 = builder4->build();
    ASSERT_TRUE(type4);

    DynamicTypeBuilder::_ref_type builder5 = factory->create_type_w_document("IDL/strings.idl", "LargeStringStruct", empty_include_paths);
    EXPECT_TRUE(builder5);
    DynamicType::_ref_type type5 = builder5->build();
    ASSERT_TRUE(type5);

    DynamicTypeBuilder::_ref_type builder6 = factory->create_type_w_document("IDL/strings.idl", "LargeWStringStruct", empty_include_paths);
    EXPECT_TRUE(builder6);
    DynamicType::_ref_type type6 = builder6->build();
    ASSERT_TRUE(type6);
}

TEST_F(IdlParserTests, structures)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> include_paths;
    include_paths.push_back("IDL/helpers/basic_inner_types.idl");

    DynamicTypeBuilder::_ref_type builder1 = factory->create_type_w_document("IDL/structures.idl", "StructShort", include_paths);
    EXPECT_TRUE(builder1);
    DynamicType::_ref_type type1 = builder1->build();
    ASSERT_TRUE(type1);
    DynamicData::_ref_type data1 {DynamicDataFactory::get_instance()->create_data(type1)};
    ASSERT_TRUE(data1);
    int16_t test1 {0};
    EXPECT_EQ(data1->set_int16_value(0, 100), RETCODE_OK);
    EXPECT_EQ(data1->get_int16_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(test1, 100);

    DynamicTypeBuilder::_ref_type builder2 = factory->create_type_w_document("IDL/structures.idl", "StructUnsignedShort", include_paths);
    EXPECT_TRUE(builder2);
    DynamicType::_ref_type type2 = builder2->build();
    ASSERT_TRUE(type2);

    DynamicTypeBuilder::_ref_type builder3 = factory->create_type_w_document("IDL/structures.idl", "StructLong", include_paths);
    EXPECT_TRUE(builder3);
    DynamicType::_ref_type type3 = builder3->build();
    ASSERT_TRUE(type3);

    DynamicTypeBuilder::_ref_type builder4 = factory->create_type_w_document("IDL/structures.idl", "StructUnsignedLong", include_paths);
    EXPECT_TRUE(builder4);
    DynamicType::_ref_type type4 = builder4->build();
    ASSERT_TRUE(type4);

    DynamicTypeBuilder::_ref_type builder5 = factory->create_type_w_document("IDL/structures.idl", "StructLongLong", include_paths);
    EXPECT_TRUE(builder5);
    DynamicType::_ref_type type5 = builder5->build();
    ASSERT_TRUE(type5);

    DynamicTypeBuilder::_ref_type builder6 = factory->create_type_w_document("IDL/structures.idl", "StructUnsignedLongLong", include_paths);
    EXPECT_TRUE(builder6);
    DynamicType::_ref_type type6 = builder6->build();
    ASSERT_TRUE(type6);

    DynamicTypeBuilder::_ref_type builder7 = factory->create_type_w_document("IDL/structures.idl", "StructFloat", include_paths);
    EXPECT_TRUE(builder7);
    DynamicType::_ref_type type7 = builder7->build();
    ASSERT_TRUE(type7);

    DynamicTypeBuilder::_ref_type builder8 = factory->create_type_w_document("IDL/structures.idl", "StructDouble", include_paths);
    EXPECT_TRUE(builder8);
    DynamicType::_ref_type type8 = builder8->build();
    ASSERT_TRUE(type8);

    DynamicTypeBuilder::_ref_type builder9 = factory->create_type_w_document("IDL/structures.idl", "StructLongDouble", include_paths);
    EXPECT_TRUE(builder9);
    DynamicType::_ref_type type9 = builder9->build();
    ASSERT_TRUE(type9);

    DynamicTypeBuilder::_ref_type builder10 = factory->create_type_w_document("IDL/structures.idl", "StructBoolean", include_paths);
    EXPECT_TRUE(builder10);
    DynamicType::_ref_type type10 = builder10->build();
    ASSERT_TRUE(type10);

    DynamicTypeBuilder::_ref_type builder11 = factory->create_type_w_document("IDL/structures.idl", "StructOctet", include_paths);
    EXPECT_TRUE(builder11);
    DynamicType::_ref_type type11 = builder11->build();
    ASSERT_TRUE(type11);

    DynamicTypeBuilder::_ref_type builder12 = factory->create_type_w_document("IDL/structures.idl", "StructChar8", include_paths);
    EXPECT_TRUE(builder12);
    DynamicType::_ref_type type12 = builder12->build();
    ASSERT_TRUE(type12);

    DynamicTypeBuilder::_ref_type builder13 = factory->create_type_w_document("IDL/structures.idl", "StructChar16", include_paths);
    EXPECT_TRUE(builder13);
    DynamicType::_ref_type type13 = builder13->build();
    ASSERT_TRUE(type13);

    DynamicTypeBuilder::_ref_type builder14 = factory->create_type_w_document("IDL/structures.idl", "StructString", include_paths);
    EXPECT_TRUE(builder14);
    DynamicType::_ref_type type14 = builder14->build();
    ASSERT_TRUE(type14);

    DynamicTypeBuilder::_ref_type builder15 = factory->create_type_w_document("IDL/structures.idl", "StructWString", include_paths);
    EXPECT_TRUE(builder15);
    DynamicType::_ref_type type15 = builder15->build();
    ASSERT_TRUE(type15);

    DynamicTypeBuilder::_ref_type builder16 = factory->create_type_w_document("IDL/structures.idl", "StructBoundedString", include_paths);
    EXPECT_TRUE(builder16);
    DynamicType::_ref_type type16 = builder16->build();
    ASSERT_TRUE(type16);

    DynamicTypeBuilder::_ref_type builder17 = factory->create_type_w_document("IDL/structures.idl", "StructBoundedWString", include_paths);
    EXPECT_TRUE(builder17);
    DynamicType::_ref_type type17 = builder17->build();
    ASSERT_TRUE(type17);

    DynamicTypeBuilder::_ref_type builder18 = factory->create_type_w_document("IDL/structures.idl", "StructEnum", include_paths);
    EXPECT_TRUE(builder18);
    DynamicType::_ref_type type18 = builder18->build();
    ASSERT_TRUE(type18);

    // TODO StructBitMask is skipped since bitmask parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder19 = factory->create_type_w_document("IDL/structures.idl", "StructBitMask", include_paths);
    // EXPECT_TRUE(builder19);
    // DynamicType::_ref_type type19 = builder19->build();
    // ASSERT_TRUE(type19);

    DynamicTypeBuilder::_ref_type builder20 = factory->create_type_w_document("IDL/structures.idl", "StructAlias", include_paths);
    EXPECT_TRUE(builder20);
    DynamicType::_ref_type type20 = builder20->build();
    ASSERT_TRUE(type20);

    DynamicTypeBuilder::_ref_type builder21 = factory->create_type_w_document("IDL/structures.idl", "StructShortArray", include_paths);
    EXPECT_TRUE(builder21);
    DynamicType::_ref_type type21 = builder21->build();
    ASSERT_TRUE(type21);

    // TODO StructSequence is skipped since sequence parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder22 = factory->create_type_w_document("IDL/structures.idl", "StructSequence", include_paths);
    // EXPECT_TRUE(builder22);
    // DynamicType::_ref_type type22 = builder22->build();
    // ASSERT_TRUE(type22);

    // TODO StructMap is skipped since map parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder23 = factory->create_type_w_document("IDL/structures.idl", "StructMap", include_paths);
    // EXPECT_TRUE(builder23);
    // DynamicType::_ref_type type23 = builder23->build();
    // ASSERT_TRUE(type23);

    DynamicTypeBuilder::_ref_type builder24 = factory->create_type_w_document("IDL/structures.idl", "StructUnion", include_paths);
    EXPECT_TRUE(builder24);
    DynamicType::_ref_type type24 = builder24->build();
    ASSERT_TRUE(type24);

    DynamicTypeBuilder::_ref_type builder25 = factory->create_type_w_document("IDL/structures.idl", "StructStructure", include_paths);
    EXPECT_TRUE(builder25);
    DynamicType::_ref_type type25 = builder25->build();
    ASSERT_TRUE(type25);

    // TODO StruStructBitsettMap is skipped since bitset parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder26 = factory->create_type_w_document("IDL/structures.idl", "StructBitset", include_paths);
    // EXPECT_TRUE(builder26);
    // DynamicType::_ref_type type26 = builder26->build();
    // ASSERT_TRUE(type26);

    DynamicTypeBuilder::_ref_type builder27 = factory->create_type_w_document("IDL/structures.idl", "StructEmpty", include_paths);
    EXPECT_TRUE(builder27);
    DynamicType::_ref_type type27 = builder27->build();
    ASSERT_TRUE(type27);

    // TODO Structures is skipped since some members parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder28 = factory->create_type_w_document("IDL/structures.idl", "Structures", include_paths);
    // EXPECT_TRUE(builder28);
    // DynamicType::_ref_type type28 = builder28->build();
    // ASSERT_TRUE(type28);

    // TODO The rest types are skipped since module parsing is not supported.
}

TEST_F(IdlParserTests, aliases)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> include_paths;
    include_paths.push_back("IDL/helpers/basic_inner_types.idl");

    DynamicTypeBuilder::_ref_type builder1 = factory->create_type_w_document("IDL/aliases.idl", "AliasInt16", include_paths);
    EXPECT_TRUE(builder1);
    DynamicType::_ref_type type1 = builder1->build();
    ASSERT_TRUE(type1);
    DynamicData::_ref_type data1 {DynamicDataFactory::get_instance()->create_data(type1)};
    ASSERT_TRUE(data1);
    int16_t test1 {0};
    EXPECT_EQ(data1->set_int16_value(0, 100), RETCODE_OK);
    EXPECT_EQ(data1->get_int16_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(test1, 100);

    DynamicTypeBuilder::_ref_type builder2 = factory->create_type_w_document("IDL/aliases.idl", "AliasUint16", include_paths);
    EXPECT_TRUE(builder2);
    DynamicType::_ref_type type2 = builder2->build();
    ASSERT_TRUE(type2);

    DynamicTypeBuilder::_ref_type builder3 = factory->create_type_w_document("IDL/aliases.idl", "AliasInt32", include_paths);
    EXPECT_TRUE(builder3);
    DynamicType::_ref_type type3 = builder3->build();
    ASSERT_TRUE(type3);

    DynamicTypeBuilder::_ref_type builder4 = factory->create_type_w_document("IDL/aliases.idl", "AliasUInt32", include_paths);
    EXPECT_TRUE(builder4);
    DynamicType::_ref_type type4 = builder4->build();
    ASSERT_TRUE(type4);

    DynamicTypeBuilder::_ref_type builder5 = factory->create_type_w_document("IDL/aliases.idl", "AliasInt64", include_paths);
    EXPECT_TRUE(builder5);
    DynamicType::_ref_type type5 = builder5->build();
    ASSERT_TRUE(type5);

    DynamicTypeBuilder::_ref_type builder6 = factory->create_type_w_document("IDL/aliases.idl", "AliasUInt64", include_paths);
    EXPECT_TRUE(builder6);
    DynamicType::_ref_type type6 = builder6->build();
    ASSERT_TRUE(type6);

    DynamicTypeBuilder::_ref_type builder7 = factory->create_type_w_document("IDL/aliases.idl", "AliasFloat32", include_paths);
    EXPECT_TRUE(builder7);
    DynamicType::_ref_type type7 = builder7->build();
    ASSERT_TRUE(type7);

    DynamicTypeBuilder::_ref_type builder8 = factory->create_type_w_document("IDL/aliases.idl", "AliasFloat64", include_paths);
    EXPECT_TRUE(builder8);
    DynamicType::_ref_type type8 = builder8->build();
    ASSERT_TRUE(type8);

    DynamicTypeBuilder::_ref_type builder9 = factory->create_type_w_document("IDL/aliases.idl", "AliasFloat128", include_paths);
    EXPECT_TRUE(builder9);
    DynamicType::_ref_type type9 = builder9->build();
    ASSERT_TRUE(type9);

    DynamicTypeBuilder::_ref_type builder10 = factory->create_type_w_document("IDL/aliases.idl", "AliasBool", include_paths);
    EXPECT_TRUE(builder10);
    DynamicType::_ref_type type10 = builder10->build();
    ASSERT_TRUE(type10);

    DynamicTypeBuilder::_ref_type builder11 = factory->create_type_w_document("IDL/aliases.idl", "AliasOctet", include_paths);
    EXPECT_TRUE(builder11);
    DynamicType::_ref_type type11 = builder11->build();
    ASSERT_TRUE(type11);

    DynamicTypeBuilder::_ref_type builder12 = factory->create_type_w_document("IDL/aliases.idl", "AliasChar8", include_paths);
    EXPECT_TRUE(builder12);
    DynamicType::_ref_type type12 = builder12->build();
    ASSERT_TRUE(type12);

    DynamicTypeBuilder::_ref_type builder13 = factory->create_type_w_document("IDL/aliases.idl", "AliasChar16", include_paths);
    EXPECT_TRUE(builder13);
    DynamicType::_ref_type type13 = builder13->build();
    ASSERT_TRUE(type13);

    DynamicTypeBuilder::_ref_type builder14 = factory->create_type_w_document("IDL/aliases.idl", "AliasString8", include_paths);
    EXPECT_TRUE(builder14);
    DynamicType::_ref_type type14 = builder14->build();
    ASSERT_TRUE(type14);

    DynamicTypeBuilder::_ref_type builder15 = factory->create_type_w_document("IDL/aliases.idl", "AliasString16", include_paths);
    EXPECT_TRUE(builder15);
    DynamicType::_ref_type type15 = builder15->build();
    ASSERT_TRUE(type15);

    DynamicTypeBuilder::_ref_type builder16 = factory->create_type_w_document("IDL/aliases.idl", "AliasEnum", include_paths);
    EXPECT_TRUE(builder16);
    DynamicType::_ref_type type16 = builder16->build();
    ASSERT_TRUE(type16);

    // TODO AliasBitmask is skipped since bitmask parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder17 = factory->create_type_w_document("IDL/aliases.idl", "AliasBitmask", include_paths);
    // EXPECT_TRUE(builder17);
    // DynamicType::_ref_type type17 = builder17->build();
    // ASSERT_TRUE(type17);

    DynamicTypeBuilder::_ref_type builder18 = factory->create_type_w_document("IDL/aliases.idl", "AliasAlias", include_paths);
    EXPECT_TRUE(builder18);
    DynamicType::_ref_type type18 = builder18->build();
    ASSERT_TRUE(type18);

    DynamicTypeBuilder::_ref_type builder19 = factory->create_type_w_document("IDL/aliases.idl", "AliasArray", include_paths);
    EXPECT_TRUE(builder19);
    DynamicType::_ref_type type19 = builder19->build();
    ASSERT_TRUE(type19);

    DynamicTypeBuilder::_ref_type builder20 = factory->create_type_w_document("IDL/aliases.idl", "AliasMultiArray", include_paths);
    EXPECT_TRUE(builder20);
    DynamicType::_ref_type type20 = builder20->build();
    ASSERT_TRUE(type20);

    // TODO AliasSequence is skipped since sequence parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder21 = factory->create_type_w_document("IDL/aliases.idl", "AliasSequence", include_paths);
    // EXPECT_TRUE(builder21);
    // DynamicType::_ref_type type21 = builder21->build();
    // ASSERT_TRUE(type21);

    // TODO AliasMap is skipped since map parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder22 = factory->create_type_w_document("IDL/aliases.idl", "AliasMap", include_paths);
    // EXPECT_TRUE(builder22);
    // DynamicType::_ref_type type22 = builder22->build();
    // ASSERT_TRUE(type22);

    DynamicTypeBuilder::_ref_type builder23 = factory->create_type_w_document("IDL/aliases.idl", "AliasUnion", include_paths);
    EXPECT_TRUE(builder23);
    DynamicType::_ref_type type23 = builder23->build();
    ASSERT_TRUE(type23);

    DynamicTypeBuilder::_ref_type builder24 = factory->create_type_w_document("IDL/aliases.idl", "AliasStruct", include_paths);
    EXPECT_TRUE(builder24);
    DynamicType::_ref_type type24 = builder24->build();
    ASSERT_TRUE(type24);

    // TODO AliasBitset is skipped since bitset parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder25 = factory->create_type_w_document("IDL/aliases.idl", "AliasBitset", include_paths);
    // EXPECT_TRUE(builder25);
    // DynamicType::_ref_type type25 = builder25->build();
    // ASSERT_TRUE(type25);
}

TEST_F(IdlParserTests, arrays)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> include_paths;
    include_paths.push_back("IDL/helpers/basic_inner_types.idl");

    DynamicTypeBuilder::_ref_type builder1 = factory->create_type_w_document("IDL/arrays.idl", "ArrayShort", include_paths);
    EXPECT_TRUE(builder1);
    DynamicType::_ref_type type1 = builder1->build();
    ASSERT_TRUE(type1);
    DynamicData::_ref_type data1 {DynamicDataFactory::get_instance()->create_data(type1)};
    ASSERT_TRUE(data1);
    Int16Seq value1 = {1, 2, 3, 4, 5, -6, -7, -8, -9, -10};
    Int16Seq test_value1;
    EXPECT_EQ(data1->set_int16_values(data1->get_member_id_by_name("var_array_short"), value1), RETCODE_OK);
    EXPECT_EQ(data1->get_int16_values(test_value1, data1->get_member_id_by_name("var_array_short")), RETCODE_OK);
    EXPECT_EQ(value1, test_value1);

    DynamicTypeBuilder::_ref_type builder2 = factory->create_type_w_document("IDL/arrays.idl", "ArrayUShort", include_paths);
    EXPECT_TRUE(builder2);
    DynamicType::_ref_type type2 = builder2->build();
    ASSERT_TRUE(type2);

    DynamicTypeBuilder::_ref_type builder3 = factory->create_type_w_document("IDL/arrays.idl", "ArrayLong", include_paths);
    EXPECT_TRUE(builder3);
    DynamicType::_ref_type type3 = builder3->build();
    ASSERT_TRUE(type3);

    DynamicTypeBuilder::_ref_type builder4 = factory->create_type_w_document("IDL/arrays.idl", "ArrayULong", include_paths);
    EXPECT_TRUE(builder4);
    DynamicType::_ref_type type4 = builder4->build();
    ASSERT_TRUE(type4);

    DynamicTypeBuilder::_ref_type builder5 = factory->create_type_w_document("IDL/arrays.idl", "ArrayLongLong", include_paths);
    EXPECT_TRUE(builder5);
    DynamicType::_ref_type type5 = builder5->build();
    ASSERT_TRUE(type5);

    DynamicTypeBuilder::_ref_type builder6 = factory->create_type_w_document("IDL/arrays.idl", "ArrayULongLong", include_paths);
    EXPECT_TRUE(builder6);
    DynamicType::_ref_type type6 = builder6->build();
    ASSERT_TRUE(type6);

    DynamicTypeBuilder::_ref_type builder7 = factory->create_type_w_document("IDL/arrays.idl", "ArrayFloat", include_paths);
    EXPECT_TRUE(builder7);
    DynamicType::_ref_type type7 = builder7->build();
    ASSERT_TRUE(type7);

    DynamicTypeBuilder::_ref_type builder8 = factory->create_type_w_document("IDL/arrays.idl", "ArrayDouble", include_paths);
    EXPECT_TRUE(builder8);
    DynamicType::_ref_type type8 = builder8->build();
    ASSERT_TRUE(type8);

    DynamicTypeBuilder::_ref_type builder9 = factory->create_type_w_document("IDL/arrays.idl", "ArrayLongDouble", include_paths);
    EXPECT_TRUE(builder9);
    DynamicType::_ref_type type9 = builder9->build();
    ASSERT_TRUE(type9);

    DynamicTypeBuilder::_ref_type builder10 = factory->create_type_w_document("IDL/arrays.idl", "ArrayBoolean", include_paths);
    EXPECT_TRUE(builder10);
    DynamicType::_ref_type type10 = builder10->build();
    ASSERT_TRUE(type10);

    DynamicTypeBuilder::_ref_type builder11 = factory->create_type_w_document("IDL/arrays.idl", "ArrayOctet", include_paths);
    EXPECT_TRUE(builder11);
    DynamicType::_ref_type type11 = builder11->build();
    ASSERT_TRUE(type11);

    DynamicTypeBuilder::_ref_type builder12 = factory->create_type_w_document("IDL/arrays.idl", "ArrayUInt8", include_paths);
    EXPECT_TRUE(builder12);
    DynamicType::_ref_type type12 = builder12->build();
    ASSERT_TRUE(type12);

    DynamicTypeBuilder::_ref_type builder13 = factory->create_type_w_document("IDL/arrays.idl", "ArrayChar", include_paths);
    EXPECT_TRUE(builder13);
    DynamicType::_ref_type type13 = builder13->build();
    ASSERT_TRUE(type13);

    DynamicTypeBuilder::_ref_type builder14 = factory->create_type_w_document("IDL/arrays.idl", "ArrayWChar", include_paths);
    EXPECT_TRUE(builder14);
    DynamicType::_ref_type type14 = builder14->build();
    ASSERT_TRUE(type14);

    DynamicTypeBuilder::_ref_type builder15 = factory->create_type_w_document("IDL/arrays.idl", "ArrayString", include_paths);
    EXPECT_TRUE(builder15);
    DynamicType::_ref_type type15 = builder15->build();
    ASSERT_TRUE(type15);

    DynamicTypeBuilder::_ref_type builder16 = factory->create_type_w_document("IDL/arrays.idl", "ArrayWString", include_paths);
    EXPECT_TRUE(builder16);
    DynamicType::_ref_type type16 = builder16->build();
    ASSERT_TRUE(type16);

    DynamicTypeBuilder::_ref_type builder17 = factory->create_type_w_document("IDL/arrays.idl", "ArrayBoundedString", include_paths);
    EXPECT_TRUE(builder17);
    DynamicType::_ref_type type17 = builder17->build();
    ASSERT_TRUE(type17);

    DynamicTypeBuilder::_ref_type builder18 = factory->create_type_w_document("IDL/arrays.idl", "ArrayBoundedWString", include_paths);
    EXPECT_TRUE(builder18);
    DynamicType::_ref_type type18 = builder18->build();
    ASSERT_TRUE(type18);

    DynamicTypeBuilder::_ref_type builder19 = factory->create_type_w_document("IDL/arrays.idl", "ArrayEnum", include_paths);
    EXPECT_TRUE(builder19);
    DynamicType::_ref_type type19 = builder19->build();
    ASSERT_TRUE(type19);

    // TODO ArrayBitMask is skipped since bitmask parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder20 = factory->create_type_w_document("IDL/arrays.idl", "ArrayBitMask", include_paths);
    // EXPECT_TRUE(builder20);
    // DynamicType::_ref_type type20 = builder20->build();
    // ASSERT_TRUE(type20);

    DynamicTypeBuilder::_ref_type builder21 = factory->create_type_w_document("IDL/arrays.idl", "ArrayAlias", include_paths);
    EXPECT_TRUE(builder21);
    DynamicType::_ref_type type21 = builder21->build();
    ASSERT_TRUE(type21);

    DynamicTypeBuilder::_ref_type builder22 = factory->create_type_w_document("IDL/arrays.idl", "ArrayShortArray", include_paths);
    EXPECT_TRUE(builder22);
    DynamicType::_ref_type type22 = builder22->build();
    ASSERT_TRUE(type22);

    // TODO ArraySequence is skipped since sequence parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder23 = factory->create_type_w_document("IDL/arrays.idl", "ArraySequence", include_paths);
    // EXPECT_TRUE(builder23);
    // DynamicType::_ref_type type23 = builder23->build();
    // ASSERT_TRUE(type23);

    // TODO ArrayMap is skipped since map parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder24 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMap", include_paths);
    // EXPECT_TRUE(builder24);
    // DynamicType::_ref_type type24 = builder24->build();
    // ASSERT_TRUE(type24);

    DynamicTypeBuilder::_ref_type builder25 = factory->create_type_w_document("IDL/arrays.idl", "ArrayUnion", include_paths);
    EXPECT_TRUE(builder25);
    DynamicType::_ref_type type25 = builder25->build();
    ASSERT_TRUE(type25);

    DynamicTypeBuilder::_ref_type builder26 = factory->create_type_w_document("IDL/arrays.idl", "ArrayStructure", include_paths);
    EXPECT_TRUE(builder26);
    DynamicType::_ref_type type26 = builder26->build();
    ASSERT_TRUE(type26);

    // TODO ArrayBitset is skipped since bitset parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder27 = factory->create_type_w_document("IDL/arrays.idl", "ArrayBitset", include_paths);
    // EXPECT_TRUE(builder27);
    // DynamicType::_ref_type type27 = builder27->build();
    // ASSERT_TRUE(type27);

    DynamicTypeBuilder::_ref_type builder28 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionShort", include_paths);
    EXPECT_TRUE(builder28);
    DynamicType::_ref_type type28 = builder28->build();
    ASSERT_TRUE(type28);

    DynamicTypeBuilder::_ref_type builder29 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionUShort", include_paths);
    EXPECT_TRUE(builder29);
    DynamicType::_ref_type type29 = builder29->build();
    ASSERT_TRUE(type29);

    DynamicTypeBuilder::_ref_type builder30 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionLong", include_paths);
    EXPECT_TRUE(builder30);
    DynamicType::_ref_type type30 = builder30->build();
    ASSERT_TRUE(type30);

    DynamicTypeBuilder::_ref_type builder31 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionULong", include_paths);
    EXPECT_TRUE(builder31);
    DynamicType::_ref_type type31 = builder31->build();
    ASSERT_TRUE(type31);

    DynamicTypeBuilder::_ref_type builder32 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionLongLong", include_paths);
    EXPECT_TRUE(builder32);
    DynamicType::_ref_type type32 = builder32->build();
    ASSERT_TRUE(type32);

    DynamicTypeBuilder::_ref_type builder33 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionULongLong", include_paths);
    EXPECT_TRUE(builder33);
    DynamicType::_ref_type type33 = builder33->build();
    ASSERT_TRUE(type33);

    DynamicTypeBuilder::_ref_type builder34 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionFloat", include_paths);
    EXPECT_TRUE(builder34);
    DynamicType::_ref_type type34 = builder34->build();
    ASSERT_TRUE(type34);

    DynamicTypeBuilder::_ref_type builder35 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionDouble", include_paths);
    EXPECT_TRUE(builder35);
    DynamicType::_ref_type type35 = builder35->build();
    ASSERT_TRUE(type35);

    DynamicTypeBuilder::_ref_type builder36 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionLongDouble", include_paths);
    EXPECT_TRUE(builder36);
    DynamicType::_ref_type type36 = builder36->build();
    ASSERT_TRUE(type36);

    DynamicTypeBuilder::_ref_type builder37 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionBoolean", include_paths);
    EXPECT_TRUE(builder37);
    DynamicType::_ref_type type37 = builder37->build();
    ASSERT_TRUE(type37);

    DynamicTypeBuilder::_ref_type builder38 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionOctet", include_paths);
    EXPECT_TRUE(builder38);
    DynamicType::_ref_type type38 = builder38->build();
    ASSERT_TRUE(type38);

    DynamicTypeBuilder::_ref_type builder39 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionChar", include_paths);
    EXPECT_TRUE(builder39);
    DynamicType::_ref_type type39 = builder39->build();
    ASSERT_TRUE(type39);

    DynamicTypeBuilder::_ref_type builder40 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionWChar", include_paths);
    EXPECT_TRUE(builder40);
    DynamicType::_ref_type type40 = builder40->build();
    ASSERT_TRUE(type40);

    DynamicTypeBuilder::_ref_type builder41 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionString", include_paths);
    EXPECT_TRUE(builder41);
    DynamicType::_ref_type type41 = builder41->build();
    ASSERT_TRUE(type41);

    DynamicTypeBuilder::_ref_type builder42 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionWString", include_paths);
    EXPECT_TRUE(builder42);
    DynamicType::_ref_type type42 = builder42->build();
    ASSERT_TRUE(type42);

    DynamicTypeBuilder::_ref_type builder43 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionBoundedString", include_paths);
    EXPECT_TRUE(builder43);
    DynamicType::_ref_type type43 = builder43->build();
    ASSERT_TRUE(type43);

    DynamicTypeBuilder::_ref_type builder44 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionBoundedWString", include_paths);
    EXPECT_TRUE(builder44);
    DynamicType::_ref_type type44 = builder44->build();
    ASSERT_TRUE(type44);

    DynamicTypeBuilder::_ref_type builder45 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionEnum", include_paths);
    EXPECT_TRUE(builder45);
    DynamicType::_ref_type type45 = builder45->build();
    ASSERT_TRUE(type45);

    // TODO ArrayMultiDimensionBitMask is skipped since bitmask parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder46 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionBitMask", include_paths);
    // EXPECT_TRUE(builder46);
    // DynamicType::_ref_type type46 = builder46->build();
    // ASSERT_TRUE(type46);

    DynamicTypeBuilder::_ref_type builder47 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionAlias", include_paths);
    EXPECT_TRUE(builder47);
    DynamicType::_ref_type type47 = builder47->build();
    ASSERT_TRUE(type47);

    // TODO ArrayMultiDimensionSequence is skipped since sequence parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder48 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionSequence", include_paths);
    // EXPECT_TRUE(builder48);
    // DynamicType::_ref_type type48 = builder48->build();
    // ASSERT_TRUE(type48);

    // TODO ArrayMultiDimensionMap is skipped since map parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder49 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionMap", include_paths);
    // EXPECT_TRUE(builder49);
    // DynamicType::_ref_type type49 = builder49->build();
    // ASSERT_TRUE(type49);

    DynamicTypeBuilder::_ref_type builder50 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionUnion", include_paths);
    EXPECT_TRUE(builder50);
    DynamicType::_ref_type type50 = builder50->build();
    ASSERT_TRUE(type50);

    DynamicTypeBuilder::_ref_type builder51 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionStructure", include_paths);
    EXPECT_TRUE(builder51);
    DynamicType::_ref_type type51 = builder51->build();
    ASSERT_TRUE(type51);

    // TODO ArrayMultiDimensionBitset is skipped since bitset parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder52 = factory->create_type_w_document("IDL/arrays.idl", "ArrayMultiDimensionBitset", include_paths);
    // EXPECT_TRUE(builder52);
    // DynamicType::_ref_type type52 = builder52->build();
    // ASSERT_TRUE(type52);

    DynamicTypeBuilder::_ref_type builder53 = factory->create_type_w_document("IDL/arrays.idl", "ArraySingleDimensionLiteralsShort", include_paths);
    EXPECT_TRUE(builder53);
    DynamicType::_ref_type type53 = builder53->build();
    ASSERT_TRUE(type53);

    DynamicTypeBuilder::_ref_type builder54 = factory->create_type_w_document("IDL/arrays.idl", "ArraySingleDimensionLiteralsUnsignedShort", include_paths);
    EXPECT_TRUE(builder54);
    DynamicType::_ref_type type54 = builder54->build();
    ASSERT_TRUE(type54);

    DynamicTypeBuilder::_ref_type builder55 = factory->create_type_w_document("IDL/arrays.idl", "ArraySingleDimensionLiteralsLong", include_paths);
    EXPECT_TRUE(builder55);
    DynamicType::_ref_type type55 = builder55->build();
    ASSERT_TRUE(type55);
}

int main(
        int argc,
        char** argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
