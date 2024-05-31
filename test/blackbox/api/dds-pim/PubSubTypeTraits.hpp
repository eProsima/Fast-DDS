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
 * @file PubSubTypeTraits.hpp
 *
 */

#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "../../common/BlackboxTests.hpp"

#ifndef _TEST_BLACKBOX_PUBSUBTYPETRAITS_HPP_
#define _TEST_BLACKBOX_PUBSUBTYPETRAITS_HPP_

/**
 * @brief This class provides TypeSupport specific traits for the PubSub wrapper classes
 *
 * Provides all the type specific functionality needed by the PubSub wrapper classes so that
 * they can be agnostic to the actual TypeSupport class used, Fast DDS Gen generated or Dynamic.
 *
 * @tparam _TypeSupport TypeSupport class for which the traits are provided
 */
template<class _TypeSupport>
struct PubSubTypeTraits
{
    //! PubSub readers and writers have a list of datas of the type they are handling
    using DataListType = typename _TypeSupport::type;

    /**
     * @brief Create a new instance of the specific TypeSupport
     *
     * @param[out] typesupport TypeSupport reference to be populated with the new concrete instance
     */
    static void build_type_support(
            eprosima::fastdds::dds::TypeSupport& typesupport)
    {
        return typesupport.reset(new _TypeSupport());
    }

    /**
     * @brief Build a LoanableSequence of the specific type
     *
     * @return The type specific LoanableSequence
     */
    static eprosima::fastdds::dds::LoanableSequence<typename _TypeSupport::type> build_loanable_sequence()
    {
        return eprosima::fastdds::dds::LoanableSequence<typename _TypeSupport::type>();
    }

    /**
     * @brief Compare two DataListType instances
     *
     * @param data1 First DataListType instance
     * @param data2 Second DataListType instance
     *
     * @return True if the two instances are equal, false otherwise
     */
    static bool compare_data(
            const DataListType& data1,
            const DataListType& data2)
    {
        return data1 == data2;
    }

    /**
     * @brief Print the received data
     *
     * @param data DataListType instance to be printed
     */
    static void print_received_data(
            const DataListType& data)
    {
        default_receive_print<DataListType>(data);
    }

    /**
     * @brief Print the sent data
     *
     * @param data DataListType instance to be printed
     */
    static void print_sent_data(
            const DataListType& data)
    {
        default_send_print<DataListType>(data);
    }

};

#endif // _TEST_BLACKBOX_PUBSUBTYPETRAITS_HPP_
