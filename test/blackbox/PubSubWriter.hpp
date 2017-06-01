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
 * @file PubSubWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBWRITER_HPP_
#define _TEST_BLACKBOX_PUBSUBWRITER_HPP_

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <string>
#include <list>
#include <map>
#include <condition_variable>
#include <asio.hpp>
#include <gtest/gtest.h>

template<class TypeSupport>
class PubSubWriter
{
    class ParticipantListener : public eprosima::fastrtps::ParticipantListener
    {
        public:

            ParticipantListener(PubSubWriter &writer) : writer_(writer) {}

            ~ParticipantListener() {}

#if HAVE_SECURITY
            void onParticipantAuthentication(Participant*, const ParticipantAuthenticationInfo& info)
            {
                if(info.rtps.status() == AUTHORIZED_RTPSPARTICIPANT)
                    writer_.authorized();
                else if(info.rtps.status() == UNAUTHORIZED_RTPSPARTICIPANT)
                    writer_.unauthorized();
            }
#endif

        private:

            ParticipantListener& operator=(const ParticipantListener&) NON_COPYABLE_CXX11;

            PubSubWriter& writer_;
    } participant_listener_;

    class Listener : public eprosima::fastrtps::PublisherListener
    {
        public:

            Listener(PubSubWriter &writer) : writer_(writer){};

            ~Listener(){};

            void onPublicationMatched(eprosima::fastrtps::Publisher* /*pub*/, MatchingInfo &info)
            {
                if (info.status == MATCHED_MATCHING)
                    writer_.matched();
                else
                    writer_.unmatched();
            }

        private:

            Listener& operator=(const Listener&) NON_COPYABLE_CXX11;

            PubSubWriter &writer_;

    } listener_;

    class EDPTakeReaderInfo: public eprosima::fastrtps::rtps::ReaderListener
    {
        public:

            EDPTakeReaderInfo(PubSubWriter &writer) : writer_(writer){};

            ~EDPTakeReaderInfo(){};

            void onNewCacheChangeAdded(RTPSReader* /*reader*/, const CacheChange_t* const change_in)
            {
                ReaderProxyData readerInfo;

                if(change_in->kind == ALIVE)
                {
                    CDRMessage_t tempMsg(0);
                    tempMsg.wraps = true;
                    tempMsg.msg_endian = change_in->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
                    tempMsg.length = change_in->serializedPayload.length;
                    tempMsg.max_size = change_in->serializedPayload.max_size;
                    tempMsg.buffer = change_in->serializedPayload.data;

                    if(readerInfo.readFromCDRMessage(&tempMsg))
                    {
                        std::cout << "Discovered reader of topic " << readerInfo.topicName() << std::endl;
                        writer_.add_reader_info(readerInfo);
                    }
                }
                else
                {
                    // Search info of entity
                    GUID_t readerGuid;
                    iHandle2GUID(readerGuid, change_in->instanceHandle);

                    if(writer_.participant_->get_remote_reader_info(readerGuid, readerInfo))
                        writer_.remove_reader_info(readerInfo);
                }
            }

        private:

            EDPTakeReaderInfo& operator=(const EDPTakeReaderInfo&) NON_COPYABLE_CXX11;

            PubSubWriter &writer_;
    };

    class EDPTakeWriterInfo: public eprosima::fastrtps::rtps::ReaderListener
    {
        public:

            EDPTakeWriterInfo(PubSubWriter &writer) : writer_(writer){};

            ~EDPTakeWriterInfo(){};

            void onNewCacheChangeAdded(RTPSReader* /*reader*/, const CacheChange_t* const change_in)
            {
                WriterProxyData writerInfo;

                if(change_in->kind == ALIVE)
                {
                    CDRMessage_t tempMsg(0);
                    tempMsg.wraps = true;
                    tempMsg.msg_endian = change_in->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
                    tempMsg.length = change_in->serializedPayload.length;
                    tempMsg.max_size = change_in->serializedPayload.max_size;
                    tempMsg.buffer = change_in->serializedPayload.data;

                    if(writerInfo.readFromCDRMessage(&tempMsg))
                    {
                        std::cout << "Discovered writer of topic " << writerInfo.topicName() << std::endl;
                        writer_.add_writer_info(writerInfo);
                    }
                }
                else
                {
                    // Search info of entity
                    GUID_t writerGuid;
                    iHandle2GUID(writerGuid, change_in->instanceHandle);

                    if(writer_.participant_->get_remote_writer_info(writerGuid, writerInfo))
                        writer_.remove_writer_info(writerInfo);
                }
            }

        private:

            EDPTakeWriterInfo& operator=(const EDPTakeWriterInfo&) NON_COPYABLE_CXX11;

