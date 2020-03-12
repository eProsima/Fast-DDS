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



#ifndef _FASTDDS_PUBLISHERIMPL_HPP_
#define _FASTDDS_PUBLISHERIMPL_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/attributes/PublisherAttributes.h>

#include <fastdds/dds/topic/DataWriterListener.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastrtps/types/TypesBase.h>

#include <fastdds/dds/topic/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include <dds/core/status/State.hpp>

#include <mutex>
#include <map>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSParticipant;

} //namespace rtps
} // namespace fastrtps

namespace fastdds {
namespace dds {

class PublisherListener;
class DomainParticipantImpl;
class DomainParticipant;
class Publisher;
class DataWriterImpl;

/**
 * Class PublisherImpl, contains the actual implementation of the behaviour of the Publisher.
 * @ingroup FASTRTPS_MODULE
 */
class PublisherImpl
{
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

    const PublisherQos& get_qos() const;

    ReturnCode_t set_qos(
            const PublisherQos& qos);

    const PublisherListener* get_listener() const;

    PublisherListener* get_listener();

    ReturnCode_t set_listener(
            PublisherListener* listener);

    DataWriter* create_datawriter(
            Topic* topic,
            const DataWriterQos& writer_qos,
            DataWriterListener* listener,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    ReturnCode_t delete_datawriter(
            DataWriter* writer);

    DataWriter* lookup_datawriter(
            const std::string& topic_name) const;

    bool contains_entity(
            const fastrtps::rtps::InstanceHandle_t& handle) const;

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
            const fastrtps::Duration_t& max_wait);

    const DomainParticipant& get_participant() const;

    ReturnCode_t delete_contained_entities();

    ReturnCode_t set_default_datawriter_qos(
            const DataWriterQos& qos);

    const DataWriterQos& get_default_datawriter_qos() const;

    ReturnCode_t copy_from_topic_qos(
            DataWriterQos& writer_qos,
            const TopicQos& topic_qos) const;

    fastrtps::rtps::RTPSParticipant* rtps_participant() const
    {
        return rtps_participant_;
    }

    const Publisher* get_publisher() const;

    const fastrtps::PublisherAttributes& get_attributes() const;

    bool set_attributes(
            const fastrtps::PublisherAttributes& att);

    //! Remove all listeners in the hierarchy to allow a quiet destruction
    void disable();

    //! Check if any writer uses the given type name
    bool type_in_use(
            const std::string& type_name) const;

    ReturnCode_t autoenable_entities();

private:

    DomainParticipantImpl* participant_;

    PublisherQos qos_;

    //! Map of Pointers to the associated Data Writers. Topic name is the key.
    std::map<std::string, std::vector<DataWriterImpl*> > writers_;

    mutable std::mutex mtx_writers_;

    //!PublisherListener
    PublisherListener* listener_;

    Publisher* user_publisher_;

    fastrtps::rtps::RTPSParticipant* rtps_participant_;

    DataWriterQos default_datawriter_qos_;

    fastrtps::rtps::InstanceHandle_t handle_;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif
#endif /* _FASTDDS_PUBLISHER_HPP_ */
