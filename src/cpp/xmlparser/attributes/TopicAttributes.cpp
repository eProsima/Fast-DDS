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
 * @file TopicAttributes.cpp
 */

#include <fastdds/dds/log/Log.hpp>

#include <xmlparser/attributes/TopicAttributes.hpp>

namespace eprosima {
namespace fastdds {
namespace xmlparser {

bool TopicAttributes::checkQos() const
{
    if (rtps::WITH_KEY == topicKind && resourceLimitsQos.max_samples > 0)
    {
        if (resourceLimitsQos.max_samples_per_instance > resourceLimitsQos.max_samples)
        {
            EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK,
                    "INCORRECT TOPIC QOS (" << topicName << "): max_samples_per_instance must be <= than max_samples");
            return false;
        }

        if (resourceLimitsQos.max_samples <
                resourceLimitsQos.max_samples_per_instance * resourceLimitsQos.max_instances)
        {
            EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                    "TOPIC QOS (" << topicName << "): max_samples < max_samples_per_instance*max_instances");
        }
    }

    if (historyQos.kind == dds::KEEP_LAST_HISTORY_QOS)
    {
        if ((resourceLimitsQos.max_samples > 0) && (historyQos.depth > resourceLimitsQos.max_samples))
        {
            EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK,
                    "INCORRECT TOPIC QOS (" << topicName << "): depth must be <= max_samples");
            return false;
        }
        if (rtps::WITH_KEY == topicKind && resourceLimitsQos.max_samples_per_instance > 0 &&
                historyQos.depth > resourceLimitsQos.max_samples_per_instance)
        {
            EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK,
                    "INCORRECT TOPIC QOS (" << topicName << "): depth must be <= max_samples_per_instance");
            return false;
        }
        if (historyQos.depth <= 0 )
        {
            EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK, "INCORRECT TOPIC QOS (" << topicName << "): depth must be > 0");
            return false;
        }
    }

    if (resourceLimitsQos.max_samples > 0 && resourceLimitsQos.allocated_samples > resourceLimitsQos.max_samples)
    {
        EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK,
                "INCORRECT TOPIC QOS (" << topicName << "): max_samples < allocated_samples");
        return false;
    }
    return true;
}

}  // namespave xmlparser
}  // namespace fastdds
}  // namespace eprosima
