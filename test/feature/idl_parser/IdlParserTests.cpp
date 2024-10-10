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
}

int main(
        int argc,
        char** argv)
{
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
