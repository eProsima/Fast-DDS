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
 * @file PubSubReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBREADER_HPP_
#define _TEST_BLACKBOX_PUBSUBREADER_HPP_

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <string>
#include <list>
#include <condition_variable>
#include <asio.hpp>
#include <gtest/gtest.h>

template<class TypeSupport>
class PubSubReader
{
    public:

        typedef TypeSupport type_support;
        typedef typename type_support::type type;

    private:

        class ParticipantListener : public eprosima::fastrtps::ParticipantListener
        {
            public:

                ParticipantListener(PubSubReader &reader) : reader_(reader) {}

                ~ParticipantListener() {}

                void onParticipantDiscovery(Participant*, ParticipantDiscoveryInfo info)
                {
                    if(reader_.onDiscovery_!=nullptr)
                    {
                        reader_.discovery_result_ = reader_.onDiscovery_(info);

                    }

                    if(info.rtps.m_status == DISCOVERED_RTPSPARTICIPANT)
                    {
                        reader_.participant_matched();
                    }
                    else if(info.rtps.m_status == REMOVED_RTPSPARTICIPANT)
                    {
                        reader_.participant_unmatched();
                    }
                }

#if HAVE_SECURITY
                void onParticipantAuthentication(Participant*, const ParticipantAuthenticationInfo& info)
                {
                    if(info.rtps.status() == AUTHORIZED_RTPSPARTICIPANT)
                        reader_.authorized();
                    else if(info.rtps.status() == UNAUTHORIZED_RTPSPARTICIPANT)
                        reader_.unauthorized();
                }
#endif

            private:

                ParticipantListener& operator=(const ParticipantListener&) = delete;

                PubSubReader& reader_;
        } participant_listener_;

        class Listener: public eprosima::fastrtps::SubscriberListener
        {
            public:
                Listener(PubSubReader &reader) : reader_(reader) {}

                ~Listener(){}

                void onNewDataMessage(eprosima::fastrtps::Subscriber *sub)
                {
                    ASSERT_NE(sub, nullptr);

                    bool ret = false;
                    reader_.receive_one(sub, ret);
                }

                void onSubscriptionMatched(eprosima::fastrtps::Subscriber* /*sub*/, MatchingInfo& info)
                {
                    if (info.status == MATCHED_MATCHING)
                        reader_.matched();
                    else
                        reader_.unmatched();
                }

            private:

                Listener& operator=(const Listener&) = delete;

                PubSubReader& reader_;
        } listener_;

        friend class Listener;

    public:

        PubSubReader(const std::string& topic_name) : participant_listener_(*this), listener_(*this),
        participant_(nullptr), subscriber_(nullptr), topic_name_(topic_name), initialized_(false),
        matched_(0), participant_matched_(0), receiving_(false), current_received_count_(0),
        number_samples_expected_(0), discovery_result_(false), onDiscovery_(nullptr)
#if HAVE_SECURITY
        , authorized_(0), unauthorized_(0)
#endif
        {
            subscriber_attr_.topic.topicDataType = type_.getName();
            // Generate topic name
            std::ostringstream t;
            t << topic_name_ << "_" << asio::ip::host_name() << "_" << GET_PID();
            subscriber_attr_.topic.topicName = t.str();

#if defined(PREALLOCATED_WITH_REALLOC_MEMORY_MODE_TEST)
            subscriber_attr_.historyMemoryPolicy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
#elif defined(DYNAMIC_RESERVE_MEMORY_MODE_TEST)
            subscriber_attr_.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
#else
            subscriber_attr_.historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
#endif

            // By default, heartbeat period delay is 100 milliseconds.
            subscriber_attr_.times.heartbeatResponseDelay.seconds = 0;
            subscriber_attr_.times.heartbeatResponseDelay.fraction = 4294967 * 100;
        }

        ~PubSubReader()
        {
            if(participant_ != nullptr)
                Domain::removeParticipant(participant_);
        }

        void init()
        {
            participant_attr_.rtps.builtin.domainId = (uint32_t)GET_PID() % 230;
            participant_ = eprosima::fastrtps::Domain::createParticipant(participant_attr_, &participant_listener_);

            ASSERT_NE(participant_, nullptr);

            // Register type
            ASSERT_EQ(eprosima::fastrtps::Domain::registerType(participant_, &type_), true);

            //Create subscribe r
            subscriber_ = eprosima::fastrtps::Domain::createSubscriber(participant_, subscriber_attr_, &listener_);
            ASSERT_NE(subscriber_, nullptr);

            initialized_ = true;
        }

        bool isInitialized() const { return initialized_; }

