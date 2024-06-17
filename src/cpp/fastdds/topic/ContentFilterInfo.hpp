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
 * @file ContentFilterInfo.hpp
 */

#ifndef _FASTDDS_TOPIC_CONTENTFILTERINFO_HPP_
#define _FASTDDS_TOPIC_CONTENTFILTERINFO_HPP_

#include <cstddef>
#include <cstdint>
#include <limits>

#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <rtps/messages/CDRMessage.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace ContentFilterInfo {

/**
 * CDR serialized size calculation for ContentFilterInfo.
 *
 * @param [in]  num_filters  The number of filters for which a ContentFilterInfo needs to be generated.
 * @param [out] cdr_size     The CDR size of a PID_CONTENT_FILTER_INFO parameter holding information
 *                           for @c num_filters filters.
 *                           Will be 0 if the function returns false, indicating the number of filters is too big.
 *                           Will be 0 if the number of filters is 0, indicating no information needs to be exchanged.
 *
 * @return whether a ContentFilterInfo parameter can be generated for the number of filters specified.
 */
inline bool cdr_serialized_size(
        std::size_t num_filters,
        uint16_t& cdr_size)
{
    bool ret_val = true;
    cdr_size = 0;

    if (0 < num_filters)
    {
        // PID + PLENGTH
        std::size_t tmp_size = 2 + 2;

        // numBitmaps
        tmp_size += 4;

        // Bitmaps
        std::size_t num_bitmaps = (num_filters + 31) / 32;
        tmp_size += 4 * num_bitmaps;

        // numSignatures
        tmp_size += 4;

        // Signatures
        tmp_size += 16 * num_filters;

        ret_val = (tmp_size <= std::numeric_limits<uint16_t>::max());
        if (ret_val)
        {
            cdr_size = static_cast<uint16_t>(tmp_size);
        }
    }

    return ret_val;
}

inline void add_bitmap(
        fastdds::rtps::CDRMessage_t& msg,
        uint32_t current_bitmap,
        uint32_t& bitmap_pos)
{
    uint32_t old_len = msg.length;
    uint32_t old_pos = msg.pos;
    msg.pos = bitmap_pos;
    msg.length = bitmap_pos;
    fastdds::rtps::CDRMessage::addUInt32(&msg, current_bitmap);
    bitmap_pos = msg.pos;
    msg.pos = old_pos;
    msg.length = old_len;
}

/**
 * Perform CDR serialization of a ContentFilterInfo parameter.
 *
 * @param [in,out] payload      Serialized payload on which to perform the serialization.
 * @param [in]     num_filters  The number of filters for which the ContentFilterInfo will be generated.
 * @param [in]     evaluate     A functor for the evaluation of a filter, receiving two arguments:
 *                              - @c size_t   index:     index of the filter being evaluated.
 *                              - @c uint8_t* signature: pointer to the serialization buffer where the 128-bit
 *                                                       filter signature should be written.
 *
 * @pre @c payload should have enough space left for the serialization of the ContentFilterInfo parameter.
 */
template<typename FilterEvaluate>
void cdr_serialize(
        fastdds::rtps::SerializedPayload_t& payload,
        std::size_t num_filters,
        FilterEvaluate evaluate)
{
    using namespace fastdds::rtps;

    uint16_t cdr_size = 0;
    if ((0 < num_filters) && cdr_serialized_size(num_filters, cdr_size))
    {
        CDRMessage_t msg(payload);

        // PID + PLENGTH
        CDRMessage::addUInt16(&msg, PID_CONTENT_FILTER_INFO);
        CDRMessage::addUInt16(&msg, cdr_size - 4u);

        // numBitmaps
        uint32_t num_bitmaps = (static_cast<uint32_t>(num_filters) + 31ul) / 32ul;
        CDRMessage::addUInt32(&msg, num_bitmaps);

        // Prepare bitmap information and skip to signatures
        constexpr uint32_t initial_mask = 1ul << 31;
        uint32_t bitmap_pos = msg.pos;
        uint32_t current_bitmap = 0;
        uint32_t current_mask = initial_mask;
        msg.pos += 4 * num_bitmaps;
        msg.length += 4 * num_bitmaps;

        // numSignatures
        CDRMessage::addUInt32(&msg, static_cast<uint32_t>(num_filters));

        // Signatures
        for (std::size_t i = 0; i < num_filters; ++i)
        {
            // Evaluate and write signature
            if (evaluate(i, &msg.buffer[msg.pos]))
            {
                current_bitmap |= current_mask;
            }

            // Advance position
            msg.pos += 16;
            msg.length += 16;

            // Advance bitmap
            current_mask >>= 1;
            if (!current_mask)
            {
                add_bitmap(msg, current_bitmap, bitmap_pos);

                current_bitmap = 0;
                current_mask = initial_mask;
            }
        }

        // Write last bitmap
        if (initial_mask != current_mask)
        {
            add_bitmap(msg, current_bitmap, bitmap_pos);
        }

        payload.length = msg.length;
        payload.pos = msg.pos;
    }
}

}  // namespace ContentFilterInfo
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_CONTENTFILTERINFO_HPP_
