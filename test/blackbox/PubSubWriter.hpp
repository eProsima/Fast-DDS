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
#include <fastrtps/xmlparser/XMLParser.h>
#include <fastrtps/xmlparser/XMLTree.h>
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

            void onParticipantDiscovery(eprosima::fastrtps::Participant*, eprosima::fastrtps::ParticipantDiscoveryInfo info)
            {
                if(writer_.onDiscovery_!=nullptr)
                {
                    writer_.discovery_result_ = writer_.onDiscovery_(info);

                }

                if(info.rtps.m_status == eprosima::fastrtps::rtps::DISCOVERED_RTPSPARTICIPANT)
                {
                    writer_.participant_matched();
                }
                else if(info.rtps.m_status == eprosima::fastrtps::rtps::REMOVED_RTPSPARTICIPANT ||
                        info.rtps.m_status == eprosima::fastrtps::rtps::DROPPED_RTPSPARTICIPANT)
                {
                    writer_.participant_unmatched();
                }
            }

#if HAVE_SECURITY
            void onParticipantAuthentication(eprosima::fastrtps::Participant*, const eprosima::fastrtps::ParticipantAuthenticationInfo& info)
            {
                if(info.rtps.status() == eprosima::fastrtps::rtps::AUTHORIZED_RTPSPARTICIPANT)
                    writer_.authorized();
                else if(info.rtps.status() == eprosima::fastrtps::rtps::UNAUTHORIZED_RTPSPARTICIPANT)
                    writer_.unauthorized();
            }
#endif

        private:

            ParticipantListener& operator=(const ParticipantListener&) = delete;

            PubSubWriter& writer_;
    } participant_listener_;

    class Listener : public eprosima::fastrtps::PublisherListener
    {
        public:

            Listener(PubSubWriter &writer) : writer_(writer){};

            ~Listener(){};

            void onPublicationMatched(eprosima::fastrtps::Publisher* /*pub*/, eprosima::fastrtps::rtps::MatchingInfo &info)
            {
                if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
                {
                    std::cout << "Matched subscriber " << info.remoteEndpointGuid << std::endl;
                    writer_.matched();
                }
                else
                {
                    std::cout << "Unmatched subscriber " << info.remoteEndpointGuid << std::endl;
                    writer_.unmatched();
                }
            }

        private:

            Listener& operator=(const Listener&) = delete;

            PubSubWriter &writer_;

    } listener_;

    class EDPTakeReaderInfo: public eprosima::fastrtps::rtps::ReaderListener
    {
        public:

            EDPTakeReaderInfo(PubSubWriter &writer) : writer_(writer){};

            ~EDPTakeReaderInfo(){};

            void onNewCacheChangeAdded(eprosima::fastrtps::rtps::RTPSReader* /*reader*/, const eprosima::fastrtps::rtps::CacheChange_t* const change_in)
            {
                eprosima::fastrtps::rtps::ReaderProxyData readerInfo;

                if(change_in->kind == eprosima::fastrtps::rtps::ALIVE)
                {
                    eprosima::fastrtps::rtps::CDRMessage_t tempMsg(0);
                    tempMsg.wraps = true;
                    tempMsg.msg_endian = change_in->serializedPayload.encapsulation == PL_CDR_BE ? eprosima::fastrtps::rtps::BIGEND : eprosima::fastrtps::rtps::LITTLEEND;
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
                    eprosima::fastrtps::rtps::GUID_t readerGuid;
                    iHandle2GUID(readerGuid, change_in->instanceHandle);

                    if(writer_.participant_->get_remote_reader_info(readerGuid, readerInfo))
                    {
                        std::cout << "Undiscovered reader of topic " << readerInfo.topicName() << std::endl;
                        writer_.remove_reader_info(readerInfo);
                    }
                    else
                    {
                        std::cout << "Error getting remote reader info for topic " << readerInfo.topicName()
                            << std::endl;
                    }
                }
            }

        private:

            EDPTakeReaderInfo& operator=(const EDPTakeReaderInfo&) = delete;

            PubSubWriter &writer_;
    };

    class EDPTakeWriterInfo: public eprosima::fastrtps::rtps::ReaderListener
    {
        public:

            EDPTakeWriterInfo(PubSubWriter &writer) : writer_(writer){};

            ~EDPTakeWriterInfo(){};

            void onNewCacheChangeAdded(eprosima::fastrtps::rtps::RTPSReader* /*reader*/, const eprosima::fastrtps::rtps::CacheChange_t* const change_in)
            {
                eprosima::fastrtps::rtps::WriterProxyData writerInfo;

                if(change_in->kind == eprosima::fastrtps::rtps::ALIVE)
                {
                    eprosima::fastrtps::rtps::CDRMessage_t tempMsg(0);
                    tempMsg.wraps = true;
                    tempMsg.msg_endian = change_in->serializedPayload.encapsulation == PL_CDR_BE ? eprosima::fastrtps::rtps::BIGEND : eprosima::fastrtps::rtps::LITTLEEND;
                    tempMsg.length = change_in->serializedPayload.length;
                    tempMsg.max_size = change_in->serializedPayload.max_size;
                    tempMsg.buffer = change_in->serializedPayload.data;

                    if(writerInfo.readFromCDRMessage(&tempMsg))
                    {
                        std::cout << "Discovered writer of topic " << writerInfo.topicName() << std::endl;
                        writer_.add_writer_info(writerInfo);
                    }
                    else
                    {
                        std::cout << "Error reading cdr message for topic " << writerInfo.topicName() << std::endl;
                    }
                }
                else
                {
                    // Search info of entity
                    eprosima::fastrtps::rtps::GUID_t writerGuid;
                    iHandle2GUID(writerGuid, change_in->instanceHandle);

                    if(writer_.participant_->get_remote_writer_info(writerGuid, writerInfo))
                    {
                        std::cout << "Undiscovered writer of topic " << writerInfo.topicName() << std::endl;
                        writer_.remove_writer_info(writerInfo);
                    }
                    else
                    {
                        std::cout << "Error getting remote writer info for topic " << writerInfo.topicName()
                            << std::endl;
                    }
                }
            }

        private:

            EDPTakeWriterInfo& operator=(const EDPTakeWriterInfo&) = delete;

            PubSubWriter &writer_;
    };

    public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

    PubSubWriter(const std::string &topic_name) : participant_listener_(*this), listener_(*this),
    participant_(nullptr), publisher_(nullptr), initialized_(false), matched_(0),
    participant_matched_(0), attachEDP_(false), edpReaderListener_(*this), edpWriterListener_(*this),
    discovery_result_(false), onDiscovery_(nullptr)
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
        publisher_attr_.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
