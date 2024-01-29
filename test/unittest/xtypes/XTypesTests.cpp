// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::xtypes;

class XTypesTests : public ::testing::Test
{
public:

    XTypesTests()
    {
    }

    ~XTypesTests()
    {
        eprosima::fastdds::dds::Log::KillThread();
    }

    virtual void TearDown()
    {
    }

};

/*TODO(richiware)
   TEST_F(XTypesTests, TypeDescriptorFullyQualifiedName)
   {
    DynamicTypeBuilder_ptr my_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());
    my_builder->add_member(0, "x", DynamicTypeBuilderFactory::get_instance()->create_float32_type());
    my_builder->add_member(0, "y", DynamicTypeBuilderFactory::get_instance()->create_float32_type());
    my_builder->add_member(0, "z", DynamicTypeBuilderFactory::get_instance()->create_float32_type());
    const TypeDescriptor* my_descriptor = my_builder->get_type_descriptor();

    my_builder->set_name("Position");
    ASSERT_TRUE(my_descriptor.is_consistent());
    my_builder->set_name("Position_");
    ASSERT_TRUE(my_descriptor.is_consistent());
    my_builder->set_name("Position123");
    ASSERT_TRUE(my_descriptor.is_consistent());
    my_builder->set_name("position_123");
    ASSERT_TRUE(my_descriptor.is_consistent());
    my_builder->set_name("_Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
    my_builder->set_name("123Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
    my_builder->set_name("Position&");
    ASSERT_FALSE(my_descriptor.is_consistent());

    my_builder->set_name("my_interface::action::dds_::Position");
    ASSERT_TRUE(my_descriptor.is_consistent());
    my_builder->set_name("my_interface:action::dds_::Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
    my_builder->set_name("my_interface:::action::dds_::Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
    my_builder->set_name("_my_interface::action::dds_::Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
    my_builder->set_name("1my_interface::action::dds_::Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
    my_builder->set_name(":my_interface::action::dds_::Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
    my_builder->set_name("::my_interface::action::dds_::Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
    my_builder->set_name("$my_interface::action::dds_::Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
    my_builder->set_name("my_interface::2action::dds_::Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
    my_builder->set_name("my_interface::_action::dds_::Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
    my_builder->set_name("my_interface::*action::dds_::Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
    my_builder->set_name("my_interface::action*::dds_::Position");
    ASSERT_FALSE(my_descriptor.is_consistent());
   }

   TEST_F(XTypesTests, MemberDescriptorFullyQualifiedName)
   {
    MemberId member_id{0};
    DynamicTypeBuilder_ptr my_builder(DynamicTypeBuilderFactory::get_instance().create_struct_type());
    my_builder->add_member(member_id++, "x", DynamicTypeBuilderFactory::get_instance().get_float32_type());
    my_builder->add_member(member_id++, "y", DynamicTypeBuilderFactory::get_instance().get_float32_type());
    my_builder->add_member(member_id++, "z", DynamicTypeBuilderFactory::get_instance().get_float32_type());

    my_builder->set_name("Position");
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, my_builder->add_member(member_id++, "t1", my_builder->build()));
    my_builder->set_name("Position_");
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, my_builder->add_member(member_id++, "t2", my_builder->build()));
    my_builder->set_name("Position123");
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, my_builder->add_member(member_id++, "t3", my_builder->build()));
    my_builder->set_name("position_123");
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, my_builder->add_member(member_id++, "t4", my_builder->build()));
    my_builder->set_name("_Position");
    EXPECT_FALSE(my_builder->build());
    my_builder->set_name("123Position");
    EXPECT_FALSE( my_builder->build());
    my_builder->set_name("Position&");
    EXPECT_FALSE( my_builder->build());

    my_builder->set_name("my_interface::action::dds_::Position");
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, my_builder->add_member(member_id++, "t8", my_builder->build()));
    my_builder->set_name("my_interface:action::dds_::Position");
    EXPECT_FALSE( my_builder->build());
    my_builder->set_name("my_interface:::action::dds_::Position");
    EXPECT_FALSE( my_builder->build());
    my_builder->set_name("_my_interface::action::dds_::Position");
    EXPECT_FALSE( my_builder->build());
    my_builder->set_name("1my_interface::action::dds_::Position");
    EXPECT_FALSE( my_builder->build());
    my_builder->set_name(":my_interface::action::dds_::Position");
    EXPECT_FALSE( my_builder->build());
    my_builder->set_name("::my_interface::action::dds_::Position");
    EXPECT_FALSE( my_builder->build());
    my_builder->set_name("$my_interface::action::dds_::Position");
    EXPECT_FALSE( my_builder->build());
    my_builder->set_name("my_interface::2action::dds_::Position");
    EXPECT_FALSE( my_builder->build());
    my_builder->set_name("my_interface::_action::dds_::Position");
    EXPECT_FALSE( my_builder->build());
    my_builder->set_name("my_interface::*action::dds_::Position");
    EXPECT_FALSE( my_builder->build());
    my_builder->set_name("my_interface::action*::dds_::Position");
    EXPECT_FALSE( my_builder->build());
   }
 */

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
