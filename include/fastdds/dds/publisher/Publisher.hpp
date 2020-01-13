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

#ifndef _FASTDDS_PUBLISHER_HPP_
#define _FASTDDS_PUBLISHER_HPP_

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/types/TypesBase.h>

#include <dds/core/status/Status.hpp>
#include <dds/core/status/State.hpp>

using eprosima::fastrtps::types::ReturnCode_t;

namespace dds {
namespace domain {
class DomainParticipant;
} // domain
namespace pub {
class Publisher;
} // pub
} // dds

namespace eprosima {
namespace fastrtps {

class TopicAttributes;

} // namespace fastrtps

namespace fastdds {
namespace dds {

class DomainParticipant;
class PublisherListener;
class PublisherImpl;
class PublisherQos;
class DataWriter;
class DataWriterListener;
class DataWriterQos;
class Topic;
class TopicQos;

/**
 * Class Publisher, used to send data to associated subscribers.
 * @ingroup FASTDDS_MODULE
 */
class RTPS_DllAPI Publisher
{
    friend class PublisherImpl;
    friend class DomainParticipantImpl;
    friend class ::dds::pub::Publisher;

    /**
     * Create a publisher, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using create_publisher from Participant.
     */
    Publisher(
            PublisherImpl* p);

    Publisher(
            const ::dds::domain::DomainParticipant& dp,
            const PublisherQos& qos,
            PublisherListener* listener = NULL,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

public:

    virtual ~Publisher();

    /**
     * Allows accessing the Publisher Qos.
     */
    const PublisherQos& get_qos() const;

    /**
     * Retrieves the Publisher Qos.
     * @return true
     */
    ReturnCode_t get_qos(
            PublisherQos& qos) const;

    /**
     * Allows modifying the Publisher Qos.
     * The given Qos must be supported by the PublisherQos.
     * @param qos
     * @return False if IMMUTABLE_POLICY or INCONSISTENT_POLICY occurs. True if updated.
     */
    ReturnCode_t set_qos(
            const PublisherQos& qos);

    /**
     * Retrieves the attached PublisherListener.
     */
    const PublisherListener* get_listener() const;

    PublisherListener* get_listener();

    /**
     * Modifies the PublisherListener.
     * @param listener
     * @return true
     */
    ReturnCode_t set_listener(
            PublisherListener* listener,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    /**
     * This operation creates a DataWriter. The returned DataWriter will be attached and belongs to the Publisher.
     * @param topic_attr
     * @param writer_qos
     * @param listener
     * @return Pointer to the created DataWriter. nullptr if failed.
     */
    DataWriter* create_datawriter(
            const fastrtps::TopicAttributes& topic_attr,
            const DataWriterQos& writer_qos,
            DataWriterListener* listener,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    DataWriter* create_datawriter(
            const Topic& topic,
            const DataWriterQos& qos,
            DataWriterListener* listener,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    /**
     * This operation deletes a DataWriter that belongs to the Publisher.
     *
     * The delete_datawriter operation must be called on the same Publisher object used to create the DataWriter.
     * If delete_datawriter is called on a different Publisher, the operation will have no effect and it will
     * return false.
     *
     * The deletion of the DataWriter will automatically unregister all instances.
     * Depending on the settings of the WRITER_DATA_LIFECYCLE QosPolicy, the deletion of the DataWriter
     * may also dispose all instances.
     * @param writer
     */
    ReturnCode_t delete_datawriter(
            DataWriter* writer);

    /**
     * This operation retrieves a previously created DataWriter belonging to the Publisher that is attached to a
     * Topic with a matching topic_name. If no such DataWriter exists, the operation will return nullptr.
     *
     * If multiple DataWriter attached to the Publisher satisfy this condition, then the operation will return
     * one of them. It is not specified which one.
     * @param topic_name
     */
    DataWriter* lookup_datawriter(
            const std::string& topic_name) const;

    /**
     * Fills the given vector with all the datawriters of this publisher.
     * @param writers
     * @return true
     */
    bool get_datawriters(
            std::vector<DataWriter*>& writers) const;

    /**
     * This operation checks if the publisher has DataWriters
     * @return true if the publisher has one or several DataWriters, false in other case
     */
    bool has_datawriters() const;

    /* TODO
       bool suspend_publications();
     */

    /* TODO
       bool resume_publications();
     */

    /* TODO
       bool begin_coherent_changes();
     */

    /* TODO
       bool end_coherent_changes();
     */

    /**
     * This operation blocks the calling thread until either all data written by the reliable DataWriter entities
     * is acknowledged by all matched reliable DataReader entities, or else the duration specified by the max_wait
     * parameter elapses, whichever happens first. A return value of true indicates that all the samples written
     * have been acknowledged by all reliable matched data readers; a return value of false indicates that max_wait
     * elapsed before all the data was acknowledged.
     * @param max_wait
     * @return False if timedout. True otherwise.
     */
    ReturnCode_t wait_for_acknowledgments(
            const fastrtps::Duration_t& max_wait);

    /**
     * This operation returns the DomainParticipant to which the Publisher belongs.
     */
    const DomainParticipant* get_participant() const;

    /* TODO
       bool delete_contained_entities();
     */

    /**
     * This operation sets a default value of the DataWriter QoS policies which will be used for newly created
     * DataWriter entities in the case where the QoS policies are defaulted in the create_datawriter operation.
     *
     * This operation will check that the resulting policies are self consistent; if they are not, the operation
     * will have no effect and return false.
     *
     * The special value DDS_DATAWRITER_QOS_DEFAULT may be passed to this operation to indicate that the default QoS
     * should be reset back to the initial values the factory would use, that is the values that would be used
     * if the set_default_datawriter_qos operation had never been called.
     * @param qos
     */
    ReturnCode_t set_default_datawriter_qos(
            const DataWriterQos& qos);

    /**
     * This operation returns the default value of the DataWriter QoS, that is, the QoS policies which will be used
     * for newly created DataWriter entities in the case where the QoS policies are defaulted in the
     * create_datawriter operation.
     *
     * The values retrieved by get_default_datawriter_qos will match the set of values specified on the last
     * successful call to set_default_datawriter_qos, or else, if the call was never made, the default values.
     * @return Current default DataWriterQos
     */
    const DataWriterQos& get_default_datawriter_qos() const;

    /**
     * This operation retrieves the default value of the DataWriter QoS, that is, the QoS policies which will be used
     * for newly created DataWriter entities in the case where the QoS policies are defaulted in the
     * create_datawriter operation.
     *
     * The values retrieved by get_default_datawriter_qos will match the set of values specified on the last
     * successful call to set_default_datawriter_qos, or else, if the call was never made, the default values.
     * @param qos Copy of the current default DataWriterQos.
     * @return Always true.
     */
    ReturnCode_t get_default_datawriter_qos(
            DataWriterQos& qos) const;

    ReturnCode_t copy_from_topic_qos(
            DataWriterQos& writer_qos,
            const TopicQos& topic_qos) const;

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
    bool set_attributes(
            const fastrtps::PublisherAttributes& att);

    /**
     * Returns the Publisher's handle.
     * @return InstanceHandle of this Publisher.
     */
    const fastrtps::rtps::InstanceHandle_t& get_instance_handle() const;

private:

    PublisherImpl* impl_;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_PUBLISHER_HPP_ */
