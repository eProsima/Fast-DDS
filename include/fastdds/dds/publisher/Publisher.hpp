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
#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>

using eprosima::fastrtps::types::ReturnCode_t;

namespace dds {
namespace pub {
class Publisher;
}
}

namespace eprosima {
namespace fastrtps {

class TopicAttributes;

} // namespace fastrtps

namespace fastdds {
namespace dds {

class DomainParticipant;
class PublisherListener;
class PublisherImpl;
class DataWriter;
class DataWriterListener;
class DataWriterQos;

/**
 * Class Publisher, used to send data to associated subscribers.
 * @ingroup FASTDDS_MODULE
 */
class Publisher : public DomainEntity
{
    friend class PublisherImpl;
    friend class DomainParticipantImpl;

    /**
     * Create a publisher, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using create_publisher from Participant.
     */
    RTPS_DllAPI Publisher(
            PublisherImpl* p,
            const StatusMask& mask = StatusMask::all());

    RTPS_DllAPI Publisher(
            DomainParticipant* dp,
            const PublisherQos& qos = PUBLISHER_QOS_DEFAULT,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

public:

    RTPS_DllAPI virtual ~Publisher();

    /**
     * Allows accessing the Publisher Qos.
     */
    RTPS_DllAPI const PublisherQos& get_qos() const;

    /**
     * Retrieves the Publisher Qos.
     * @return ReturnCode OK
     */
    RTPS_DllAPI ReturnCode_t get_qos(
            PublisherQos& qos) const;

    /**
     * Allows modifying the Publisher Qos.
     * The given Qos must be supported by the PublisherQos.
     * @param qos
     * @return IMMUTABLE_POLICY cannot be updated. OK if updated.
     */
    RTPS_DllAPI ReturnCode_t set_qos(
            const PublisherQos& qos);

    /**
     * Retrieves the attached PublisherListener.
     */
    RTPS_DllAPI const PublisherListener* get_listener() const;

    /**
     * Modifies the PublisherListener.
     * @param listener
     * @return true
     */
    RTPS_DllAPI ReturnCode_t set_listener(
            PublisherListener* listener);

    /**
     * This operation creates a DataWriter. The returned DataWriter will be attached and belongs to the Publisher.
     * @param topic_attr
     * @param writer_qos
     * @param listener
     * @return Pointer to the created DataWriter. nullptr if failed.
     */
    RTPS_DllAPI DataWriter* create_datawriter(
            const fastrtps::TopicAttributes& topic_attr,
            const DataWriterQos& qos,
            DataWriterListener* listener);

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
    RTPS_DllAPI ReturnCode_t delete_datawriter(
            DataWriter* writer);

    /**
     * This operation retrieves a previously created DataWriter belonging to the Publisher that is attached to a
     * Topic with a matching topic_name. If no such DataWriter exists, the operation will return nullptr.
     *
     * If multiple DataWriter attached to the Publisher satisfy this condition, then the operation will return
     * one of them. It is not specified which one.
     * @param topic_name
     */
    RTPS_DllAPI DataWriter* lookup_datawriter(
            const std::string& topic_name) const;

    /**
     * Fills the given vector with all the datawriters of this publisher.
     * @param writers
     * @return true
     */
    RTPS_DllAPI bool get_datawriters(
            std::vector<DataWriter*>& writers) const;

    /**
     * This operation checks if the publisher has DataWriters
     * @return true if the publisher has one or several DataWriters, false in other case
     */
    RTPS_DllAPI bool has_datawriters() const;

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
    RTPS_DllAPI ReturnCode_t wait_for_acknowledgments(
            const fastrtps::Duration_t& max_wait);

    /**
     * This operation returns the DomainParticipant to which the Publisher belongs.
     */
    RTPS_DllAPI const DomainParticipant* get_participant() const;

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
     * The special value DATAWRITER_QOS_DEFAULT may be passed to this operation to indicate that the default QoS
     * should be reset back to the initial values the factory would use, that is the values that would be used
     * if the set_default_datawriter_qos operation had never been called.
     * @param qos
     */
    RTPS_DllAPI ReturnCode_t set_default_datawriter_qos(
            const DataWriterQos& qos);

    /**
     * This operation returns the default value of the DataWriter QoS, that is, the QoS policies which will be used
     * for newly created DataWriter entities in the case where the QoS policies are defaulted in the
     * create_datawriter operation.
     *
     * The values retrieved by get_default_datawriter_qos will match the set of values specified on the last
     * successful call to set_default_datawriter_qos, or else, if the call was never made, the default values.
     * @return Current default WriterQos
     */
    RTPS_DllAPI const DataWriterQos& get_default_datawriter_qos() const;

    /**
     * This operation retrieves the default value of the DataWriter QoS, that is, the QoS policies which will be used
     * for newly created DataWriter entities in the case where the QoS policies are defaulted in the
     * create_datawriter operation.
     *
     * The values retrieved by get_default_datawriter_qos will match the set of values specified on the last
     * successful call to set_default_datawriter_qos, or else, if the call was never made, the default values.
     * @param qos Copy of the current default WriterQos.
     * @return Always true.
     */
    RTPS_DllAPI ReturnCode_t get_default_datawriter_qos(
            DataWriterQos& qos) const;

    /* TODO
       bool copy_from_topic_qos(
            WriterQos& writer_qos,
            const fastrtps::TopicAttributes& topic_qos) const;
     */

    /**
     * Returns the Publisher's handle.
     * @return InstanceHandle of this Publisher.
     */
    RTPS_DllAPI const fastrtps::rtps::InstanceHandle_t& get_instance_handle() const;

private:

    PublisherImpl* impl_;

    friend class ::dds::pub::Publisher;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_PUBLISHER_HPP_ */
