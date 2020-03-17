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

#ifndef EPROSIMA_DDS_CORE_XTYPES_DETAIL_COLLECTION_TYPES_HPP_
#define EPROSIMA_DDS_CORE_XTYPES_DETAIL_COLLECTION_TYPES_HPP_

#include <dds/core/xtypes/detail/DynamicType.hpp>

namespace dds {
namespace core {
namespace xtypes {
namespace detail {

class CollectionType : public DynamicType
{
};
class MapType : public CollectionType
{
};
class SequenceType : public CollectionType
{
};
class StringType : public CollectionType
{
};

} //namespace detail
} //namespace xtypes
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_XTYPES_DETAIL_COLLECTION_TYPES_HPP_

