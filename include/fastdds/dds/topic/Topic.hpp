// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Topic.hpp
 */

#ifndef FASTDDS_DDS_TOPIC__TOPIC_HPP
#define FASTDDS_DDS_TOPIC__TOPIC_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

namespace dds {
namespace topic {

class Topic;

} // namespace topic
} // namespace dds

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;
class TopicListener;
class DomainParticipantImpl;
class TopicProxy;

/**
 * Class Topic, represents the fact that both publications
 * and subscriptions are tied to a single data-type
 *
 * @ingroup FASTDDS_MODULE
 */
class Topic : public DomainEntity, public TopicDescription
{
    friend class TopicProxy;
    friend class DomainParticipantImpl;

    /**
     * Create a topic, assigning its pointer to the associated implementation.
     * Don't use directly, create Topic using create_topic from DomainParticipant.
     */
    Topic(
            const std::string& topic_name,
            const std::string& type_name,
            TopicProxy* p,
            const StatusMask& mask = StatusMask::all());

    Topic(
            DomainParticipant* dp,
            const std::string& topic_name,
            const std::string& type_name,
            const TopicQos& qos = TOPIC_QOS_DEFAULT,
            TopicListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

public:

    virtual ~Topic();

    /**
     * @brief Getter for the DomainParticipant
     *
     * @return DomainParticipant pointer
     */
    virtual DomainParticipant* get_participant() const override;

    /**
     * Allows the application to retrieve the INCONSISTENT_TOPIC_STATUS status of a Topic.
     *
     * @param status [out] Status to be retrieved.
     * @return RETCODE_OK
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_inconsistent_topic_status(
            InconsistentTopicStatus& status);

    /**
     * Allows accessing the Topic Qos.
     *
     * @return reference to TopicQos
     */
    FASTDDS_EXPORTED_API const TopicQos& get_qos() const;

    /**
     * Retrieves the Topic Qos.
     *
     * @param qos TopicQos where the qos is returned
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_qos(
            TopicQos& qos) const;

    /**
     * Allows modifying the Topic Qos.
     * The given Qos must be supported by the Topic.
     *
     * @param qos new TopicQos value to set for the Topic.
     * @retval RETCODE_IMMUTABLE_POLICY if a change was not allowed.
     * @retval RETCODE_INCONSISTENT_POLICY if new qos has inconsistent values.
     * @retval RETCODE_OK if qos was updated.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_qos(
            const TopicQos& qos);

    /**
     * Retrieves the attached TopicListener.
     *
     * @return pointer to TopicListener
     */
    FASTDDS_EXPORTED_API const TopicListener* get_listener() const;

    /**
     * Modifies the TopicListener.
     *
     * @param listener new value for the TopicListener
     * @param mask StatusMask that holds statuses the listener responds to (default: all).
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_listener(
            TopicListener* listener,
            const StatusMask& mask = StatusMask::all());

    TopicDescriptionImpl* get_impl() const override;

protected:

    TopicProxy* impl_;

    friend class ::dds::topic::Topic;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_TOPIC__TOPIC_HPP
