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
 * @file PubSubWriterReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBWRITERREADER_HPP_
#define _TEST_BLACKBOX_PUBSUBWRITERREADER_HPP_

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <string>
#include <list>
#include <map>
#include <condition_variable>
#include <asio.hpp>
#include <gtest/gtest.h>

template<class TypeSupport>
class PubSubWriterReader
{
    class ParticipantListener : public eprosima::fastrtps::ParticipantListener
    {
        public:

            ParticipantListener(PubSubWriterReader &wreader) : wreader_(wreader) {}

            ~ParticipantListener() {}

#if HAVE_SECURITY
            void onParticipantAuthentication(eprosima::fastrtps::Participant*, const eprosima::fastrtps::ParticipantAuthenticationInfo& info)
            {
                if(info.rtps.status() == eprosima::fastrtps::rtps::AUTHORIZED_RTPSPARTICIPANT)
                    wreader_.authorized();
                else if(info.rtps.status() == eprosima::fastrtps::rtps::UNAUTHORIZED_RTPSPARTICIPANT)
                    wreader_.unauthorized();
            }
#endif

        private:

            ParticipantListener& operator=(const ParticipantListener&) = delete;

            PubSubWriterReader& wreader_;
    } participant_listener_;

    class PubListener : public eprosima::fastrtps::PublisherListener
    {
        public:

            PubListener(PubSubWriterReader &wreader) : wreader_(wreader){};

            ~PubListener(){};

            void onPublicationMatched(eprosima::fastrtps::Publisher* /*pub*/, eprosima::fastrtps::rtps::MatchingInfo &info)
            {
                if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
                    wreader_.matched();
                else
                    wreader_.unmatched();
            }

        private:

            PubListener& operator=(const PubListener&) = delete;

            PubSubWriterReader &wreader_;

    } pub_listener_;

    class SubListener: public eprosima::fastrtps::SubscriberListener
    {
        public:
            SubListener(PubSubWriterReader &wreader) : wreader_(wreader) {}

            ~SubListener(){}

            void onNewDataMessage(eprosima::fastrtps::Subscriber *sub)
            {
                ASSERT_NE(sub, nullptr);

                if(wreader_.receiving_.load())
                {
                    bool ret = false;
                    do
                    {
                        wreader_.receive_one(sub, ret);
                    } while(ret);
                }
            }

            void onSubscriptionMatched(eprosima::fastrtps::Subscriber* /*sub*/, eprosima::fastrtps::rtps::MatchingInfo& info)
            {
                if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
                    wreader_.matched();
                else
                    wreader_.unmatched();
            }

        private:

            SubListener& operator=(const SubListener&) = delete;

            PubSubWriterReader& wreader_;
    } sub_listener_;

    friend class PubListener;
    friend class SubListener;

    public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

    PubSubWriterReader(const std::string &topic_name) : participant_listener_(*this), pub_listener_(*this),
    sub_listener_(*this), participant_(nullptr), publisher_(nullptr), subscriber_(nullptr), initialized_(false),
    matched_(0), receiving_(false), current_received_count_(0), number_samples_expected_(0)
#if HAVE_SECURITY
    , authorized_(0), unauthorized_(0)
#endif
    {
        publisher_attr_.topic.topicDataType = type_.getName();
        subscriber_attr_.topic.topicDataType = type_.getName();
        // Generate topic name
        std::ostringstream t;
        t << topic_name << "_" << asio::ip::host_name() << "_" << GET_PID();
        publisher_attr_.topic.topicName = t.str();
        subscriber_attr_.topic.topicName = t.str();
        topic_name_ = t.str();

#if defined(PREALLOCATED_WITH_REALLOC_MEMORY_MODE_TEST)
        publisher_attr_.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        subscriber_attr_.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
#elif defined(DYNAMIC_RESERVE_MEMORY_MODE_TEST)
        publisher_attr_.historyMemoryPolicy = eprosima::fastrtps::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
        subscriber_attr_.historyMemoryPolicy = eprosima::fastrtps::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
#else
        publisher_attr_.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_MEMORY_MODE;
        subscriber_attr_.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_MEMORY_MODE;
#endif

        // By default, heartbeat period and nack response delay are 100 milliseconds.
        publisher_attr_.times.heartbeatPeriod.seconds = 0;
        publisher_attr_.times.heartbeatPeriod.fraction = 4294967 * 100;
        publisher_attr_.times.nackResponseDelay.seconds = 0;
        publisher_attr_.times.nackResponseDelay.fraction = 4294967 * 100;

        // By default, heartbeat period delay is 100 milliseconds.
        subscriber_attr_.times.heartbeatResponseDelay.seconds = 0;
        subscriber_attr_.times.heartbeatResponseDelay.fraction = 4294967 * 100;
    }

