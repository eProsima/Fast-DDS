// Copyright 2026 Fast DDS contributors.
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

// Regression workload for https://github.com/eProsima/Fast-DDS/issues/6603.
// Each role is executed in its own process by type_lookup_concurrency.py.

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/rtps/writer/WriterDiscoveryStatus.hpp>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

using namespace eprosima::fastdds::dds;

namespace {

class DiscoveryListener : public DomainParticipantListener
{
public:

    void on_data_writer_discovery(
            DomainParticipant*,
            eprosima::fastdds::rtps::WriterDiscoveryStatus reason,
            const PublicationBuiltinTopicData&,
            bool& should_be_ignored) override
    {
        should_be_ignored = false;
        if (reason == eprosima::fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER)
        {
            ++discovered_writers_;
        }
    }

    std::atomic<unsigned> discovered_writers_ {0};
};

DynamicType::_ref_type make_type(
        int index)
{
    auto descriptor = traits<TypeDescriptor>::make_shared();
    descriptor->kind(TK_STRUCTURE);
    descriptor->name("TypeLookupConcurrency" + std::to_string(index));
    auto builder = DynamicTypeBuilderFactory::get_instance()->create_type(descriptor);

    auto member = traits<MemberDescriptor>::make_shared();
    member->name("value" + std::to_string(index));
    member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    builder->add_member(member);
    return builder->build();
}

} // namespace

int main(
        int argc,
        char** argv)
{
    if (argc != 3 || (std::string(argv[1]) != "announce" && std::string(argv[1]) != "discover"))
    {
        return 2;
    }

    // The two roles must run in separate processes: the TypeObjectRegistry is
    // process-wide, and sharing it would bypass remote type lookup entirely.
    const bool announce = std::string(argv[1]) == "announce";
    const auto domain_id = static_cast<DomainId_t>(std::atoi(argv[2]));
    auto* factory = DomainParticipantFactory::get_instance();
    DiscoveryListener listener;
    DomainParticipant* participant = factory->create_participant(domain_id, PARTICIPANT_QOS_DEFAULT,
                    announce ? nullptr : &listener);
    if (participant == nullptr)
    {
        return 3;
    }

    bool successful = true;
    if (announce)
    {
        Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
        successful = publisher != nullptr;
        for (int index = 0; successful && index < 200; ++index)
        {
            TypeSupport type(new DynamicPubSubType(make_type(index)));
            successful = type.register_type(participant) == RETCODE_OK;
            if (successful)
            {
                Topic* topic = participant->create_topic(
                    "TypeLookupConcurrencyTopic" + std::to_string(index), type.get_type_name(), TOPIC_QOS_DEFAULT);
                successful = topic != nullptr && publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT) != nullptr;
            }
        }
    }

    // Keep the announcing participant alive while the other participant
    // concurrently requests the type objects for the discovered writers.
    std::this_thread::sleep_for(std::chrono::seconds(15));
    if (!announce && listener.discovered_writers_ == 0)
    {
        std::cerr << "No remote writers discovered; type lookup was not exercised\n";
        successful = false;
    }
    successful = participant->delete_contained_entities() == RETCODE_OK && successful;
    successful = factory->delete_participant(participant) == RETCODE_OK && successful;
    return successful ? 0 : 1;
}
