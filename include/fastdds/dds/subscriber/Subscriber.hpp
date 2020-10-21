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


#ifndef _FASTDDS_SUBSCRIBER_HPP_
#define _FASTDDS_SUBSCRIBER_HPP_

#include <fastrtps/attributes/SubscriberAttributes.h>

#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/core/Entity.hpp>

using eprosima::fastrtps::types::ReturnCode_t;

namespace dds {
namespace sub {

class Subscriber;

} // namespace sub
} // namespace dds

namespace eprosima {
namespace fastrtps {

class TopicAttributes;

} // namespace fastrtps

namespace fastdds {
namespace dds {

class DomainParticipant;
class SubscriberListener;
class SubscriberImpl;
class DataReader;
class DataReaderListener;
class DataReaderQos;
class TopicDescription;
/**
 * Class Subscriber, contains the public API that allows the user to control the reception of messages.
 * This class should not be instantiated directly.
 * DomainRTPSParticipant class should be used to correctly create this element.
 * @ingroup FASTDDS_MODULE
 */
class Subscriber : public DomainEntity
{
protected:

    friend class SubscriberImpl;
    friend class DomainParticipantImpl;

    /**
     * Create a subscriber, assigning its pointer to the associated implementation.
     * Don't use directly, create Subscriber using create_subscriber from DomainParticipant.
     */
    RTPS_DllAPI Subscriber(
            SubscriberImpl* pimpl,
            const StatusMask& mask = StatusMask::all());

