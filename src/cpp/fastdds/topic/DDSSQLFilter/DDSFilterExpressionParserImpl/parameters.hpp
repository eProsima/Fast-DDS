// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file parameters.hpp
 *
 * Note: this is an implementation file, designed to be included inside the
 * DDSFilterExpressionParser.hpp file of the parent folder.
 */

struct parameter_processor
    : parse_tree::apply< parameter_processor >
{
    template< typename ... States >
    static void transform(
            std::unique_ptr< ParseNode >& n,
            States&&... /*st*/)
    {
        n->parameter_index = static_cast<int32_t>(n->m_begin.data[1] - '0');
        if (n->m_end.byte - n->m_begin.byte == 3)
        {
            n->parameter_index *= 10;
            n->parameter_index += static_cast<int32_t>(n->m_begin.data[2] - '0');
        }
    }

};
