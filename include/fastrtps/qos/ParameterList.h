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

namespace eprosima {
namespace fastrtps {

/**
 * ParameterList_t class, used to store multiple parameters as a vector of pointers to the base class.
 * @ingroup PARAMETER_MODULE
 */
class ParameterList_t
{
    public:

        ParameterList_t() {}

        ParameterList_t(ParameterList_t&& plist) : m_parameters(std::move(plist.m_parameters)) {}

        virtual ~ParameterList_t()
        {
            for(std::vector<Parameter_t*>::iterator it = m_parameters.begin();
                    it!=m_parameters.end();++it)
                delete(*it);
            m_parameters.clear();
        }

        //! Vector of the pointers to the parameters.
        std::vector<Parameter_t*> m_parameters;

    private:

        ParameterList_t(const ParameterList_t& plist) = delete;
};

/**
 * ParameterList class has static methods to update or read a ParameterList_t
 * @ingroup PARAMETER_MODULE
 */

class ParameterList
{
    public:

        /**
         * Update the CDRMessage of a parameterList.
         * @param msg Pointer to the message (the pos should be correct, otherwise the behaviour is undefined).
         * @param plist Pointer to the parameterList.
         * @return True if correct.
         */
        static bool writeParameterListToCDRMsg(rtps::CDRMessage_t* msg, ParameterList_t* plist, bool use_encapsulation);

        /**
         * Read a parameterList from a CDRMessage
         * @param[in] msg Pointer to the message (the pos should be correct, otherwise the behaviour is undefined).
         * @param[out] plist Pointer to the parameter list.
         * @param[out] change Pointer to the cache change.
         * @return Number of bytes of the parameter list.
         */
        static int32_t readParameterListfromCDRMsg(rtps::CDRMessage_t* msg, ParameterList_t* plist, rtps::CacheChange_t* change,
                bool encapsulation);

        /**
         * Read change instanceHandle from the KEY_HASH or another specific PID parameter of a CDRMessage
         * @param[in-out] change Pointer to the cache change.
         * @param[in] pid Specific PID to search
         * @return True when instanceHandle is updated.
         */
        static bool readInstanceHandleFromCDRMsg(rtps::CacheChange_t* change, const uint16_t pid);
};

} /* namespace  */
} /* namespace eprosima */
#endif
#endif /* PARAMLISTT_H_ */
