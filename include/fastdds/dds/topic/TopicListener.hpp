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

#ifndef FASTDDS_DDS_TOPIC__TOPICLISTENER_HPP
#define FASTDDS_DDS_TOPIC__TOPICLISTENER_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class Topic;

/**
 * Class TopicListener, it should be used by the end user to implement specific callbacks to certain actions.
 *
 * @ingroup FASTDDS_MODULE
 */
class FASTDDS_EXPORTED_API TopicListener
{
public:

    /**
     * @brief Constructor
     */
    TopicListener()
    {
    }

    /**
     * @brief Destructor
     */
    virtual ~TopicListener()
    {
    }

    /**
     * Virtual function to be implemented by the user containing the actions to be performed when
     * another topic exists with the same name but different characteristics.
     *
     * @param topic Topic
     * @param status The inconsistent topic status
     */
    virtual void on_inconsistent_topic(
            Topic* topic,
            InconsistentTopicStatus status)
    {
        (void)topic;
        (void)status;
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_TOPIC__TOPICLISTENER_HPP
