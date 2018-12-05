// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#if defined(PREALLOCATED_WITH_REALLOC_MEMORY_MODE_TEST)
#define MEMORY_MODE_STRING ReallocMem
#define MEMORY_MODE_BYTE 1
#elif defined(DYNAMIC_RESERVE_MEMORY_MODE_TEST)
#define MEMORY_MODE_STRING DynMem
#define MEMORY_MODE_BYTE 2
#else
#define MEMORY_MODE_STRING PreallocMem
#define MEMORY_MODE_BYTE 3
#endif

#define PASTER(x, y) x ## _ ## y
#define EVALUATOR(x, y) PASTER(x, y)
#define BLACKBOXTEST(test_case_name, test_name) TEST(EVALUATOR(test_case_name, MEMORY_MODE_STRING), test_name)
#define BLACKBOXTEST_F(test_case_name, test_name) TEST_F(EVALUATOR(test_case_name, MEMORY_MODE_STRING), test_name)
#define TEST_TOPIC_NAME std::string(test_info_->test_case_name() + std::string("_") + test_info_->name())

#if defined(_WIN32)
#define GET_PID _getpid
#include <process.h>
#else
#define GET_PID getpid
#include <sys/types.h>
#include <unistd.h>
#endif

#include "types/HelloWorldType.h"
#include "types/StringType.h"
#include "types/Data64kbType.h"
#include "types/Data1mbType.h"

#include <list>
#include <functional>
#include <gtest/gtest.h>

/****** Auxiliary print functions  ******/
template<class Type>
void default_receive_print(const Type&)
{
    std::cout << "Received data" << std::endl;
}

template<>
void default_receive_print(const HelloWorld& hello);

template<>
void default_receive_print(const String& str);

template<>
void default_receive_print(const Data64kb& data);

template<>
void default_receive_print(const Data1mb& data);

template<class Type>
void default_send_print(const Type&)
{
    std::cout << "Sent data" << std::endl;
}

template<>
void default_send_print(const StringType&);

template<>
void default_send_print(const HelloWorld& hello);

template<>
void default_send_print(const String& str);

template<>
void default_send_print(const Data64kb& data);

template<>
void default_send_print(const Data1mb& data);

/****** Auxiliary data generators *******/
std::list<HelloWorld> default_helloworld_data_generator(size_t max = 0);

std::list<String> default_large_string_data_generator(size_t max = 0);

std::list<Data64kb> default_data64kb_data_generator(size_t max = 0);

std::list<Data1mb> default_data300kb_data_generator(size_t max = 0);

std::list<Data1mb> default_data300kb_mix_data_generator(size_t max = 0);

/****** Auxiliary lambda functions  ******/
extern const std::function<void(const HelloWorld&)>  default_helloworld_print;

extern const std::function<void(const String&)>  default_string_print;

extern const std::function<void(const Data64kb&)>  default_data64kb_print;

extern const std::function<void(const Data1mb&)>  default_data300kb_print;

template<typename T>
void print_non_received_messages(const std::list<T>& data, const std::function<void(const T&)>& printer)
{
    if(data.size() != 0)
    {
        std::cout << "Samples not received: ";
        std::for_each(data.begin(), data.end(), printer);
        std::cout << std::endl;
    }
}
/***** End auxiliary lambda function *****/

#endif // __BLACKBOX_BLACKBOXTESTS_HPP__
