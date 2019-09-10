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

#include <fastrtps/participant/Participant.h>
#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/Domain.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <types/HelloWorldType.h>

#include <mutex>
#include <condition_variable>
#include <fstream>
#include <string>
#include <map>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

bool run = true;

class ParListener : public ParticipantListener
{
    public:
        ParListener() {};
        virtual ~ParListener(){};

        /**
         * This method is called when a new Participant is discovered, or a previously discovered participant changes its QOS or is removed.
         * @param p Pointer to the Participant
         * @param info DiscoveryInfo.
         */
        void onParticipantDiscovery(
                Participant* /*participant*/,
                rtps::ParticipantDiscoveryInfo&& info) override
        {
            if(info.status == rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
            {
                std::cout << "Subscriber participant " << //participant->getGuid() <<
                    " discovered participant " << info.info.m_guid << std::endl;
            }
            else if(info.status == rtps::ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT)
            {
                std::cout << "Subscriber participant " << //participant->getGuid() <<
                    " detected changes on participant " << info.info.m_guid << std::endl;
            }
            else if(info.status == rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
            {
                std::cout << "Subscriber participant " << //participant->getGuid() <<
                    " removed participant " << info.info.m_guid << std::endl;
            }
            else if(info.status == rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
            {
                std::cout << "Subscriber participant " << //participant->getGuid() <<
                    " dropped participant " << info.info.m_guid << std::endl;
            }
        }

#if HAVE_SECURITY
        void onParticipantAuthentication(
                Participant* /*participant*/,
                rtps::ParticipantAuthenticationInfo&& info) override
        {
            if (rtps::ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT == info.status)
            {
                std::cout << "Subscriber participant " << //participant->getGuid() <<
                    " authorized participant " << info.guid << std::endl;
            }
            else
            {
                std::cout << "Subscriber participant " << //participant->getGuid() <<
                    " unauthorized participant " << info.guid << std::endl;
            }
        }
#endif
};

class SubListener : public SubscriberListener
{
    public:

        SubListener(const uint32_t max_number_samples)
            : max_number_samples_(max_number_samples)
        {
        }

        void onSubscriptionMatched(Subscriber* /*subscriber*/, MatchingInfo& info) override
        {
            if(info.status == MATCHED_MATCHING)
            {
                std::cout << "Subscriber matched with publisher " << info.remoteEndpointGuid << std::endl;
            }
            else
            {
                std::cout << "Subscriber unmatched with publisher " << info.remoteEndpointGuid << std::endl;
            }
        }

        void onNewDataMessage(Subscriber* subscriber) override
        {
            HelloWorld sample;
            SampleInfo_t info;

            if(subscriber->takeNextData((void*)&sample, &info))
            {
                if(info.sampleKind == ALIVE)
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    std::cout << "Received sample (" << info.sample_identity.writer_guid() << " - " <<
                        info.sample_identity.sequence_number() << "): index(" << sample.index() << "), message("
                        << sample.message() << ")" << std::endl;
                    if(max_number_samples_ <= ++number_samples_[info.sample_identity.writer_guid()])
                    {
                        cv_.notify_all();
                    }
                }
            }
        }

        void on_liveliness_changed(
                Subscriber* sub,
                const LivelinessChangedStatus& status) override
        {
            (void)sub;
            if (status.alive_count_change == 1)
            {
                std::cout << "Publisher recovered liveliness" << std::endl;
            }
            else if (status.not_alive_count_change == 1)
            {
                std::cout << "Publisher lost liveliness" << std::endl;
                run = false;
            }
        }

        std::mutex mutex_;
        std::condition_variable cv_;
        const uint32_t max_number_samples_;
        std::map<GUID_t, uint32_t> number_samples_;
};

int main(int argc, char** argv)
{
    int arg_count = 1;
    bool notexit = false;
    uint32_t seed = 7800;
    uint32_t samples = 4;
    uint32_t publishers = 1;
    char* xml_file = nullptr;
    std::string magic;

    while(arg_count < argc)
    {
        if(strcmp(argv[arg_count], "--notexit") == 0)
        {
            notexit = true;
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
        else if(strcmp(argv[arg_count], "--publishers") == 0)
        {
            if(++arg_count >= argc)
            {
                std::cout << "--publishers expects a parameter" << std::endl;
                return -1;
            }

            publishers = strtol(argv[arg_count], nullptr, 10);
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
    ParListener participant_listener;
    Participant* participant = Domain::createParticipant(participant_attributes, &participant_listener);
    if(participant==nullptr)
        return 1;

    //REGISTER THE TYPE

    HelloWorldType type;
    Domain::registerType(participant, &type);

    SubListener listener(samples);

    // Generate topic name
    std::ostringstream topic;
    topic << "HelloWorldTopic_" << ((magic.empty()) ? asio::ip::host_name() : magic) << "_" << seed;

    //CREATE THE SUBSCRIBER
    SubscriberAttributes subscriber_attributes;
    //Domain::getDefaultSubscriberAttributes(subscriber_attributes);
    subscriber_attributes.topic.topicKind = NO_KEY;
    subscriber_attributes.topic.topicDataType = type.getName();
    subscriber_attributes.topic.topicName = topic.str();
    subscriber_attributes.qos.m_liveliness.lease_duration = 3;
    subscriber_attributes.qos.m_liveliness.kind = AUTOMATIC_LIVELINESS_QOS;
    Subscriber* subscriber = Domain::createSubscriber(participant, subscriber_attributes, &listener);

    if(subscriber == nullptr)
    {
        Domain::removeParticipant(participant);
        return 1;
    }

    while(notexit && run)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    if (run)
    {
        std::unique_lock<std::mutex> lock(listener.mutex_);
        listener.cv_.wait(lock, [&]
                {
                    if(publishers < listener.number_samples_.size())
                    {
                        // Will fail later.
                        return true;
                    }
                    else if( publishers > listener.number_samples_.size())
                    {
                        return false;
                    }

                    for(auto& number_samples : listener.number_samples_)
                    {
                        if(samples > number_samples.second)
                        {
                            return false;
                        }
                    }

                    return true;
                });
    }

    Domain::removeParticipant(participant);

    if(publishers < listener.number_samples_.size())
    {
        std::cout << "ERROR: detected more than " << publishers << " publishers" << std::endl;
        return -1;
    }

    return 0;
}
