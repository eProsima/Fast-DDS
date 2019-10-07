/*
 * Copyright 2010, Object Management Group, Inc.
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

#ifndef OMG_DDS_CORE_XTYPES_DYNAMIC_COLLECTION_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_DYNAMIC_COLLECTION_TYPE_HPP_

#include <dds/core/xtypes/CollectionType.hpp>

namespace dds {
namespace core {
namespace xtypes {

class DynamicCollectionType : public CollectionType
{
public:
    uint32_t bounds() const { return bounds_; }

protected:
    DynamicCollectionType(
            TypeKind kind,
            DynamicType::Ptr&& content,
            uint32_t bounds)
        : CollectionType(kind, std::move(content))
        , bounds_(bounds)
    {}

private:
    uint32_t bounds_;
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_COLLECTION_TYPE_HPP_
