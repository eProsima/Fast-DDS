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
#include "IdlParserTests.hpp"

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

    void SetUp() override
    {
        DynamicTypeBuilderFactory::get_instance()->set_preprocessor(IDL_PARSER_PREPROCESSOR_EXEC);
    }

    void TearDown() override
    {
        DynamicDataFactory::delete_instance();
        DynamicTypeBuilderFactory::delete_instance();
    }

};

TEST_F(IdlParserTests, primitives)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> empty_include_paths;

    DynamicTypeBuilder::_ref_type builder1 = factory->create_type_w_uri("IDL/primitives.idl", "ShortStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder1);
    DynamicType::_ref_type type1 = builder1->build();
    ASSERT_TRUE(type1);
    DynamicData::_ref_type data1 {DynamicDataFactory::get_instance()->create_data(type1)};
    ASSERT_TRUE(data1);
    int16_t test1 {0};
    EXPECT_EQ(data1->set_int16_value(0, 100), RETCODE_OK);
    EXPECT_EQ(data1->get_int16_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(test1, 100);

    DynamicTypeBuilder::_ref_type builder2 = factory->create_type_w_uri("IDL/primitives.idl", "UShortStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder2);
    DynamicType::_ref_type type2 = builder2->build();
    ASSERT_TRUE(type2);
    DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(type2)};
    ASSERT_TRUE(data2);

    DynamicTypeBuilder::_ref_type builder3 = factory->create_type_w_uri("IDL/primitives.idl", "LongStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder3);
    DynamicType::_ref_type type3 = builder3->build();
    ASSERT_TRUE(type3);

    DynamicTypeBuilder::_ref_type builder4 = factory->create_type_w_uri("IDL/primitives.idl", "ULongStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder4);
    DynamicType::_ref_type type4 = builder4->build();
    ASSERT_TRUE(type4);

    DynamicTypeBuilder::_ref_type builder5 = factory->create_type_w_uri("IDL/primitives.idl", "LongLongStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder5);
    DynamicType::_ref_type type5 = builder5->build();
    ASSERT_TRUE(type5);

    DynamicTypeBuilder::_ref_type builder6 = factory->create_type_w_uri("IDL/primitives.idl", "ULongLongStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder6);
    DynamicType::_ref_type type6 = builder6->build();
    ASSERT_TRUE(type6);

    DynamicTypeBuilder::_ref_type builder7 = factory->create_type_w_uri("IDL/primitives.idl", "FloatStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder7);
    DynamicType::_ref_type type7 = builder7->build();
    ASSERT_TRUE(type7);

    DynamicTypeBuilder::_ref_type builder8 = factory->create_type_w_uri("IDL/primitives.idl", "DoubleStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder8);
    DynamicType::_ref_type type8 = builder8->build();
    ASSERT_TRUE(type8);

    DynamicTypeBuilder::_ref_type builder9 = factory->create_type_w_uri("IDL/primitives.idl", "LongDoubleStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder9);
    DynamicType::_ref_type type9 = builder9->build();
    ASSERT_TRUE(type9);

    DynamicTypeBuilder::_ref_type builder10 = factory->create_type_w_uri("IDL/primitives.idl", "BooleanStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder10);
    DynamicType::_ref_type type10 = builder10->build();
    ASSERT_TRUE(type10);

    DynamicTypeBuilder::_ref_type builder11 = factory->create_type_w_uri("IDL/primitives.idl", "OctetStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder11);
    DynamicType::_ref_type type11 = builder11->build();
    ASSERT_TRUE(type11);

    DynamicTypeBuilder::_ref_type builder12 = factory->create_type_w_uri("IDL/primitives.idl", "CharStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder12);
    DynamicType::_ref_type type12 = builder12->build();
    ASSERT_TRUE(type12);

    DynamicTypeBuilder::_ref_type builder13 = factory->create_type_w_uri("IDL/primitives.idl", "WCharStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder13);
    DynamicType::_ref_type type13 = builder13->build();
    ASSERT_TRUE(type13);

    DynamicTypeBuilder::_ref_type builder14 = factory->create_type_w_uri("IDL/primitives.idl", "Int8Struct",
                    empty_include_paths);
    EXPECT_TRUE(builder14);
    DynamicType::_ref_type type14 = builder14->build();
    ASSERT_TRUE(type14);

    DynamicTypeBuilder::_ref_type builder15 = factory->create_type_w_uri("IDL/primitives.idl", "Uint8Struct",
                    empty_include_paths);
    EXPECT_TRUE(builder15);
    DynamicType::_ref_type type15 = builder15->build();
    ASSERT_TRUE(type15);

    DynamicTypeBuilder::_ref_type builder16 = factory->create_type_w_uri("IDL/primitives.idl", "Int16Struct",
                    empty_include_paths);
    EXPECT_TRUE(builder16);
    DynamicType::_ref_type type16 = builder16->build();
    ASSERT_TRUE(type16);

    DynamicTypeBuilder::_ref_type builder17 = factory->create_type_w_uri("IDL/primitives.idl", "Uint16Struct",
                    empty_include_paths);
    EXPECT_TRUE(builder17);
    DynamicType::_ref_type type17 = builder17->build();
    ASSERT_TRUE(type17);

    DynamicTypeBuilder::_ref_type builder18 = factory->create_type_w_uri("IDL/primitives.idl", "Int32Struct",
                    empty_include_paths);
    EXPECT_TRUE(builder18);
    DynamicType::_ref_type type18 = builder18->build();
    ASSERT_TRUE(type18);

    DynamicTypeBuilder::_ref_type builder19 = factory->create_type_w_uri("IDL/primitives.idl", "Uint32Struct",
                    empty_include_paths);
    EXPECT_TRUE(builder19);
    DynamicType::_ref_type type19 = builder19->build();
    ASSERT_TRUE(type19);

    DynamicTypeBuilder::_ref_type builder20 = factory->create_type_w_uri("IDL/primitives.idl", "Int64Struct",
                    empty_include_paths);
    EXPECT_TRUE(builder20);
    DynamicType::_ref_type type20 = builder20->build();
    ASSERT_TRUE(type20);

    DynamicTypeBuilder::_ref_type builder21 = factory->create_type_w_uri("IDL/primitives.idl", "Uint64Struct",
                    empty_include_paths);
    EXPECT_TRUE(builder21);
    DynamicType::_ref_type type21 = builder21->build();
    ASSERT_TRUE(type21);
}

TEST_F(IdlParserTests, strings)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> empty_include_paths;

    DynamicTypeBuilder::_ref_type builder1 = factory->create_type_w_uri("IDL/strings.idl", "StringStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder1);
    DynamicType::_ref_type type1 = builder1->build();
    ASSERT_TRUE(type1);
    DynamicData::_ref_type data1 {DynamicDataFactory::get_instance()->create_data(type1)};
    ASSERT_TRUE(data1);
    std::string test1;
    EXPECT_EQ(data1->set_string_value(0, "hello"), RETCODE_OK);
    EXPECT_EQ(data1->get_string_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(test1, "hello");

    DynamicTypeBuilder::_ref_type builder2 = factory->create_type_w_uri("IDL/strings.idl", "WStringStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder2);
    DynamicType::_ref_type type2 = builder2->build();
    ASSERT_TRUE(type2);

    DynamicTypeBuilder::_ref_type builder3 = factory->create_type_w_uri("IDL/strings.idl", "SmallStringStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder3);
    DynamicType::_ref_type type3 = builder3->build();
    ASSERT_TRUE(type3);

    DynamicTypeBuilder::_ref_type builder4 = factory->create_type_w_uri("IDL/strings.idl", "SmallWStringStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder4);
    DynamicType::_ref_type type4 = builder4->build();
    ASSERT_TRUE(type4);

    DynamicTypeBuilder::_ref_type builder5 = factory->create_type_w_uri("IDL/strings.idl", "LargeStringStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder5);
    DynamicType::_ref_type type5 = builder5->build();
    ASSERT_TRUE(type5);

    DynamicTypeBuilder::_ref_type builder6 = factory->create_type_w_uri("IDL/strings.idl", "LargeWStringStruct",
                    empty_include_paths);
    EXPECT_TRUE(builder6);
    DynamicType::_ref_type type6 = builder6->build();
    ASSERT_TRUE(type6);
}

TEST_F(IdlParserTests, structures)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> include_paths;
    include_paths.push_back("IDL/helpers");

    DynamicTypeBuilder::_ref_type builder1 = factory->create_type_w_uri("IDL/structures.idl", "StructShort",
                    include_paths);
    EXPECT_TRUE(builder1);
    DynamicType::_ref_type type1 = builder1->build();
    ASSERT_TRUE(type1);
    DynamicData::_ref_type data1 {DynamicDataFactory::get_instance()->create_data(type1)};
    ASSERT_TRUE(data1);
    int16_t test1 {0};
    EXPECT_EQ(data1->set_int16_value(0, 100), RETCODE_OK);
    EXPECT_EQ(data1->get_int16_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(test1, 100);

    DynamicTypeBuilder::_ref_type builder2 = factory->create_type_w_uri("IDL/structures.idl", "StructUnsignedShort",
                    include_paths);
    EXPECT_TRUE(builder2);
    DynamicType::_ref_type type2 = builder2->build();
    ASSERT_TRUE(type2);

    DynamicTypeBuilder::_ref_type builder3 = factory->create_type_w_uri("IDL/structures.idl", "StructLong",
                    include_paths);
    EXPECT_TRUE(builder3);
    DynamicType::_ref_type type3 = builder3->build();
    ASSERT_TRUE(type3);

    DynamicTypeBuilder::_ref_type builder4 = factory->create_type_w_uri("IDL/structures.idl", "StructUnsignedLong",
                    include_paths);
    EXPECT_TRUE(builder4);
    DynamicType::_ref_type type4 = builder4->build();
    ASSERT_TRUE(type4);

    DynamicTypeBuilder::_ref_type builder5 = factory->create_type_w_uri("IDL/structures.idl", "StructLongLong",
                    include_paths);
    EXPECT_TRUE(builder5);
    DynamicType::_ref_type type5 = builder5->build();
    ASSERT_TRUE(type5);

    DynamicTypeBuilder::_ref_type builder6 = factory->create_type_w_uri("IDL/structures.idl", "StructUnsignedLongLong",
                    include_paths);
    EXPECT_TRUE(builder6);
    DynamicType::_ref_type type6 = builder6->build();
    ASSERT_TRUE(type6);

    DynamicTypeBuilder::_ref_type builder7 = factory->create_type_w_uri("IDL/structures.idl", "StructFloat",
                    include_paths);
    EXPECT_TRUE(builder7);
    DynamicType::_ref_type type7 = builder7->build();
    ASSERT_TRUE(type7);

    DynamicTypeBuilder::_ref_type builder8 = factory->create_type_w_uri("IDL/structures.idl", "StructDouble",
                    include_paths);
    EXPECT_TRUE(builder8);
    DynamicType::_ref_type type8 = builder8->build();
    ASSERT_TRUE(type8);

    DynamicTypeBuilder::_ref_type builder9 = factory->create_type_w_uri("IDL/structures.idl", "StructLongDouble",
                    include_paths);
    EXPECT_TRUE(builder9);
    DynamicType::_ref_type type9 = builder9->build();
    ASSERT_TRUE(type9);

    DynamicTypeBuilder::_ref_type builder10 = factory->create_type_w_uri("IDL/structures.idl", "StructBoolean",
                    include_paths);
    EXPECT_TRUE(builder10);
    DynamicType::_ref_type type10 = builder10->build();
    ASSERT_TRUE(type10);

    DynamicTypeBuilder::_ref_type builder11 = factory->create_type_w_uri("IDL/structures.idl", "StructOctet",
                    include_paths);
    EXPECT_TRUE(builder11);
    DynamicType::_ref_type type11 = builder11->build();
    ASSERT_TRUE(type11);

    DynamicTypeBuilder::_ref_type builder12 = factory->create_type_w_uri("IDL/structures.idl", "StructChar8",
                    include_paths);
    EXPECT_TRUE(builder12);
    DynamicType::_ref_type type12 = builder12->build();
    ASSERT_TRUE(type12);

    DynamicTypeBuilder::_ref_type builder13 = factory->create_type_w_uri("IDL/structures.idl", "StructChar16",
                    include_paths);
    EXPECT_TRUE(builder13);
    DynamicType::_ref_type type13 = builder13->build();
    ASSERT_TRUE(type13);

    DynamicTypeBuilder::_ref_type builder14 = factory->create_type_w_uri("IDL/structures.idl", "StructString",
                    include_paths);
    EXPECT_TRUE(builder14);
    DynamicType::_ref_type type14 = builder14->build();
    ASSERT_TRUE(type14);

    DynamicTypeBuilder::_ref_type builder15 = factory->create_type_w_uri("IDL/structures.idl", "StructWString",
                    include_paths);
    EXPECT_TRUE(builder15);
    DynamicType::_ref_type type15 = builder15->build();
    ASSERT_TRUE(type15);

    DynamicTypeBuilder::_ref_type builder16 = factory->create_type_w_uri("IDL/structures.idl", "StructBoundedString",
                    include_paths);
    EXPECT_TRUE(builder16);
    DynamicType::_ref_type type16 = builder16->build();
    ASSERT_TRUE(type16);

    DynamicTypeBuilder::_ref_type builder17 = factory->create_type_w_uri("IDL/structures.idl", "StructBoundedWString",
                    include_paths);
    EXPECT_TRUE(builder17);
    DynamicType::_ref_type type17 = builder17->build();
    ASSERT_TRUE(type17);

    DynamicTypeBuilder::_ref_type builder18 = factory->create_type_w_uri("IDL/structures.idl", "StructEnum",
                    include_paths);
    EXPECT_TRUE(builder18);
    DynamicType::_ref_type type18 = builder18->build();
    ASSERT_TRUE(type18);

    // TODO StructBitMask is skipped since bitmask parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder19 = factory->create_type_w_uri("IDL/structures.idl", "StructBitMask", include_paths);
    // EXPECT_TRUE(builder19);
    // DynamicType::_ref_type type19 = builder19->build();
    // ASSERT_TRUE(type19);

    DynamicTypeBuilder::_ref_type builder20 = factory->create_type_w_uri("IDL/structures.idl", "StructAlias",
                    include_paths);
    EXPECT_TRUE(builder20);
    DynamicType::_ref_type type20 = builder20->build();
    ASSERT_TRUE(type20);

    DynamicTypeBuilder::_ref_type builder21 = factory->create_type_w_uri("IDL/structures.idl", "StructShortArray",
                    include_paths);
    EXPECT_TRUE(builder21);
    DynamicType::_ref_type type21 = builder21->build();
    ASSERT_TRUE(type21);

    DynamicTypeBuilder::_ref_type builder22 = factory->create_type_w_uri("IDL/structures.idl", "StructSequence", include_paths);
    EXPECT_TRUE(builder22);
    DynamicType::_ref_type type22 = builder22->build();
    ASSERT_TRUE(type22);

    // TODO StructMap is skipped since map parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder23 = factory->create_type_w_uri("IDL/structures.idl", "StructMap", include_paths);
    // EXPECT_TRUE(builder23);
    // DynamicType::_ref_type type23 = builder23->build();
    // ASSERT_TRUE(type23);

    DynamicTypeBuilder::_ref_type builder24 = factory->create_type_w_uri("IDL/structures.idl", "StructUnion",
                    include_paths);
    EXPECT_TRUE(builder24);
    DynamicType::_ref_type type24 = builder24->build();
    ASSERT_TRUE(type24);

    DynamicTypeBuilder::_ref_type builder25 = factory->create_type_w_uri("IDL/structures.idl", "StructStructure",
                    include_paths);
    EXPECT_TRUE(builder25);
    DynamicType::_ref_type type25 = builder25->build();
    ASSERT_TRUE(type25);

    // TODO StruStructBitsettMap is skipped since bitset parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder26 = factory->create_type_w_uri("IDL/structures.idl", "StructBitset", include_paths);
    // EXPECT_TRUE(builder26);
    // DynamicType::_ref_type type26 = builder26->build();
    // ASSERT_TRUE(type26);

    DynamicTypeBuilder::_ref_type builder27 = factory->create_type_w_uri("IDL/structures.idl", "StructEmpty",
                    include_paths);
    EXPECT_TRUE(builder27);
    DynamicType::_ref_type type27 = builder27->build();
    ASSERT_TRUE(type27);

    // TODO Structures is skipped since some members parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder28 = factory->create_type_w_uri("IDL/structures.idl", "Structures", include_paths);
    // EXPECT_TRUE(builder28);
    // DynamicType::_ref_type type28 = builder28->build();
    // ASSERT_TRUE(type28);

    // TODO The rest types are skipped since module parsing is not supported.
}

TEST_F(IdlParserTests, aliases)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> include_paths;
    include_paths.push_back("IDL/helpers");

    DynamicTypeBuilder::_ref_type builder1 = factory->create_type_w_uri("IDL/aliases.idl", "AliasInt16", include_paths);
    EXPECT_TRUE(builder1);
    DynamicType::_ref_type type1 = builder1->build();
    ASSERT_TRUE(type1);
    DynamicData::_ref_type data1 {DynamicDataFactory::get_instance()->create_data(type1)};
    ASSERT_TRUE(data1);
    int16_t test1 {0};
    EXPECT_EQ(data1->set_int16_value(0, 100), RETCODE_OK);
    EXPECT_EQ(data1->get_int16_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(test1, 100);

    DynamicTypeBuilder::_ref_type builder2 =
            factory->create_type_w_uri("IDL/aliases.idl", "AliasUint16", include_paths);
    EXPECT_TRUE(builder2);
    DynamicType::_ref_type type2 = builder2->build();
    ASSERT_TRUE(type2);

    DynamicTypeBuilder::_ref_type builder3 = factory->create_type_w_uri("IDL/aliases.idl", "AliasInt32", include_paths);
    EXPECT_TRUE(builder3);
    DynamicType::_ref_type type3 = builder3->build();
    ASSERT_TRUE(type3);

    DynamicTypeBuilder::_ref_type builder4 =
            factory->create_type_w_uri("IDL/aliases.idl", "AliasUInt32", include_paths);
    EXPECT_TRUE(builder4);
    DynamicType::_ref_type type4 = builder4->build();
    ASSERT_TRUE(type4);

    DynamicTypeBuilder::_ref_type builder5 = factory->create_type_w_uri("IDL/aliases.idl", "AliasInt64", include_paths);
    EXPECT_TRUE(builder5);
    DynamicType::_ref_type type5 = builder5->build();
    ASSERT_TRUE(type5);

    DynamicTypeBuilder::_ref_type builder6 =
            factory->create_type_w_uri("IDL/aliases.idl", "AliasUInt64", include_paths);
    EXPECT_TRUE(builder6);
    DynamicType::_ref_type type6 = builder6->build();
    ASSERT_TRUE(type6);

    DynamicTypeBuilder::_ref_type builder7 =
            factory->create_type_w_uri("IDL/aliases.idl", "AliasFloat32", include_paths);
    EXPECT_TRUE(builder7);
    DynamicType::_ref_type type7 = builder7->build();
    ASSERT_TRUE(type7);

    DynamicTypeBuilder::_ref_type builder8 =
            factory->create_type_w_uri("IDL/aliases.idl", "AliasFloat64", include_paths);
    EXPECT_TRUE(builder8);
    DynamicType::_ref_type type8 = builder8->build();
    ASSERT_TRUE(type8);

    DynamicTypeBuilder::_ref_type builder9 = factory->create_type_w_uri("IDL/aliases.idl", "AliasFloat128",
                    include_paths);
    EXPECT_TRUE(builder9);
    DynamicType::_ref_type type9 = builder9->build();
    ASSERT_TRUE(type9);

    DynamicTypeBuilder::_ref_type builder10 = factory->create_type_w_uri("IDL/aliases.idl", "AliasBool", include_paths);
    EXPECT_TRUE(builder10);
    DynamicType::_ref_type type10 = builder10->build();
    ASSERT_TRUE(type10);

    DynamicTypeBuilder::_ref_type builder11 =
            factory->create_type_w_uri("IDL/aliases.idl", "AliasOctet", include_paths);
    EXPECT_TRUE(builder11);
    DynamicType::_ref_type type11 = builder11->build();
    ASSERT_TRUE(type11);

    DynamicTypeBuilder::_ref_type builder12 =
            factory->create_type_w_uri("IDL/aliases.idl", "AliasChar8", include_paths);
    EXPECT_TRUE(builder12);
    DynamicType::_ref_type type12 = builder12->build();
    ASSERT_TRUE(type12);

    DynamicTypeBuilder::_ref_type builder13 =
            factory->create_type_w_uri("IDL/aliases.idl", "AliasChar16", include_paths);
    EXPECT_TRUE(builder13);
    DynamicType::_ref_type type13 = builder13->build();
    ASSERT_TRUE(type13);

    DynamicTypeBuilder::_ref_type builder14 = factory->create_type_w_uri("IDL/aliases.idl", "AliasString8",
                    include_paths);
    EXPECT_TRUE(builder14);
    DynamicType::_ref_type type14 = builder14->build();
    ASSERT_TRUE(type14);

    DynamicTypeBuilder::_ref_type builder15 = factory->create_type_w_uri("IDL/aliases.idl", "AliasString16",
                    include_paths);
    EXPECT_TRUE(builder15);
    DynamicType::_ref_type type15 = builder15->build();
    ASSERT_TRUE(type15);

    DynamicTypeBuilder::_ref_type builder16 = factory->create_type_w_uri("IDL/aliases.idl", "AliasEnum", include_paths);
    EXPECT_TRUE(builder16);
    DynamicType::_ref_type type16 = builder16->build();
    ASSERT_TRUE(type16);

    // TODO AliasBitmask is skipped since bitmask parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder17 = factory->create_type_w_uri("IDL/aliases.idl", "AliasBitmask", include_paths);
    // EXPECT_TRUE(builder17);
    // DynamicType::_ref_type type17 = builder17->build();
    // ASSERT_TRUE(type17);

    DynamicTypeBuilder::_ref_type builder18 =
            factory->create_type_w_uri("IDL/aliases.idl", "AliasAlias", include_paths);
    EXPECT_TRUE(builder18);
    DynamicType::_ref_type type18 = builder18->build();
    ASSERT_TRUE(type18);

    DynamicTypeBuilder::_ref_type builder19 =
            factory->create_type_w_uri("IDL/aliases.idl", "AliasArray", include_paths);
    EXPECT_TRUE(builder19);
    DynamicType::_ref_type type19 = builder19->build();
    ASSERT_TRUE(type19);

    DynamicTypeBuilder::_ref_type builder20 = factory->create_type_w_uri("IDL/aliases.idl", "AliasMultiArray",
                    include_paths);
    EXPECT_TRUE(builder20);
    DynamicType::_ref_type type20 = builder20->build();
    ASSERT_TRUE(type20);

    DynamicTypeBuilder::_ref_type builder21 = factory->create_type_w_uri("IDL/aliases.idl", "AliasSequence", include_paths);
    EXPECT_TRUE(builder21);
    DynamicType::_ref_type type21 = builder21->build();
    ASSERT_TRUE(type21);

    // TODO AliasMap is skipped since map parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder22 = factory->create_type_w_uri("IDL/aliases.idl", "AliasMap", include_paths);
    // EXPECT_TRUE(builder22);
    // DynamicType::_ref_type type22 = builder22->build();
    // ASSERT_TRUE(type22);

    DynamicTypeBuilder::_ref_type builder23 =
            factory->create_type_w_uri("IDL/aliases.idl", "AliasUnion", include_paths);
    EXPECT_TRUE(builder23);
    DynamicType::_ref_type type23 = builder23->build();
    ASSERT_TRUE(type23);

    DynamicTypeBuilder::_ref_type builder24 =
            factory->create_type_w_uri("IDL/aliases.idl", "AliasStruct", include_paths);
    EXPECT_TRUE(builder24);
    DynamicType::_ref_type type24 = builder24->build();
    ASSERT_TRUE(type24);

    // TODO AliasBitset is skipped since bitset parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder25 = factory->create_type_w_uri("IDL/aliases.idl", "AliasBitset", include_paths);
    // EXPECT_TRUE(builder25);
    // DynamicType::_ref_type type25 = builder25->build();
    // ASSERT_TRUE(type25);
}

TEST_F(IdlParserTests, arrays)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> include_paths;
    include_paths.push_back("IDL/helpers");

    DynamicTypeBuilder::_ref_type builder1 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayShort", include_paths);
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

    DynamicTypeBuilder::_ref_type builder2 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayUShort", include_paths);
    EXPECT_TRUE(builder2);
    DynamicType::_ref_type type2 = builder2->build();
    ASSERT_TRUE(type2);

    DynamicTypeBuilder::_ref_type builder3 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayLong", include_paths);
    EXPECT_TRUE(builder3);
    DynamicType::_ref_type type3 = builder3->build();
    ASSERT_TRUE(type3);

    DynamicTypeBuilder::_ref_type builder4 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayULong", include_paths);
    EXPECT_TRUE(builder4);
    DynamicType::_ref_type type4 = builder4->build();
    ASSERT_TRUE(type4);

    DynamicTypeBuilder::_ref_type builder5 =
            factory->create_type_w_uri("IDL/arrays.idl", "ArrayLongLong", include_paths);
    EXPECT_TRUE(builder5);
    DynamicType::_ref_type type5 = builder5->build();
    ASSERT_TRUE(type5);

    DynamicTypeBuilder::_ref_type builder6 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayULongLong",
                    include_paths);
    EXPECT_TRUE(builder6);
    DynamicType::_ref_type type6 = builder6->build();
    ASSERT_TRUE(type6);

    DynamicTypeBuilder::_ref_type builder7 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayFloat", include_paths);
    EXPECT_TRUE(builder7);
    DynamicType::_ref_type type7 = builder7->build();
    ASSERT_TRUE(type7);

    DynamicTypeBuilder::_ref_type builder8 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayDouble", include_paths);
    EXPECT_TRUE(builder8);
    DynamicType::_ref_type type8 = builder8->build();
    ASSERT_TRUE(type8);

    DynamicTypeBuilder::_ref_type builder9 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayLongDouble",
                    include_paths);
    EXPECT_TRUE(builder9);
    DynamicType::_ref_type type9 = builder9->build();
    ASSERT_TRUE(type9);

    DynamicTypeBuilder::_ref_type builder10 =
            factory->create_type_w_uri("IDL/arrays.idl", "ArrayBoolean", include_paths);
    EXPECT_TRUE(builder10);
    DynamicType::_ref_type type10 = builder10->build();
    ASSERT_TRUE(type10);

    DynamicTypeBuilder::_ref_type builder11 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayOctet", include_paths);
    EXPECT_TRUE(builder11);
    DynamicType::_ref_type type11 = builder11->build();
    ASSERT_TRUE(type11);

    DynamicTypeBuilder::_ref_type builder12 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayUInt8", include_paths);
    EXPECT_TRUE(builder12);
    DynamicType::_ref_type type12 = builder12->build();
    ASSERT_TRUE(type12);

    DynamicTypeBuilder::_ref_type builder13 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayChar", include_paths);
    EXPECT_TRUE(builder13);
    DynamicType::_ref_type type13 = builder13->build();
    ASSERT_TRUE(type13);

    DynamicTypeBuilder::_ref_type builder14 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayWChar", include_paths);
    EXPECT_TRUE(builder14);
    DynamicType::_ref_type type14 = builder14->build();
    ASSERT_TRUE(type14);

    DynamicTypeBuilder::_ref_type builder15 =
            factory->create_type_w_uri("IDL/arrays.idl", "ArrayString", include_paths);
    EXPECT_TRUE(builder15);
    DynamicType::_ref_type type15 = builder15->build();
    ASSERT_TRUE(type15);

    DynamicTypeBuilder::_ref_type builder16 =
            factory->create_type_w_uri("IDL/arrays.idl", "ArrayWString", include_paths);
    EXPECT_TRUE(builder16);
    DynamicType::_ref_type type16 = builder16->build();
    ASSERT_TRUE(type16);

    DynamicTypeBuilder::_ref_type builder17 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayBoundedString",
                    include_paths);
    EXPECT_TRUE(builder17);
    DynamicType::_ref_type type17 = builder17->build();
    ASSERT_TRUE(type17);

    DynamicTypeBuilder::_ref_type builder18 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayBoundedWString",
                    include_paths);
    EXPECT_TRUE(builder18);
    DynamicType::_ref_type type18 = builder18->build();
    ASSERT_TRUE(type18);

    DynamicTypeBuilder::_ref_type builder19 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayEnum", include_paths);
    EXPECT_TRUE(builder19);
    DynamicType::_ref_type type19 = builder19->build();
    ASSERT_TRUE(type19);

    // TODO ArrayBitMask is skipped since bitmask parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder20 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayBitMask", include_paths);
    // EXPECT_TRUE(builder20);
    // DynamicType::_ref_type type20 = builder20->build();
    // ASSERT_TRUE(type20);

    DynamicTypeBuilder::_ref_type builder21 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayAlias", include_paths);
    EXPECT_TRUE(builder21);
    DynamicType::_ref_type type21 = builder21->build();
    ASSERT_TRUE(type21);

    DynamicTypeBuilder::_ref_type builder22 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayShortArray",
                    include_paths);
    EXPECT_TRUE(builder22);
    DynamicType::_ref_type type22 = builder22->build();
    ASSERT_TRUE(type22);

    DynamicTypeBuilder::_ref_type builder23 = factory->create_type_w_uri("IDL/arrays.idl", "ArraySequence", include_paths);
    EXPECT_TRUE(builder23);
    DynamicType::_ref_type type23 = builder23->build();
    ASSERT_TRUE(type23);

    // TODO ArrayMap is skipped since map parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder24 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMap", include_paths);
    // EXPECT_TRUE(builder24);
    // DynamicType::_ref_type type24 = builder24->build();
    // ASSERT_TRUE(type24);

    DynamicTypeBuilder::_ref_type builder25 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayUnion", include_paths);
    EXPECT_TRUE(builder25);
    DynamicType::_ref_type type25 = builder25->build();
    ASSERT_TRUE(type25);

    DynamicTypeBuilder::_ref_type builder26 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayStructure",
                    include_paths);
    EXPECT_TRUE(builder26);
    DynamicType::_ref_type type26 = builder26->build();
    ASSERT_TRUE(type26);

    // TODO ArrayBitset is skipped since bitset parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder27 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayBitset", include_paths);
    // EXPECT_TRUE(builder27);
    // DynamicType::_ref_type type27 = builder27->build();
    // ASSERT_TRUE(type27);

    DynamicTypeBuilder::_ref_type builder28 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionShort",
                    include_paths);
    EXPECT_TRUE(builder28);
    DynamicType::_ref_type type28 = builder28->build();
    ASSERT_TRUE(type28);

    DynamicTypeBuilder::_ref_type builder29 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionUShort",
                    include_paths);
    EXPECT_TRUE(builder29);
    DynamicType::_ref_type type29 = builder29->build();
    ASSERT_TRUE(type29);

    DynamicTypeBuilder::_ref_type builder30 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionLong",
                    include_paths);
    EXPECT_TRUE(builder30);
    DynamicType::_ref_type type30 = builder30->build();
    ASSERT_TRUE(type30);

    DynamicTypeBuilder::_ref_type builder31 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionULong",
                    include_paths);
    EXPECT_TRUE(builder31);
    DynamicType::_ref_type type31 = builder31->build();
    ASSERT_TRUE(type31);

    DynamicTypeBuilder::_ref_type builder32 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLongLong", include_paths);
    EXPECT_TRUE(builder32);
    DynamicType::_ref_type type32 = builder32->build();
    ASSERT_TRUE(type32);

    DynamicTypeBuilder::_ref_type builder33 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionULongLong", include_paths);
    EXPECT_TRUE(builder33);
    DynamicType::_ref_type type33 = builder33->build();
    ASSERT_TRUE(type33);

    DynamicTypeBuilder::_ref_type builder34 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionFloat",
                    include_paths);
    EXPECT_TRUE(builder34);
    DynamicType::_ref_type type34 = builder34->build();
    ASSERT_TRUE(type34);

    DynamicTypeBuilder::_ref_type builder35 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionDouble",
                    include_paths);
    EXPECT_TRUE(builder35);
    DynamicType::_ref_type type35 = builder35->build();
    ASSERT_TRUE(type35);

    DynamicTypeBuilder::_ref_type builder36 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLongDouble",
                    include_paths);
    EXPECT_TRUE(builder36);
    DynamicType::_ref_type type36 = builder36->build();
    ASSERT_TRUE(type36);

    DynamicTypeBuilder::_ref_type builder37 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionBoolean",
                    include_paths);
    EXPECT_TRUE(builder37);
    DynamicType::_ref_type type37 = builder37->build();
    ASSERT_TRUE(type37);

    DynamicTypeBuilder::_ref_type builder38 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionOctet",
                    include_paths);
    EXPECT_TRUE(builder38);
    DynamicType::_ref_type type38 = builder38->build();
    ASSERT_TRUE(type38);

    DynamicTypeBuilder::_ref_type builder39 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionChar",
                    include_paths);
    EXPECT_TRUE(builder39);
    DynamicType::_ref_type type39 = builder39->build();
    ASSERT_TRUE(type39);

    DynamicTypeBuilder::_ref_type builder40 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionWChar",
                    include_paths);
    EXPECT_TRUE(builder40);
    DynamicType::_ref_type type40 = builder40->build();
    ASSERT_TRUE(type40);

    DynamicTypeBuilder::_ref_type builder41 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionString",
                    include_paths);
    EXPECT_TRUE(builder41);
    DynamicType::_ref_type type41 = builder41->build();
    ASSERT_TRUE(type41);

    DynamicTypeBuilder::_ref_type builder42 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionWString",
                    include_paths);
    EXPECT_TRUE(builder42);
    DynamicType::_ref_type type42 = builder42->build();
    ASSERT_TRUE(type42);

    DynamicTypeBuilder::_ref_type builder43 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionBoundedString",
                    include_paths);
    EXPECT_TRUE(builder43);
    DynamicType::_ref_type type43 = builder43->build();
    ASSERT_TRUE(type43);

    DynamicTypeBuilder::_ref_type builder44 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionBoundedWString",
                    include_paths);
    EXPECT_TRUE(builder44);
    DynamicType::_ref_type type44 = builder44->build();
    ASSERT_TRUE(type44);

    DynamicTypeBuilder::_ref_type builder45 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionEnum",
                    include_paths);
    EXPECT_TRUE(builder45);
    DynamicType::_ref_type type45 = builder45->build();
    ASSERT_TRUE(type45);

    // TODO ArrayMultiDimensionBitMask is skipped since bitmask parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder46 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionBitMask", include_paths);
    // EXPECT_TRUE(builder46);
    // DynamicType::_ref_type type46 = builder46->build();
    // ASSERT_TRUE(type46);

    DynamicTypeBuilder::_ref_type builder47 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionAlias",
                    include_paths);
    EXPECT_TRUE(builder47);
    DynamicType::_ref_type type47 = builder47->build();
    ASSERT_TRUE(type47);

    DynamicTypeBuilder::_ref_type builder48 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionSequence", include_paths);
    EXPECT_TRUE(builder48);
    DynamicType::_ref_type type48 = builder48->build();
    ASSERT_TRUE(type48);

    // TODO ArrayMultiDimensionMap is skipped since map parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder49 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionMap", include_paths);
    // EXPECT_TRUE(builder49);
    // DynamicType::_ref_type type49 = builder49->build();
    // ASSERT_TRUE(type49);

    DynamicTypeBuilder::_ref_type builder50 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionUnion",
                    include_paths);
    EXPECT_TRUE(builder50);
    DynamicType::_ref_type type50 = builder50->build();
    ASSERT_TRUE(type50);

    DynamicTypeBuilder::_ref_type builder51 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionStructure", include_paths);
    EXPECT_TRUE(builder51);
    DynamicType::_ref_type type51 = builder51->build();
    ASSERT_TRUE(type51);

    // TODO ArrayMultiDimensionBitset is skipped since bitset parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder52 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionBitset", include_paths);
    // EXPECT_TRUE(builder52);
    // DynamicType::_ref_type type52 = builder52->build();
    // ASSERT_TRUE(type52);

    DynamicTypeBuilder::_ref_type builder53 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsShort",
                    include_paths);
    EXPECT_TRUE(builder53);
    DynamicType::_ref_type type53 = builder53->build();
    ASSERT_TRUE(type53);

    DynamicTypeBuilder::_ref_type builder54 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsUnsignedShort",
                    include_paths);
    EXPECT_TRUE(builder54);
    DynamicType::_ref_type type54 = builder54->build();
    ASSERT_TRUE(type54);

    DynamicTypeBuilder::_ref_type builder55 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsLong",
                    include_paths);
    EXPECT_TRUE(builder55);
    DynamicType::_ref_type type55 = builder55->build();
    ASSERT_TRUE(type55);

    DynamicTypeBuilder::_ref_type builder56 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsUnsignedLong",
                    include_paths);
    EXPECT_TRUE(builder56);
    DynamicType::_ref_type type56 = builder56->build();
    ASSERT_TRUE(type56);

    DynamicTypeBuilder::_ref_type builder57 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsLongLong",
                    include_paths);
    EXPECT_TRUE(builder57);
    DynamicType::_ref_type type57 = builder57->build();
    ASSERT_TRUE(type57);

    DynamicTypeBuilder::_ref_type builder58 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsUnsignedLongLong",
                    include_paths);
    EXPECT_TRUE(builder58);
    DynamicType::_ref_type type58 = builder58->build();
    ASSERT_TRUE(type58);

    DynamicTypeBuilder::_ref_type builder59 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsFloat",
                    include_paths);
    EXPECT_TRUE(builder59);
    DynamicType::_ref_type type59 = builder59->build();
    ASSERT_TRUE(type59);

    DynamicTypeBuilder::_ref_type builder60 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsDouble",
                    include_paths);
    EXPECT_TRUE(builder60);
    DynamicType::_ref_type type60 = builder60->build();
    ASSERT_TRUE(type60);

    DynamicTypeBuilder::_ref_type builder61 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsLongDouble",
                    include_paths);
    EXPECT_TRUE(builder61);
    DynamicType::_ref_type type61 = builder61->build();
    ASSERT_TRUE(type61);

    DynamicTypeBuilder::_ref_type builder62 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsBoolean",
                    include_paths);
    EXPECT_TRUE(builder62);
    DynamicType::_ref_type type62 = builder62->build();
    ASSERT_TRUE(type62);

    DynamicTypeBuilder::_ref_type builder63 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsOctet",
                    include_paths);
    EXPECT_TRUE(builder63);
    DynamicType::_ref_type type63 = builder63->build();
    ASSERT_TRUE(type63);

    DynamicTypeBuilder::_ref_type builder64 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsChar",
                    include_paths);
    EXPECT_TRUE(builder64);
    DynamicType::_ref_type type64 = builder64->build();
    ASSERT_TRUE(type64);

    DynamicTypeBuilder::_ref_type builder65 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsWChar",
                    include_paths);
    EXPECT_TRUE(builder65);
    DynamicType::_ref_type type65 = builder65->build();
    ASSERT_TRUE(type65);

    DynamicTypeBuilder::_ref_type builder66 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsString",
                    include_paths);
    EXPECT_TRUE(builder66);
    DynamicType::_ref_type type66 = builder66->build();
    ASSERT_TRUE(type66);

    DynamicTypeBuilder::_ref_type builder67 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsWString",
                    include_paths);
    EXPECT_TRUE(builder67);
    DynamicType::_ref_type type67 = builder67->build();
    ASSERT_TRUE(type67);

    DynamicTypeBuilder::_ref_type builder68 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsBoundedString",
                    include_paths);
    EXPECT_TRUE(builder68);
    DynamicType::_ref_type type68 = builder68->build();
    ASSERT_TRUE(type68);

    DynamicTypeBuilder::_ref_type builder69 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsBoundedWString",
                    include_paths);
    EXPECT_TRUE(builder69);
    DynamicType::_ref_type type69 = builder69->build();
    ASSERT_TRUE(type69);

    DynamicTypeBuilder::_ref_type builder70 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsEnum",
                    include_paths);
    EXPECT_TRUE(builder70);
    DynamicType::_ref_type type70 = builder70->build();
    ASSERT_TRUE(type70);

    // TODO ArraySingleDimensionLiteralsBitMask is skipped since bitmask parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder71 = factory->create_type_w_uri("IDL/arrays.idl", "ArraySingleDimensionLiteralsBitMask", include_paths);
    // EXPECT_TRUE(builder71);
    // DynamicType::_ref_type type71 = builder71->build();
    // ASSERT_TRUE(type71);

    DynamicTypeBuilder::_ref_type builder72 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsAlias",
                    include_paths);
    EXPECT_TRUE(builder72);
    DynamicType::_ref_type type72 = builder72->build();
    ASSERT_TRUE(type72);

    DynamicTypeBuilder::_ref_type builder73 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsShortArray",
                    include_paths);
    EXPECT_TRUE(builder73);
    DynamicType::_ref_type type73 = builder73->build();
    ASSERT_TRUE(type73);

    DynamicTypeBuilder::_ref_type builder74 = factory->create_type_w_uri("IDL/arrays.idl", "ArraySingleDimensionLiteralsSequence", include_paths);
    EXPECT_TRUE(builder74);
    DynamicType::_ref_type type74 = builder74->build();
    ASSERT_TRUE(type74);

    // TODO ArraySingleDimensionLiteralsMap is skipped since map parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder75 = factory->create_type_w_uri("IDL/arrays.idl", "ArraySingleDimensionLiteralsMap", include_paths);
    // EXPECT_TRUE(builder75);
    // DynamicType::_ref_type type75 = builder75->build();
    // ASSERT_TRUE(type75);

    DynamicTypeBuilder::_ref_type builder76 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsUnion",
                    include_paths);
    EXPECT_TRUE(builder76);
    DynamicType::_ref_type type76 = builder76->build();
    ASSERT_TRUE(type76);

    DynamicTypeBuilder::_ref_type builder77 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArraySingleDimensionLiteralsStructure",
                    include_paths);
    EXPECT_TRUE(builder77);
    DynamicType::_ref_type type77 = builder77->build();
    ASSERT_TRUE(type77);

    // TODO ArraySingleDimensionLiteralsBitset is skipped since bitset parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder78 = factory->create_type_w_uri("IDL/arrays.idl", "ArraySingleDimensionLiteralsBitset", include_paths);
    // EXPECT_TRUE(builder78);
    // DynamicType::_ref_type type78 = builder78->build();
    // ASSERT_TRUE(type78);

    DynamicTypeBuilder::_ref_type builder79 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsShort",
                    include_paths);
    EXPECT_TRUE(builder79);
    DynamicType::_ref_type type79 = builder79->build();
    ASSERT_TRUE(type79);

    DynamicTypeBuilder::_ref_type builder80 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsUShort",
                    include_paths);
    EXPECT_TRUE(builder80);
    DynamicType::_ref_type type80 = builder80->build();
    ASSERT_TRUE(type80);

    DynamicTypeBuilder::_ref_type builder81 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsLong",
                    include_paths);
    EXPECT_TRUE(builder81);
    DynamicType::_ref_type type81 = builder81->build();
    ASSERT_TRUE(type81);

    DynamicTypeBuilder::_ref_type builder82 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsULong",
                    include_paths);
    EXPECT_TRUE(builder82);
    DynamicType::_ref_type type82 = builder82->build();
    ASSERT_TRUE(type82);

    DynamicTypeBuilder::_ref_type builder83 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsLongLong",
                    include_paths);
    EXPECT_TRUE(builder83);
    DynamicType::_ref_type type83 = builder83->build();
    ASSERT_TRUE(type83);

    DynamicTypeBuilder::_ref_type builder84 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsULongLong",
                    include_paths);
    EXPECT_TRUE(builder84);
    DynamicType::_ref_type type84 = builder84->build();
    ASSERT_TRUE(type84);

    DynamicTypeBuilder::_ref_type builder85 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsFloat",
                    include_paths);
    EXPECT_TRUE(builder85);
    DynamicType::_ref_type type85 = builder85->build();
    ASSERT_TRUE(type85);

    DynamicTypeBuilder::_ref_type builder86 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsDouble",
                    include_paths);
    EXPECT_TRUE(builder86);
    DynamicType::_ref_type type86 = builder86->build();
    ASSERT_TRUE(type86);

    DynamicTypeBuilder::_ref_type builder87 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsLongDouble",
                    include_paths);
    EXPECT_TRUE(builder87);
    DynamicType::_ref_type type87 = builder87->build();
    ASSERT_TRUE(type87);

    DynamicTypeBuilder::_ref_type builder88 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsBoolean",
                    include_paths);
    EXPECT_TRUE(builder88);
    DynamicType::_ref_type type88 = builder88->build();
    ASSERT_TRUE(type88);

    DynamicTypeBuilder::_ref_type builder89 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsOctet",
                    include_paths);
    EXPECT_TRUE(builder89);
    DynamicType::_ref_type type89 = builder89->build();
    ASSERT_TRUE(type89);

    DynamicTypeBuilder::_ref_type builder90 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsChar",
                    include_paths);
    EXPECT_TRUE(builder90);
    DynamicType::_ref_type type90 = builder90->build();
    ASSERT_TRUE(type90);

    DynamicTypeBuilder::_ref_type builder91 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsWChar",
                    include_paths);
    EXPECT_TRUE(builder91);
    DynamicType::_ref_type type91 = builder91->build();
    ASSERT_TRUE(type91);

    DynamicTypeBuilder::_ref_type builder92 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsString",
                    include_paths);
    EXPECT_TRUE(builder92);
    DynamicType::_ref_type type92 = builder92->build();
    ASSERT_TRUE(type92);

    DynamicTypeBuilder::_ref_type builder93 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsWString",
                    include_paths);
    EXPECT_TRUE(builder93);
    DynamicType::_ref_type type93 = builder93->build();
    ASSERT_TRUE(type93);

    DynamicTypeBuilder::_ref_type builder94 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsBoundedString",
                    include_paths);
    EXPECT_TRUE(builder94);
    DynamicType::_ref_type type94 = builder94->build();
    ASSERT_TRUE(type94);

    DynamicTypeBuilder::_ref_type builder95 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsBoundedWString",
                    include_paths);
    EXPECT_TRUE(builder95);
    DynamicType::_ref_type type95 = builder95->build();
    ASSERT_TRUE(type95);

    DynamicTypeBuilder::_ref_type builder96 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsEnum",
                    include_paths);
    EXPECT_TRUE(builder96);
    DynamicType::_ref_type type96 = builder96->build();
    ASSERT_TRUE(type96);

    // TODO ArrayMultiDimensionLiteralsBitMask is skipped since bitmask parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder97 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionLiteralsBitMask", include_paths);
    // EXPECT_TRUE(builder97);
    // DynamicType::_ref_type type97 = builder97->build();
    // ASSERT_TRUE(type97);

    DynamicTypeBuilder::_ref_type builder98 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsAlias",
                    include_paths);
    EXPECT_TRUE(builder98);
    DynamicType::_ref_type type98 = builder98->build();
    ASSERT_TRUE(type98);

    DynamicTypeBuilder::_ref_type builder99 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionLiteralsSequence", include_paths);
    EXPECT_TRUE(builder99);
    DynamicType::_ref_type type99 = builder99->build();
    ASSERT_TRUE(type99);

    // TODO ArrayMultiDimensionLiteralsMap is skipped since map parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder100 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionLiteralsMap", include_paths);
    // EXPECT_TRUE(builder100);
    // DynamicType::_ref_type type100 = builder100->build();
    // ASSERT_TRUE(type100);

    DynamicTypeBuilder::_ref_type builder101 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsUnion",
                    include_paths);
    EXPECT_TRUE(builder101);
    DynamicType::_ref_type type101 = builder101->build();
    ASSERT_TRUE(type101);

    DynamicTypeBuilder::_ref_type builder102 = factory->create_type_w_uri("IDL/arrays.idl",
                    "ArrayMultiDimensionLiteralsStructure",
                    include_paths);
    EXPECT_TRUE(builder102);
    DynamicType::_ref_type type102 = builder102->build();
    ASSERT_TRUE(type102);

    // TODO ArrayMultiDimensionLiteralsBitSet is skipped since bitset parsing is not supported.
    // DynamicTypeBuilder::_ref_type builder103 = factory->create_type_w_uri("IDL/arrays.idl", "ArrayMultiDimensionLiteralsBitSet", include_paths);
    // EXPECT_TRUE(builder103);
    // DynamicType::_ref_type type103 = builder103->build();
    // ASSERT_TRUE(type103);

    DynamicTypeBuilder::_ref_type builder104 = factory->create_type_w_uri("IDL/arrays.idl", "BoundedSmallArrays",
                    include_paths);
    EXPECT_TRUE(builder104);
    DynamicType::_ref_type type104 = builder104->build();
    ASSERT_TRUE(type104);

    DynamicTypeBuilder::_ref_type builder105 = factory->create_type_w_uri("IDL/arrays.idl", "BoundedBigArrays",
                    include_paths);
    EXPECT_TRUE(builder105);
    DynamicType::_ref_type type105 = builder105->build();
    ASSERT_TRUE(type105);
}

