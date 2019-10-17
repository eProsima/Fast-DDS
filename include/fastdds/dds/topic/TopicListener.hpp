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
 * @file TopicListener.hpp
 */

#ifndef _FASTDDS_TOPICLISTENER_HPP_
#define _FASTDDS_TOPICLISTENER_HPP_

#include <fastrtps/fastrtps_dll.h>

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class RTPS_DllAPI TopicListener
{
public:
    TopicListener() = default;

    ~TopicListener() = default;

    virtual void on_inconsistent_topic(
            const Topic* topic,
            const InconsistentTopicStatus& status)
    {
        (void)topic;
        (void)status;
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TOPICLISTENER_HPP_
