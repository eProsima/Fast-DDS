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

/**
 * Compute a filter signature according to RTPS 2.5 section 9.6.4.1
 *
 * @param [in]  filter_property   Filtering discovery information from which to compute the filter signature.
 * @param [out] filter_signature  Filter signature.
 */
void compute_signature(
        const rtps::ContentFilterProperty& filter_property,
        std::array<uint8_t, 16>& filter_signature);

/**
 * Compute two filter signatures, one according to RTPS 2.5 section 9.6.4.1, and one interoperable with
 * RTI Connext 6.1 and below.
 *
 * @param [in]  filter_property               Filtering discovery information from which to compute the filter
 *                                            signatures.
 * @param [out] filter_signature_rtps         Filter signature according to RTPS 2.5 section 9.6.4.1.
 * @param [out] filter_signature_rti_connext  Filter signature interoperable with RTI Connext 6.1 and below.
 */
void compute_signature(
        const rtps::ContentFilterProperty& filter_property,
        std::array<uint8_t, 16>& filter_signature_rtps,
        std::array<uint8_t, 16>& filter_signature_rti_connext);

}  // namespace ContentFilterUtils
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_CONTENTFILTERUTILS_HPP_
