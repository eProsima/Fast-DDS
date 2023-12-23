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

#ifndef UTILS__STRING_UTILITIES_HPP_
#define UTILS__STRING_UTILITIES_HPP_

template<typename _Enum>
static bool get_element_enum_value(
        const char* input,
        _Enum& output,
        const char* name,
        _Enum value)
{
    if (0 == strcmp(input, name))
    {
        output = value;
        return true;
    }

    return false;
}

template<typename _Enum, typename ... Targs>
static bool get_element_enum_value(
        const char* input,
        _Enum& output,
        const char* name,
        _Enum value,
        Targs... args)
{
    if (0 == strcmp(input, name))
    {
        output = value;
        return true;
    }

    return get_element_enum_value(input, output, args ...);
}

#endif // UTILS__STRING_UTILITIES_HPP_
