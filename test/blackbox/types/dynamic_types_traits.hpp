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

/**
 * @file dynamic_types_traits.hpp
 *
 */

#ifndef _FASTDDS_TEST_DYNAMIC_TYPES_TRAITS_HPP_
#define _FASTDDS_TEST_DYNAMIC_TYPES_TRAITS_HPP_

#include <iostream>
#include <string>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicLoanableSequence.hpp>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * @brief PubSubTypeTraits for the PubSubReader and PubSubWriter
 *
 * This struct enables instantiation of a PubSubReader and PubSubWriter with a DynamicTypeSupport
 * for a type equivalent to that of KeyedHelloWorld.idl.
 * That and the fact that this builder sets the auto_fill_type_object flag to true allow for instance to create a
 * PubSubReader with a content filter, which right now is not possible to do with the Fast DDS generated
 * TypeSupport, as it does not contain the TypeObject code.
 *
 * To use it, do one of:
 *   - PubSubReader<fastrtps::types::DynamicPubSubType, DynamicKeyedHelloworldTypeTraits> reader;
 *   - PubSubWriter<fastrtps::types::DynamicPubSubType, DynamicKeyedHelloworldTypeTraits> writer;
 */
struct DynamicKeyedHelloworldTypeTraits
{
    using DataListType = fastrtps::types::DynamicData*;

    /**
     * Define the members ids of the struct for readability
     */
    enum class KeyedHelloWorldMembers : fastrtps::types::MemberId
    {
        KEY = 0,
        INDEX = 1,
        MESSAGE = 2
    };

    /**
     * @brief Build the DynamicTypeSupport for the KeyedHelloWorld type
     *
     * @param[out] type_support The type support to be reset
     */
    static void build_type_support(
            TypeSupport& type_support)
    {
        // Create type support
        type_support.reset(new fastrtps::types::DynamicPubSubType(get_type()));

        // Set the autofill type object to true so that TypeObject is registered upon type registration
        type_support->auto_fill_type_object(true);
    }

    /**
     * @brief Build a DynamicLoanableSequence of the specific type
     *
     * @return The DynamicLoanableSequence
     */
    static DynamicLoanableSequence build_loanable_sequence()
    {
        return DynamicLoanableSequence(get_type());
    }

    /**
     * @brief Compare two data instances
     *
     * PubSubReader and PubSubWriter compare data within std::find_if calls to remove elements from
     * their message lists. For Fast DDS Gen generated types, the TypeSupport::type and the type used
     * for data list elements is the same. However, for the case of DynamicData, the DataListType needs
     * to be a DynamicData*, as the DynamicData ctor and dtor are protected and so they cannot be called to remove elements from the message lists.
     * This leads to the need to compare datas of different types when finding elements, as the sent or received data would be a DynamicDaa, whereas the list elements are DynamicData* (a.k.a DataListType).
     *
     * @param data1 The compared element (DynamicData)
     * @param data2 The list element (DataListType)
     *
     * @return True if equal, false otherwise
     */
    static bool compare_data(
            const fastrtps::types::DynamicData& data1,
            const DataListType& data2)
    {
        return data1.equals(data2);
    }

    /**
     * @brief Print the received data
     *
     * @param data DynamicData to be printed
     */
    static void print_received_data(
            const fastrtps::types::DynamicData* data)
    {
        print_received_data(*data);
    }

    /**
     * @brief Print the received data
     *
     * @param data DynamicData to be printed
     */
    static void print_received_data(
            const fastrtps::types::DynamicData& data)
    {
        print_data_msg("Received", data);
    }

    /**
     * @brief Print the sent data
     *
     * @param data DynamicData to be printed
     */
    static void print_sent_data(
            const fastrtps::types::DynamicData* data)
    {
        print_sent_data(*data);
    }

    /**
     * @brief Print the sent data
     *
     * @param data DynamicData to be printed
     */
    static void print_sent_data(
            const fastrtps::types::DynamicData& data)
    {
        print_data_msg("Sent", data);
    }

private:

    /**
     * @brief Get the specific DynamicType equivalent to the KeyedHelloWorld.idl type
     *
     * @return The DynamicType_ptr
     */
    static fastrtps::types::DynamicType_ptr get_type()
    {
        // Using statics here in combination with std::call_once to avoid rebuilding the type every time.
        // As the DynamicTypes API is shared_ptr based, we don't need to delete these references manually.
        static std::once_flag once_flag;
        static fastrtps::types::DynamicTypeBuilder_ptr struct_builder;
        static fastrtps::types::DynamicTypeBuilder_ptr key_member_builder;
        static fastrtps::types::DynamicType_ptr key_member;
        static fastrtps::types::DynamicType_ptr struct_type;

        std::call_once(once_flag, [&]()
                {
                    // Create the struct type builder
                    const std::string topic_type_name = "keyed_hello_world";
                    struct_builder = fastrtps::types::DynamicTypeBuilderFactory::get_instance()->create_struct_builder();
                    struct_builder->set_name(topic_type_name);

                    // Create the key member with the @key annotation
                    key_member_builder = fastrtps::types::DynamicTypeBuilderFactory::get_instance()->create_uint16_builder();
                    key_member_builder->apply_annotation(fastrtps::types::ANNOTATION_KEY_ID, "value", "true");
                    key_member = key_member_builder->build();

                    // Add members to the struct builder
                    struct_builder->add_member(
                        static_cast<fastrtps::types::MemberId>(KeyedHelloWorldMembers::KEY),
                        "key",
                        key_member);

                    struct_builder->add_member(
                        static_cast<fastrtps::types::MemberId>(KeyedHelloWorldMembers::INDEX),
                        "index",
                        fastrtps::types::DynamicTypeBuilderFactory::get_instance()->create_uint16_type());

                    struct_builder->add_member(
                        static_cast<fastrtps::types::MemberId>(KeyedHelloWorldMembers::MESSAGE),
                        "message",
                        fastrtps::types::DynamicTypeBuilderFactory::get_instance()->create_string_type(128));

                    // Build the type
                    struct_type = struct_builder->build();
                });

        return struct_type;
    }

    /**
     * @brief Print a data message
     *
     * @param intro Introduction message
     * @param data DynamicData to be printed.
     *
     * @pre data must have been built using the type returned by get_type(), that is through the DynamicTypeSupport
     * created by build_type_support().
     */
    static void print_data_msg(
            const std::string& intro,
            const fastrtps::types::DynamicData& data)
    {
        auto key_value =
                data.get_uint16_value(static_cast<fastrtps::types::MemberId>(KeyedHelloWorldMembers::KEY));
        auto index_value =
                data.get_uint16_value(static_cast<fastrtps::types::MemberId>(KeyedHelloWorldMembers::INDEX));
        auto message_value =
                data.get_string_value(static_cast<fastrtps::types::MemberId>(KeyedHelloWorldMembers::MESSAGE));

        std::cout << intro << " data: (" << key_value << ", " << index_value << ", \"" << message_value << "\")" <<
            std::endl;
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TEST_DYNAMIC_TYPES_TRAITS_HPP_
