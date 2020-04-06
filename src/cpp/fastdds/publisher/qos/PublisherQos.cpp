// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PublisherQos.cpp
 *
 */

#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastdds::dds;

RTPS_DllAPI const PublisherQos eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT;

PublisherQos::PublisherQos()
{
}

PublisherQos::~PublisherQos()
{

}

void PublisherQos::set_qos(
        const PublisherQos& qos,
        bool first_time)
{
    if (first_time && !(presentation_ == qos.presentation()))
    {
        presentation_ = qos.presentation();
        presentation_.hasChanged = true;
    }
    if (qos.partition().names().size() > 0)
    {
        partition_ = qos.partition();
        partition_.hasChanged = true;
    }
    if (entity_factory_.autoenable_created_entities != qos.entity_factory().autoenable_created_entities)
    {
        entity_factory_ = qos.entity_factory();
    }
    //    if (group_data.getValue() != qos.group_data.getValue())
    //    {
    //        group_data = qos.group_data;
    //        group_data.hasChanged = true;
    //    }
    //    if (first_time)
    //    {
    //        disable_positive_acks = qos.disable_positive_acks;
    //        disable_positive_acks.hasChanged = true;
    //    }
}

bool PublisherQos::check_qos() const
{
    return true;
}

bool PublisherQos::can_qos_be_updated(
        const PublisherQos& qos) const
{
    (void) qos;
    return true;

}
