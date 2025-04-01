// Copyright 2019, 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <list>
#include <sstream>

std::list<HelloWorld> default_helloworld_data_generator(
        size_t max)
{
    uint16_t index = 1;
    size_t maximum = max ? max : 10;
    std::list<HelloWorld> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
            {
                HelloWorld hello;
                hello.index(index);
                std::stringstream ss;
                ss << "HelloWorld " << index;
                hello.message(ss.str());
                ++index;
                return hello;
            });

    return returnedValue;
}

std::list<FixedSized> default_fixed_sized_data_generator(
        size_t max)
{
    uint16_t index = 1;
    size_t maximum = max ? max : 10;
    std::list<FixedSized> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
            {
                FixedSized fs;
                fs.index(index);
                ++index;
                return fs;
            });

    return returnedValue;
}

std::list<KeyedHelloWorld> default_keyedhelloworld_data_generator(
        size_t max,
        bool unique_key)
{
    uint16_t index = 0;
    size_t maximum = max ? max : 10;
    std::list<KeyedHelloWorld> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index, unique_key]
            {
                KeyedHelloWorld hello;
                hello.index(index);
                hello.key(unique_key
                        ? (index + 1)
                        : (index % 2 + 1));
                std::stringstream ss;
                ss << "HelloWorld " << index;
                hello.message(ss.str());
                ++index;
                return hello;
            });

    return returnedValue;
}

std::list<KeyedHelloWorld> default_keyedhelloworld_per_participant_data_generator(
        size_t participants,
        size_t max)
{
    uint16_t participant_key = 0;
    uint16_t index = 0;
    size_t maximum = max ? max : 10;
    std::list<KeyedHelloWorld> returnedValue(maximum * participants);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index, &participant_key, &maximum]
            {
                KeyedHelloWorld hello;
                hello.index(index);
                hello.key(participant_key);
                std::stringstream ss;
                ss << "HelloWorld " << index;
                hello.message(ss.str());
                ++index;
                if (index == maximum)
                {
                    index = 0;
                    ++participant_key;
                }
                return hello;
            });

    return returnedValue;
}

std::list<StringTest> default_large_string_data_generator(
        size_t max)
{
    uint16_t index = 1;
    size_t maximum = max ? max : 10;
    std::list<StringTest> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
            {
                StringTest str;
                std::stringstream ss;
                ss << std::string(998, 'a') << std::setw(2) << std::setfill('0') << index;
                str.message(ss.str());
                ++index;
                return str;
            });

    return returnedValue;
}

const size_t data64kb_length = 63996;
std::list<Data64kb> default_data64kb_data_generator(
        size_t max)
{
    unsigned char index = 1;
    size_t maximum = max ? max : 10;
    std::list<Data64kb> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
            {
                Data64kb data;
                data.data().resize(data64kb_length);
                data.data()[0] = index;
                for (size_t i = 1; i < data64kb_length; ++i)
                {
                    data.data()[i] = static_cast<unsigned char>(i + data.data()[0]);
                }
                ++index;
                return data;
            });

    return returnedValue;
}

const size_t data16kb_length = 16384;
std::list<Data1mb> default_data16kb_data_generator(
        size_t max)
{
    unsigned char index = 1;
    size_t maximum = max ? max : 10;
    std::list<Data1mb> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
            {
                Data1mb data;
                data.data().resize(data16kb_length);
                data.data()[0] = index;
                for (size_t i = 1; i < data16kb_length; ++i)
                {
                    data.data()[i] = static_cast<unsigned char>(i + data.data()[0]);
                }
                ++index;
                return data;
            });

    return returnedValue;
}

const size_t data300kb_length = 307201;
std::list<Data1mb> default_data300kb_data_generator(
        size_t max)
{
    unsigned char index = 1;
    size_t maximum = max ? max : 10;
    std::list<Data1mb> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
            {
                Data1mb data;
                data.data().resize(data300kb_length);
                data.data()[0] = index;
                for (size_t i = 1; i < data300kb_length; ++i)
                {
                    data.data()[i] = static_cast<unsigned char>(i + data.data()[0]);
                }
                ++index;
                return data;
            });

    return returnedValue;
}

std::list<Data1mb> default_data300kb_mix_data_generator(
        size_t max)
{
    unsigned char index = 1;
    size_t maximum = max ? max : 10;
    std::list<Data1mb> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
            {
                Data1mb data;
                size_t length = index % 2 != 0 ? data300kb_length : 30000;
                data.data().resize(length);
                data.data()[0] = index;
                for (size_t i = 1; i < length; ++i)
                {
                    data.data()[i] = static_cast<unsigned char>(i + data.data()[0]);
                }
                ++index;
                return data;
            });

    return returnedValue;
}

const size_t data96kb_length = 96 * 1024;
std::list<Data1mb> default_data96kb_data300kb_data_generator(
        size_t max)
{
    unsigned char index = 1;
    size_t maximum = max ? max : 10;
    std::list<Data1mb> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
            {
                Data1mb data;
                size_t length = index % 2 != 0 ? data96kb_length : data300kb_length;
                data.data().resize(length);
                data.data()[0] = index;
                for (size_t i = 1; i < length; ++i)
                {
                    data.data()[i] = static_cast<unsigned char>(i + data.data()[0]);
                }
                ++index;
                return data;
            });

    return returnedValue;
}

const size_t data100kb_length = 102400;
std::list<Data100kb> default_data100kb_data_generator(
        size_t max)
{
    unsigned char index = 1;
    size_t maximum = max ? max : 10;
    std::list<Data100kb> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
            {
                Data100kb data;
                data.data().resize(data100kb_length);
                data.data()[0] = index;
                for (size_t i = 1; i < data100kb_length; ++i)
                {
                    data.data()[i] = static_cast<unsigned char>(i + data.data()[0]);
                }
                ++index;
                return data;
            });

    return returnedValue;
}

std::list<KeyedData1mb> default_keyeddata300kb_data_generator(
        size_t max)
{
    uint16_t index = 1;
    size_t maximum = max ? max : 10;
    std::list<KeyedData1mb> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
            {
                KeyedData1mb data;
                data.key(index % 2 + 1);
                data.data().resize(data300kb_length);
                data.data()[0] = static_cast<unsigned char>(index);
                for (size_t i = 1; i < data300kb_length; ++i)
                {
                    data.data()[i] = static_cast<unsigned char>(i + data.data()[0]);
                }
                ++index;
                return data;
            });

    return returnedValue;
}

std::list<UnboundedHelloWorld> default_unbounded_helloworld_data_generator(
        size_t max)
{
    uint16_t index = 1;
    size_t maximum = max ? max : 10;
    std::list<UnboundedHelloWorld> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
            {
                UnboundedHelloWorld hello;
                hello.index(index);
                std::stringstream ss;
                ss << "HelloWorld " << index;
                hello.message(ss.str());
                ++index;
                return hello;
            });

    return returnedValue;
}