        void destroy()
        {
            if(participant_ != nullptr)
            {
                Domain::removeParticipant(participant_);
                participant_ = nullptr;
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

            receiving_.store(true);

            bool ret = false;
            do
            {
                receive_one(subscriber_, ret);
            }
            while(ret);
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

        size_t block_for_at_least(size_t at_least)
        {
            block([this, at_least]() -> bool {
                    return current_received_count_ >= at_least;
                    });
            return current_received_count_;
        }

        void block(std::function<bool()> checker)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, checker);
        }

        template<class _Rep,
            class _Period
                >
                size_t block_for_all(const std::chrono::duration<_Rep, _Period>& max_wait)
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cv_.wait_for(lock, max_wait, [this]() -> bool {
                            return number_samples_expected_ == current_received_count_;
                            });

                    return current_received_count_;
                }

        void waitDiscovery()
        {
            std::unique_lock<std::mutex> lock(mutexDiscovery_);

            std::cout << "Reader is waiting discovery..." << std::endl;

            cvDiscovery_.wait(lock, [&](){return matched_ != 0;});

            std::cout << "Reader discovery finished..." << std::endl;
        }

        void wait_participant_undiscovery()
        {
            std::unique_lock<std::mutex> lock(mutexDiscovery_);

            std::cout << "Reader is waiting undiscovery..." << std::endl;

            cvDiscovery_.wait(lock, [&](){return participant_matched_ == 0;});

            std::cout << "Reader undiscovery finished..." << std::endl;
        }

        void wait_writer_undiscovery()
        {
            std::unique_lock<std::mutex> lock(mutexDiscovery_);

            std::cout << "Reader is waiting removal..." << std::endl;

            cvDiscovery_.wait(lock, [&](){return matched_ == 0;});

            std::cout << "Reader removal finished..." << std::endl;
        }

#if HAVE_SECURITY
        void waitAuthorized(unsigned int how_many = 1)
        {
            std::unique_lock<std::mutex> lock(mutexAuthentication_);

            std::cout << "Reader is waiting authorization..." << std::endl;

            while(authorized_ != how_many)
                cvAuthentication_.wait(lock);

            ASSERT_EQ(authorized_, how_many);
            std::cout << "Reader authorization finished..." << std::endl;
        }

        void waitUnauthorized(unsigned int how_many = 1)
        {
            std::unique_lock<std::mutex> lock(mutexAuthentication_);

            std::cout << "Reader is waiting unauthorization..." << std::endl;

            while(unauthorized_ != how_many)
                cvAuthentication_.wait(lock);

            ASSERT_EQ(unauthorized_, how_many);
            std::cout << "Reader unauthorization finished..." << std::endl;
        }