    ~PubSubWriterReader()
    {
        if(participant_ != nullptr)
            eprosima::fastrtps::Domain::removeParticipant(participant_);
    }

    void init()
    {
        //Create participant
        participant_attr_.rtps.builtin.domainId = (uint32_t)GET_PID() % 230;
        participant_ = eprosima::fastrtps::Domain::createParticipant(participant_attr_, &participant_listener_);

        if(participant_ != nullptr)
        {
            // Register type
            eprosima::fastrtps::Domain::registerType(participant_, &type_);

            //Create publisher
            publisher_ = eprosima::fastrtps::Domain::createPublisher(participant_, publisher_attr_, &pub_listener_);

            if(publisher_ != nullptr)
            {
                //Create subscribe r
                subscriber_ = eprosima::fastrtps::Domain::createSubscriber(participant_, subscriber_attr_, &sub_listener_);

                if(subscriber_ != nullptr)
                {
                    initialized_ = true;
                    return;
                }
            }

            eprosima::fastrtps::Domain::removeParticipant(participant_);
        }
    }

    bool isInitialized() const { return initialized_; }

    void destroy()
    {
        if(participant_ != nullptr)
        {
            eprosima::fastrtps::Domain::removeParticipant(participant_);
            participant_ = nullptr;
        }
    }

    void send(std::list<type>& msgs)
    {
        auto it = msgs.begin();

        while(it != msgs.end())
        {
            if(publisher_->write((void*)&(*it)))
            {
                default_send_print<type>(*it);
                it = msgs.erase(it);
            }
            else
                break;
        }
    }

    std::list<type> data_not_received()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return total_msgs_;
    }

    void startReception(std::list<type>& msgs)
    {
        mutex_.lock();
        total_msgs_ = msgs;
        number_samples_expected_ = total_msgs_.size();
        current_received_count_ = 0;
        mutex_.unlock();

        bool ret = false;
        do
        {
            receive_one(subscriber_, ret);
        }
        while(ret);

        receiving_.store(true);
    }

    void stopReception()
    {
        receiving_.store(false);
    }

    void block_for_all()
    {
        block([this]() -> bool {
                return number_samples_expected_ == current_received_count_;
                });
    }

    void block(std::function<bool()> checker)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, checker);
    }

    void waitDiscovery()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        std::cout << "WReader is waiting discovery..." << std::endl;

        if(matched_ < 2)
            cvDiscovery_.wait(lock);

        ASSERT_GE(matched_, 2u);
        std::cout << "WReader discovery finished..." << std::endl;
    }

    void waitRemoval()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        std::cout << "WReader is waiting removal..." << std::endl;

        if(matched_ != 0)
            cvDiscovery_.wait(lock);

        ASSERT_EQ(matched_, 0u);
        std::cout << "WReader removal finished..." << std::endl;
    }

