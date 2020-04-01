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
 * @file SubscriberQos.cpp
 *
 */

#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

RTPS_DllAPI const SubscriberQos SUBSCRIBER_QOS_DEFAULT;

void SubscriberQos::set_qos(
        const SubscriberQos& qos,
        bool first_time)
{
    if (first_time || presentation.access_scope != qos.presentation.access_scope ||
            presentation.coherent_access != qos.presentation.coherent_access ||
            presentation.ordered_access != qos.presentation.ordered_access)
    {
        presentation = qos.presentation;
        presentation.hasChanged = true;
    }
    if (qos.partition.names().size() > 0)
    {
        partition = qos.partition;
        partition.hasChanged = true;
    }
    if (group_data.getValue() != qos.group_data.getValue() )
    {
        group_data = qos.group_data;
        group_data.hasChanged = true;
    }
    if (entity_factory.autoenable_created_entities != qos.entity_factory.autoenable_created_entities)
    {
        entity_factory = qos.entity_factory;
        entity_factory.hasChanged = true;
    }
    if (subscriber_attr != qos.subscriber_attr)
    {
        subscriber_attr = qos.subscriber_attr;
    }
}

bool SubscriberQos::check_qos() const
{
    return true;
}

bool SubscriberQos::can_qos_be_updated(
        const SubscriberQos& qos) const
{
    (void) qos;
    return true;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
