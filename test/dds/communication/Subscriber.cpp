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

#include <asio.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/TypeObjectFactory.h>

#include <mutex>
#include <condition_variable>
#include <fstream>
#include <string>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

static bool g_run = true;
static types::DynamicType_ptr g_type;
static Topic* g_topic = nullptr;
static DataReaderQos g_qos;
static Subscriber* g_subscriber = nullptr;
static DataReader* g_reader = nullptr;
static DomainParticipant* g_participant = nullptr;


class ParListener : public DomainParticipantListener
{
public:

    ParListener()
    {
    }

    virtual ~ParListener() override
    {
    }

    /**
     * This method is called when a new Participant is discovered, or a previously discovered participant changes its QOS or is removed.
     * @param p Pointer to the Participant
     * @param info DiscoveryInfo.
     */
    void on_participant_discovery(
            DomainParticipant* /*participant*/,
            rtps::ParticipantDiscoveryInfo&& info) override
    {
        if (info.status == rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
        {
            std::cout << "Subscriber participant " << //participant->getGuid() <<
                " discovered participant " << info.info.m_guid << std::endl;
        }
        else if (info.status == rtps::ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT)
        {
            std::cout << "Subscriber participant " << //participant->getGuid() <<
                " detected changes on participant " << info.info.m_guid << std::endl;
        }
        else if (info.status == rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
        {
            std::cout << "Subscriber participant " << //participant->getGuid() <<
                " removed participant " << info.info.m_guid << std::endl;
        }
        else if (info.status == rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
        {
            std::cout << "Subscriber participant " << //participant->getGuid() <<
                " dropped participant " << info.info.m_guid << std::endl;
        }
    }

    void on_type_information_received(
            eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastrtps::string_255 topic_name,
            const eprosima::fastrtps::string_255 type_name,
            const eprosima::fastrtps::types::TypeInformation& type_information) override
    {
        std::function<void(const std::string&, const types::DynamicType_ptr)> callback =
                [topic_name, type_name](const std::string& name, const types::DynamicType_ptr type)
                {
                    if (nullptr != g_subscriber)
                    {
                        if (g_participant->lookup_topicdescription(topic_name.to_string()) != nullptr)
                        {
                            std::cout << "ERROR: Cannot create Topic with name " << topic_name
                                      << " - Topic already exists" << std::endl;
                            return;
                        }
                        std::cout << "Discovered type: " << name << " from topic " << topic_name << std::endl;
                        g_topic = g_participant->create_topic(
                            topic_name.to_string(),
                            type_name.to_string(),
                            TOPIC_QOS_DEFAULT);
                        if (g_topic == nullptr)
                        {
                            std::cout << "ERROR: Could not create topic " << topic_name << std::endl;
                            return;
                        }

                        g_reader = g_subscriber->create_datareader(
                            g_topic,
                            g_qos,
                            nullptr);
                        if (g_reader == nullptr)
                        {
                            std::cout << "ERROR: Could not create reader for topic " << topic_name << std::endl;
                            return;
                        }

                        if (type == nullptr)
                        {
                            const types::TypeIdentifier* ident =
                                    types::TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(name);

                            if (nullptr != ident)
                            {
                                const types::TypeObject* obj =
                                        types::TypeObjectFactory::get_instance()->get_type_object(ident);

                                types::DynamicType_ptr dyn_type =
                                        types::TypeObjectFactory::get_instance()->build_dynamic_type(name, ident, obj);

                                if (nullptr != dyn_type)
                                {
                                    g_type = dyn_type;
                                }
                                else
                                {
                                    std::cout << "ERROR: DynamicType cannot be created for type: " << name << std::endl;
                                }
                            }
                            else
                            {
                                std::cout << "ERROR: TypeIdentifier cannot be retrieved for type: "
                                          << name << std::endl;
                            }
                        }
                        else
                        {
                            g_type = type;
                        }
                    }
                };

        participant->register_remote_type(
            type_information,
            type_name.to_string(),
            callback);
    }

#if HAVE_SECURITY
    void onParticipantAuthentication(
            DomainParticipant* /*participant*/,
            rtps::ParticipantAuthenticationInfo&& info) override
    {
        if (rtps::ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT == info.status)
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
};

class SubListener : public SubscriberListener
{
public:

    SubListener()
        : number_samples_(0)
    {
    }

    ~SubListener() override
    {
    }

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

    void on_data_available(
            DataReader* reader) override
    {
        if (nullptr != g_type)
        {
            types::DynamicPubSubType pst(g_type);
            types::DynamicData_ptr sample(static_cast<types::DynamicData*>(pst.createData()));
            eprosima::fastdds::dds::SampleInfo info;

            if (nullptr != reader && !!reader->take_next_sample(sample.get(), &info))
            {
                if (info.valid_data)
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    ++number_samples_;
                    std::string message;
                    uint32_t index;
                    octet count;
                    sample->get_string_value(message, 0);
                    sample->get_uint32_value(index, 1);
                    types::DynamicData* inner = sample->loan_value(2);
                    inner->get_byte_value(count, 0);
                    sample->return_loaned_value(inner);
                    std::cout << "Received sample: index(" << index << "), message("
                              << message << "), inner_count(" << std::hex << (uint32_t)count << ")" << std::endl;
                    cv_.notify_all();
                }
            }
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
            g_run = false;
        }
    }

    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int number_samples_;
};

int main(
        int argc,
        char** argv)
{
    int arg_count = 1;
    bool notexit = false;
    uint32_t seed = 7800;
    uint32_t samples = 4;
    //char* xml_file = nullptr;
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

            //xml_file = argv[arg_count];
        }

        ++arg_count;
    }

    /* TODO - XMLProfileManager doesn't support DDS yet
       if(xml_file)
       {
        DomainParticipantFactory::get_instance()->load_XML_profiles_file(xml_file);
       }
     */

    //Do not enable entities on creation
    DomainParticipantFactoryQos factory_qos;
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    DomainParticipantQos participant_qos;
    participant_qos.wire_protocol().builtin.typelookup_config.use_client = true;
    ParListener participant_listener;
    StatusMask participant_mask = StatusMask::none();
    g_participant =
            DomainParticipantFactory::get_instance()->create_participant(seed % 230, participant_qos,
                    &participant_listener, participant_mask);

    if (g_participant == nullptr)
    {
        std::cout << "Error creating subscriber participant" << std::endl;
        return 1;
    }

    // Generate topic name
    std::ostringstream topic;
    topic << "HelloWorldTopic_" << ((magic.empty()) ? asio::ip::host_name() : magic) << "_" << seed;

    SubListener listener;
    StatusMask mask = StatusMask::subscription_matched()
            << StatusMask::data_available()
            << StatusMask::liveliness_changed();

    //CREATE THE SUBSCRIBER
    g_subscriber = g_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, &listener, mask);

    if (g_subscriber == nullptr)
    {
        std::cout << "Error creating subscriber" << std::endl;
        DomainParticipantFactory::get_instance()->delete_participant(g_participant);
        return 1;
    }

    //Now that everything is created we can enable the protocols
    g_participant->enable();

    while (notexit && g_run)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    if (g_run)
    {
        std::unique_lock<std::mutex> lock(listener.mutex_);
        listener.cv_.wait(lock, [&]
                {
                    return listener.number_samples_ >= samples;
                });
    }

    if (g_reader != nullptr)
    {
        g_subscriber->delete_datareader(g_reader);
    }
    if (g_subscriber != nullptr)
    {
        g_participant->delete_subscriber(g_subscriber);
    }
    if (g_topic != nullptr)
    {
        g_participant->delete_topic(g_topic);
    }
    DomainParticipantFactory::get_instance()->delete_participant(g_participant);

    return 0;
}
