// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Subscriber.cpp
 *
 */

#include <atomic>
#include <future>
#include <string>
#include <utility>

#include <asio.hpp>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::xtypes;
using namespace eprosima::fastdds::rtps;

class ParListener : public DomainParticipantListener
{
public:

    ParListener()
    {
        remote_names_ = is_worth_a_type_.get_future();
    }

    /**
     * This method is called when a new Participant is discovered, or a previously discovered participant changes its QOS or is removed.
     * @param p Pointer to the Participant
     * @param info DiscoveryInfo.
     */
    void on_participant_discovery(
            DomainParticipant* /*participant*/,
            ParticipantDiscoveryStatus status,
            const ParticipantBuiltinTopicData& info,
            bool& /*should_be_ignored*/) override
    {
        if (status == ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT)
        {
            std::cout << "Subscriber participant " << //participant->getGuid() <<
                " discovered participant " << info.guid << std::endl;
        }
        else if (status == ParticipantDiscoveryStatus::CHANGED_QOS_PARTICIPANT)
        {
            std::cout << "Subscriber participant " << //participant->getGuid() <<
                " detected changes on participant " << info.guid << std::endl;
        }
        else if (status == ParticipantDiscoveryStatus::REMOVED_PARTICIPANT)
        {
            std::cout << "Subscriber participant " << //participant->getGuid() <<
                " removed participant " << info.guid << std::endl;
        }
        else if (status == ParticipantDiscoveryStatus::DROPPED_PARTICIPANT)
        {
            std::cout << "Subscriber participant " << //participant->getGuid() <<
                " dropped participant " << info.guid << std::endl;
        }
    }

#if HAVE_SECURITY
    void onParticipantAuthentication(
            DomainParticipant* /*participant*/,
            ParticipantAuthenticationInfo&& info) override
    {
        if (ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT == info.status)
        {
            std::cout << "Subscriber participant " << //participant->guid() <<
                " authorized participant " << info.guid << std::endl;
        }
        else
        {
            std::cout << "Subscriber participant " << //participant->guid() <<
                " unauthorized participant " << info.guid << std::endl;
        }
    }

#endif // if HAVE_SECURITY

    using topic_type_names = std::pair<std::string, std::string>;

    std::future<topic_type_names> remote_names_;

private:

    using DomainParticipantListener::on_participant_discovery;

    std::promise<topic_type_names> is_worth_a_type_;
};

class SubListener : public SubscriberListener
{
public:

    void on_subscription_matched(
            DataReader* /*reader*/,
            const SubscriptionMatchedStatus& info) override
    {
        if (info.current_count_change == 1)
        {
            std::cout << "Subscriber matched with publisher " << info.last_publication_handle << std::endl;
        }
        else if (info.current_count_change == -1)
        {
            std::cout << "Subscriber unmatched with publisher " << info.last_publication_handle << std::endl;
        }
        else
        {
            std::cout << info.current_count_change
                      << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
        }
    }

    void on_liveliness_changed(
            DataReader* /*reader*/,
            const eprosima::fastdds::dds::LivelinessChangedStatus& status) override
    {
        if (status.alive_count_change == 1)
        {
            std::cout << "Publisher recovered liveliness" << std::endl;
        }
        else if (status.not_alive_count_change == 1)
        {
            std::cout << "Publisher lost liveliness" << std::endl;
            run_ = false;
        }
    }

    std::atomic_bool run_ = { true };
};