#elif defined(DYNAMIC_RESERVE_MEMORY_MODE_TEST)
        publisher_attr_.historyMemoryPolicy = eprosima::fastrtps::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
#else
        publisher_attr_.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_MEMORY_MODE;
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
            participant_guid_ = participant_->getGuid();

            if(attachEDP_)
            {
                std::pair<eprosima::fastrtps::rtps::StatefulReader*, eprosima::fastrtps::rtps::StatefulReader*> edpReaders = participant_->getEDPReaders();
                edpReaders.first->setListener(&edpReaderListener_);
                edpReaders.second->setListener(&edpWriterListener_);
            }

            // Register type
            eprosima::fastrtps::Domain::registerType(participant_, &type_);

            //Create publisher
            publisher_ = eprosima::fastrtps::Domain::createPublisher(participant_, publisher_attr_, &listener_);

            if(publisher_ != nullptr)
            {
                std::cout << "Created publisher " << publisher_->getGuid() << " for topic " <<
                    publisher_attr_.topic.topicName << std::endl;
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

    void send(std::list<type>& msgs, uint32_t milliseconds = 0)
    {
        auto it = msgs.begin();

        while(it != msgs.end())
        {
            if(publisher_->write((void*)&(*it)))
            {
                default_send_print<type>(*it);
                it = msgs.erase(it);
                if(milliseconds > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
            }
            else
                break;
        }
    }

    bool send_sample(type& msg)
    {
        return publisher_->write((void*)&msg);
    }

    void waitDiscovery()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        std::cout << "Writer is waiting discovery..." << std::endl;

        cv_.wait(lock, [&](){return matched_ != 0;});

        std::cout << "Writer discovery finished..." << std::endl;
    }

    void wait_participant_undiscovery()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        std::cout << "Writer is waiting undiscovery..." << std::endl;

        cv_.wait(lock, [&](){return participant_matched_ == 0;});

        std::cout << "Writer undiscovery finished..." << std::endl;
    }

    void wait_reader_undiscovery()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);

        std::cout << "Writer is waiting removal..." << std::endl;

        cv_.wait(lock, [&](){return matched_ == 0;});

        std::cout << "Writer removal finished..." << std::endl;
    }

