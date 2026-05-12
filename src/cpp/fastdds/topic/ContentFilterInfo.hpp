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
#include <vector>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
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
 * @param [in,out] payload            Serialized payload on which to perform the serialization.
 * @param [in]     readers_to_filter  List of reader GUIDs whose associated filters are to be evaluated.
 * @param [in]     evaluate           A functor for the evaluation of a filter, receiving two arguments:
 *                                    - @c size_t   index:     index of the filter being evaluated.
 *                                    - @c uint8_t* signature: pointer to the serialization buffer where the 128-bit
 *                                                             filter signature should be written.
 *
 * @pre @c payload should have enough space left for the serialization of the ContentFilterInfo parameter.
 */
template<typename FilterEvaluate>
void cdr_serialize(
        fastdds::rtps::SerializedPayload_t& payload,
        const std::vector<fastdds::rtps::GUID_t>& readers_to_filter,
        FilterEvaluate evaluate)
{
    using namespace fastdds::rtps;

    std::size_t num_filters = readers_to_filter.size();
    uint16_t cdr_size = 0;
    if ((0 < num_filters) && cdr_serialized_size(num_filters, cdr_size))
    {
        fastdds::rtps::CDRMessage_t msg(payload);

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
        for (const auto& reader_to_filter : readers_to_filter)
        {
            // Evaluate and write signature
            if (evaluate(reader_to_filter, &msg.buffer[msg.pos]))
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

/**
 * Read ContentFilterInfo parameter from inline QoS serialized payload.
 *
 * @param [in]  inline_qos          Serialized payload containing the inline QoS where to search for
 *                                  the ContentFilterInfo parameter.
 * @param [out] info_found          Whether a ContentFilterInfo parameter was found.
 * @param [out] read_end_pos        Position in @c inline_qos where parameter read ends.
 * @param [out] num_bitmaps         Number of bitmaps in the ContentFilterInfo parameter.
 * @param [out] bitmaps_init_pos    Position in @c inline_qos where the bitmaps start.
 * @param [out] num_signatures      Number of signatures in the ContentFilterInfo parameter.
 * @param [out] signatures_init_pos Position in @c inline_qos where the signatures start.
 *
 * @return whether reading succeeded or not (success != CFT parameter found).
 */
inline bool read_cdr_parameter(
        const fastdds::rtps::SerializedPayload_t& inline_qos,
        bool& info_found,
        uint32_t& read_end_pos,
        uint32_t& num_bitmaps,
        uint32_t& bitmaps_init_pos,
        uint32_t& num_signatures,
        uint32_t& signatures_init_pos)
{
    // Reset output values
    info_found = false;
    read_end_pos = 0;
    num_bitmaps = 0;
    bitmaps_init_pos = 0;
    num_signatures = 0;
    signatures_init_pos = 0;

    auto parameter_process = [&](
        fastdds::rtps::CDRMessage_t* msg,
        const ParameterId_t pid,
        uint16_t plength)
            {
                if (PID_CONTENT_FILTER_INFO == pid)
                {
                    bool valid;

                    // Validate and consume length for numBitmaps and numSignatures
                    valid = 8 < plength;
                    if (!valid)
                    {
                        return false;
                    }
                    plength -= 8;

                    // Read and validate numBitmaps
                    valid &= fastdds::rtps::CDRMessage::readUInt32(msg, &num_bitmaps);
                    valid &= num_bitmaps * 4 <= plength;
                    if (!valid || 0 == num_bitmaps)
                    {
                        return false;
                    }

                    // Save starting position of bitmaps after numBitmaps
                    bitmaps_init_pos = msg->pos;

                    // Skip bitmaps to read signatures section
                    msg->pos += num_bitmaps * 4;
                    plength -= static_cast<uint16_t>(num_bitmaps * 4);

                    // Read and validate numSignatures
                    valid &= fastdds::rtps::CDRMessage::readUInt32(msg, &num_signatures);
                    valid &= num_signatures * 16 <= plength;
                    if (!valid || 0 == num_signatures || ((num_signatures + 31) / 32) != num_bitmaps)
                    {
                        return false;
                    }

                    // Save starting position of signatures after numSignatures
                    signatures_init_pos = msg->pos;

                    info_found = true;

                    // Stop processing parameters
                    // NOTE: should there be multiple ContentFilterInfo parameters, only modify first one found
                    return false;
                }

                return true;
            };

    // Look for content filter info parameter and read its values
    fastdds::rtps::CDRMessage_t msg(inline_qos);
    msg.pos = 0;
    uint32_t qos_size = 0;
    if (!ParameterList::readParameterListfromCDRMsg(msg, parameter_process, false,
            qos_size) && !info_found && msg.pos < msg.length)
    {
        // CDR deserialization failed
        return false;
    }

    // Set read end position to adjust coordinates from local (wrt to content filter param) to inline_qos context
    read_end_pos = msg.pos;
    return true;
}

/**
 * Update ContentFilterInfo parameter in inline QoS serialized payload.
 *
 * @param [in,out] inline_qos          Serialized payload containing the inline QoS where to update
 *                                     the ContentFilterInfo parameter.
 * @param [in]     filter_result       Result of the filter evaluation to use on update.
 * @param [in]     filter_signature    Signature of the filter whose result is being added.
 * @param [in]     info_start_pos      Position in @c inline_qos where the ContentFilterInfo parameter starts.
 * @param [in]     num_bitmaps         Number of bitmaps to set in the ContentFilterInfo parameter.
 * @param [in]     bitmaps_init_pos    Position in @c inline_qos where the bitmaps start.
 * @param [in]     num_signatures      Number of signatures to set in the ContentFilterInfo parameter.
 * @param [in]     signatures_init_pos Position in @c inline_qos where the signatures start.
 *
 * @return whether parameter update succeeded or not.
 */
inline bool update_cdr_parameter(
        fastdds::rtps::SerializedPayload_t& inline_qos,
        bool filter_result,
        const std::array<uint8_t, 16>& filter_signature,
        uint32_t info_start_pos,
        uint32_t num_bitmaps,
        uint32_t bitmaps_init_pos,
        uint32_t num_signatures,
        uint32_t signatures_init_pos)
{
    fastdds::rtps::CDRMessage_t msg(inline_qos);

    uint16_t cdr_size = 0;
    if (!cdr_serialized_size(num_signatures, cdr_size))
    {
        // Abort due to overflow in size calculation (too many filters)
        return false;
    }

    // Modify parameter length value
    msg.pos = info_start_pos + 2u;
    msg.length -= 2u;
    fastdds::rtps::CDRMessage::addUInt16(&msg, cdr_size - 4u);

    // Check if a new bitmap is needed
    if ((num_signatures & 31u) == 1u)
    {
        // Increase number of bitmaps by one
        num_bitmaps += 1;

        // Shift signatures to make room for new bitmap
        const uint32_t shift_size = 4;
        fastdds::rtps::octet* num_signatures_pos = msg.buffer + signatures_init_pos - 4u;
        std::memmove(num_signatures_pos + shift_size, num_signatures_pos, 4u + (num_signatures - 1) * 16);
        signatures_init_pos += shift_size;
        msg.length += shift_size;

        // Update number of bitmaps in message
        msg.pos = bitmaps_init_pos - 4u;
        msg.length -= 4;
        fastdds::rtps::CDRMessage::addUInt32(&msg, num_bitmaps);

        // Add new bitmap according to filter result
        uint32_t bitmap = filter_result ? 1u : 0u;
        assert(num_bitmaps >= 1);
        msg.pos = bitmaps_init_pos + (num_bitmaps - 1) * 4;
        msg.length -= 4;
        fastdds::rtps::CDRMessage::addUInt32(&msg, bitmap);
    }
    else if (filter_result) // NOTE: No need to modify bitmap if filter result is false
    {
        // Read current (last) bitmap
        assert(num_bitmaps >= 1);
        msg.pos = bitmaps_init_pos + (num_bitmaps - 1) * 4;
        uint32_t bitmap = 0;
        fastdds::rtps::CDRMessage::readUInt32(&msg, &bitmap);

        // Update bitmap by setting the corresponding bit
        uint32_t bitmask = 1 << (31 - ((num_signatures - 1) & 31));
        bitmap |= bitmask;

        // Write back updated bitmap
        msg.pos = bitmaps_init_pos + (num_bitmaps - 1) * 4;
        msg.length -= 4;
        fastdds::rtps::CDRMessage::addUInt32(&msg, bitmap);
    }

    // Update number of signatures in message
    msg.pos = signatures_init_pos - 4u;
    msg.length -= 4;
    fastdds::rtps::CDRMessage::addUInt32(&msg, num_signatures);

    // Add new signature at the end of the signatures section
    msg.pos += (num_signatures - 1) * 16;
    std::copy(filter_signature.begin(), filter_signature.end(), msg.buffer + msg.pos);
    msg.length += 16;

    // Update inline_qos length and pos
    inline_qos.length = msg.length;
    inline_qos.pos = msg.length;

    return true;
}

}  // namespace ContentFilterInfo
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_CONTENTFILTERINFO_HPP_