            PubSubWriter &writer_;
    };

    public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

    PubSubWriter(const std::string &topic_name) : participant_listener_(*this), listener_(*this), participant_(nullptr),
    publisher_(nullptr), initialized_(false), matched_(0),
    attachEDP_(false), edpReaderListener_(*this), edpWriterListener_(*this)
#if HAVE_SECURITY
    , authorized_(0), unauthorized_(0)
#endif
    {
        publisher_attr_.topic.topicDataType = type_.getName();
        // Generate topic name
        std::ostringstream t;
        t << topic_name << "_" << asio::ip::host_name() << "_" << GET_PID();
        publisher_attr_.topic.topicName = t.str();
        topic_name_ = t.str();

#if defined(PREALLOCATED_WITH_REALLOC_MEMORY_MODE_TEST)
        publisher_attr_.historyMemoryPolicy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
#elif defined(DYNAMIC_RESERVE_MEMORY_MODE_TEST)
        publisher_attr_.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
#else
        publisher_attr_.historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
#endif

        // By default, heartbeat period and nack response delay are 100 milliseconds.
        publisher_attr_.times.heartbeatPeriod.seconds = 0;
        publisher_attr_.times.heartbeatPeriod.fraction = 4294967 * 100;
        publisher_attr_.times.nackResponseDelay.seconds = 0;
        publisher_attr_.times.nackResponseDelay.fraction = 4294967 * 100;
    }

    ~PubSubWriter()
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
            if(attachEDP_)
            {
                std::pair<StatefulReader*, StatefulReader*> edpReaders = participant_->getEDPReaders();
                edpReaders.first->setListener(&edpReaderListener_);
                edpReaders.second->setListener(&edpWriterListener_);
            }

            // Register type
            eprosima::fastrtps::Domain::registerType(participant_, &type_);

            //Create publisher
            publisher_ = eprosima::fastrtps::Domain::createPublisher(participant_, publisher_attr_, &listener_);

            if(publisher_ != nullptr)
            {
                initialized_ = true;
                return;
            }

            eprosima::fastrtps::Domain::removeParticipant(participant_);
        }
    }

    bool isInitialized() const { return initialized_; }

    eprosima::fastrtps::Participant* getParticipant()
    {
        return participant_;
    }

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

    void waitDiscovery()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        std::cout << "Writer is waiting discovery..." << std::endl;

        if(matched_ == 0)
            cv_.wait(lock);

        ASSERT_NE(matched_, 0u);
        std::cout << "Writer discovery finished..." << std::endl;
    }

    void waitRemoval()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        std::cout << "Writer is waiting removal..." << std::endl;

        if(matched_ != 0)
            cv_.wait(lock);

        ASSERT_EQ(matched_, 0u);
        std::cout << "Writer removal finished..." << std::endl;
    }

#if HAVE_SECURITY
    void waitAuthorized(unsigned int how_many = 1)
    {
        std::unique_lock<std::mutex> lock(mutexAuthentication_);

        std::cout << "Writer is waiting authorization..." << std::endl;

        while(authorized_ != how_many)
            cvAuthentication_.wait(lock);

        ASSERT_EQ(authorized_, how_many);
        std::cout << "Writer authorization finished..." << std::endl;
    }

    void waitUnauthorized(unsigned int how_many = 1)
    {
        std::unique_lock<std::mutex> lock(mutexAuthentication_);

        std::cout << "Writer is waiting unauthorization..." << std::endl;

        while(unauthorized_ != how_many)
            cvAuthentication_.wait(lock);

        ASSERT_EQ(unauthorized_, how_many);
        std::cout << "Writer unauthorization finished..." << std::endl;
    }
