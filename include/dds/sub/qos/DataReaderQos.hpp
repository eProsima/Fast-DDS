/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OMG_DDS_SUB_QOS_DATA_READER_QOS_HPP_
#define OMG_DDS_SUB_QOS_DATA_READER_QOS_HPP_

#include <dds/sub/qos/detail/DataReaderQos.hpp>
#include <dds/topic/qos/TopicQos.hpp>

namespace dds {
namespace sub {
namespace qos {

//typedef dds::sub::qos::detail::DataReaderQos DataReaderQos;
class DataReaderQos : public detail::DataReaderQos
{
public:
    DataReaderQos()
        : detail::DataReaderQos ()
    {
    }

    DataReaderQos(
            const DataReaderQos& qos)
        : detail::DataReaderQos(qos)
    {
    }

    DataReaderQos(
            const topic::qos::TopicQos& qos)
    {
        history = qos.history;
        deadline = qos.deadline;
        ownership = qos.ownership;
        durability = qos.durability;
        liveliness = qos.liveliness;
        reliability = qos.reliability;
        latency_budget = qos.latency_budget;
        resource_limits = qos.resource_limits;
        destination_order = qos.destination_order;
    }

    DataReaderQos& operator =(
            const topic::qos::TopicQos& qos)
    {
        history = qos.history;
        deadline = qos.deadline;
        ownership = qos.ownership;
        durability = qos.durability;
        liveliness = qos.liveliness;
        reliability = qos.reliability;
        latency_budget = qos.latency_budget;
        resource_limits = qos.resource_limits;
        destination_order = qos.destination_order;
        return *this;
    }
};


} //namespace qos
} //namespace sub
} //namespace dds


#endif //OMG_DDS_SUB_QOS_DATA_READER_QOS_HPP_
