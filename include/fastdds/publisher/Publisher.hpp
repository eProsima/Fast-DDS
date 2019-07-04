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
#include "../../fastrtps/types/TypesBase.h"

using namespace eprosima::fastrtps::types;

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

class DomainParticipant;
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
    friend class DomainParticipantImpl;
    virtual ~Publisher();

    /**
     * Create a publisher, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using create_publisher from Participant.
     */
    Publisher(
        PublisherImpl* p);

public:
    ReturnCode_t get_qos(PublisherQos& qos) const;

    ReturnCode_t set_qos(
            const PublisherQos& qos);

    const PublisherListener* get_listener() const;

    ReturnCode_t set_listener(
            PublisherListener* listener);

    DataWriter* create_datawriter(
            const fastrtps::TopicAttributes& topic_attr,
            fastrtps::WriterQos& writer_qos,
            DataWriterListener* listener);

    ReturnCode_t delete_datawriter(
            DataWriter* writer);

    DataWriter* lookup_datawriter(
            const std::string& topic_name) const;

    bool get_datawriters(
        std::vector<DataWriter*>& writers) const;

    ReturnCode_t suspend_publications();

    ReturnCode_t resume_publications();

    ReturnCode_t begin_coherent_changes();

    ReturnCode_t end_coherent_changes();

    ReturnCode_t wait_for_acknowledgments(
            const fastrtps::Duration_t& max_wait);

    const DomainParticipant* get_participant() const;

    ReturnCode_t delete_contained_entities();

    ReturnCode_t set_default_datawriter_qos(
            const fastrtps::WriterQos& qos);

    ReturnCode_t get_default_datawriter_qos(fastrtps::WriterQos& qos) const;

    ReturnCode_t copy_from_topic_qos(
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
