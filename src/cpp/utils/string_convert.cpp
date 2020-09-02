// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file string_convert.cpp
 *
 */

#include <fastrtps/utils/string_convert.hpp>
#include <locale>
#include <stdexcept>
#include <codecvt> // deprecated but old compilers require it

namespace eprosima {
namespace fastrtps {

template<typename ochar, typename ichar>
std::basic_string<ochar> string_convert(
        const ichar* pbeg,
        const ichar* pend)
{
    using namespace std;
    using ostring = std::basic_string<ochar>;

    locale loc;
    auto& conv_facet = use_facet<codecvt<ochar, ichar, mbstate_t> >(loc);

    const unsigned int buffer_size = 256;
    ochar buffer[buffer_size];

    mbstate_t mb;
    const ichar* from_next;
    ochar* to_next;
    ostring output;

    bool processing = true;
    from_next = pbeg;
    // iterate if buffer gets filled
    while (processing)
    {
        codecvt_base::result res =
                conv_facet.in(mb, from_next, pend, from_next, buffer, buffer + buffer_size, to_next);

        switch (res)
        {
            case codecvt_base::ok:
                // we are done
                processing = false;
                // append the contents remember
                output.append(buffer, to_next - buffer);
                break;

            case codecvt_base::partial:
                // insert current buffer content
                output.append(buffer, buffer_size);
                break;

            case codecvt_base::error:
                throw std::range_error("this facet is non-converting, no output written");
            case codecvt_base::noconv:
                throw std::range_error("encountered a character that could not be converted");
        }
    }

    return output;
}

std::wstring wstring_from_bytes(
        const std::string& str)
{
    const char* pbeg = str.c_str();
    return string_convert<wchar_t>(pbeg, pbeg + str.size());
}

std::string wstring_to_bytes(
        const std::wstring& str)
{
    const wchar_t* pbeg = str.c_str();
    return string_convert<char>(pbeg, pbeg + str.size());
}

} /* namespace fastrtps */
} /* namespace eprosima */
