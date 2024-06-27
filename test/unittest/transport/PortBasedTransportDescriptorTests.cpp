// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/transport/PortBasedTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>

using namespace eprosima::fastdds::rtps;

class CustomPortBasedTransportDescriptor : public PortBasedTransportDescriptor
{
public:

    CustomPortBasedTransportDescriptor()
        : PortBasedTransportDescriptor(0, 0)
    {
    }

    TransportInterface* create_transport() const override
    {
        return nullptr;
    }

    //! Returns the minimum size required for a send operation.
    uint32_t min_send_buffer_size() const override
    {
        return 0;
    }

};

class PortBasedTransportDescriptorTests : public CustomPortBasedTransportDescriptor, public testing::Test
{
public:

    TransportInterface* create_transport() const override
    {
        return nullptr;
    }

    //! Returns the minimum size required for a send operation.
    uint32_t min_send_buffer_size() const override
    {
        return 0;
    }

};

TEST_F(PortBasedTransportDescriptorTests, get_thread_config_for_port)
{
    // Add an entry to the user-defined settings map
    PortBasedTransportDescriptor::ReceptionThreadsConfigMap set_settings;
    set_settings[1234].scheduling_policy = 33;
    set_settings[1234].priority = 33;
    set_settings[1234].affinity = 33;
    set_settings[1234].stack_size = 33;

    ASSERT_TRUE(reception_threads(set_settings));

    // Check that the new entry can be retrieved
    ASSERT_EQ(set_settings[1234], get_thread_config_for_port(1234));

    // Check that the new entry is not the same as the default settings
    ASSERT_NE(default_reception_threads(), get_thread_config_for_port(1234));

    // Check that a non-existing entry is returns default settings
    ASSERT_EQ(default_reception_threads(), get_thread_config_for_port(4321));
}

TEST_F(PortBasedTransportDescriptorTests, set_thread_config_for_port)
{
    // Set some initial config
    PortBasedTransportDescriptor::ReceptionThreadsConfigMap set_settings;
    set_settings[1234].scheduling_policy = 33;
    set_settings[1234].priority = 33;
    set_settings[1234].affinity = 33;
    set_settings[1234].stack_size = 33;
    ASSERT_TRUE(reception_threads(set_settings));

    // Check updating a config
    ThreadSettings other_settings;
    ASSERT_NE(set_settings[1234], other_settings);
    ASSERT_TRUE(set_thread_config_for_port(1234, other_settings));
    ASSERT_EQ(other_settings, get_thread_config_for_port(1234));

    // Setting a new config
    other_settings.priority += 1;
    ASSERT_NE(set_settings[4321], other_settings);
    ASSERT_TRUE(set_thread_config_for_port(4321, other_settings));
    ASSERT_EQ(other_settings, get_thread_config_for_port(4321));
    ASSERT_NE(other_settings, get_thread_config_for_port(1234));
}

TEST_F(PortBasedTransportDescriptorTests, get_default_reception_threads)
{
    ASSERT_EQ(default_reception_threads_, default_reception_threads());
}

TEST_F(PortBasedTransportDescriptorTests, set_default_reception_threads)
{
    ThreadSettings& initial_settings = default_reception_threads_;

    ThreadSettings set_settings;
    set_settings.scheduling_policy = 33;
    set_settings.priority = 33;
    set_settings.affinity = 33;
    set_settings.stack_size = 33;

    ASSERT_NE(initial_settings, set_settings);

    default_reception_threads(set_settings);
    ASSERT_EQ(set_settings, default_reception_threads());
}

TEST_F(PortBasedTransportDescriptorTests, get_reception_threads)
{
    ASSERT_EQ(reception_threads_, reception_threads());
}

TEST_F(PortBasedTransportDescriptorTests, set_reception_threads)
{
    PortBasedTransportDescriptor::ReceptionThreadsConfigMap& initial_settings = reception_threads_;

    PortBasedTransportDescriptor::ReceptionThreadsConfigMap set_settings;
    set_settings[1234].scheduling_policy = 33;
    set_settings[1234].priority = 33;
    set_settings[1234].affinity = 33;
    set_settings[1234].stack_size = 33;

    ASSERT_NE(initial_settings, set_settings);

    ASSERT_TRUE(reception_threads(set_settings));
    ASSERT_EQ(set_settings, reception_threads());
}