    RTPS_DllAPI Subscriber(
            DomainParticipant* dp,
            const SubscriberQos& qos = SUBSCRIBER_QOS_DEFAULT,
            SubscriberListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

public:

    /**
     * @brief Destructor
     */
    RTPS_DllAPI virtual ~Subscriber()
    {
    }

    /**
     * @brief This operation enables the Subscriber
     * @return RETCODE_OK is successfully enabled. RETCODE_PRECONDITION_NOT_MET if the participant creating this
     *         Subscriber is not enabled.
     */
    RTPS_DllAPI ReturnCode_t enable() override;

    /**
     * Allows accessing the Subscriber Qos.
     * @return SubscriberQos reference
     */
    RTPS_DllAPI const SubscriberQos& get_qos() const;

    /**
     * Retrieves the Subscriber Qos.
     * @param qos SubscriberQos where the qos is returned
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_qos(
            SubscriberQos& qos) const;

    /**
     * Allows modifying the Subscriber Qos.
     * The given Qos must be supported by the SubscriberQos.
     * @param qos new value for SubscriberQos
     * @return RETCODE_IMMUTABLE_POLICY if any of the Qos cannot be changed, RETCODE_INCONSISTENT_POLICY if the Qos is not
     * self consistent and RETCODE_OK if the qos is changed correctly.
     */
    RTPS_DllAPI ReturnCode_t set_qos(
            const SubscriberQos& qos);

    /**
     * Retrieves the attached SubscriberListener.
     * @return Pointer to the SubscriberListener
     */
    RTPS_DllAPI const SubscriberListener* get_listener() const;

    /**
     * Modifies the SubscriberListener, sets the mask to StatusMask::all()
     * @param listener new value for SubscriberListener
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t set_listener(
            SubscriberListener* listener);

    /**
     * Modifies the SubscriberListener.
     * @param listener new value for the SubscriberListener
     * @param mask StatusMask that holds statuses the listener responds to.
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t set_listener(
            SubscriberListener* listener,
            const StatusMask& mask);
    /**
     * This operation creates a DataReader. The returned DataReader will be attached and belong to the Subscriber.
     * @param topic Topic the DataReader will be listening.
     * @param reader_qos QoS of the DataReader.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all).
     * @return Pointer to the created DataReader. nullptr if failed.
     */
    RTPS_DllAPI DataReader* create_datareader(
            TopicDescription* topic,
            const DataReaderQos& reader_qos,
            DataReaderListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * This operation creates a DataReader. The returned DataReader will be attached and belongs to the Subscriber.
     * @param topic Topic the DataReader will be listening.
     * @param profile_name DataReader profile name.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all).
     * @return Pointer to the created DataReader. nullptr if failed.
     */
    RTPS_DllAPI DataReader* create_datareader_with_profile(
            TopicDescription* topic,
            const std::string& profile_name,
            DataReaderListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * This operation deletes a DataReader that belongs to the Subscriber.
     *
     * The delete_datareader operation must be called on the same Subscriber object used to create the DataReader.
     * If delete_datareader is called on a different Subscriber, the operation will have no effect and it will
     * return an error.
     * @param reader DataReader to delete
     * @return RETCODE_PRECONDITION_NOT_MET if the datareader does not belong to this subscriber, RETCODE_OK if it is correctly
     * deleted and RETCODE_ERROR otherwise.
     */
    RTPS_DllAPI ReturnCode_t delete_datareader(
            DataReader* reader);

    /**
     * This operation retrieves a previously-created DataReader belonging to the Subscriber that is attached to a
     * Topic with a matching topic_name. If no such DataReader exists, the operation will return nullptr.
     *
     * If multiple DataReaders attached to the Subscriber satisfy this condition, then the operation will return
     * one of them. It is not specified which one.
     * @param topic_name Name of the topic associated to the DataReader
     * @return Pointer to a previously created DataReader created on a Topic with that topic_name
     */
    RTPS_DllAPI DataReader* lookup_datareader(
            const std::string& topic_name) const;

    /**
     * This operation allows the application to access the DataReader objects.
     * @param readers Vector of DataReader where the list of existing readers is returned
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_datareaders(
            std::vector<DataReader*>& readers) const;

    /**
     * This operation checks if the subscriber has DataReaders
     * @return true if the subscriber has one or several DataReaders, false in other case
     */
    RTPS_DllAPI bool has_datareaders() const;

    /* TODO
       bool begin_access();
     */

    /* TODO
       bool end_access();
     */

    /**
     * This operation invokes the operation on_data_available on the DataReaderListener objects attached to
     * contained DataReader entities.
     *
     * This operation is typically invoked from the on_data_on_readers operation in the SubscriberListener.
     * That way the SubscriberListener can delegate to the DataReaderListener objects the handling of the data.
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t notify_datareaders() const;

    /* TODO
       bool delete_contained_entities();
     */

    /**
     * This operation sets a default value of the DataReader QoS policies which will be used for newly created
     * DataReader entities in the case where the QoS policies are defaulted in the create_datareader operation.
     *
     * This operation will check that the resulting policies are self consistent; if they are not, the operation
     * will have no effect and return false.
     *
     * The special value DATAREADER_QOS_DEFAULT may be passed to this operation to indicate that the default QoS
     * should be reset back to the initial values the factory would use, that is the values that would be used
     * if the set_default_datareader_qos operation had never been called.
     * @param qos new value for DataReaderQos to set as default
     * @return RETCODE_INCONSISTENT_POLICY if the Qos is not self consistent and RETCODE_OK if the qos is changed correctly.
     */
    RTPS_DllAPI ReturnCode_t set_default_datareader_qos(
            const DataReaderQos& qos);

    /**
     * This operation returns the default value of the DataReader QoS, that is, the QoS policies which will be
     * used for newly created DataReader entities in the case where the QoS policies are defaulted in the
     * create_datareader operation.
     *
     * The values retrieved get_default_datareader_qos will match the set of values specified on the last successful
     * call to get_default_datareader_qos, or else, if the call was never made, the default values.
     * @return Current default DataReaderQos.
     */
    RTPS_DllAPI const DataReaderQos& get_default_datareader_qos() const;


    /**
     * This operation returns the default value of the DataReader QoS, that is, the QoS policies which will be
     * used for newly created DataReader entities in the case where the QoS policies are defaulted in the
     * create_datareader operation.
     *
     * The values retrieved get_default_datareader_qos will match the set of values specified on the last successful
     * call to get_default_datareader_qos, or else, if the call was never made, the default values.
     * @return Current default DataReaderQos.
     */
    RTPS_DllAPI DataReaderQos& get_default_datareader_qos();

    /**
     * This operation retrieves the default value of the DataReader QoS, that is, the QoS policies which will be
     * used for newly created DataReader entities in the case where the QoS policies are defaulted in the
     * create_datareader operation.
     *
     * The values retrieved get_default_datareader_qos will match the set of values specified on the last successful
     * call to get_default_datareader_qos, or else, if the call was never made, the default values.
     * @param qos DataReaderQos where the default_qos is returned
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_default_datareader_qos(
            DataReaderQos& qos) const;

    /**
     * Fills the DataReaderQos with the values of the XML profile.
     * @param profile_name DataReader profile name.
     * @param qos DataReaderQos object where the qos is returned.
     * @return RETCODE_OK if the profile exists. RETCODE_BAD_PARAMETER otherwise.
     */
    RTPS_DllAPI ReturnCode_t get_datareader_qos_from_profile(
            const std::string& profile_name,
            DataReaderQos& qos) const;

    /* TODO
       bool copy_from_topic_qos(
            DataReaderQos& reader_qos,
            const fastrtps::TopicAttributes& topic_qos) const;
     */

    /**
     * This operation returns the DomainParticipant to which the Subscriber belongs.
     * @return DomainParticipant Pointer
     */
    RTPS_DllAPI const DomainParticipant* get_participant() const;

    /**
     * Returns the Subscriber's handle.
     * @return InstanceHandle of this Subscriber.
     */
    RTPS_DllAPI const fastrtps::rtps::InstanceHandle_t& get_instance_handle() const;

protected:

    SubscriberImpl* impl_;

    friend class ::dds::sub::Subscriber;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_SUBSCRIBER_HPP_ */
