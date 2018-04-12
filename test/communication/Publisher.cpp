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
#include <fastrtps/utils/eClock.h>

#include <types/HelloWorldType.h>

#include <mutex>
#include <fstream>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

bool run = true;

class ParListener : public ParticipantListener
{
    public:
        ParListener(bool exit_on_lost_liveliness) : exit_on_lost_liveliness_(exit_on_lost_liveliness) {};
        virtual ~ParListener(){};

        /**
         * This method is called when a new Participant is discovered, or a previously discovered participant changes its QOS or is removed.
         * @param p Pointer to the Participant
         * @param info DiscoveryInfo.
         */
        void onParticipantDiscovery(Participant* /*p*/, ParticipantDiscoveryInfo info) override
        {
            if(info.rtps.m_status == DISCOVERED_RTPSPARTICIPANT)
                std::cout << "Published discovered a participant" << std::endl;
            else if(info.rtps.m_status == CHANGED_QOS_RTPSPARTICIPANT)
                std::cout << "Published detected changes on a participant" << std::endl;
            else if(info.rtps.m_status == REMOVED_RTPSPARTICIPANT)
                std::cout << "Published removed a participant" << std::endl;
            else if(info.rtps.m_status == DROPPED_RTPSPARTICIPANT)
            {
                std::cout << "Published dropped a participant" << std::endl;
                if(exit_on_lost_liveliness_)
                    run = false;
            }
        }

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
                std::cout << "Subscriber matched" << std::endl;
                ++matched_;
            }
            else
            {
                std::cout << "Subscriber unmatched" << std::endl;
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
    uint32_t seed = 7800;

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

        ++arg_count;
    }

    ParticipantAttributes participant_attributes;
    participant_attributes.rtps.builtin.domainId = seed % 230;
    participant_attributes.rtps.builtin.leaseDuration.seconds = 3;
    participant_attributes.rtps.builtin.leaseDuration_announcementperiod.seconds = 1;
    ParListener participant_listener(exit_on_lost_liveliness);
    Participant* participant = Domain::createParticipant(participant_attributes, &participant_listener);

    if(participant == nullptr)
        return 1;

    HelloWorldType type;
    Domain::registerType(participant,&type);

    PubListener listener;

    // Generate topic name
    std::ostringstream topic;
    topic << "HelloWorldTopic_" << asio::ip::host_name() << "_" << seed;

    //CREATE THE PUBLISHER
    PublisherAttributes publisher_attributes;
    publisher_attributes.topic.topicKind = NO_KEY;
    publisher_attributes.topic.topicDataType = type.getName();
    publisher_attributes.topic.topicName = topic.str();
    publisher_attributes.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Publisher* publisher = Domain::createPublisher(participant, publisher_attributes, &listener);
    if(publisher == nullptr)
    {
        Domain::removeParticipant(participant);
        return 1;
    }

    {
        std::unique_lock<std::mutex> lock(listener.mutex_);
        listener.cv_.wait(lock, [&]{return listener.matched_ > 0;});
    }

    HelloWorld data;
    data.index(1);
    data.message("HelloWorld");

    while(run)
    {
        publisher->write((void*)&data);

        if(data.index() == 4)
            data.index() = 1;
        else
            ++data.index();

        eClock::my_sleep(250);
    };

    Domain::removeParticipant(participant);

    return 0;
}