#if HAVE_SECURITY
    void waitAuthorized(unsigned int how_many = 1)
    {
        std::unique_lock<std::mutex> lock(mutexAuthentication_);

        std::cout << "WReader is waiting authorization..." << std::endl;

        while(authorized_ != how_many)
            cvAuthentication_.wait(lock);

        ASSERT_EQ(authorized_, how_many);
        std::cout << "WReader authorization finished..." << std::endl;
    }

    void waitUnauthorized(unsigned int how_many = 1)
    {
        std::unique_lock<std::mutex> lock(mutexAuthentication_);

        std::cout << "WReader is waiting unauthorization..." << std::endl;

        while(unauthorized_ != how_many)
            cvAuthentication_.wait(lock);

        ASSERT_EQ(unauthorized_, how_many);
        std::cout << "WReader unauthorization finished..." << std::endl;
    }
#endif

    PubSubWriterReader& property_policy(const eprosima::fastrtps::rtps::PropertyPolicy property_policy)
    {
        participant_attr_.rtps.properties = property_policy;
        return *this;
    }

    PubSubWriterReader& pub_property_policy(const eprosima::fastrtps::rtps::PropertyPolicy property_policy)
    {
        publisher_attr_.properties = property_policy;
        return *this;
    }

    PubSubWriterReader& sub_property_policy(const eprosima::fastrtps::rtps::PropertyPolicy property_policy)
    {
        subscriber_attr_.properties = property_policy;
        return *this;
    }

    private:

    void receive_one(eprosima::fastrtps::Subscriber* subscriber, bool& returnedValue)
    {
        returnedValue = false;
        type data;
        eprosima::fastrtps::SampleInfo_t info;

        if(subscriber->takeNextData((void*)&data, &info))
        {
            returnedValue = true;

            std::unique_lock<std::mutex> lock(mutex_);

            // Check order of changes.
            ASSERT_LT(last_seq, info.sample_identity.sequence_number());
            last_seq = info.sample_identity.sequence_number();

            if(info.sampleKind == eprosima::fastrtps::rtps::ALIVE)
            {
                auto it = std::find(total_msgs_.begin(), total_msgs_.end(), data);
                ASSERT_NE(it, total_msgs_.end());
                total_msgs_.erase(it);
                ++current_received_count_;
                default_receive_print<type>(data);
                cv_.notify_one();
            }
        }
    }

    void matched()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);
        ++matched_;
        cvDiscovery_.notify_one();
    }

    void unmatched()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);
        --matched_;
        cvDiscovery_.notify_one();
    }

#if HAVE_SECURITY
    void authorized()
    {
        mutexAuthentication_.lock();
        ++authorized_;
        mutexAuthentication_.unlock();
        cvAuthentication_.notify_all();
    }

    void unauthorized()
    {
        mutexAuthentication_.lock();
        ++unauthorized_;
        mutexAuthentication_.unlock();
        cvAuthentication_.notify_all();
    }
#endif

    PubSubWriterReader& operator=(const PubSubWriterReader&)= delete;

    eprosima::fastrtps::Participant *participant_;
    eprosima::fastrtps::ParticipantAttributes participant_attr_;
    eprosima::fastrtps::Publisher *publisher_;
    eprosima::fastrtps::PublisherAttributes publisher_attr_;
    eprosima::fastrtps::Subscriber *subscriber_;
    eprosima::fastrtps::SubscriberAttributes subscriber_attr_;
    std::string topic_name_;
    bool initialized_;
    std::list<type> total_msgs_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::mutex mutexDiscovery_;
    std::condition_variable cvDiscovery_;
    unsigned int matched_;
    std::atomic<bool> receiving_;
    type_support type_;
	eprosima::fastrtps::rtps::SequenceNumber_t last_seq;
    size_t current_received_count_;
    size_t number_samples_expected_;
#if HAVE_SECURITY
    std::mutex mutexAuthentication_;
    std::condition_variable cvAuthentication_;
    unsigned int authorized_;
    unsigned int unauthorized_;
#endif
};

#endif // _TEST_BLACKBOX_PUBSUBWRITER_HPP_
