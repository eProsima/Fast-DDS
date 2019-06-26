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
 * @file Publisher.hpp
 *
 */

#ifndef _FASTDDS_PUBLISHER_H_
#define _FASTDDS_PUBLISHER_H_

#include "../../fastrtps/fastrtps_dll.h"
#include "../../fastrtps/rtps/common/Time_t.h"
#include "../../fastrtps/attributes/PublisherAttributes.h"

namespace eprosima {
namespace fastrtps {

namespace rtps
{
class RTPSParticipant;
}

class TopicAttributes;
class WriterQos;

} // namespace fastrtps

namespace fastdds {

class Participant;
class PublisherListener;
class PublisherImpl;
class PublisherQos;
class DataWriter;
class DataWriterListener;

/**
 * Class Publisher, used to send data to associated subscribers.
 * @ingroup FASTRTPS_MODULE
 */
class RTPS_DllAPI Publisher
{
    friend class PublisherImpl;
    friend class ParticipantImpl;
    virtual ~Publisher();

    /**
     * Create a publisher, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using create_publisher from Participant.
     */
    Publisher(
        PublisherImpl* p);

public:
    const PublisherQos& get_qos() const;

    bool set_qos(
            const PublisherQos& qos);

    const PublisherListener* get_listener() const;

    bool set_listener(
            PublisherListener* listener);

    DataWriter* create_datawriter(
            const fastrtps::TopicAttributes& topic_attr,
            const fastrtps::WriterQos& writer_qos,
            DataWriterListener* listener);

    bool delete_datawriter(
            DataWriter* writer);

    DataWriter* lookup_datawriter(
            const std::string& topic_name) const;

    bool get_datawriters(
        std::vector<DataWriter*>& writers) const;

    bool suspend_publications();

    bool resume_publications();

    bool begin_coherent_changes();

    bool end_coherent_changes();

    bool wait_for_acknowledments(
            const fastrtps::Duration_t& max_wait);

    const Participant* get_participant() const;

    bool delete_contained_entities();

    bool set_default_datawriter_qos(
            const fastrtps::WriterQos& qos);

    const fastrtps::WriterQos& get_default_datawriter_qos() const;

    bool copy_from_topic_qos(
            fastrtps::WriterQos& writer_qos,
            const fastrtps::TopicAttributes& topic_qos) const;

    const Publisher* get_publisher() const;

    /**
     * Get the Attributes of the Publisher.
     * @return Attributes of the publisher
     */
    const fastrtps::PublisherAttributes& get_attributes() const;

    /**
     * Update the Attributes of the publisher.
     * @param att Reference to a PublisherAttributes object to update the parameters.
     * @return True if correctly updated, false if ANY of the updated parameters cannot be updated.
     */
    bool set_attributes(const fastrtps::PublisherAttributes& att);

    fastrtps::rtps::RTPSParticipant* rtps_participant();

    const fastrtps::rtps::RTPSParticipant* rtps_participant() const;

    const fastrtps::rtps::InstanceHandle_t& get_instance_handle() const;

private:

    PublisherImpl* impl_;
};

} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_PUBLISHER_H_ */
