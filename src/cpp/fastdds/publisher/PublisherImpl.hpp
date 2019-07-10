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



#ifndef _FASTDDS_PUBLISHERIMPL_H_
#define _FASTDDS_PUBLISHERIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/attributes/PublisherAttributes.h>

#include <fastdds/topic/DataWriterListener.hpp>
#include <fastdds/publisher/qos/PublisherQos.hpp>

#include <mutex>
#include <map>

namespace eprosima {
namespace fastrtps{
namespace rtps {

class RTPSWriter;
class RTPSParticipant;

}

class TopicAttributes;
class WriterQos;

} // namespace fastrtps

namespace fastdds {

class PublisherListener;
class ParticipantImpl;
class Participant;
class Publisher;

/**
 * Class PublisherImpl, contains the actual implementation of the behaviour of the Publisher.
 * @ingroup FASTRTPS_MODULE
 */
class PublisherImpl
{
    friend class ParticipantImpl;

    /**
     * Create a publisher, assigning its pointer to the associated writer.
     * Don't use directly, create Publisher using create_publisher from Participant.
     */
    PublisherImpl(
            ParticipantImpl* p,
            const PublisherQos& qos,
            const fastrtps::PublisherAttributes& att,
            PublisherListener* p_listen = nullptr);

public:

    virtual ~PublisherImpl();

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

    fastrtps::rtps::RTPSParticipant* rtps_participant() const
    {
        return rtps_participant_;
    }

    const Publisher* get_publisher() const;

    const fastrtps::PublisherAttributes& get_attributes() const;

    bool set_attributes(const fastrtps::PublisherAttributes& att);

    const fastrtps::rtps::InstanceHandle_t& get_instance_handle() const;

private:

    ParticipantImpl* participant_;

    PublisherQos qos_;

    fastrtps::PublisherAttributes att_;

    //! Map of Pointers to the associated Data Writers. Topic name is the key.
    std::map<std::string, DataWriter*> writers_;

    mutable std::mutex mtx_writers_;

    //!PublisherListener
    PublisherListener* listener_;

    //!Listener to capture the events of the Writer
    class PublisherWriterListener: public DataWriterListener
    {
        public:
            PublisherWriterListener(
                    PublisherImpl* p)
                : publisher_(p)
            {}

            virtual ~PublisherWriterListener() override {}

            void on_publication_matched(
                    DataWriter* writer,
                    fastrtps::rtps::MatchingInfo& info) override;

            void on_offered_deadline_missed(
                DataWriter* writer,
                const fastrtps::OfferedDeadlineMissedStatus& status) override;

            void on_liveliness_lost(
                    DataWriter* writer,
                    const fastrtps::LivelinessLostStatus& status) override;

            PublisherImpl* publisher_;
    } publisher_listener_;

    Publisher* user_publisher_;

    fastrtps::rtps::RTPSParticipant* rtps_participant_;

    fastrtps::WriterQos default_datawriter_qos_;

    fastrtps::rtps::InstanceHandle_t handle_;

};


} /* namespace fastdds */
} /* namespace eprosima */
#endif
#endif /* _FASTDDS_PUBLISHER_H_ */
