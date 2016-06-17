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
 * @file RTPSAsNonReliableWithRegistrationWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSASNONRELIABLEWITHREGISTRATIONWRITER_HPP_
#define _TEST_BLACKBOX_RTPSASNONRELIABLEWITHREGISTRATIONWRITER_HPP_

#include "RTPSWithRegistrationWriter.hpp" 
#include <boost/asio.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>

class RTPSAsNonReliableWithRegistrationWriter : public RTPSWithRegistrationWriter
{
    public:
        void configWriter(WriterAttributes &wattr, eprosima::fastrtps::WriterQos &wqos)
        {
            wattr.endpoint.reliabilityKind = BEST_EFFORT;
            wqos.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
        }

        void configTopic(TopicAttributes &tattr)
        {
            std::ostringstream t;

            t << "RTPSAsNonReliableWithRegistration_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();

            tattr.topicName = t.str();
        }
};

#endif // _TEST_BLACKBOX_RTPSASNONRELIABLEWITHREGISTRATIONWRITER_HPP_

