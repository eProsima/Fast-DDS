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
 * @file RTPSWithRegistrationWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSWITHREGISTRATIONWRITER_HPP_
#define _TEST_BLACKBOX_RTPSWITHREGISTRATIONWRITER_HPP_

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/qos/WriterQos.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/history/WriterHistory.h>

#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include <string>
#include <list>
#include <condition_variable>
#include <gtest/gtest.h>

template<class TypeSupport>
class RTPSWithRegistrationWriter
{
    public:

        typedef TypeSupport type_support;
        typedef typename type_support::type type;

    private:

    class Listener : public eprosima::fastrtps::rtps::WriterListener
    {
        public:

            Listener(RTPSWithRegistrationWriter &writer) : writer_(writer){};

            ~Listener(){};

            void onWriterMatched(eprosima::fastrtps::rtps::RTPSWriter* /*writer*/, eprosima::fastrtps::rtps::MatchingInfo& info)
            {
                if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
                    writer_.matched();
            }

        private:

            Listener& operator=(const Listener&) = delete;

            RTPSWithRegistrationWriter &writer_;

    } listener_;

    public:

    RTPSWithRegistrationWriter(const std::string& topic_name) : listener_(*this), participant_(nullptr),
    writer_(nullptr), history_(nullptr), initialized_(false), matched_(0)
    {
        topic_attr_.topicDataType = type_.getName();
        // Generate topic name
        std::ostringstream t;
        t << topic_name << "_" << asio::ip::host_name() << "_" << GET_PID();
        topic_attr_.topicName = t.str();

        // By default, heartbeat period and nack response delay are 100 milliseconds.
        writer_attr_.times.heartbeatPeriod.seconds = 0;
        writer_attr_.times.heartbeatPeriod.nanosec = 100000000;
        writer_attr_.times.nackResponseDelay.seconds = 0;
        writer_attr_.times.nackResponseDelay.nanosec = 100000000;
    }

    virtual ~RTPSWithRegistrationWriter()
    {
        if(participant_ != nullptr)
            eprosima::fastrtps::rtps::RTPSDomain::removeRTPSParticipant(participant_);
        if(history_ != nullptr)
            delete(history_);
    }

    void init()
    {
        //Create participant
        eprosima::fastrtps::rtps::RTPSParticipantAttributes pattr;
        pattr.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
        pattr.builtin.use_WriterLivelinessProtocol = true;
        pattr.builtin.domainId = (uint32_t)GET_PID() % 230;
        participant_ = eprosima::fastrtps::rtps::RTPSDomain::createParticipant(pattr);
        ASSERT_NE(participant_, nullptr);

        //Create writerhistory
        hattr_.payloadMaxSize = type_.m_typeSize;
        history_ = new eprosima::fastrtps::rtps::WriterHistory(hattr_);

        //Create writer
        writer_ = eprosima::fastrtps::rtps::RTPSDomain::createRTPSWriter(participant_, writer_attr_, history_, &listener_);
        ASSERT_NE(writer_, nullptr);

        ASSERT_EQ(participant_->registerWriter(writer_, topic_attr_, writer_qos_), true);

        initialized_ = true;
    }

    void destroy()
    {
        if (participant_ != nullptr)
            eprosima::fastrtps::rtps::RTPSDomain::removeRTPSParticipant(participant_);
        if (history_ != nullptr)
            delete(history_);

        participant_ = nullptr;
        history_ = nullptr;
        writer_ = nullptr;
        initialized_ = false;
        matched_ = 0;
    }

    bool isInitialized() const { return initialized_; }

    void send(std::list<type>& msgs)
    {
        auto it = msgs.begin();

        while(it != msgs.end())
        {
	    eprosima::fastrtps::rtps::CacheChange_t * ch = writer_->new_change(*it,eprosima::fastrtps::rtps::ALIVE);

	    eprosima::fastcdr::FastBuffer buffer((char*)ch->serializedPayload.data, ch->serializedPayload.max_size);
            eprosima::fastcdr::Cdr cdr(buffer);

            cdr << *it;

            ch->serializedPayload.length = static_cast<uint32_t>(cdr.getSerializedDataLength());

            history_->add_change(ch);
            it = msgs.erase(it);
        }
    }

