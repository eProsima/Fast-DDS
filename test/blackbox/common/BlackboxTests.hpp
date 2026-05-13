// Copyright 2018, 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef __BLACKBOX_BLACKBOXTESTS_HPP__
#define __BLACKBOX_BLACKBOXTESTS_HPP__

#define TEST_TOPIC_NAME std::string( \
        ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name() + std::string( \
            "_") + ::testing::UnitTest::GetInstance()->current_test_info()->name())

#if defined(__cplusplus_winrt)
#define GET_PID GetCurrentProcessId
#include <process.h>
#elif defined(_WIN32)
#define GET_PID _getpid
#include <process.h>
#else
#define GET_PID getpid
#include <sys/types.h>
#include <unistd.h>
#endif // if defined(_WIN32)

#include "../types/Data1mbPubSubTypes.hpp"
#include "../types/Data100kbPubSubTypes.hpp"
#include "../types/Data64kbPubSubTypes.hpp"
#include "../types/FixedSizedPubSubTypes.hpp"
#include "../types/HelloWorldPubSubTypes.hpp"
#include "../types/KeyedData1mbPubSubTypes.hpp"
#include "../types/KeyedHelloWorldPubSubTypes.hpp"
#include "../types/core/core_typesPubSubTypes.hpp"
#include "../types/StringTestPubSubTypes.hpp"
#include "../types/UnboundedHelloWorldPubSubTypes.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <list>
#include <functional>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct BuiltinTopicKey_t;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#if HAVE_SECURITY
extern void blackbox_security_init();
#endif // if HAVE_SECURITY
#if TLS_FOUND
extern void tls_init();
#endif // if TLS_FOUND

extern const char* certs_path;
extern uint16_t global_port;
extern bool enable_datasharing;
extern bool use_pull_mode;
extern bool use_udpv4;
extern bool use_ipv6;

/****** Auxiliary print functions  ******/
template<class Type>
void default_receive_print(
        const Type&)
{
    std::cout << "Received data" << std::endl;
}

template<>
void default_receive_print(
        const HelloWorld& hello);

template<>
void default_receive_print(
        const FixedSized& hello);

template<>
void default_receive_print(
        const KeyedHelloWorld& str);

template<>
void default_receive_print(
        const StringTest& str);

template<>
void default_receive_print(
        const Data64kb& data);

template<>
void default_receive_print(
        const Data100kb& data);

template<>
void default_receive_print(
        const Data1mb& data);

template<>
void default_receive_print(
        const KeyedData1mb& data);

template<class Type>
void default_send_print(
        const Type&)
{
    std::cout << "Sent data" << std::endl;
}

template<>
void default_send_print(
        const StringTest&);

template<>
void default_send_print(
        const HelloWorld& hello);

template<>
void default_send_print(
        const FixedSized& hello);

template<>
void default_send_print(
        const KeyedHelloWorld& str);

template<>
void default_send_print(
        const StringTest& str);

template<>
void default_send_print(
        const Data64kb& data);

template<>
void default_send_print(
        const Data100kb& data);

template<>
void default_send_print(
        const Data1mb& data);

template<>
void default_send_print(
        const KeyedData1mb& data);

/****** Auxiliary data generators *******/
std::list<HelloWorld> default_helloworld_data_generator(
        size_t max = 0);

std::list<FixedSized> default_fixed_sized_data_generator(
        size_t max = 0);

std::list<KeyedHelloWorld> default_keyedhelloworld_data_generator(
        size_t max = 0,
        bool unique_key = false);

std::list<KeyedHelloWorld> default_keyedhelloworld_per_participant_data_generator(
        size_t participants,
        size_t max = 0);

std::list<StringTest> default_large_string_data_generator(
        size_t max = 0);

std::list<Data64kb> default_data64kb_data_generator(
        size_t max = 0);

std::list<Data1mb> default_data16kb_data_generator(
        size_t max = 0);

std::list<Data1mb> default_data300kb_data_generator(
        size_t max = 0);

std::list<Data1mb> default_data300kb_mix_data_generator(
        size_t max = 0);

std::list<Data1mb> default_data96kb_data300kb_data_generator(
        size_t max = 0);

std::list<Data100kb> default_data100kb_data_generator(
        size_t max = 0);

std::list<KeyedData1mb> default_keyeddata300kb_data_generator(
        size_t max = 0);

std::list<UnboundedHelloWorld> default_unbounded_helloworld_data_generator(
        size_t max = 0);

/****** Auxiliary lambda functions  ******/
extern const std::function<void(const HelloWorld&)>  default_helloworld_print;

extern const std::function<void(const FixedSized&)>  default_fixed_sized_print;

extern const std::function<void(const KeyedHelloWorld&)>  default_keyedhelloworld_print;

extern const std::function<void(const StringTest&)>  default_string_print;

extern const std::function<void(const Data64kb&)>  default_data64kb_print;

extern const std::function<void(const Data100kb&)>  default_data100kb_print;

extern const std::function<void(const Data1mb&)>  default_data300kb_print;

template<typename T>
void print_non_received_messages(
        const std::list<T>& data,
        const std::function<void(const T&)>& printer)
{
    if (data.size() != 0)
    {
        std::cout << "Samples not received: ";
        std::for_each(data.begin(), data.end(), printer);
        std::cout << std::endl;
    }
}

/***** End auxiliary lambda function *****/

/****** Auxiliary conversion helpers *******/
void entity_id_to_builtin_topic_key(
        eprosima::fastdds::rtps::BuiltinTopicKey_t& bt_key,
        const eprosima::fastdds::rtps::EntityId_t& entity_id);

void guid_prefix_to_builtin_topic_key(
        eprosima::fastdds::rtps::BuiltinTopicKey_t& bt_key,
        const eprosima::fastdds::rtps::GuidPrefix_t& guid_prefix);

/****** End Auxiliary conversion helpers *******/

#endif // __BLACKBOX_BLACKBOXTESTS_HPP__