#if HAVE_SECURITY
    void waitAuthorized()
    {
        std::unique_lock<std::mutex> lock(mutexAuthentication_);

        std::cout << "Writer is waiting authorization..." << std::endl;

        cvAuthentication_.wait(lock, [&]() -> bool { return authorized_ > 0; });

        std::cout << "Writer authorization finished..." << std::endl;
    }

    void waitUnauthorized()
    {
        std::unique_lock<std::mutex> lock(mutexAuthentication_);

        std::cout << "Writer is waiting unauthorization..." << std::endl;

        cvAuthentication_.wait(lock, [&]() -> bool { return unauthorized_ > 0; });

        std::cout << "Writer unauthorization finished..." << std::endl;
    }
#endif

    template<class _Rep,
        class _Period
            >
            bool waitForAllAcked(const std::chrono::duration<_Rep, _Period>& max_wait)
            {
                return publisher_->wait_for_all_acked(eprosima::fastrtps::rtps::Time_t((int32_t)max_wait.count(), 0));
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

    PubSubWriter& max_blocking_time(const eprosima::fastrtps::rtps::Duration_t time)
    {
        publisher_attr_.qos.m_reliability.max_blocking_time = time;
        return *this;
    }

    PubSubWriter& add_throughput_controller_descriptor_to_pparams(uint32_t bytesPerPeriod, uint32_t periodInMs)
    {
        eprosima::fastrtps::rtps::ThroughputControllerDescriptor descriptor {bytesPerPeriod, periodInMs};
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

    PubSubWriter& add_user_transport_to_pparams(std::shared_ptr<eprosima::fastrtps::rtps::TransportDescriptorInterface> userTransportDescriptor)
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

    PubSubWriter& unicastLocatorList(eprosima::fastrtps::rtps::LocatorList_t unicastLocators)
    {
        publisher_attr_.unicastLocatorList = unicastLocators;
        return *this;
    }

    PubSubWriter& multicastLocatorList(eprosima::fastrtps::rtps::LocatorList_t multicastLocators)
    {
        publisher_attr_.multicastLocatorList = multicastLocators;
        return *this;
    }

    PubSubWriter& metatraffic_unicast_locator_list(eprosima::fastrtps::rtps::LocatorList_t unicastLocators)
    {
        participant_attr_.rtps.builtin.metatrafficUnicastLocatorList = unicastLocators;
        return *this;
    }

    PubSubWriter& initial_peers(eprosima::fastrtps::rtps::LocatorList_t initial_peers)
    {
        participant_attr_.rtps.builtin.initialPeersList = initial_peers;
        return *this;
    }

    PubSubWriter& outLocatorList(eprosima::fastrtps::rtps::LocatorList_t outLocators)
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

        eprosima::fastrtps::rtps::LocatorList_t default_unicast_locators;
        eprosima::fastrtps::rtps::Locator_t default_unicast_locator;

        default_unicast_locators.push_back(default_unicast_locator);
        participant_attr_.rtps.builtin.metatrafficUnicastLocatorList = default_unicast_locators;

        eprosima::fastrtps::rtps::Locator_t loopback_locator;
        loopback_locator.set_IP4_address(127, 0, 0, 1);
        participant_attr_.rtps.builtin.initialPeersList.push_back(loopback_locator);
        return *this;
    }

    PubSubWriter& partition(std::string partition)
    {
        publisher_attr_.qos.m_partition.push_back(partition.c_str());
        return *this;
    }

    PubSubWriter& userData(std::vector<eprosima::fastrtps::rtps::octet> user_data)
    {
        participant_attr_.rtps.userData = user_data;
        return *this;
    }

    PubSubWriter& attach_edp_listeners()
    {
        attachEDP_ = true;
        return *this;
    }

    PubSubWriter& lease_duration(eprosima::fastrtps::rtps::Duration_t lease_duration, eprosima::fastrtps::rtps::Duration_t announce_period)
    {
        participant_attr_.rtps.builtin.leaseDuration = lease_duration;
        participant_attr_.rtps.builtin.leaseDuration_announcementperiod = announce_period;
        return *this;
    }

    PubSubWriter& load_participant_attr(const std::string& xml)
    {
        std::unique_ptr<eprosima::fastrtps::xmlparser::BaseNode> root;
        if (eprosima::fastrtps::xmlparser::XMLParser::loadXML(xml.data(), xml.size(), root) == eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK)
        {
            for (const auto& profile : root->getChildren())
            {
                if (profile->getType() == eprosima::fastrtps::xmlparser::NodeType::PARTICIPANT)
                {
                    participant_attr_ = *(dynamic_cast<eprosima::fastrtps::xmlparser::DataNode<eprosima::fastrtps::ParticipantAttributes>*>(profile.get())->get());
                }
            }
        }
        return *this;
    }

    PubSubWriter& load_publisher_attr(const std::string& xml)
    {
        std::unique_ptr<eprosima::fastrtps::xmlparser::BaseNode> root;
        if (eprosima::fastrtps::xmlparser::XMLParser::loadXML(xml.data(), xml.size(), root) == eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK)
        {
            for (const auto& profile : root->getChildren())
            {
                if (profile->getType() == eprosima::fastrtps::xmlparser::NodeType::PUBLISHER)
                {
                    publisher_attr_ = *(dynamic_cast<eprosima::fastrtps::xmlparser::DataNode<eprosima::fastrtps::PublisherAttributes>*>(profile.get())->get());
                }
            }
        }
        return *this;
    }

    const std::string& topic_name() const { return topic_name_; }

    eprosima::fastrtps::rtps::GUID_t participant_guid()
    {
        return participant_guid_;
    }

    bool remove_all_changes(size_t* number_of_changes_removed)
    {
        return publisher_->removeAllChange(number_of_changes_removed);
    }

    private:

    void participant_matched()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);
        ++participant_matched_;
        cv_.notify_one();
    }

    void participant_unmatched()
    {
        std::unique_lock<std::mutex> lock(mutexDiscovery_);
        --participant_matched_;
        cv_.notify_one();
    }

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

    void add_writer_info(const eprosima::fastrtps::rtps::WriterProxyData& writer_data)
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

    void add_reader_info(const eprosima::fastrtps::rtps::ReaderProxyData& reader_data)
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

    void remove_writer_info(const eprosima::fastrtps::rtps::WriterProxyData& writer_data)
    {
        std::unique_lock<std::mutex> lock(mutexEntitiesInfoList_);

        ASSERT_GT(mapWriterInfoList_.count(writer_data.guid()), 0ul);

        mapWriterInfoList_.erase(writer_data.guid());

        ASSERT_GT(mapTopicCountList_.count(writer_data.topicName()), 0ul);

        --mapTopicCountList_[writer_data.topicName()];

        for(auto partition : writer_data.m_qos.m_partition.getNames())
        {
            ASSERT_GT(mapPartitionCountList_.count(partition), 0ul);

            --mapPartitionCountList_[partition];
        }

        lock.unlock();
        cvEntitiesInfoList_.notify_all();
    }

    void remove_reader_info(const eprosima::fastrtps::rtps::ReaderProxyData& reader_data)
    {
        std::unique_lock<std::mutex> lock(mutexEntitiesInfoList_);

        ASSERT_GT(mapReaderInfoList_.count(reader_data.guid()), 0ul);

        mapReaderInfoList_.erase(reader_data.guid());

        ASSERT_GT(mapTopicCountList_.count(reader_data.topicName()), 0ul);

        --mapTopicCountList_[reader_data.topicName()];

        for(auto partition : reader_data.m_qos.m_partition.getNames())
        {
            ASSERT_GT(mapPartitionCountList_.count(partition), 0ul);

            --mapPartitionCountList_[partition];
        }

        lock.unlock();
        cvEntitiesInfoList_.notify_all();
    }

    PubSubWriter& operator=(const PubSubWriter&)= delete;

    eprosima::fastrtps::Participant *participant_;
    eprosima::fastrtps::ParticipantAttributes participant_attr_;
    eprosima::fastrtps::Publisher *publisher_;
    eprosima::fastrtps::PublisherAttributes publisher_attr_;
    std::string topic_name_;
    eprosima::fastrtps::rtps::GUID_t participant_guid_;
    bool initialized_;
    std::mutex mutexDiscovery_;
    std::condition_variable cv_;
    unsigned int matched_;
    unsigned int participant_matched_;
    type_support type_;
    bool attachEDP_;
    EDPTakeReaderInfo edpReaderListener_;
    EDPTakeWriterInfo edpWriterListener_;
    std::mutex mutexEntitiesInfoList_;
    std::condition_variable cvEntitiesInfoList_;
    std::map<eprosima::fastrtps::rtps::GUID_t, eprosima::fastrtps::rtps::WriterProxyData> mapWriterInfoList_;
    std::map<eprosima::fastrtps::rtps::GUID_t, eprosima::fastrtps::rtps::ReaderProxyData> mapReaderInfoList_;
    std::map<std::string,  int> mapTopicCountList_;
    std::map<std::string,  int> mapPartitionCountList_;
    bool discovery_result_;

    std::function<bool(const eprosima::fastrtps::ParticipantDiscoveryInfo& info)> onDiscovery_;

#if HAVE_SECURITY
    std::mutex mutexAuthentication_;
    std::condition_variable cvAuthentication_;
    unsigned int authorized_;
    unsigned int unauthorized_;
#endif
};

#endif // _TEST_BLACKBOX_PUBSUBWRITER_HPP_
