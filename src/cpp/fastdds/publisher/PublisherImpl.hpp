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
 * @file PublisherImpl.h
 */



#ifndef FASTDDS_PUBLISHER_PUBLISHERIMPL_HPP
#define FASTDDS_PUBLISHER_PUBLISHERIMPL_HPP
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC


#include <map>
#include <mutex>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#ifdef FASTDDS_STATISTICS
#include <statistics/rtps/monitor-service/interfaces/IStatusQueryable.hpp>
#endif // ifdef FASTDDS_STATISTICS

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipant;
class IPayloadPool;

} //namespace rtps

class TopicAttributes;

namespace dds {

class PublisherListener;
class DomainParticipantImpl;
class DomainParticipant;
class Publisher;
class DataWriterImpl;
class Topic;
class TypeSupport;

/**
 * Class PublisherImpl, contains the actual implementation of the behaviour of the Publisher.
 * @ingroup FASTDDS_MODULE
 */
class PublisherImpl
{
protected:

    friend class DomainParticipantImpl;
    friend class DataWriterImpl;

    /**
     * Create a publisher, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using create_publisher from Participant.
     */
    PublisherImpl(
            DomainParticipantImpl* p,
            const PublisherQos& qos,
            PublisherListener* p_listen = nullptr);

public:

    virtual ~PublisherImpl();

    ReturnCode_t enable();

    const PublisherQos& get_qos() const;

    ReturnCode_t set_qos(
            const PublisherQos& qos);

    const PublisherListener* get_listener() const;

    ReturnCode_t set_listener(
            PublisherListener* listener);

    DataWriter* create_datawriter(
            Topic* topic,
            DataWriterImpl* impl,
            const StatusMask& mask);

    DataWriter* create_datawriter(
            Topic* topic,
            const DataWriterQos& qos,
            DataWriterListener* listener,
            const StatusMask& mask = StatusMask::all(),
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool = nullptr);

    DataWriter* create_datawriter_with_profile(
            Topic* topic,
            const std::string& profile_name,
            DataWriterListener* listener,
            const StatusMask& mask = StatusMask::all(),
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool = nullptr);

    ReturnCode_t delete_datawriter(
            const DataWriter* writer);

    DataWriter* lookup_datawriter(
            const std::string& topic_name) const;

    bool contains_entity(
            const fastdds::rtps::InstanceHandle_t& handle) const;

    bool get_datawriters(
            std::vector<DataWriter*>& writers) const;

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

    ReturnCode_t wait_for_acknowledgments(
            const fastdds::dds::Duration_t& max_wait);

    const DomainParticipant* get_participant() const;


    ReturnCode_t delete_contained_entities();

    DomainParticipantImpl* get_participant_impl()
    {
        return participant_;
    }

    ReturnCode_t set_default_datawriter_qos(
            const DataWriterQos& qos);

    void reset_default_datawriter_qos();

    const DataWriterQos& get_default_datawriter_qos() const;

    ReturnCode_t get_datawriter_qos_from_profile(
            const std::string& profile_name,
            DataWriterQos& qos) const;

    ReturnCode_t get_datawriter_qos_from_profile(
            const std::string& profile_name,
            DataWriterQos& qos,
            std::string& topic_name) const;

    ReturnCode_t get_datawriter_qos_from_xml(
            const std::string& xml,
            DataWriterQos& qos) const;

    ReturnCode_t get_datawriter_qos_from_xml(
            const std::string& xml,
            DataWriterQos& qos,
            std::string& topic_name) const;

    ReturnCode_t get_datawriter_qos_from_xml(
            const std::string& xml,
            DataWriterQos& qos,
            const std::string& profile_name) const;

    ReturnCode_t get_datawriter_qos_from_xml(
            const std::string& xml,
            DataWriterQos& qos,
            std::string& topic_name,
            const std::string& profile_name) const;

    ReturnCode_t get_default_datawriter_qos_from_xml(
            const std::string& xml,
            DataWriterQos& qos) const;

    ReturnCode_t get_default_datawriter_qos_from_xml(
            const std::string& xml,
            DataWriterQos& qos,
            std::string& topic_name) const;

    ReturnCode_t static copy_from_topic_qos(
            DataWriterQos& writer_qos,
            const TopicQos& topic_qos);

    fastdds::rtps::RTPSParticipant* rtps_participant() const
    {
        return rtps_participant_;
    }

    const Publisher* get_publisher() const;

    const fastdds::rtps::InstanceHandle_t& get_instance_handle() const;

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    void disable();

    //! Check if any writer uses the given type name
    bool type_in_use(
            const std::string& type_name) const;

    /**
     * Returns the most appropriate listener to handle the callback for the given status,
     * or nullptr if there is no appropriate listener.
     */
    PublisherListener* get_listener_for(
            const StatusMask& status);

    bool can_be_deleted();

    /**
     * Check if a given DataWriter can be deleted.
     */
    bool can_be_deleted(
            DataWriter* writer) const;

#ifdef FASTDDS_STATISTICS
    bool get_monitoring_status(
            statistics::MonitorServiceData& status,
            const fastdds::rtps::GUID_t& entity_guid);
#endif //FASTDDS_STATISTICS

protected:

    DomainParticipantImpl* participant_;

    PublisherQos qos_;

    //! Map of Pointers to the associated Data Writers. Topic name is the key.
    std::map<std::string, std::vector<DataWriterImpl*>> writers_;

    mutable std::mutex mtx_writers_;

    //!PublisherListener
    PublisherListener* listener_;

    //!Listener to capture the events of the Writer
    class PublisherWriterListener : public DataWriterListener
    {
    public:

        PublisherWriterListener(
                PublisherImpl* p)
            : publisher_(p)
        {
        }

        virtual ~PublisherWriterListener() override
        {
        }

        void on_publication_matched(
                DataWriter* writer,
                const PublicationMatchedStatus& info) override;

        void on_offered_deadline_missed(
                DataWriter* writer,
                const OfferedDeadlineMissedStatus& status) override;

        void on_liveliness_lost(
                DataWriter* writer,
                const LivelinessLostStatus& status) override;

        PublisherImpl* publisher_;
    }
    publisher_listener_;

    Publisher* user_publisher_;

    fastdds::rtps::RTPSParticipant* rtps_participant_;

    DataWriterQos default_datawriter_qos_;

    fastdds::rtps::InstanceHandle_t handle_;

    virtual DataWriterImpl* create_datawriter_impl(
            const TypeSupport& type,
            Topic* topic,
            const DataWriterQos& qos,
            DataWriterListener* listener,
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool);

    static void set_qos(
            PublisherQos& to,
            const PublisherQos& from,
            bool first_time);

    static ReturnCode_t check_qos(
            const PublisherQos& qos);

    static bool can_qos_be_updated(
            const PublisherQos& to,
            const PublisherQos& from);

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // FASTDDS_PUBLISHER_PUBLISHERIMPL_HPP
