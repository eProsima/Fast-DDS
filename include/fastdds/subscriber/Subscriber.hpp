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
 * @file Subscriber.hpp
 */


#ifndef _FASTDDS_SUBSCRIBER_H_
#define _FASTDDS_SUBSCRIBER_H_

#include <fastrtps/attributes/SubscriberAttributes.h>

#include <fastdds/topic/DataReaderListener.hpp>
#include <fastdds/subscriber/qos/SubscriberQos.hpp>

namespace eprosima {
namespace fastrtps {

namespace rtps
{
class RTPSReader;
class RTPSParticipant;
}

class TopicAttributes;
class ReaderQos;

} // namespace fastrtps

namespace fastdds {

class DomainParticipant;
class SubscriberListener;
class SubscriberImpl;
class SubscriberQos;
class DataReader;
class DataReaderListener;

/**
 * Class Subscriber, contains the public API that allows the user to control the reception of messages.
 * This class should not be instantiated directly.
 * DomainRTPSParticipant class should be used to correctly create this element.
 * @ingroup FASTRTPS_MODULE
 * @snippet fastrtps_example.cpp ex_Subscriber
 */
class RTPS_DllAPI Subscriber
{
    friend class SubscriberImpl;
    friend class DomainParticipantImpl;

    /**
     * Constructor from a SubscriberImpl pointer
     * @param pimpl Actual implementation of the subscriber
     */
    Subscriber(SubscriberImpl* pimpl)
        : impl_(pimpl)
    {}


    virtual ~Subscriber() {}

public:

    const SubscriberQos& get_qos() const;

    bool set_qos(
            const SubscriberQos& qos);

    const SubscriberListener* get_listener() const;

    bool set_listener(
            SubscriberListener* listener);

    DataReader* create_datareader(
            const fastrtps::TopicAttributes& topic_attr,
            const fastrtps::ReaderQos& reader_qos,
            DataReaderListener* listener);

    bool delete_datareader(
            DataReader* reader);

    DataReader* lookup_datareader(
            const std::string& topic_name) const;

    bool get_datareaders(
        std::vector<DataReader*>& readers) const;

    bool begin_access();

    bool end_access();

    bool notify_datareaders() const;

    bool delete_contained_entities();

    bool set_default_datareader_qos(
            const fastrtps::ReaderQos& qos);

    const fastrtps::ReaderQos& get_default_datareader_qos() const;

    bool copy_from_topic_qos(
            fastrtps::ReaderQos& reader_qos,
            const fastrtps::TopicAttributes& topic_qos) const;

    /**
     * Update the Attributes of the subscriber;
     * @param att Reference to a SubscriberAttributes object to update the parameters;
     * @return True if correctly updated, false if ANY of the updated parameters cannot be updated
     */
    bool set_attributes(
            const fastrtps::SubscriberAttributes& att);

    /**
     * Get the Attributes of the Subscriber.
     * @return Attributes of the Subscriber.
     */
    const fastrtps::SubscriberAttributes& get_attributes() const;

    const DomainParticipant* get_participant() const;

    const fastrtps::rtps::RTPSParticipant* rtps_participant() const;

    fastrtps::rtps::RTPSParticipant* rtps_participant();

    const Subscriber* get_subscriber() const;

    const fastrtps::rtps::InstanceHandle_t& get_instance_handle() const;

private:
    SubscriberImpl* impl_;
};



} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_SUBSCRIBER_H_ */
