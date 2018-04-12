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
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/Domain.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/utils/eClock.h>

#include <types/HelloWorldType.h>

#include <mutex>
#include <fstream>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class SubListener : public SubscriberListener
{
    public:

        SubListener() : number_samples_(0) {}

        ~SubListener() {}

        void onSubscriptionMatched(Subscriber* /*subscriber*/, MatchingInfo& info) override
        {
            if(info.status == MATCHED_MATCHING)
                std::cout << "Publisher matched" << std::endl;
            else
                std::cout << "Publisher unmatched" << std::endl;
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
                    ++number_samples_;
                    std::cout << "Received sample: index(" << sample.index() << "), message(" << sample.message() << ")" << std::endl;
                    cv_.notify_all();
                }
            }
        }

        std::mutex mutex_;
        std::condition_variable cv_;
        unsigned int number_samples_;
};

int main(int argc, char** argv)
{
    int arg_count = 1;
    bool notexit = false;
    uint32_t seed = 7800;

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

        ++arg_count;
    }

    ParticipantAttributes participant_attributes;
    participant_attributes.rtps.builtin.domainId = seed % 230;
    participant_attributes.rtps.builtin.leaseDuration.seconds = 3;
    participant_attributes.rtps.builtin.leaseDuration_announcementperiod.seconds = 1;
    Participant* participant = Domain::createParticipant(participant_attributes);
    if(participant==nullptr)
        return 1;

    //REGISTER THE TYPE

    HelloWorldType type;
    Domain::registerType(participant, &type);

    SubListener listener;

    // Generate topic name
    std::ostringstream topic;
    topic << "HelloWorldTopic_" << asio::ip::host_name() << "_" << seed;

    //CREATE THE SUBSCRIBER
    SubscriberAttributes subscriber_attributes;
    subscriber_attributes.topic.topicKind = NO_KEY;
    subscriber_attributes.topic.topicDataType = type.getName();
    subscriber_attributes.topic.topicName = topic.str();
    subscriber_attributes.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Subscriber* subscriber = Domain::createSubscriber(participant, subscriber_attributes, &listener);

    if(subscriber == nullptr)
    {
        Domain::removeParticipant(participant);
        return 1;
    }

    while(notexit)
    {
        eClock::my_sleep(250);
    }

    {
        std::unique_lock<std::mutex> lock(listener.mutex_);
        listener.cv_.wait(lock, [&]{ return listener.number_samples_ >= 4; });
    }

    Domain::removeParticipant(participant);

    return 0;
}