TEST_F(PortBasedTransportDescriptorTests, equal_operator)
{
    // Two new instances are equal
    CustomPortBasedTransportDescriptor other;
    auto original_max_message_size = other.maxMessageSize;
    auto original_default_reception_thread_settings = other.default_reception_threads();
    auto original_thread_reception_settings = other.reception_threads();
    ASSERT_EQ(*this, other);

    {
        // Parent is different
        other.maxMessageSize += 1;
        ASSERT_FALSE(*this == other);
    }
    {
        // default_reception_threads is different
        other.maxMessageSize = original_max_message_size;
        other.default_reception_threads(original_default_reception_thread_settings);
        ASSERT_TRUE(other.reception_threads(original_thread_reception_settings));
        ASSERT_EQ(*this, other);

        ThreadSettings set_settings;
        set_settings.scheduling_policy = 33;
        set_settings.priority = 33;
        set_settings.affinity = 33;
        set_settings.stack_size = 33;
        other.default_reception_threads(set_settings);
        ASSERT_FALSE(*this == other);
    }
    {
        // reception_threads is different
        other.maxMessageSize = original_max_message_size;
        other.default_reception_threads(original_default_reception_thread_settings);
        ASSERT_TRUE(other.reception_threads(original_thread_reception_settings));
        ASSERT_EQ(*this, other);

        PortBasedTransportDescriptor::ReceptionThreadsConfigMap set_settings_map;
        set_settings_map[1234].scheduling_policy = 33;
        set_settings_map[1234].priority = 33;
        set_settings_map[1234].affinity = 33;
        set_settings_map[1234].stack_size = 33;
        ASSERT_TRUE(other.reception_threads(set_settings_map));
        ASSERT_FALSE(*this == other);
    }
    {
        // Parent & default_reception_threads are different
        other.maxMessageSize = original_max_message_size;
        other.default_reception_threads(original_default_reception_thread_settings);
        ASSERT_TRUE(other.reception_threads(original_thread_reception_settings));
        ASSERT_EQ(*this, other);

        other.maxMessageSize += 1;

        ThreadSettings set_settings;
        set_settings.scheduling_policy = 33;
        set_settings.priority = 33;
        set_settings.affinity = 33;
        set_settings.stack_size = 33;
        other.default_reception_threads(set_settings);

        ASSERT_FALSE(*this == other);
    }
    {
        // Parent & reception_threads are different
        other.maxMessageSize = original_max_message_size;
        other.default_reception_threads(original_default_reception_thread_settings);
        ASSERT_TRUE(other.reception_threads(original_thread_reception_settings));
        ASSERT_EQ(*this, other);

        other.maxMessageSize += 1;

        PortBasedTransportDescriptor::ReceptionThreadsConfigMap set_settings_map;
        set_settings_map[1234].scheduling_policy = 33;
        set_settings_map[1234].priority = 33;
        set_settings_map[1234].affinity = 33;
        set_settings_map[1234].stack_size = 33;
        ASSERT_TRUE(other.reception_threads(set_settings_map));

        ASSERT_FALSE(*this == other);
    }
    {
        // default_reception_threads & reception_threads are different
        other.maxMessageSize = original_max_message_size;
        other.default_reception_threads(original_default_reception_thread_settings);
        ASSERT_TRUE(other.reception_threads(original_thread_reception_settings));
        ASSERT_EQ(*this, other);

        ThreadSettings set_settings;
        set_settings.scheduling_policy = 33;
        set_settings.priority = 33;
        set_settings.affinity = 33;
        set_settings.stack_size = 33;
        other.default_reception_threads(set_settings);

        PortBasedTransportDescriptor::ReceptionThreadsConfigMap set_settings_map;
        set_settings_map[1234].scheduling_policy = 33;
        set_settings_map[1234].priority = 33;
        set_settings_map[1234].affinity = 33;
        set_settings_map[1234].stack_size = 33;
        ASSERT_TRUE(other.reception_threads(set_settings_map));

        ASSERT_FALSE(*this == other);
    }
    {
        // Parent, default_reception_threads, & reception_threads are different
        other.maxMessageSize = original_max_message_size;
        other.default_reception_threads(original_default_reception_thread_settings);
        ASSERT_TRUE(other.reception_threads(original_thread_reception_settings));
        ASSERT_EQ(*this, other);

        other.maxMessageSize += 1;

        ThreadSettings set_settings;
        set_settings.scheduling_policy = 33;
        set_settings.priority = 33;
        set_settings.affinity = 33;
        set_settings.stack_size = 33;
        other.default_reception_threads(set_settings);

        PortBasedTransportDescriptor::ReceptionThreadsConfigMap set_settings_map;
        set_settings_map[1234].scheduling_policy = 33;
        set_settings_map[1234].priority = 33;
        set_settings_map[1234].affinity = 33;
        set_settings_map[1234].stack_size = 33;
        ASSERT_TRUE(other.reception_threads(set_settings_map));

        ASSERT_FALSE(*this == other);
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