int main(
        int argc,
        char** argv)
{
    int result = 0;
    int arg_count = 1;
    bool notexit = false;
    uint32_t seed = 7800;
    uint32_t samples = 4;
    std::string magic;

    while (arg_count < argc)
    {
        if (strcmp(argv[arg_count], "--notexit") == 0)
        {
            notexit = true;
        }
        else if (strcmp(argv[arg_count], "--seed") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--seed expects a parameter" << std::endl;
                return -1;
            }

            seed = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--samples") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--samples expects a parameter" << std::endl;
                return -1;
            }

            samples = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--magic") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--magic expects a parameter" << std::endl;
                return -1;
            }

            magic = argv[arg_count];
        }
        else if (strcmp(argv[arg_count], "--xmlfile") == 0)
        {
            std::cout << "--xmlfile option isn't implemented yet." << std::endl;
            if (++arg_count >= argc)
            {
                std::cout << "--xmlfile expects a parameter" << std::endl;
                return -1;
            }

        }

        ++arg_count;
    }

    ParListener participant_listener;
    DomainParticipant* participant = nullptr;

    SubListener listener;
    Subscriber* subscriber = nullptr;

    Topic* topic = nullptr;
    DataReader* reader = nullptr;

    try
    {

        DomainParticipantQos participant_qos;
        StatusMask participant_mask = StatusMask::none();
        participant =
                DomainParticipantFactory::get_instance()->create_participant(seed % 230, participant_qos,
                        &participant_listener, participant_mask);

        if (participant == nullptr)
        {
            std::cout << "Error creating subscriber participant" << std::endl;
            throw 1;
        }

        StatusMask mask = StatusMask::subscription_matched()
                << StatusMask::liveliness_changed();

        // Create the Subscriber
        subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, &listener, mask);

        if (subscriber == nullptr)
        {
            std::cout << "Error creating subscriber" << std::endl;
            throw 1;
        }

        // Get the dynamic type factory
        auto remote_names = participant_listener.remote_names_.get();
        auto& topic_name = remote_names.first;
        auto& type_name = remote_names.second;

        DynamicType::_ref_type type;

        {
            TypeObjectPair type_objects;
            if (RETCODE_OK != DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                        type_name, type_objects))

            {
                std::cout << "ERROR: TypeObject cannot be retrieved for type: "
                          << type_name << std::endl;
                throw 1;
            }

            type = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
                type_objects.complete_type_object)->build();

            if (!type)
            {
                std::cout << "ERROR: DynamicType cannot be created for type: " << type_name << std::endl;
                throw 1;
            }
        }

        // Create the Topic & DataReader
        TopicDescription* desc = participant->lookup_topicdescription(topic_name);
        if (desc != nullptr)
        {
            std::cout << "ERROR: Cannot create Topic with name " << topic_name
                      << " - Topic already exists" << std::endl;
            topic = static_cast<Topic*>(desc);
        }
        else
        {
            topic = participant->create_topic( topic_name, type_name, TOPIC_QOS_DEFAULT);
            if (topic == nullptr)
            {
                std::cout << "ERROR: Could not create topic " << topic_name << std::endl;
                throw 1;
            }
        }

        DataReaderQos qos;
        reader = subscriber->create_datareader(topic, qos, nullptr);
        if (reader == nullptr)
        {
            std::cout << "ERROR: Could not create reader for topic " << topic_name << std::endl;
            throw 1;
        }

        // samples received
        uint32_t number_samples = 0;

        while ((notexit || number_samples < samples ) && listener.run_)
        {
            // loop taking samples
            DynamicPubSubType pst(type);
            DynamicData* sample {static_cast<DynamicData*>(pst.createData())};
            eprosima::fastdds::dds::SampleInfo info;

            if (RETCODE_OK == reader->take_next_sample(sample, &info))
            {
                if (info.valid_data)
                {
                    std::string message;
                    uint32_t index;
                    octet count;

                    ++number_samples;

                    sample->get_string_value(message, 0);
                    sample->get_uint32_value(index, 1);

                    DynamicData::_ref_type inner {sample->loan_value(2)};
                    inner->get_byte_value(count, 0);
                    sample->return_loaned_value(inner);

                    std::cout << "Received sample: index(" << index << "), message("
                              << message << "), inner_count(" << std::hex << (uint32_t)count << ")" << std::endl;
                }
            }
        }
    }
    catch (int res)
    {
        result = res;
    }

    if (participant != nullptr)
    {
        if (RETCODE_OK != participant->delete_contained_entities() && !result)
        {
            std::cout << "ERROR: precondition not met on participant entities removal" << std::endl;
            result = 1;
        }

        DomainParticipantFactory::get_instance()->delete_participant(participant);
    }

    return result;
}
