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

#include <fastrtps/participant/Participant.h>
#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/Domain.h>

#include <types/HelloWorldType.h>

#include <mutex>
#include <condition_variable>
#include <fstream>
#include <string>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

bool run = true;

class ParListener : public ParticipantListener
{
    public:
        ParListener(bool exit_on_lost_liveliness) : exit_on_lost_liveliness_(exit_on_lost_liveliness) {};
        virtual ~ParListener(){};

        /**
         * This method is called when a new Participant is discovered, or a previously discovered participant
         * changes its QOS or is removed.
         * @param p Pointer to the Participant
         * @param info DiscoveryInfo.
         */
        void onParticipantDiscovery(
                Participant* /*participant*/,
                rtps::ParticipantDiscoveryInfo&& info) override
        {
            if(info.status == rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
            {
                std::cout << "Publisher participant " << //participant->getGuid() <<
                    " discovered participant " << info.info.m_guid << std::endl;
            }
            else if(info.status == rtps::ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT)
            {
                std::cout << "Publisher participant " << //participant->getGuid() <<
                    " detected changes on participant " << info.info.m_guid << std::endl;
            }
            else if(info.status == rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
            {
                std::cout << "Publisher participant " << //participant->getGuid() <<
                    " removed participant " << info.info.m_guid << std::endl;
            }
            else if(info.status == rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
            {
                std::cout << "Publisher participant " << //participant->getGuid() <<
                    " dropped participant " << info.info.m_guid << std::endl;
                if(exit_on_lost_liveliness_)
                {
                    run = false;
                }
            }
        }

#if HAVE_SECURITY
        void onParticipantAuthentication(
                Participant* participant,
                rtps::ParticipantAuthenticationInfo&& info) override
        {
            if (rtps::ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT == info.status)
            {
                std::cout << "Publisher participant " << participant->getGuid() <<
                    " authorized participant " << info.guid << std::endl;
            }
            else
            {
                std::cout << "Publisher participant " << participant->getGuid() <<
                    " unauthorized participant " << info.guid << std::endl;
            }
        }
#endif

    private:

        bool exit_on_lost_liveliness_;
};

class PubListener : public PublisherListener
{
    public:

        PubListener() : matched_(0) {};

        ~PubListener() {};

        void onPublicationMatched(Publisher* /*publisher*/, MatchingInfo& info) override
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if(info.status == MATCHED_MATCHING)
            {
                std::cout << "Publisher matched with subscriber " << info.remoteEndpointGuid << std::endl;
                ++matched_;
            }
            else
            {
                std::cout << "Publisher unmatched with subscriber " << info.remoteEndpointGuid << std::endl;
                --matched_;
            }
            cv_.notify_all();
        }

        std::mutex mutex_;
        std::condition_variable cv_;
        unsigned int matched_;
};

int main(int argc, char** argv)
{
    int arg_count = 1;
    bool exit_on_lost_liveliness = false;
    uint32_t seed = 7800, wait = 0;
    char* xml_file = nullptr;
    uint32_t samples = 4;
    std::string magic;

    while(arg_count < argc)
    {
        if(strcmp(argv[arg_count], "--exit_on_lost_liveliness") == 0)
        {
            exit_on_lost_liveliness = true;
        }
        else if(strcmp(argv[arg_count], "--seed") == 0)
        {
            if(++arg_count >= argc)
            {
                std::cout << "--seed expects a parameter" << std::endl;
                return -1;
            }

            seed = strtol(argv[arg_count], nullptr, 10);
        }
        else if(strcmp(argv[arg_count], "--wait") == 0)
        {
            if(++arg_count >= argc)
            {
                std::cout << "--wait expects a parameter" << std::endl;
                return -1;
            }

            wait = strtol(argv[arg_count], nullptr, 10);
        }
        else if(strcmp(argv[arg_count], "--samples") == 0)
        {
            if(++arg_count >= argc)
            {
                std::cout << "--samples expects a parameter" << std::endl;
                return -1;
            }

            samples = strtol(argv[arg_count], nullptr, 10);
        }
        else if(strcmp(argv[arg_count], "--magic") == 0)
        {
            if(++arg_count >= argc)
            {
                std::cout << "--magic expects a parameter" << std::endl;
                return -1;
            }

            magic = argv[arg_count];
        }
        else if(strcmp(argv[arg_count], "--xmlfile") == 0)
        {
            if(++arg_count >= argc)
            {
                std::cout << "--xmlfile expects a parameter" << std::endl;
                return -1;
            }

            xml_file = argv[arg_count];
        }

        ++arg_count;
    }

    if(xml_file)
    {
        Domain::loadXMLProfilesFile(xml_file);
    }

    ParticipantAttributes participant_attributes;
    Domain::getDefaultParticipantAttributes(participant_attributes);
    participant_attributes.rtps.builtin.domainId = seed % 230;
    ParListener participant_listener(exit_on_lost_liveliness);
    Participant* participant = Domain::createParticipant(participant_attributes, &participant_listener);

    if(participant == nullptr)
    {
        return 1;
    }

    HelloWorldType type;
    Domain::registerType(participant,&type);

    PubListener listener;

    // Generate topic name
    std::ostringstream topic;
    topic << "HelloWorldTopic_" << ((magic.empty()) ? asio::ip::host_name() : magic) << "_" << seed;

    //CREATE THE PUBLISHER
    PublisherAttributes publisher_attributes;
    //Domain::getDefaultPublisherAttributes(publisher_attributes);
    publisher_attributes.topic.topicKind = NO_KEY;
    publisher_attributes.topic.topicDataType = type.getName();
    publisher_attributes.topic.topicName = topic.str();
    publisher_attributes.qos.m_liveliness.lease_duration = 3;
    publisher_attributes.qos.m_liveliness.announcement_period = 1;
    publisher_attributes.qos.m_liveliness.kind = AUTOMATIC_LIVELINESS_QOS;
    Publisher* publisher = Domain::createPublisher(participant, publisher_attributes, &listener);
    if(publisher == nullptr)
    {
        Domain::removeParticipant(participant);
        return 1;
    }

    if(wait > 0)
    {
        std::unique_lock<std::mutex> lock(listener.mutex_);
        listener.cv_.wait(lock, [&]{return listener.matched_ >= wait;});
    }

    HelloWorld data;
    data.index(1);
    data.message("HelloWorld");

    while(run)
    {
        publisher->write((void*)&data);

        if(data.index() == samples)
        {
            data.index() = 1;
        }
        else
        {
            ++data.index();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    };

    Domain::removeParticipant(participant);

    return 0;
}
