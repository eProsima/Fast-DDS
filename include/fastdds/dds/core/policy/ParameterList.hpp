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

#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/rtps/messages/RTPS_messages.h>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/messages/CDRMessage.h>
#include <fastdds/rtps/common/CacheChange.h>

#include <functional>

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
            fastrtps::rtps::CDRMessage_t* msg);

    /**
     * Update the information of a cache change parsing the inline qos from a CDRMessage
     * @param[inout] change Reference to the cache change to be updated.
     * @param[in] msg Pointer to the message (the pos should be correct, otherwise the behaviour is undefined).
     * @param[out] qos_size Number of bytes processed.
     * @return true if parsing was correct, false otherwise.
     */
    static bool updateCacheChangeFromInlineQos(
            fastrtps::rtps::CacheChange_t& change,
            fastrtps::rtps::CDRMessage_t* msg,
            uint32_t& qos_size);

    /**
     * Read a parameterList from a CDRMessage
     * @param[in] msg Reference to the message (the pos should be correct, otherwise the behaviour is undefined).
     * @param[in] processor Function to process each of the parameters in the list.
     * @param[in] use_encapsulation Whether encapsulation field should be read.
     * @param[out] qos_size Number of bytes processed.
     * @return true if parsing was correct, false otherwise.
     */
    static bool readParameterListfromCDRMsg(
            fastrtps::rtps::CDRMessage_t& msg,
            std::function<bool(fastrtps::rtps::CDRMessage_t* msg, const ParameterId_t, uint16_t)> processor,
            bool use_encapsulation,
            uint32_t& qos_size);

    /**
     * Read guid from the KEY_HASH or another specific PID parameter of a CDRMessage
     * @param[in,out] msg Reference to the message (pos should be correct, otherwise the behaviour is undefined).
     * @param[in] search_pid Specific PID to search
     * @param[out] guid Reference where the guid will be written.
     * @return true if a guid is returned, false otherwise.
     */
    static bool read_guid_from_cdr_msg(
            fastrtps::rtps::CDRMessage_t& msg,
            uint16_t search_pid,
            fastrtps::rtps::GUID_t& guid);

    /**
     * Read change instanceHandle from the KEY_HASH or another specific PID parameter of a CDRMessage
     * @param[in,out] change Pointer to the cache change.
     * @param[in] search_pid Specific PID to search
     * @return True when instanceHandle is updated.
     */
    static bool readInstanceHandleFromCDRMsg(
            fastrtps::rtps::CacheChange_t* change,
            const uint16_t search_pid);
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif

#endif // _FASTDDS_DDS_QOS_PARAMETERLIST_HPP_
