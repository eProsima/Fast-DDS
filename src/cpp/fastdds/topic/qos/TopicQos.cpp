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
 * @file TopicQos.cpp
 *
 */

#include <fastdds/dds/topic/qos/TopicQos.hpp>

using namespace eprosima::fastdds::dds;

const TopicQos eprosima::fastdds::dds::TOPIC_QOS_DEFAULT;

TopicQos::TopicQos()
{
    reliability_.kind = RELIABLE_RELIABILITY_QOS;
    durability_.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
}
