/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <dds/core/xtypes/TypeProvider.hpp>

namespace dds {
namespace core {
namespace xtypes {

template<typename DELEGATE>
TDynamicType<DELEGATE> TTypeProvider<DELEGATE>::load_type(
        const std::string& uri)
{
    (void) uri;
}

template<typename DELEGATE>
std::vector<TDynamicType<DELEGATE>> TTypeProvider<DELEGATE>::load_types(
        const std::string& uri)
{
    (void) uri;
}

template<typename DELEGATE>
TDynamicType<DELEGATE> TTypeProvider<DELEGATE>::load_type(
        const std::string& uri,
        const std::string& name)
{
    (void) uri;
    (void) name;
}

} //namespace xtypes
} //namespace core
} //namespace dds