#endif

        size_t getReceivedCount() const
        {
            return current_received_count_;
        }

        /*** Function to change QoS ***/
        PubSubReader& reliability(const eprosima::fastrtps::ReliabilityQosPolicyKind kind)
        {
            subscriber_attr_.qos.m_reliability.kind = kind;
            return *this;
        }

        PubSubReader& history_kind(const eprosima::fastrtps::HistoryQosPolicyKind kind)
        {
            subscriber_attr_.topic.historyQos.kind = kind;
            return *this;
        }

        PubSubReader& history_depth(const int32_t depth)
        {
            subscriber_attr_.topic.historyQos.depth = depth;
            return *this;
        }

        PubSubReader& disable_builtin_transport()
        {
            participant_attr_.rtps.useBuiltinTransports = false;
            return *this;
        }

        PubSubReader& add_user_transport_to_pparams(std::shared_ptr<TransportDescriptorInterface> userTransportDescriptor)
        {
            participant_attr_.rtps.userTransports.push_back(userTransportDescriptor);
            return *this;
        }

        PubSubReader& resource_limits_allocated_samples(const int32_t initial)
        {
            subscriber_attr_.topic.resourceLimitsQos.allocated_samples = initial;
            return *this;
        }

        PubSubReader& resource_limits_max_samples(const int32_t max)
        {
            subscriber_attr_.topic.resourceLimitsQos.max_samples = max;
            return *this;
        }

        PubSubReader& heartbeatResponseDelay(const int32_t secs, const int32_t frac)
        {
            subscriber_attr_.times.heartbeatResponseDelay.seconds = secs;
            subscriber_attr_.times.heartbeatResponseDelay.fraction = frac;
            return *this;
        }

        PubSubReader& unicastLocatorList(LocatorList_t unicastLocators)
        {
            subscriber_attr_.unicastLocatorList = unicastLocators;
            return *this;
        }

        PubSubReader& multicastLocatorList(LocatorList_t multicastLocators)
        {
            subscriber_attr_.multicastLocatorList = multicastLocators;
            return *this;
        }

        PubSubReader& metatraffic_unicast_locator_list(LocatorList_t unicastLocators)
        {
            participant_attr_.rtps.builtin.metatrafficUnicastLocatorList = unicastLocators;
            return *this;
        }

        PubSubReader& initial_peers(LocatorList_t initial_peers)
        {
            participant_attr_.rtps.builtin.initialPeersList = initial_peers;
            return *this;
        }

        PubSubReader& outLocatorList(LocatorList_t outLocators)
        {
            subscriber_attr_.outLocatorList = outLocators;
            return *this;
        }

        PubSubReader& durability_kind(const eprosima::fastrtps::DurabilityQosPolicyKind kind)
        {
            subscriber_attr_.qos.m_durability.kind = kind;
            return *this;
        }

        PubSubReader& static_discovery(const char* filename)
        {
            participant_attr_.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = false;
            participant_attr_.rtps.builtin.use_STATIC_EndpointDiscoveryProtocol = true;
            participant_attr_.rtps.builtin.setStaticEndpointXMLFilename(filename);
            return *this;
        }

        PubSubReader& setSubscriberIDs(uint8_t UserID, uint8_t EntityID)
        {
            subscriber_attr_.setUserDefinedID(UserID);
            subscriber_attr_.setEntityID(EntityID);
            return *this;

        }

        PubSubReader& setManualTopicName(std::string topicName)
        {
            subscriber_attr_.topic.topicName=topicName;
            return *this;
        }

        PubSubReader& disable_multicast(int32_t participantId)
        {
            participant_attr_.rtps.participantID = participantId;

            LocatorList_t default_unicast_locators;
            Locator_t default_unicast_locator;

            default_unicast_locators.push_back(default_unicast_locator);
            participant_attr_.rtps.builtin.metatrafficUnicastLocatorList = default_unicast_locators;

            Locator_t loopback_locator;
            loopback_locator.set_IP4_address(127, 0, 0, 1);
            participant_attr_.rtps.builtin.initialPeersList.push_back(loopback_locator);
            return *this;
        }

        PubSubReader& property_policy(const eprosima::fastrtps::rtps::PropertyPolicy property_policy)
        {
            participant_attr_.rtps.properties = property_policy;
            return *this;
        }

        PubSubReader& entity_property_policy(const eprosima::fastrtps::rtps::PropertyPolicy property_policy)
        {
            subscriber_attr_.properties = property_policy;
            return *this;
        }

        PubSubReader& partition(std::string partition)
        {
            subscriber_attr_.qos.m_partition.push_back(partition.c_str());
            return *this;
        }

        PubSubReader& userData(std::vector<octet> user_data)
        {
            participant_attr_.rtps.userData = user_data;
            return *this;
        }

        PubSubReader& lease_duration(Duration_t lease_duration, Duration_t announce_period)
        {
            participant_attr_.rtps.builtin.leaseDuration = lease_duration;
            participant_attr_.rtps.builtin.leaseDuration_announcementperiod = announce_period;
            return *this;
        }

        /*** Function for discovery callback ***/

        bool getDiscoveryResult(){
            return discovery_result_;
        }

        void setOnDiscoveryFunction(std::function<bool(const ParticipantDiscoveryInfo&)> f){
            onDiscovery_ = f;
        }

    private:

        void receive_one(eprosima::fastrtps::Subscriber* subscriber, bool& returnedValue)
        {
            returnedValue = false;

            if(receiving_.load())
            {
                type data;
                SampleInfo_t info;

                if(subscriber->takeNextData((void*)&data, &info))
                {
                    returnedValue = true;

                    std::unique_lock<std::mutex> lock(mutex_);

                    // Check order of changes.
                    ASSERT_LT(last_seq, info.sample_identity.sequence_number());
                    last_seq = info.sample_identity.sequence_number();

                    if(info.sampleKind == ALIVE)
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
        }

        void participant_matched()
        {
            std::unique_lock<std::mutex> lock(mutexDiscovery_);
            ++participant_matched_;
            cvDiscovery_.notify_one();
        }

        void participant_unmatched()
        {
            std::unique_lock<std::mutex> lock(mutexDiscovery_);
            --participant_matched_;
            cvDiscovery_.notify_one();
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

        PubSubReader& operator=(const PubSubReader&)= delete;

        eprosima::fastrtps::Participant *participant_;
        eprosima::fastrtps::ParticipantAttributes participant_attr_;
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
        unsigned int participant_matched_;
        std::atomic<bool> receiving_;
        type_support type_;
        SequenceNumber_t last_seq;
        size_t current_received_count_;
        size_t number_samples_expected_;
        bool discovery_result_;

        std::function<bool(const ParticipantDiscoveryInfo& info)> onDiscovery_;

#if HAVE_SECURITY
        std::mutex mutexAuthentication_;
        std::condition_variable cvAuthentication_;
        unsigned int authorized_;
        unsigned int unauthorized_;
#endif
};

#endif // _TEST_BLACKBOX_PUBSUBREADER_HPP_
