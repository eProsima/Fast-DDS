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
 * @file QosPolicyUtils.hpp
 *
 */

#ifndef _FASTDDS_DDS_QOS_QOSPOLICYUTILS_HPP_
#define _FASTDDS_DDS_QOS_QOSPOLICYUTILS_HPP_

#include <stdint.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace utils {

// Compute the default DataSharing domain ID
uint64_t default_domain_id();

}  // namespace utils
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif // _FASTDDS_DDS_QOS_QOSPOLICYUTILS_HPP_
