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
 * @file Publisher.cpp
 */

#include <asio.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <mutex>
#include <condition_variable>
#include <fstream>
#include <string>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

static bool run = true;

class ParListener : public DomainParticipantListener
{
public:

    ParListener(
            bool exit_on_lost_liveliness)
        : exit_on_lost_liveliness_(exit_on_lost_liveliness)
    {
    }

    virtual ~ParListener() override
    {
    }

    /**
     * This method is called when a new Participant is discovered, or a previously discovered participant
     * changes its QOS or is removed.
     * @param p Pointer to the Participant
     * @param info DiscoveryInfo.
     */
    void on_participant_discovery(
            DomainParticipant* /*participant*/,
            rtps::ParticipantDiscoveryInfo&& info) override
    {
        if (info.status == rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
        {
            std::cout << "Publisher participant " << //participant->getGuid() <<
                " discovered participant " << info.info.m_guid << std::endl;
        }
        else if (info.status == rtps::ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT)
        {
            std::cout << "Publisher participant " << //participant->getGuid() <<
                " detected changes on participant " << info.info.m_guid << std::endl;
        }
        else if (info.status == rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
        {
            std::cout << "Publisher participant " << //participant->getGuid() <<
                " removed participant " << info.info.m_guid << std::endl;
        }
        else if (info.status == rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
        {
            std::cout << "Publisher participant " << //participant->getGuid() <<
                " dropped participant " << info.info.m_guid << std::endl;
            if (exit_on_lost_liveliness_)
            {
                run = false;
            }
        }
    }

#if HAVE_SECURITY
    void onParticipantAuthentication(
            DomainParticipant* participant,
            rtps::ParticipantAuthenticationInfo&& info) override
    {
        if (rtps::ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT == info.status)
        {
            std::cout << "Publisher participant " << participant->guid() <<
                " authorized participant " << info.guid << std::endl;
        }
        else
        {
            std::cout << "Publisher participant " << participant->guid() <<
                " unauthorized participant " << info.guid << std::endl;
        }
    }

#endif // if HAVE_SECURITY

private:

    bool exit_on_lost_liveliness_;
};

class PubListener : public PublisherListener
{
public:

    PubListener()
        : matched_(0)
    {
    }

    ~PubListener() override
    {
    }

    void on_publication_matched(
            DataWriter* /*publisher*/,
            const PublicationMatchedStatus& info) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (info.current_count_change == 1)
        {
            std::cout << "Publisher matched with subscriber " << info.last_subscription_handle
                      << ": " << ++matched_ << std::endl;
        }
        else if (info.current_count_change == -1)
        {
            std::cout << "Publisher unmatched with subscriber " << info.last_subscription_handle
                      << ": " << --matched_ << std::endl;
        }
        else
        {
            std::cout << info.current_count_change
                      << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
        }
        cv_.notify_all();
    }

    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int matched_;
};

int main(
        int argc,
        char** argv)
{
    int arg_count = 1;
    bool exit_on_lost_liveliness = false;
    uint32_t seed = 7800, wait = 0;
    //char* xml_file = nullptr;
    uint32_t samples = 4;
    std::string magic;

    while (arg_count < argc)
    {
        if (strcmp(argv[arg_count], "--exit_on_lost_liveliness") == 0)
        {
            exit_on_lost_liveliness = true;
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
        else if (strcmp(argv[arg_count], "--wait") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--wait expects a parameter" << std::endl;
                return -1;
            }

            wait = strtol(argv[arg_count], nullptr, 10);
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
       if (xml_file)
       {
        DomainParticipantFactory::get_instance()->load_XML_profiles_file(xml_file);
       }
     */

    xmlparser::XMLProfileManager::loadXMLFile("example_type.xml");

    DomainParticipantQos participant_qos;
    participant_qos.wire_protocol().builtin.typelookup_config.use_server = true;
    ParListener participant_listener(exit_on_lost_liveliness);
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(seed % 230, participant_qos,
                    &participant_listener);

    if (participant == nullptr)
    {
        std::cout << "Error creating publisher participant" << std::endl;
        return 1;
    }

    types::DynamicType_ptr dyn_type = xmlparser::XMLProfileManager::getDynamicTypeByName("TypeLookup")->build();
    TypeSupport type(new types::DynamicPubSubType(dyn_type));
    type.register_type(participant);

    PubListener listener;
    StatusMask mask = StatusMask::publication_matched();

    // Generate topic name
    std::ostringstream topic_name;
    topic_name << "HelloWorldTopic_" << ((magic.empty()) ? asio::ip::host_name() : magic) << "_" << seed;

    //CREATE THE PUBLISHER
    DataWriterQos wqos;
    wqos.liveliness().lease_duration = 3;
    wqos.liveliness().announcement_period = 1;
    wqos.liveliness().kind = eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS;

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT, &listener, mask);
    if (publisher == nullptr)
    {
        std::cout << "Error creating publisher" << std::endl;
        DomainParticipantFactory::get_instance()->delete_participant(participant);
        return 1;
    }
    Topic* topic = participant->create_topic(topic_name.str(), type.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic == nullptr)
    {
        std::cout << "Error creating publisher topic" << std::endl;
        participant->delete_publisher(publisher);
        DomainParticipantFactory::get_instance()->delete_participant(participant);
        return 1;
    }

    DataWriter* writer = publisher->create_datawriter(topic, wqos, nullptr);
    if (writer == nullptr)
    {
        participant->delete_publisher(publisher);
        participant->delete_topic(topic);
        DomainParticipantFactory::get_instance()->delete_participant(participant);
        return 1;
    }

    if (wait > 0)
    {
        std::unique_lock<std::mutex> lock(listener.mutex_);
        listener.cv_.wait(lock, [&]
                {
                    return listener.matched_ >= wait;
                });
    }

    types::DynamicData_ptr data(types::DynamicDataFactory::get_instance()->create_data(dyn_type));
    data->set_string_value("Hello DDS Dynamic World", 0);
    data->set_uint32_value(1, 1);
    types::DynamicData* inner = data->loan_value(2);
    inner->set_byte_value(10, 0);
    data->return_loaned_value(inner);

    while (run)
    {
        writer->write(data.get());

        uint32_t index;
        data->get_uint32_value(index, 1);

        if (index == samples)
        {
            data->set_uint32_value(1, 1);
        }
        else
        {
            data->set_uint32_value(index + 1, 1);
        }

        inner = data->loan_value(2);
        octet inner_count;
        inner->get_byte_value(inner_count, 0);
        inner->set_byte_value(inner_count + 1, 0);
        data->return_loaned_value(inner);

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    if (writer != nullptr)
    {
        publisher->delete_datawriter(writer);
    }
    if (publisher != nullptr)
    {
        participant->delete_publisher(publisher);
    }
    if (topic != nullptr)
    {
        participant->delete_topic(topic);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant);

    return 0;
}
