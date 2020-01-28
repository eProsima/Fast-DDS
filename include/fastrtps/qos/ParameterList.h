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
 * @file ParameterList.h
 */

#ifndef PARAM_LIST_T_H_
#define PARAM_LIST_T_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "ParameterTypes.h"
#include "../rtps/messages/RTPS_messages.h"
#include "../rtps/common/CDRMessage_t.h"
#include "../rtps/messages/CDRMessage.h"
#include "../rtps/common/CacheChange.h"

#include <functional>

namespace eprosima {
namespace fastrtps {

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
        static bool writeEncapsulationToCDRMsg(rtps::CDRMessage_t* msg);

        /**
         * Update the information of a cache change parsing the inline qos from a CDRMessage
         * @param[inout] change Reference to the cache change to be updated.
         * @param[in] msg Pointer to the message (the pos should be correct, otherwise the behaviour is undefined).
         * @param[out] qos_size Number of bytes processed.
         * @return true if parsing was correct, false otherwise.
         */
        static bool updateCacheChangeFromInlineQos(
                rtps::CacheChange_t& change,
                rtps::CDRMessage_t* msg,
                uint32_t& qos_size);

        /**
         * Read change instanceHandle from the KEY_HASH or another specific PID parameter of a CDRMessage
         * @param[in,out] change Pointer to the cache change.
         * @param[in] pid Specific PID to search
         * @return True when instanceHandle is updated.
         */
        static bool readInstanceHandleFromCDRMsg(
                rtps::CacheChange_t* change,
                const uint16_t pid);
};

} /* namespace fastrtps */
} /* namespace eprosima */
#endif
#endif /* PARAMLISTT_H_ */