#endif

    template<class _Rep,
        class _Period
            >
            bool waitForAllAcked(const std::chrono::duration<_Rep, _Period>& max_wait)
            {
                return publisher_->wait_for_all_acked(Time_t((int32_t)max_wait.count(), 0));
            }

    void block_until_discover_topic(const std::string& topicName, int repeatedTimes)
    {
        std::unique_lock<std::mutex> lock(mutexEntitiesInfoList_);

        int times = mapTopicCountList_.count(topicName) == 0 ? -1 : mapTopicCountList_[topicName];

        while(times != repeatedTimes)
        {
            cvEntitiesInfoList_.wait(lock);
            times = mapTopicCountList_.count(topicName) == 0 ? -1 : mapTopicCountList_[topicName];
        }
    }

    void block_until_discover_partition(const std::string& partition, int repeatedTimes)
    {
        std::unique_lock<std::mutex> lock(mutexEntitiesInfoList_);

        int times = mapPartitionCountList_.count(partition) == 0 ? -1 : mapPartitionCountList_[partition];

        while(times != repeatedTimes)
        {
            cvEntitiesInfoList_.wait(lock);
            times = mapPartitionCountList_.count(partition) == 0 ? -1 : mapPartitionCountList_[partition];
        }
    }

    /*** Function to change QoS ***/
    PubSubWriter& reliability(const eprosima::fastrtps::ReliabilityQosPolicyKind kind)
    {
        publisher_attr_.qos.m_reliability.kind = kind;
        return *this;
    }

    PubSubWriter& add_throughput_controller_descriptor_to_pparams(uint32_t bytesPerPeriod, uint32_t periodInMs)
    {
        ThroughputControllerDescriptor descriptor {bytesPerPeriod, periodInMs};
        publisher_attr_.throughputController = descriptor;

        return *this;
    }

    PubSubWriter& asynchronously(const eprosima::fastrtps::PublishModeQosPolicyKind kind)
    {
        publisher_attr_.qos.m_publishMode.kind = kind;
        return *this;
    }

    PubSubWriter& history_kind(const eprosima::fastrtps::HistoryQosPolicyKind kind)
    {
        publisher_attr_.topic.historyQos.kind = kind;
        return *this;
    }

    PubSubWriter& history_depth(const int32_t depth)
    {
        publisher_attr_.topic.historyQos.depth = depth;
        return *this;
    }

    PubSubWriter& disable_builtin_transport()
    {
        participant_attr_.rtps.useBuiltinTransports = false;
        return *this;
    }

    PubSubWriter& add_user_transport_to_pparams(std::shared_ptr<TransportDescriptorInterface> userTransportDescriptor)
    {
        participant_attr_.rtps.userTransports.push_back(userTransportDescriptor);
        return *this;
    }

    PubSubWriter& durability_kind(const eprosima::fastrtps::DurabilityQosPolicyKind kind)
    {
        publisher_attr_.qos.m_durability.kind = kind;
        return *this;
    }

    PubSubWriter& resource_limits_allocated_samples(const int32_t initial)
    {
        publisher_attr_.topic.resourceLimitsQos.allocated_samples = initial;
        return *this;
    }

    PubSubWriter& resource_limits_max_samples(const int32_t max)
    {
        publisher_attr_.topic.resourceLimitsQos.max_samples = max;
        return *this;
    }

    PubSubWriter& heartbeat_period_seconds(int32_t sec)
    {
        publisher_attr_.times.heartbeatPeriod.seconds = sec;
        return *this;
    }

    PubSubWriter& heartbeat_period_fraction(uint32_t frac)
    {
        publisher_attr_.times.heartbeatPeriod.fraction = frac;
        return *this;
    }

    PubSubWriter& unicastLocatorList(LocatorList_t unicastLocators)
    {
        publisher_attr_.unicastLocatorList = unicastLocators;
        return *this;
    }

    PubSubWriter& multicastLocatorList(LocatorList_t multicastLocators)
    {
        publisher_attr_.multicastLocatorList = multicastLocators;
        return *this;
    }

    PubSubWriter& outLocatorList(LocatorList_t outLocators)
    {
        publisher_attr_.outLocatorList = outLocators;
        return *this;
    }

    PubSubWriter& static_discovery(const char* filename)
    {
        participant_attr_.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = false;
        participant_attr_.rtps.builtin.use_STATIC_EndpointDiscoveryProtocol = true;
        participant_attr_.rtps.builtin.setStaticEndpointXMLFilename(filename);
        return *this;
    }

    PubSubWriter& property_policy(const eprosima::fastrtps::rtps::PropertyPolicy property_policy)
    {
        participant_attr_.rtps.properties = property_policy;
        return *this;
    }

    PubSubWriter& entity_property_policy(const eprosima::fastrtps::rtps::PropertyPolicy property_policy)
    {
        publisher_attr_.properties = property_policy;
        return *this;
    }

    PubSubWriter& setPublisherIDs(uint8_t UserID, uint8_t EntityID)
    {
        publisher_attr_.setUserDefinedID(UserID);
        publisher_attr_.setEntityID(EntityID);
        return *this;
    }

    PubSubWriter& setManualTopicName(std::string topicName)
    {
        publisher_attr_.topic.topicName=topicName;
        return *this;
    }

    PubSubWriter& disable_multicast(int32_t participantId)
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

    PubSubWriter& partition(std::string partition)
    {
        publisher_attr_.qos.m_partition.push_back(partition.c_str());
        return *this;
    }

    PubSubWriter& userData(std::vector<octet> user_data)
    {
        participant_attr_.rtps.userData = user_data;
        return *this;
    }

    PubSubWriter& attach_edp_listeners()
    {
        attachEDP_ = true;
        return *this;
    }

    const std::string& topic_name() const { return topic_name_; }

    private:

    void matched()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);
        ++matched_;
        cv_.notify_one();
    }

    void unmatched()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);
        --matched_;
        cv_.notify_one();
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

    void add_writer_info(const WriterProxyData& writer_data)
    {
        mutexEntitiesInfoList_.lock();
        auto ret = mapWriterInfoList_.insert(std::make_pair(writer_data.guid(), writer_data));

        if(!ret.second)
            ret.first->second = writer_data;

        auto ret_topic = mapTopicCountList_.insert(std::make_pair(writer_data.topicName(), 1));

        if(!ret_topic.second)
            ++ret_topic.first->second;

        for(auto partition : writer_data.m_qos.m_partition.getNames())
        {
            auto ret_partition = mapPartitionCountList_.insert(std::make_pair(partition, 1));

            if(!ret_partition.second)
                ++ret_partition.first->second;
        }

        mutexEntitiesInfoList_.unlock();
        cvEntitiesInfoList_.notify_all();
    }

    void add_reader_info(const ReaderProxyData& reader_data)
    {
        mutexEntitiesInfoList_.lock();
        auto ret = mapReaderInfoList_.insert(std::make_pair(reader_data.guid(), reader_data));

        if(!ret.second)
            ret.first->second = reader_data;

        auto ret_topic = mapTopicCountList_.insert(std::make_pair(reader_data.topicName(), 1));

        if(!ret_topic.second)
            ++ret_topic.first->second;

        for(auto partition : reader_data.m_qos.m_partition.getNames())
        {
            auto ret_partition = mapPartitionCountList_.insert(std::make_pair(partition, 1));

            if(!ret_partition.second)
                ++ret_partition.first->second;
        }

        mutexEntitiesInfoList_.unlock();
        cvEntitiesInfoList_.notify_all();
    }

    void remove_writer_info(const WriterProxyData& writer_data)
    {
        std::unique_lock<std::mutex> lock(mutexEntitiesInfoList_);

        ASSERT_GT(mapWriterInfoList_.count(writer_data.guid()), 0);

        mapWriterInfoList_.erase(writer_data.guid());

        ASSERT_GT(mapTopicCountList_.count(writer_data.topicName()), 0);

        --mapTopicCountList_[writer_data.topicName()];

        for(auto partition : writer_data.m_qos.m_partition.getNames())
        {
            ASSERT_GT(mapPartitionCountList_.count(partition), 0);

            --mapPartitionCountList_[partition];
        }

        lock.unlock();
        cvEntitiesInfoList_.notify_all();
    }

    void remove_reader_info(const ReaderProxyData& reader_data)
    {
        std::unique_lock<std::mutex> lock(mutexEntitiesInfoList_);

        ASSERT_GT(mapReaderInfoList_.count(reader_data.guid()), 0);

        mapReaderInfoList_.erase(reader_data.guid());

        ASSERT_GT(mapTopicCountList_.count(reader_data.topicName()), 0);

        --mapTopicCountList_[reader_data.topicName()];

        for(auto partition : reader_data.m_qos.m_partition.getNames())
        {
            ASSERT_GT(mapPartitionCountList_.count(partition), 0);

            --mapPartitionCountList_[partition];
        }

        lock.unlock();
        cvEntitiesInfoList_.notify_all();
    }

    PubSubWriter& operator=(const PubSubWriter&)NON_COPYABLE_CXX11;

    eprosima::fastrtps::Participant *participant_;
    eprosima::fastrtps::ParticipantAttributes participant_attr_;
    eprosima::fastrtps::Publisher *publisher_;
    eprosima::fastrtps::PublisherAttributes publisher_attr_;
    std::string topic_name_;
    bool initialized_;
    std::mutex mutexDiscovery_;
    std::condition_variable cv_;
    unsigned int matched_;
    type_support type_;
    bool attachEDP_;
    EDPTakeReaderInfo edpReaderListener_;
    EDPTakeWriterInfo edpWriterListener_;
    std::mutex mutexEntitiesInfoList_;
    std::condition_variable cvEntitiesInfoList_;
    std::map<GUID_t, WriterProxyData> mapWriterInfoList_;
    std::map<GUID_t, ReaderProxyData> mapReaderInfoList_;
    std::map<std::string,  int> mapTopicCountList_;
    std::map<std::string,  int> mapPartitionCountList_;
#if HAVE_SECURITY
    std::mutex mutexAuthentication_;
    std::condition_variable cvAuthentication_;
    unsigned int authorized_;
    unsigned int unauthorized_;
#endif
};

#endif // _TEST_BLACKBOX_PUBSUBWRITER_HPP_