TEST_F(IdlParserTests, no_path_included)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> include_paths;
    include_paths.push_back("IDL/helpers");

    DynamicTypeBuilder::_ref_type builder1 = factory->create_type_w_uri("IDL/no_path_included.idl",
                    "RelativePathIncludeStruct", include_paths);
    EXPECT_TRUE(builder1);
    DynamicType::_ref_type type1 = builder1->build();
    ASSERT_TRUE(type1);
    DynamicData::_ref_type data1 {DynamicDataFactory::get_instance()->create_data(type1)};
    ASSERT_TRUE(data1);
    int32_t test1 {0};
    EXPECT_EQ(data1->set_int32_value(0, 2), RETCODE_OK);
    EXPECT_EQ(data1->get_int32_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(test1, 2);
}

TEST_F(IdlParserTests, relative_path_include)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> include_paths;
    include_paths.push_back("IDL/helpers");

    DynamicTypeBuilder::_ref_type builder1 = factory->create_type_w_uri("IDL/relative_path_include.idl",
                    "RelativePathIncludeStruct", include_paths);
    EXPECT_TRUE(builder1);
    DynamicType::_ref_type type1 = builder1->build();
    ASSERT_TRUE(type1);
    DynamicData::_ref_type data1 {DynamicDataFactory::get_instance()->create_data(type1)};
    ASSERT_TRUE(data1);
    int32_t test1 {0};
    EXPECT_EQ(data1->set_int32_value(0, 2), RETCODE_OK);
    EXPECT_EQ(data1->get_int32_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(test1, 2);
}

