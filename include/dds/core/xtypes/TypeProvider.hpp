/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
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
 */

#ifndef OMG_DDS_CORE_XTYPES_TYPE_PROVIDER_HPP_
#define OMG_DDS_CORE_XTYPES_TYPE_PROVIDER_HPP_

#include <dds/core/xtypes/detail/TypeProvider.hpp>

#include <dds/core/xtypes/DynamicType.hpp>

namespace dds {
namespace core {
namespace xtypes {

/**
 * TypeProvider that allows creation of types from external representations.
 */
template<typename DELEGATE>
class TTypeProvider
{
public:
    /**
     * Load a type from the specified URI. If multiple types are defined
     * only the first one is returned.
     */
    static TDynamicType<DELEGATE> load_type(
            const std::string& uri)
    {
        throw "Not implemented";
    }

    /**
     * Load a type from the specified URI. If multiple types are defined
     * only the first one is returned.
     */
    static std::vector<TDynamicType<DELEGATE>> load_types(
            const std::string& uri)
    {
        throw "Not implemented";
    }

    /**
     * Load a named type from the specified URI.
     */
    static TDynamicType<DELEGATE> load_type(
            const std::string& uri,
            const std::string& name)
    {
        throw "Not implemented";
    }
};

typedef TTypeProvider<detail::TypeProvider> TypeProvider;

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_TYPE_PROVIDER_HPP_
