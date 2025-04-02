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

#include <iostream>

template<>
void default_receive_print(
        const HelloWorld& hello)
{
    std::cout << "Received HelloWorld " << hello.index() << std::endl;
}

template<>
void default_receive_print(
        const KeyedHelloWorld& hello)
{
    std::cout << "Received HelloWorld " << hello.index() << " with key " << hello.key() << std::endl;
}

template<>
void default_receive_print(
        const FixedSized& hello)
{
    std::cout << "Received FixedSized " << hello.index() << std::endl;
}

template<>
void default_receive_print(
        const StringTest& str)
{
    std::cout << "Received String " << str.message()[str.message().size() - 2]
              << str.message()[str.message().size() - 1] << std::endl;
}

template<>
void default_receive_print(
        const Data64kb& data)
{
    std::cout << "Received Data64kb " << (uint16_t)data.data()[0] << std::endl;
}

template<>
void default_receive_print(
        const Data100kb& data)
{
    std::cout << "Received Data100kb " << (uint16_t)data.data()[0] << std::endl;
}

template<>
void default_receive_print(
        const Data1mb& data)
{
    std::cout << "Received Data1mb " << (uint16_t)data.data()[0] << std::endl;
}

template<>
void default_receive_print(
        const KeyedData1mb& data)
{
    std::cout << "Received KeyedData1mb " << data.key() << " " << (uint16_t)data.data()[0] << std::endl;
}

template<>
void default_send_print(
        const StringTestPubSubType&)
{
    std::cout << "Sent StringType" << std::endl;
}

template<>
void default_send_print(
        const HelloWorld& hello)
{
    std::cout << "Sent HelloWorld " << hello.index() << std::endl;
}

template<>
void default_send_print(
        const KeyedHelloWorld& hello)
{
    std::cout << "Sent HelloWorld " << hello.index() << " with key " << hello.key() << std::endl;
}

template<>
void default_send_print(
        const FixedSized& hello)
{
    std::cout << "Sent FixedSized " << hello.index() << std::endl;
}

template<>
void default_send_print(
        const StringTest& str)
{
    std::cout << "Sent String " << str.message()[str.message().size() - 2]
              << str.message()[str.message().size() - 1] << std::endl;
}

template<>
void default_send_print(
        const Data64kb& data)
{
    std::cout << "Sent Data64kb " << (uint16_t)data.data()[0] << std::endl;
}

template<>
void default_send_print(
        const Data100kb& data)
{
    std::cout << "Sent Data100kb " << (uint16_t)data.data()[0] << std::endl;
}

template<>
void default_send_print(
        const Data1mb& data)
{
    std::cout << "Sent Data1mb " << (uint16_t)data.data()[0] << std::endl;
}

template<>
void default_send_print(
        const KeyedData1mb& data)
{
    std::cout << "Sent KeyedData1mb " << data.key() << " " << (uint16_t)data.data()[0] << std::endl;
}
