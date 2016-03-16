/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBWRITER_HPP_
#define _TEST_BLACKBOX_PUBSUBWRITER_HPP_

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include <string>
#include <list>
#include <condition_variable>
#include <boost/asio.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <gtest/gtest.h>

template<class TypeSupport, eprosima::fastrtps::ReliabilityQosPolicyKind Mode>
class PubSubWriter 
{
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

    public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

    PubSubWriter(std::function<void (eprosima::fastrtps::Publisher*, const std::list<uint16_t>&)> sender) : listener_(*this), sender_(sender), participant_(nullptr),
    publisher_(nullptr), initialized_(false), matched_(0)
    {
    }
    
    ~PubSubWriter()
    {
        if(participant_ != nullptr)
            eprosima::fastrtps::Domain::removeParticipant(participant_);
    }

    void init(bool async = false)
    {
        //Create participant
        eprosima::fastrtps::ParticipantAttributes pattr;
        pattr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
        participant_ = eprosima::fastrtps::Domain::createParticipant(pattr);

        if(participant_ != nullptr)
        {
            // Register type
            eprosima::fastrtps::Domain::registerType(participant_, &type_);

            //Create publisher
            eprosima::fastrtps::PublisherAttributes puattr;
            puattr.topic.topicKind = NO_KEY;
            puattr.topic.topicDataType = type_.getName();
            configPublisher(puattr);

            // Asynchronous
            if(async)
                puattr.qos.m_publishMode.kind = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;

            publisher_ = eprosima::fastrtps::Domain::createPublisher(participant_, puattr, &listener_);

            if(publisher_ != nullptr)
            {
                initialized_ = true;
                return;
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

    void send(const std::list<uint16_t> &msgs)
    {
        waitDiscovery();

        sender_(publisher_, msgs);
    }

    void matched()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        ++matched_;
        cv_.notify_one();
    }

    void unmatched()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        --matched_;
        cv_.notify_one();
    }

    void waitDiscovery()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if(matched_ == 0)
            cv_.wait_for(lock, std::chrono::seconds(10));

        ASSERT_NE(matched_, 0u);
    }

    void waitRemoval()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if(matched_ != 0)
            cv_.wait_for(lock, std::chrono::seconds(10));

        ASSERT_EQ(matched_, 0u);
    }

    void configPublisher(eprosima::fastrtps::PublisherAttributes& puattr)
    {
        std:: string reliability_str;

        if(mode_ == eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
        {
            puattr.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
            reliability_str = "AsReliable";
        }
        else
        {
            puattr.qos.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
            reliability_str = "AsNonReliable";
        }

        std::ostringstream t;

        t << "PubSub" << reliability_str << type_.getName() << "_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();

        puattr.topic.topicName = t.str();
    }

    private:

    PubSubWriter& operator=(const PubSubWriter&)NON_COPYABLE_CXX11;

    const eprosima::fastrtps::ReliabilityQosPolicyKind mode_ = Mode;
    std::function<void (eprosima::fastrtps::Publisher*, const std::list<uint16_t>&)> sender_;

    eprosima::fastrtps::Participant *participant_;
    eprosima::fastrtps::Publisher *publisher_;
    bool initialized_;
    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int matched_;
    type_support type_;
};

#endif // _TEST_BLACKBOX_PUBSUBWRITER_HPP_
