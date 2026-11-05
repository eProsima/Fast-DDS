// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeLookupServiceManyTypes.cpp
 *
 * Checks that every TypeLookup request is answered when the replies cannot be acknowledged for a while, so the
 * reply writer history of the type server fills up.
 */

#include "TypeLookupServiceManyTypes.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

namespace {

const std::string type_prefix = "TypeLookupManyTypes_";

class ParticipantTracker : public DomainParticipantListener
{
public:

    void on_participant_discovery(
            DomainParticipant*,
            rtps::ParticipantDiscoveryStatus status,
            const ParticipantBuiltinTopicData&,
            bool&) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT == status)
        {
            ++discovered_;
        }
        else if (rtps::ParticipantDiscoveryStatus::REMOVED_PARTICIPANT == status ||
                rtps::ParticipantDiscoveryStatus::DROPPED_PARTICIPANT == status)
        {
            ++removed_;
        }
        cv_.notify_all();
    }

    void on_data_writer_discovery(
            DomainParticipant*,
            rtps::WriterDiscoveryStatus reason,
            const PublicationBuiltinTopicData& info,
            bool& should_be_ignored) override
    {
        should_be_ignored = false;
        // The listener is only called for a remote writer once its type has been resolved
        if (rtps::WriterDiscoveryStatus::DISCOVERED_WRITER == reason &&
                0 == info.type_name.to_string().rfind(type_prefix, 0))
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ++writers_;
            cv_.notify_all();
        }
    }

    std::mutex mutex_;
    std::condition_variable cv_;
    uint32_t discovered_ {0};
    uint32_t removed_ {0};
    uint32_t writers_ {0};
};

DynamicType::_ref_type build_type(
        uint32_t index)
{
    auto factory = DynamicTypeBuilderFactory::get_instance();
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->name(type_prefix + std::to_string(index));
    type_descriptor->kind(TK_STRUCTURE);
    DynamicTypeBuilder::_ref_type builder = factory->create_type(type_descriptor);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name("value");
    member_descriptor->type(factory->get_primitive_type(TK_UINT32));
    builder->add_member(member_descriptor);
    return builder->build();
}

} // namespace

int run_many_types_publisher(
        uint32_t domain_id,
        uint32_t num_types,
        uint32_t timeout_s)
{
    ParticipantTracker tracker;
    auto factory = DomainParticipantFactory::get_instance();
    DomainParticipant* participant = factory->create_participant(domain_id, PARTICIPANT_QOS_DEFAULT, &tracker);
    if (nullptr == participant)
    {
        std::cout << "ERROR: create_participant" << std::endl;
        return -1;
    }
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    for (uint32_t i = 0; i < num_types; ++i)
    {
        TypeSupport type_support(new DynamicPubSubType(build_type(i)));
        std::string type_name = type_prefix + std::to_string(i);
        if (RETCODE_OK != participant->register_type(type_support, type_name))
        {
            std::cout << "ERROR: register_type " << type_name << std::endl;
            return -1;
        }
        Topic* topic = participant->create_topic(type_name, type_name, TOPIC_QOS_DEFAULT);
        if (nullptr == topic || nullptr == publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT))
        {
            std::cout << "ERROR: create_datawriter " << type_name << std::endl;
            return -1;
        }
    }
    std::cout << "Publisher created " << num_types << " writers" << std::endl;

    // Keep serving types until the subscriber has finished
    std::unique_lock<std::mutex> lock(tracker.mutex_);
    bool done = tracker.cv_.wait_for(lock, std::chrono::seconds(timeout_s), [&]()
                    {
                        return tracker.discovered_ > 0 && tracker.removed_ > 0;
                    });
    lock.unlock();

    participant->delete_contained_entities();
    factory->delete_participant(participant);
    return done ? 0 : -1;
}

int run_many_types_subscriber(
        uint32_t domain_id,
        uint32_t num_types,
        uint32_t timeout_s)
{
    // Drop the acknowledgements of the TypeLookup replies for a while, so the replies of the remote type server stay
    // in its reply writer history until it is full.
    std::atomic<bool> drop_reply_acks {true};
    auto transport = std::make_shared<rtps::test_UDPv4TransportDescriptor>();
    transport->drop_ack_nack_messages_filter_ = [&drop_reply_acks](rtps::CDRMessage_t& msg)
            {
                // msg.pos points to the ACKNACK body: readerId followed by writerId
                rtps::EntityId_t writer_id;
                std::memcpy(writer_id.value, &msg.buffer[msg.pos + 4], 4);
                return drop_reply_acks && rtps::c_EntityId_TypeLookup_reply_writer == writer_id;
            };

    DomainParticipantQos qos = PARTICIPANT_QOS_DEFAULT;
    qos.transport().use_builtin_transports = false;
    qos.transport().user_transports.push_back(transport);

    ParticipantTracker tracker;
    auto factory = DomainParticipantFactory::get_instance();
    DomainParticipant* participant = factory->create_participant(domain_id, qos, &tracker);
    if (nullptr == participant)
    {
        std::cout << "ERROR: create_participant" << std::endl;
        return -1;
    }

    // Give the type server time to receive every request and try to answer it while the acknowledgements are dropped
    std::this_thread::sleep_for(std::chrono::seconds(5));
    uint32_t resolved_without_acks = 0;
    {
        std::lock_guard<std::mutex> lock(tracker.mutex_);
        resolved_without_acks = tracker.writers_;
    }
    drop_reply_acks = false;

    std::unique_lock<std::mutex> lock(tracker.mutex_);
    bool all_resolved = tracker.cv_.wait_for(lock, std::chrono::seconds(timeout_s), [&]()
                    {
                        return tracker.writers_ >= num_types;
                    });
    std::cout << "Subscriber resolved " << resolved_without_acks << " types while dropping acknowledgements and "
              << tracker.writers_ << " of " << num_types << " in total" << std::endl;
    lock.unlock();

    participant->delete_contained_entities();
    factory->delete_participant(participant);
    return all_resolved ? 0 : -1;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
