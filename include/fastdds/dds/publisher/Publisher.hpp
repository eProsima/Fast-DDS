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

#ifndef FASTDDS_DDS_PUBLISHER__PUBLISHER_HPP
#define FASTDDS_DDS_PUBLISHER__PUBLISHER_HPP

#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/Time_t.hpp>

#include <fastdds/fastdds_dll.hpp>

namespace dds {
namespace pub {
class Publisher;
} // namespace pub
} // namespace dds

namespace eprosima {
namespace fastdds {
namespace rtps {

class IPayloadPool;

} // namespace rtps

namespace dds {

class DomainParticipant;
class PublisherListener;
class PublisherImpl;
class DataWriter;
class DataWriterListener;
class Topic;

/**
 * Class Publisher, used to send data to associated subscribers.
 *
 * @ingroup FASTDDS_MODULE
 */
class Publisher : public DomainEntity
{
protected:

    friend class PublisherImpl;
    friend class DomainParticipantImpl;

    /**
     * Create a publisher, assigning its pointer to the associated implementation.
     * Don't use directly, create Publisher using create_publisher from DomainParticipant.
     */
    Publisher(
            PublisherImpl* p,
            const StatusMask& mask = StatusMask::all());

    Publisher(
            DomainParticipant* dp,
            const PublisherQos& qos = PUBLISHER_QOS_DEFAULT,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

public:

    /**
     * @brief Destructor
     */
    virtual ~Publisher();

    /**
     * @brief This operation enables the Publisher
     *
     * @return RETCODE_OK is successfully enabled. RETCODE_PRECONDITION_NOT_MET if the participant creating this
     *         Publisher is not enabled.
     */
    FASTDDS_EXPORTED_API ReturnCode_t enable() override;

    /**
     * Allows accessing the Publisher Qos.
     *
     * @return PublisherQos reference
     */
    FASTDDS_EXPORTED_API const PublisherQos& get_qos() const;

    /**
     * Retrieves the Publisher Qos.
     *
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_qos(
            PublisherQos& qos) const;

    /**
     * Allows modifying the Publisher Qos.
     * The given Qos must be supported by the PublisherQos.
     *
     * @param qos PublisherQos to be set
     * @return RETCODE_IMMUTABLE_POLICY if any of the Qos cannot be changed, RETCODE_INCONSISTENT_POLICY if the Qos is not
     * self consistent and RETCODE_OK if the qos is changed correctly.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_qos(
            const PublisherQos& qos);

    /**
     * Retrieves the attached PublisherListener.
     *
     * @return PublisherListener pointer
     */
    FASTDDS_EXPORTED_API const PublisherListener* get_listener() const;

    /**
     * Modifies the PublisherListener, sets the mask to StatusMask::all()
     *
     * @param listener new value for the PublisherListener
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_listener(
            PublisherListener* listener);

    /**
     * Modifies the PublisherListener.
     *
     * @param listener new value for the PublisherListener
     * @param mask StatusMask that holds statuses the listener responds to
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_listener(
            PublisherListener* listener,
            const StatusMask& mask);

    /**
     * This operation creates a DataWriter. The returned DataWriter will be attached and belongs to the Publisher.
     *
     * @param topic Topic the DataWriter will be listening
     * @param qos QoS of the DataWriter.
     * @param listener Pointer to the listener (default: nullptr).
     * @param mask StatusMask that holds statuses the listener responds to (default: all).
     * @param payload_pool IPayloadPool shared pointer that defines writer payload (default: nullptr).
     * @return Pointer to the created DataWriter. nullptr if failed.
     */
    FASTDDS_EXPORTED_API DataWriter* create_datawriter(
            Topic* topic,
            const DataWriterQos& qos,
            DataWriterListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all(),
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool = nullptr);

    /**
     * This operation creates a DataWriter. The returned DataWriter will be attached and belongs to the Publisher.
     *
     * @param topic Topic the DataWriter will be listening
     * @param profile_name DataWriter profile name.
     * @param listener Pointer to the listener (default: nullptr).
     * @param mask StatusMask that holds statuses the listener responds to (default: all).
     * @param payload_pool IPayloadPool shared pointer that defines writer payload (default: nullptr).
     * @return Pointer to the created DataWriter. nullptr if failed.
     */
    FASTDDS_EXPORTED_API DataWriter* create_datawriter_with_profile(
            Topic* topic,
            const std::string& profile_name,
            DataWriterListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all(),
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool = nullptr);

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
     *
     * @param writer DataWriter to delete
     * @return RETCODE_PRECONDITION_NOT_MET if it does not belong to this Publisher, RETCODE_OK if it is correctly deleted and
     * RETCODE_ERROR otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_datawriter(
            const DataWriter* writer);

    /**
     * This operation retrieves a previously created DataWriter belonging to the Publisher that is attached to a
     * Topic with a matching topic_name. If no such DataWriter exists, the operation will return nullptr.
     *
     * If multiple DataWriter attached to the Publisher satisfy this condition, then the operation will return
     * one of them. It is not specified which one.
     *
     * @param topic_name Name of the Topic
     * @return Pointer to a previously created DataWriter associated to a Topic with the requested topic_name
     */
    FASTDDS_EXPORTED_API DataWriter* lookup_datawriter(
            const std::string& topic_name) const;

    /**
     * @brief Indicates to FastDDS that the contained DataWriters are about to be modified
     *
     * @return RETCODE_OK if successful, an error code otherwise
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t suspend_publications();

    /**
     * @brief Indicates to FastDDS that the modifications to the DataWriters are complete.
     *
     * @return RETCODE_OK if successful, an error code otherwise
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t resume_publications();

    /**
     * @brief Signals the beginning of a set of coherent cache changes using the Datawriters attached to the publisher
     *
     * @return RETCODE_OK if successful, an error code otherwise
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t begin_coherent_changes();

    /**
     * @brief Signals the end of a set of coherent cache changes
     *
     * @return RETCODE_OK if successful, an error code otherwise
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t end_coherent_changes();

    /**
     * This operation blocks the calling thread until either all data written by the reliable DataWriter entities
     * is acknowledged by all matched reliable DataReader entities, or else the duration specified by the max_wait
     * parameter elapses, whichever happens first. A return value of true indicates that all the samples written
     * have been acknowledged by all reliable matched data readers; a return value of false indicates that max_wait
     * elapsed before all the data was acknowledged.
     *
     * @param max_wait Maximum blocking time for this operation
     * @return RETCODE_TIMEOUT if the function takes more than the maximum blocking time established, RETCODE_OK if the
     * Publisher receives the acknowledgments and RETCODE_ERROR otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t wait_for_acknowledgments(
            const fastdds::dds::Duration_t& max_wait);

    /**
     * This operation returns the DomainParticipant to which the Publisher belongs.
     *
     * @return Pointer to the DomainParticipant
     */
    FASTDDS_EXPORTED_API const DomainParticipant* get_participant() const;

    /**
     * @brief Deletes all contained DataWriters
     *
     * @return RETCODE_OK if successful, an error code otherwise
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_contained_entities();

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
     *
     * @param qos DataWriterQos to be set
     * @return RETCODE_INCONSISTENT_POLICY if the Qos is not self consistent and RETCODE_OK if the qos is changed correctly.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_default_datawriter_qos(
            const DataWriterQos& qos);

    /**
     * This operation returns the default value of the DataWriter QoS, that is, the QoS policies which will be used
     * for newly created DataWriter entities in the case where the QoS policies are defaulted in the
     * create_datawriter operation.
     *
     * The values retrieved by get_default_datawriter_qos will match the set of values specified on the last
     * successful call to set_default_datawriter_qos, or else, if the call was never made, the default values.
     *
     * @return Current default WriterQos
     */
    FASTDDS_EXPORTED_API const DataWriterQos& get_default_datawriter_qos() const;

    /**
     * This operation retrieves the default value of the DataWriter QoS, that is, the QoS policies which will be used
     * for newly created DataWriter entities in the case where the QoS policies are defaulted in the
     * create_datawriter operation.
     *
     * The values retrieved by get_default_datawriter_qos will match the set of values specified on the last
     * successful call to set_default_datawriter_qos, or else, if the call was never made, the default values.
     *
     * @param qos Reference to the current default WriterQos.
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_datawriter_qos(
            DataWriterQos& qos) const;

    /**
     * @brief Copies TopicQos into the corresponding DataWriterQos
     *
     * @param [out] writer_qos
     * @param [in] topic_qos
     * @return RETCODE_OK if successful, an error code otherwise
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API static ReturnCode_t copy_from_topic_qos(
            fastdds::dds::DataWriterQos& writer_qos,
            const fastdds::dds::TopicQos& topic_qos);

    /**
     * Fills the @ref DataWriterQos with the values of the XML profile.
     *
     * @param profile_name DataWriter profile name.
     * @param qos @ref DataWriterQos object where the qos is returned.
     * @return @ref RETCODE_OK if the profile exists. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_datawriter_qos_from_profile(
            const std::string& profile_name,
            DataWriterQos& qos) const;

    /**
     * Fills the @ref DataWriterQos with the values of the XML profile, and also its corresponding topic name (if specified).
     *
     * @param profile_name DataWriter profile name.
     * @param qos @ref DataWriterQos object where the qos is returned.
     * @param topic_name String where the name of the topic associated to this profile is returned (if specified).
     * @return @ref RETCODE_OK if the profile exists. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_datawriter_qos_from_profile(
            const std::string& profile_name,
            DataWriterQos& qos,
            std::string& topic_name) const;

    /**
     * Fills the @ref DataWriterQos with the first DataWriter profile found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref DataWriterQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_datawriter_qos_from_xml(
            const std::string& xml,
            DataWriterQos& qos) const;

    /**
     * Fills the @ref DataWriterQos with the first DataWriter profile found in the provided XML, and also its corresponding topic name (if specified).
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref DataWriterQos object where the qos is returned.
     * @param topic_name String where the name of the topic associated to this profile is returned (if specified).
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_datawriter_qos_from_xml(
            const std::string& xml,
            DataWriterQos& qos,
            std::string& topic_name) const;

    /**
     * Fills the @ref DataWriterQos with the DataWriter profile with \c profile_name to be found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref DataWriterQos object where the qos is returned.
     * @param profile_name DataWriter profile name.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_datawriter_qos_from_xml(
            const std::string& xml,
            DataWriterQos& qos,
            const std::string& profile_name) const;

    /**
     * Fills the @ref DataWriterQos with the DataWriter profile with \c profile_name to be found in the provided XML, and also its corresponding topic name (if specified).
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref DataWriterQos object where the qos is returned.
     * @param topic_name String where the name of the topic associated to this profile is returned (if specified).
     * @param profile_name DataWriter profile name.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_datawriter_qos_from_xml(
            const std::string& xml,
            DataWriterQos& qos,
            std::string& topic_name,
            const std::string& profile_name) const;

    /**
     * Fills the @ref DataWriterQos with the default DataWriter profile found in the provided XML (if there is).
     *
     * @note This method does not update the default datawriter qos (returned by \c get_default_datawriter_qos).
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref DataWriterQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_datawriter_qos_from_xml(
            const std::string& xml,
            DataWriterQos& qos) const;

    /**
     * Fills the @ref DataWriterQos with the default DataWriter profile found in the provided XML (if there is), and also its corresponding topic name (if specified).
     *
     * @note This method does not update the default datawriter qos (returned by \c get_default_datawriter_qos).
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref DataWriterQos object where the qos is returned.
     * @param topic_name String where the name of the topic associated to this profile is returned (if specified).
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_datawriter_qos_from_xml(
            const std::string& xml,
            DataWriterQos& qos,
            std::string& topic_name) const;

    /**
     * Returns the Publisher's handle.
     *
     * @return InstanceHandle of this Publisher.
     */
    FASTDDS_EXPORTED_API const InstanceHandle_t& get_instance_handle() const;

    /**
     * Fills the given vector with all the datawriters of this publisher.
     *
     * @param writers Vector where the DataWriters are returned
     * @return true
     */
    FASTDDS_EXPORTED_API bool get_datawriters(
            std::vector<DataWriter*>& writers) const;

    /**
     * This operation checks if the publisher has DataWriters
     *
     * @return true if the publisher has one or several DataWriters, false otherwise
     */
    FASTDDS_EXPORTED_API bool has_datawriters() const;

protected:

    PublisherImpl* impl_;

    friend class ::dds::pub::Publisher;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_PUBLISHER__PUBLISHER_HPP
