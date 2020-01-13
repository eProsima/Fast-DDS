/* Copyright 2010, Object Management Group, Inc.
* Copyright 2010, PrismTech, Corp.
* Copyright 2010, Real-Time Innovations, Inc.
* All rights reserved.
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

#ifndef OMG_DDS_CORE_BUILTIN_TOPIC_TYPES_HPP_
#define OMG_DDS_CORE_BUILTIN_TOPIC_TYPES_HPP_

#include <dds/core/detail/conformance.hpp>
#include <dds/core/TBuiltinTopicTypes.hpp>
#include <dds/core/detail/BuiltinTopicTypes.hpp>

#if defined (OMG_DDS_X_TYPES_BUILTIN_TOPIC_TYPES_SUPPORT)
namespace dds
{
namespace core
{
/**
 * @file
 * This file contains the type definitions for BuiltinTopicTypes
 */
typedef dds::core::detail::BytesTopicType BytesTopicType;
typedef dds::core::detail::StringTopicType StringTopicType;
typedef dds::core::detail::KeyedBytesTopicType KeyedBytesTopicType;
typedef dds::core::detail::KeyedStringTopicType KeyedStringTopicType;
}
}

#endif

#endif /* OMG_DDS_CORE_BUILTIN_TOPIC_TYPES_HPP_ */
