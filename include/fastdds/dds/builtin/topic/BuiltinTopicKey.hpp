// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BuiltinTopicKey.hpp
 *
 */

#ifndef FASTDDS_DDS_BUILTIN_TOPIC_BUILTINTOPICKEY_HPP
#define FASTDDS_DDS_BUILTIN_TOPIC_BUILTINTOPICKEY_HPP

#include <stdint.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace builtin {

// following API definition:
// #define BUILTIN_TOPIC_KEY_TYPE_NATIVE uint32_t

struct BuiltinTopicKey_t
{
    // BUILTIN_TOPIC_KEY_TYPE_NATIVE = long type
    //!Value
    uint32_t value[3];
};

} // builtin
} // dds
} // fastdds
} // eprosima

#endif // FASTDDS_DDS_BUILTIN_TOPIC_BUILTINTOPICKEY_HPP
