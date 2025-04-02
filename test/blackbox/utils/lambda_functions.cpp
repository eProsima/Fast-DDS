// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "../common/BlackboxTests.hpp"

#include <functional>
#include <iostream>

const std::function<void(const HelloWorld&)>  default_helloworld_print = [](const HelloWorld& hello)
        {
            std::cout << hello.index() << " ";
        };

const std::function<void(const FixedSized&)>  default_fixed_sized_print = [](const FixedSized& hello)
        {
            std::cout << hello.index() << " ";
        };

const std::function<void(const KeyedHelloWorld&)>  default_keyedhelloworld_print = [](const KeyedHelloWorld& hello)
        {
            std::cout << hello.message() << " " << hello.key();
        };

const std::function<void(const StringTest&)> default_string_print = [](const StringTest& str)
        {
            std::cout << str.message()[str.message().size() - 2]
                      << str.message()[str.message().size() - 1] << " ";
        };

const std::function<void(const Data64kb&)> default_data64kb_print = [](const Data64kb& data)
        {
            std::cout << (uint16_t)data.data()[0] << " ";
        };

const std::function<void(const Data100kb&)> default_data100kb_print = [](const Data100kb& data)
        {
            std::cout << (uint16_t)data.data()[0] << " ";
        };

const std::function<void(const Data1mb&)> default_data300kb_print = [](const Data1mb& data)
        {
            std::cout << (uint16_t)data.data()[0] << " ";
        };

const std::function<void(const KeyedData1mb&)> default_keyeddata300kb_print = [](const KeyedData1mb& data)
        {
            std::cout << data.key() << " " << (uint16_t)data.data()[0] << " ";
        };
