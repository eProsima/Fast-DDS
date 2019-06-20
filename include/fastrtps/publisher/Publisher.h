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
 * @file Publisher.h
 *
 */

#ifndef PUBLISHER_H_
#define PUBLISHER_H_

#include "../fastrtps_dll.h"
#include "../rtps/common/Guid.h"
#include "../rtps/common/Time_t.h"
#include "../attributes/PublisherAttributes.h"
#include "../qos/DeadlineMissedStatus.h"
#include "../qos/LivelinessLostStatus.h"

namespace eprosima {
namespace fastrtps {

namespace rtps
{
    struct GUID_t;
    class WriteParams;
    class RTPSWriter;
}

class Participant;
class PublisherImpl;
class TopicAttributes;
class WriterQos;
class DataWriter;
class DataWriterListener;
class PublisherListener;

/**
 * Class Publisher, used to send data to associated subscribers.
 * @ingroup FASTRTPS_MODULE
 */
class RTPS_DllAPI Publisher
{
    friend class PublisherImpl;
    virtual ~Publisher();

public:
    /**
     * Constructor from a PublisherImpl pointer
     * @param pimpl Actual implementation of the publisher
     */
    Publisher(PublisherImpl* pimpl);

    bool wait_for_all_acked(
            const Duration_t& max_wait);

    /**
     * Get the Attributes of the Publisher.
     * @return Attributes of the publisher
     */
    const PublisherAttributes& getAttributes() const;

    /**
     * Update the Attributes of the publisher.
     * @param att Reference to a PublisherAttributes object to update the parameters.
     * @return True if correctly updated, false if ANY of the updated parameters cannot be updated.
     */
    bool updateAttributes(const PublisherAttributes& att);

    /**
     * @brief Created a new writer
     * @param topic_att TopicAttributes
     * @param wqos WriterQos
     * @param listener DataWriterListener
     */
    DataWriter* create_writer(
            const TopicAttributes& topic_att,
            const WriterQos& wqos,
            DataWriterListener* listener = nullptr);

    bool update_writer(
            DataWriter* Writer,
            const TopicAttributes& topicAtt,
            const WriterQos& wqos);

    PublisherListener* listener() const;

    void listener(PublisherListener* listener);

    bool delete_writer(
            DataWriter* writer);

    DataWriter* lookup_writer(const std::string& topic_name) const;

    Participant* participant() const;

private:

    PublisherImpl* mp_impl;
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* PUBLISHER_H_ */
