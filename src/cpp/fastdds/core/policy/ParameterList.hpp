// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ParameterList.hpp
 */

#ifndef _FASTDDS_DDS_QOS_PARAMETERLIST_HPP_
#define _FASTDDS_DDS_QOS_PARAMETERLIST_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <functional>

#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/messages/RTPS_messages.hpp>

#include <rtps/messages/CDRMessage.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * ParameterList class has static methods to update or read a list of Parameter_t
 * @ingroup PARAMETER_MODULE
 */

class ParameterList
{
public:

    /**
     * Write parameterList encapsulation to the CDRMessage.
     * @param msg Pointer to the message (the pos should be correct, otherwise the behaviour is undefined).
     * @return True if correct.
     */
    static bool writeEncapsulationToCDRMsg(
            rtps::CDRMessage_t* msg);

    /**
     * Update the information of a cache change parsing the inline qos from a CDRMessage
     * @param [inout] change Reference to the cache change to be updated.
     * @param [in] msg Pointer to the message (the pos should be correct, otherwise the behaviour is undefined).
     * @param [out] qos_size Number of bytes processed.
     * @return true if parsing was correct, false otherwise.
     */
    static bool updateCacheChangeFromInlineQos(
            rtps::CacheChange_t& change,
            rtps::CDRMessage_t* msg,
            uint32_t& qos_size);

    /**
     * Read a parameterList from a CDRMessage
     * @param [in] msg Reference to the message (the pos should be correct, otherwise the behaviour is undefined).
     * @param [in] processor Function to process each of the parameters in the list.
     * @param [in] use_encapsulation Whether encapsulation field should be read.
     * @param [out] qos_size Number of bytes processed.
     * @return true if parsing was correct, false otherwise.
     */
    template<typename Pred>
    static bool readParameterListfromCDRMsg(
            rtps::CDRMessage_t& msg,
            Pred processor,
            bool use_encapsulation,
            uint32_t& qos_size)
    {
        qos_size = 0;

        if (use_encapsulation)
        {
            // Read encapsulation
            msg.pos += 1;
            rtps::octet encapsulation = 0;
            rtps::CDRMessage::readOctet(&msg, &encapsulation);
            if (encapsulation == PL_CDR_BE)
            {
                msg.msg_endian = rtps::Endianness_t::BIGEND;
            }
            else if (encapsulation == PL_CDR_LE)
            {
                msg.msg_endian = rtps::Endianness_t::LITTLEEND;
            }
            else
            {
                return false;
            }
            // Skip encapsulation options
            msg.pos += 2;
        }

        uint32_t original_pos = msg.pos;
        bool is_sentinel = false;
        while (!is_sentinel)
        {
            msg.pos = original_pos + qos_size;

            ParameterId_t pid{PID_SENTINEL};
            uint16_t plength = 0;
            bool valid = true;
            valid &= rtps::CDRMessage::readUInt16(&msg, (uint16_t*)&pid);
            valid &= rtps::CDRMessage::readUInt16(&msg, &plength);

            if (pid == PID_SENTINEL)
            {
                // PID_SENTINEL is always considered of length 0
                plength = 0;
                is_sentinel = true;
            }

            qos_size += (4 + plength);

            // Align to 4 byte boundary and prepare for next iteration
            qos_size = (qos_size + 3) & ~3;

            if (!valid || ((msg.pos + plength) > msg.length))
            {
                return false;
            }
            else if (!is_sentinel)
            {
                if (!processor(&msg, pid, plength))
                {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * Read guid from the KEY_HASH or another specific PID parameter of a CDRMessage
     * @param [in,out] msg Reference to the message (pos should be correct, otherwise the behaviour is undefined).
     * @param [in] search_pid Specific PID to search
     * @param [out] guid Reference where the guid will be written.
     * @return true if a guid is returned, false otherwise.
     */
    static bool read_guid_from_cdr_msg(
            rtps::CDRMessage_t& msg,
            uint16_t search_pid,
            rtps::GUID_t& guid);

    /**
     * Read change instanceHandle from the KEY_HASH or another specific PID parameter of a CDRMessage
     * @param [in,out] change Pointer to the cache change.
     * @param [in] search_pid Specific PID to search
     * @return True when instanceHandle is updated.
     */
    static bool readInstanceHandleFromCDRMsg(
            rtps::CacheChange_t* change,
            const uint16_t search_pid);
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif // _FASTDDS_DDS_QOS_PARAMETERLIST_HPP_