    void matched()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        ++matched_;
        cv_.notify_one();
    }

    void wait_discovery()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (matched_ == 0)
        {
            cv_.wait(lock, [this]() -> bool {
                return matched_ != 0;
            });
        }

        ASSERT_NE(matched_, 0u);
    }

    /*** Function to change QoS ***/
    RTPSWithRegistrationWriter& memoryMode(const eprosima::fastrtps::rtps::MemoryManagementPolicy_t memoryPolicy)
    {
	hattr_.memoryPolicy = memoryPolicy;
	return *this;
    }


    RTPSWithRegistrationWriter& reliability(const eprosima::fastrtps::rtps::ReliabilityKind_t kind)
    {
        writer_attr_.endpoint.reliabilityKind = kind;

        if(kind == eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT)
                writer_qos_.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
        else
                writer_qos_.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;

        return *this;
    }

    RTPSWithRegistrationWriter& durability(const eprosima::fastrtps::rtps::DurabilityKind_t kind)
    {
        writer_attr_.endpoint.durabilityKind = kind;
        writer_qos_.m_durability.durabilityKind(kind);

        return *this;
    }

    RTPSWithRegistrationWriter& asynchronously(const eprosima::fastrtps::rtps::RTPSWriterPublishMode mode)
    {
        writer_attr_.mode = mode;

        return *this;
    }

    RTPSWithRegistrationWriter& add_throughput_controller_descriptor_to_pparams(uint32_t bytesPerPeriod, uint32_t periodInMs)
    {
        eprosima::fastrtps::rtps::ThroughputControllerDescriptor descriptor {bytesPerPeriod, periodInMs};
        writer_attr_.throughputController = descriptor;

        return *this;
    }

    RTPSWithRegistrationWriter& heartbeat_period_seconds(int32_t sec)
    {
        writer_attr_.times.heartbeatPeriod.seconds = sec;
        return *this;
    }

    RTPSWithRegistrationWriter& heartbeat_period_nanosec(uint32_t nanosec)
    {
        writer_attr_.times.heartbeatPeriod.nanosec = nanosec;
        return *this;
    }

    RTPSWithRegistrationWriter& add_property(const std::string& prop, const std::string& value)
    {
        writer_attr_.endpoint.properties.properties().emplace_back(prop, value);
        return *this;
    }

    RTPSWithRegistrationWriter& make_persistent(const std::string& filename, const eprosima::fastrtps::rtps::GuidPrefix_t& guidPrefix)
    {
        writer_attr_.endpoint.persistence_guid.guidPrefix = guidPrefix;
        writer_attr_.endpoint.persistence_guid.entityId = 0xAAAAAAAA;

        std::cout << "Initializing persistent WRITER " << writer_attr_.endpoint.persistence_guid << " with file " << filename << std::endl;

        return durability(eprosima::fastrtps::rtps::DurabilityKind_t::PERSISTENT)
            .add_property("dds.persistence.plugin", "builtin.SQLITE3")
            .add_property("dds.persistence.sqlite3.filename", filename);
    }

    private:

        RTPSWithRegistrationWriter& operator=(const RTPSWithRegistrationWriter&) = delete;

        eprosima::fastrtps::rtps::RTPSParticipant *participant_;
        eprosima::fastrtps::rtps::RTPSWriter *writer_;
        eprosima::fastrtps::rtps::WriterAttributes writer_attr_;
        eprosima::fastrtps::WriterQos writer_qos_;
        eprosima::fastrtps::TopicAttributes topic_attr_;
        eprosima::fastrtps::rtps::WriterHistory *history_;
        eprosima::fastrtps::rtps::HistoryAttributes hattr_;
        bool initialized_;
        std::mutex mutex_;
        std::condition_variable cv_;
        unsigned int matched_;
        type_support type_;
};

#endif // _TEST_BLACKBOX_RTPSWITHREGISTRATIONWRITER_HPP_