TEST_F(IdlParserTests, unions)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> include_paths;
    include_paths.push_back("IDL/helpers");

    DynamicTypeBuilder::_ref_type builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Short", include_paths);
    EXPECT_TRUE(builder);
    DynamicType::_ref_type type = builder->build();
    ASSERT_TRUE(type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(type)};
    ASSERT_TRUE(data);
    int32_t test1 {0};
    EXPECT_EQ(data->set_int32_value(0, 100), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(test1, 100);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_UShort", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Long", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_ULong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_LongLong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_ULongLOng", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Float", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Double", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_LongDouble", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Boolean", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Octet", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Char", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_WChar", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_String", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_WString", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

     builder = factory->create_type_w_uri("IDL/unions.idl", "Union_BoundedString", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_BoundedWString", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_InnerEnumHelper", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    // TODO Union_InnerBitMaskHelper is skipped since bitmask parsing is not supported.
    // builder = factory->create_type_w_uri("IDL/unions.idl", "Union_InnerBitMaskHelper", include_paths);
    // EXPECT_TRUE(builder);
    // type = builder->build();
    // ASSERT_TRUE(type);
    // data = DynamicDataFactory::get_instance()->create_data(type);
    // ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_InnerAliasHelper", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Array", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Sequence", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    // TODO Union_Map is skipped since map parsing is not supported.
    // builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Map", include_paths);
    // EXPECT_TRUE(builder);
    // type = builder->build();
    // ASSERT_TRUE(type);
    // data = DynamicDataFactory::get_instance()->create_data(type);
    // ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_InnerUnionHelper", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_InnerStructureHelper", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    // TODO Union_InnerBitsetHelper is skipped since bitset parsing is not supported.
    // builder = factory->create_type_w_uri("IDL/unions.idl", "Union_InnerBitsetHelper", include_paths);
    // EXPECT_TRUE(builder);
    // type = builder->build();
    // ASSERT_TRUE(type);
    // data = DynamicDataFactory::get_instance()->create_data(type);
    // ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_short", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_unsigned_short", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_long", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_unsigned_long", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_long_long", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_unsigned_long_long", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_boolean", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_octet", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_char", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_wchar", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_enum", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_enum_labels", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Discriminator_alias", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Several_Fields", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Several_Fields_With_Default", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    // TODO Union_Fixed_String_In_Module_Alias is skipped since module parsing is not supported.
    // builder = factory->create_type_w_uri("IDL/unions.idl", "Union_Fixed_String_In_Module_Alias", include_paths);
    // EXPECT_TRUE(builder);
    // type = builder->build();
    // ASSERT_TRUE(type);
    // data = DynamicDataFactory::get_instance()->create_data(type);
    // ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionShort", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionUShort", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionLong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionULong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionLongLong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionULongLong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionFloat", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDouble", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionLongDouble", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionBoolean", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionOctet", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionChar", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionWChar", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionString", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionWString", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionBoundedString", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionBoundedWString", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionInnerEnumHelper", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    // TODO UnionInnerBitMaskHelper is skipped since bitmask parsing is not supported.
    // builder = factory->create_type_w_uri("IDL/unions.idl", "UnionInnerBitMaskHelper", include_paths);
    // EXPECT_TRUE(builder);
    // type = builder->build();
    // ASSERT_TRUE(type);
    // data = DynamicDataFactory::get_instance()->create_data(type);
    // ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionInnerAliasHelper", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionArray", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionSequence", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    // TODO UnionMap is skipped since map parsing is not supported.
    // builder = factory->create_type_w_uri("IDL/unions.idl", "UnionMap", include_paths);
    // EXPECT_TRUE(builder);
    // type = builder->build();
    // ASSERT_TRUE(type);
    // data = DynamicDataFactory::get_instance()->create_data(type);
    // ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionInnerUnionHelper", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionInnerStructureHelper", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    // TODO UnionInnerBitsetHelper is skipped since bitset parsing is not supported.
    // builder = factory->create_type_w_uri("IDL/unions.idl", "UnionInnerBitsetHelper", include_paths);
    // EXPECT_TRUE(builder);
    // type = builder->build();
    // ASSERT_TRUE(type);
    // data = DynamicDataFactory::get_instance()->create_data(type);
    // ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorShort", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorUShort", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorLong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorULong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorLongLong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorULongLong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorBoolean", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorOctet", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorChar", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorWChar", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorEnum", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorEnumLabel", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionDiscriminatorAlias", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionSeveralFields", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    builder = factory->create_type_w_uri("IDL/unions.idl", "UnionSeveralFieldsWithDefault", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    // TODO The rest types are skipped since annotation/module parsing are not supported.
}

TEST_F(IdlParserTests, sequences)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    std::vector<std::string> include_paths;
    include_paths.push_back("IDL/helpers/basic_inner_types.idl");

    /* sequence<short> */
    DynamicTypeBuilder::_ref_type builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceShort", include_paths);
    EXPECT_TRUE(builder);
    DynamicType::_ref_type type = builder->build();
    ASSERT_TRUE(type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(type)};
    ASSERT_TRUE(data);

    /* sequence<unsigned short> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceUShort", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<long> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceLong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<unsigned long> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceULong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<long long> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceLongLong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<unsigned long long> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceULongLong", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<float> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceFloat", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<double> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceDouble", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<long double> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceLongDouble", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<boolean> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceBoolean", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<octet> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceOctet", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<char> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceChar", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<wchar> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceWChar", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<string> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceString", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<wstring> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceWString", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<Inner_alias_bounded_string_helper> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceStringBounded", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<Inner_alias_bounded_wstring_helper> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceWStringBounded", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<InnerEnumHelper> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceEnum", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<InnerBitMaskHelper> */
    // TODO: SequenceBitMask is skipped since bitmask parsing is not supported.
    // builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceBitMask", include_paths);
    // EXPECT_TRUE(builder);
    // type = builder->build();
    // ASSERT_TRUE(type);
    // data = DynamicDataFactory::get_instance()->create_data(type);
    // ASSERT_TRUE(data);

    /* sequence<InnerAliasHelper> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceAlias", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<Inner_alias_array_helper> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceShortArray", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<Inner_alias_sequence_helper> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceSequence", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<Inner_alias_map_helper> */
    // TODO: SequenceMap is skipped since map parsing is not supported.
    // builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceMap", include_paths);
    // EXPECT_TRUE(builder);
    // type = builder->build();
    // ASSERT_TRUE(type);
    // data = DynamicDataFactory::get_instance()->create_data(type);
    // ASSERT_TRUE(data);

    /* sequence<InnerUnionHelper> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceUnion", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<InnerStructureHelper> */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceStructure", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /* sequence<InnerBitsetHelper> */
    // TODO: SequenceBitset is skipped since bitset parsing is not supported.
    // builder = factory->create_type_w_uri("IDL/sequences.idl", "SequenceBitset", include_paths);
    // EXPECT_TRUE(builder);
    // type = builder->build();
    // ASSERT_TRUE(type);
    // data = DynamicDataFactory::get_instance()->create_data(type);
    // ASSERT_TRUE(data);

    /**
     * sequence<short, 1>
     * sequence<string, 5>
     */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "BoundedSmallSequences", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    /**
     * sequence<short, 41925>
     * sequence<string, 256>
     */
    builder = factory->create_type_w_uri("IDL/sequences.idl", "BoundedBigSequences", include_paths);
    EXPECT_TRUE(builder);
    type = builder->build();
    ASSERT_TRUE(type);
    data = DynamicDataFactory::get_instance()->create_data(type);
    ASSERT_TRUE(data);

    // TODO: The rest types are skipped since module parsing is not supported.
    // builder = factory->create_type_w_uri("IDL/sequences.idl", "Common_Module", include_paths);
    // EXPECT_TRUE(builder);
    // type = builder->build();
    // ASSERT_TRUE(type);
    // data = DynamicDataFactory::get_instance()->create_data(type);
    // ASSERT_TRUE(data);

    // builder = factory->create_type_w_uri("IDL/sequences.idl", "NoCommon_Module", include_paths);
    // EXPECT_TRUE(builder);
    // type = builder->build();
    // ASSERT_TRUE(type);
    // data = DynamicDataFactory::get_instance()->create_data(type);
    // ASSERT_TRUE(data);
}

int main(
        int argc,
        char** argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
