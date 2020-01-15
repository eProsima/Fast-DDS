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

#include "../BlackboxTests.hpp"

std::list<HelloWorld> default_helloworld_data_generator(size_t max)
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

std::list<FixedSized> default_fixed_sized_data_generator(size_t max)
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

std::list<KeyedHelloWorld> default_keyedhelloworld_data_generator(size_t max)
{
    uint16_t index = 0;
    size_t maximum = max ? max : 10;
    std::list<KeyedHelloWorld> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
    {
        KeyedHelloWorld hello;
        hello.index(index);
        hello.key(index % 2);
        std::stringstream ss;
        ss << "HelloWorld " << index;
        hello.message(ss.str());
        ++index;
        return hello;
    });

    return returnedValue;
}

std::list<String> default_large_string_data_generator(size_t max)
{
    uint16_t index = 1;
    size_t maximum = max ? max : 10;
    std::list<String> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index]
    {
        String str;
        std::stringstream ss;
        ss << std::string(998, 'a') << std::setw(2) << std::setfill('0') << index;
        str.message(ss.str());
        ++index;
        return str;
    });

    return returnedValue;
}

const size_t data64kb_length = 63996;
std::list<Data64kb> default_data64kb_data_generator(size_t max)
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

const size_t data300kb_length = 307201;
std::list<Data1mb> default_data300kb_data_generator(size_t max)
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

std::list<Data1mb> default_data300kb_mix_data_generator(size_t max)
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
