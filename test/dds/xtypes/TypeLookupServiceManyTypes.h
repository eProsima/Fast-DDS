// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeLookupServiceManyTypes.h
 *
 */

#ifndef _TEST_DDS_XTYPES_TYPELOOKUPSERVICETEST_MANYTYPES_H_
#define _TEST_DDS_XTYPES_TYPELOOKUPSERVICETEST_MANYTYPES_H_

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Creates a writer for each of @p num_types distinct types and serves their type information until the subscriber
 * leaves.
 * @return 0 on success.
 */
int run_many_types_publisher(
        uint32_t domain_id,
        uint32_t num_types,
        uint32_t timeout_s);

/**
 * Discovers the writers of the publisher, dropping the acknowledgements of the TypeLookup replies during the first
 * seconds, and checks that the types of all of them are eventually resolved.
 * @return 0 on success.
 */
int run_many_types_subscriber(
        uint32_t domain_id,
        uint32_t num_types,
        uint32_t timeout_s);

} // dds
} // fastdds
} // eprosima

#endif /* _TEST_DDS_XTYPES_TYPELOOKUPSERVICETEST_MANYTYPES_H_ */
