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
 * @file ContentFilterUtils.hpp
 */

#ifndef _FASTDDS_TOPIC_CONTENTFILTERUTILS_HPP_
#define _FASTDDS_TOPIC_CONTENTFILTERUTILS_HPP_

#include <array>
#include <cstddef>
#include <cstdint>

#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace ContentFilterUtils {

void compute_signature(
        const rtps::ContentFilterProperty& filter_property,
        std::array<uint8_t, 16>& filter_signature);

void compute_signature(
        const rtps::ContentFilterProperty& filter_property,
        std::array<uint8_t, 16>& filter_signature_rtps,
        std::array<uint8_t, 16>& filter_signature_rti_connext);

}  // namespace ContentFilterUtils
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_CONTENTFILTERUTILS_HPP_
