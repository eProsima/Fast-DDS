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
 * @file RTPSAsSocketWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSASSOCKETWRITER_HPP_
#define _TEST_BLACKBOX_RTPSASSOCKETWRITER_HPP_

#include <list>
#include <string>

#include <asio.hpp>

#include <gtest/gtest.h>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>

#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

#include <fastdds/utils/IPLocator.hpp>

using eprosima::fastdds::rtps::IPLocator;

template<class TypeSupport>
class RTPSAsSocketWriter : public eprosima::fastdds::rtps::WriterListener
{
public:

    typedef TypeSupport type_support;
    typedef typename type_support::type type;

    RTPSAsSocketWriter(
            const std::string& magicword)
        : participant_(nullptr)
        , writer_(nullptr)
        , history_(nullptr)
        , initialized_(false)
        , auto_remove_(false)
        , port_(0)
    {
        std::ostringstream mw;
        mw << magicword << "_" << asio::ip::host_name() << "_" << GET_PID();
        magicword_ = mw.str();

        // By default, memory mode is PREALLOCATED_WITH_REALLOC_MEMORY_MODE
        hattr_.memoryPolicy = eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

        // By default, heartbeat period and nack response delay are 100 milliseconds.
        writer_attr_.times.heartbeat_period.seconds = 0;
        writer_attr_.times.heartbeat_period.nanosec = 100000000;
        writer_attr_.times.nack_response_delay.seconds = 0;
        writer_attr_.times.nack_response_delay.nanosec = 100000000;
    }

    virtual ~RTPSAsSocketWriter()
    {
        if (participant_ != nullptr)
        {
            eprosima::fastdds::rtps::RTPSDomain::removeRTPSParticipant(participant_);
        }
        if (history_ != nullptr)
        {
            delete(history_);
        }
    }

    void on_writer_change_received_by_all(
            eprosima::fastdds::rtps::RTPSWriter* /*writer*/,
            eprosima::fastdds::rtps::CacheChange_t* change) override
    {
        if (writer_attr_.endpoint.durabilityKind == eprosima::fastdds::rtps::VOLATILE)
        {
            history_->remove_change_g(change);
            std::cout << "Change removed" << std::endl;
        }
    }

    void init()
    {
        //Create participant
        participant_attr_.builtin.discovery_config.discoveryProtocol =
                eprosima::fastdds::rtps::DiscoveryProtocol::NONE;
        participant_attr_.builtin.use_WriterLivelinessProtocol = false;
        participant_attr_.participantID = 2;
        participant_ = eprosima::fastdds::rtps::RTPSDomain::createParticipant(
            (uint32_t)GET_PID() % 230, participant_attr_);
        ASSERT_NE(participant_, nullptr);

        //Create writerhistory
        hattr_.payloadMaxSize = 255 + type_.max_serialized_type_size;
        history_ = new eprosima::fastdds::rtps::WriterHistory(hattr_);

        //Create writer
        eprosima::fastdds::rtps::WriterListener* listener = auto_remove_ ? this : nullptr;
        writer_ =
                eprosima::fastdds::rtps::RTPSDomain::createRTPSWriter(participant_, writer_attr_, history_, listener);
        ASSERT_NE(writer_, nullptr);

        register_reader();

        initialized_ = true;
    }

    bool isInitialized() const
    {
        return initialized_;
    }

    void send(
            std::list<type>& msgs)
    {
        auto it = msgs.begin();

        while (it != msgs.end())
        {
            size_t current_alignment =  4 + magicword_.size() + 1;
            eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);
            uint32_t cdr_size = static_cast<uint32_t>(
                current_alignment + calculator.calculate_serialized_size(*it, current_alignment));
            eprosima::fastdds::rtps::CacheChange_t* ch = history_->create_change(
                cdr_size, eprosima::fastdds::rtps::ALIVE);

            eprosima::fastcdr::FastBuffer buffer((char*)ch->serializedPayload.data, ch->serializedPayload.max_size);
            eprosima::fastcdr::Cdr cdr(buffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                    eprosima::fastcdr::CdrVersion::DDS_CDR);

            cdr << magicword_;
            cdr << *it;
            ch->serializedPayload.length = static_cast<uint32_t>(cdr.get_serialized_data_length());

            history_->add_change(ch);
            it = msgs.erase(it);
        }
    }

    bool wait_for_all_acked(
            const std::chrono::seconds& seconds)
    {
        eprosima::fastdds::dds::Duration_t max_time(int32_t(seconds.count()), 0);
        return writer_->wait_for_all_acked(max_time);
    }

    bool is_history_empty ()
    {
        return history_->getHistorySize() == 0;
    }

    RTPSAsSocketWriter& auto_remove_on_volatile()
    {
        auto_remove_ = true;
        return *this;
    }

    /*** Function to change QoS ***/
    RTPSAsSocketWriter& reliability(
            const eprosima::fastdds::rtps::ReliabilityKind_t kind)
    {
        writer_attr_.endpoint.reliabilityKind = kind;

        if (kind == eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE)
        {
            writer_attr_.endpoint.setEntityID(2);
        }
        return *this;
    }

    RTPSAsSocketWriter& durability(
            const eprosima::fastdds::rtps::DurabilityKind_t kind)
    {
        writer_attr_.endpoint.durabilityKind = kind;

        return *this;
    }

    RTPSAsSocketWriter& add_to_multicast_locator_list(
            const std::string& ip,
            uint32_t port)
    {
        ip_ = ip;
        port_ = port;

        eprosima::fastdds::rtps::Locator_t loc;
        IPLocator::setIPv4(loc, ip);
        loc.port = static_cast<uint16_t>(port);
        writer_attr_.endpoint.multicastLocatorList.push_back(loc);

        return *this;
    }

    void register_reader()
    {
        if (port_ == 0)
        {
            std::cout << "ERROR: locator has to be registered previous to call this" << std::endl;
        }

        //Add remote reader (in this case a reader in the same machine)
        eprosima::fastdds::rtps::GUID_t guid = participant_->getGuid();

        eprosima::fastdds::rtps::SubscriptionBuiltinTopicData rdata;

        eprosima::fastdds::rtps::Locator_t loc;
        IPLocator::setIPv4(loc, ip_);
        loc.port = static_cast<uint16_t>(port_);
        rdata.remote_locators.add_unicast_locator(loc);

        if (writer_attr_.endpoint.reliabilityKind == eprosima::fastdds::rtps::RELIABLE)
        {
            rdata.reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        }

        // Check disable_positive_acks_ attribute
        if (writer_attr_.disable_positive_acks)
        {
            rdata.disable_positive_acks.enabled = writer_attr_.disable_positive_acks;
            rdata.disable_positive_acks.duration = writer_attr_.keep_duration;
        }

        rdata.guid.guidPrefix.value[0] = guid.guidPrefix.value[0];
        rdata.guid.guidPrefix.value[1] = guid.guidPrefix.value[1];
        rdata.guid.guidPrefix.value[2] = guid.guidPrefix.value[2];
        rdata.guid.guidPrefix.value[3] = guid.guidPrefix.value[3];
        rdata.guid.guidPrefix.value[4] = guid.guidPrefix.value[4];
        rdata.guid.guidPrefix.value[5] = guid.guidPrefix.value[5];
        rdata.guid.guidPrefix.value[6] = guid.guidPrefix.value[6];
        rdata.guid.guidPrefix.value[7] = guid.guidPrefix.value[7];
        rdata.guid.guidPrefix.value[8] = 1;
        rdata.guid.guidPrefix.value[9] = 0;
        rdata.guid.guidPrefix.value[10] = 0;
        rdata.guid.guidPrefix.value[11] = 0;
        rdata.guid.entityId.value[0] = 0;
        rdata.guid.entityId.value[1] = 0;
        rdata.guid.entityId.value[2] = 1;
        rdata.guid.entityId.value[3] = 4;

        writer_->matched_reader_add(rdata);
    }

    RTPSAsSocketWriter& asynchronously(
            const eprosima::fastdds::rtps::RTPSWriterPublishMode mode)
    {
        writer_attr_.mode = mode;

        return *this;
    }

    RTPSAsSocketWriter& add_flow_controller_descriptor_to_pparams(
            uint32_t bytesPerPeriod,
            uint32_t periodInMs)
    {
        auto new_flow_controller = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
        new_flow_controller->name = "MyFlowController";
        new_flow_controller->max_bytes_per_period = bytesPerPeriod;
        new_flow_controller->period_ms = static_cast<uint64_t>(periodInMs);
        participant_attr_.flow_controllers.push_back(new_flow_controller);
        writer_attr_.flow_controller_name = new_flow_controller->name;

        return *this;
    }

    RTPSAsSocketWriter& heartbeat_period_seconds(
            int32_t sec)
    {
        writer_attr_.times.heartbeat_period.seconds = sec;
        return *this;
    }

    RTPSAsSocketWriter& heartbeat_period_nanosec(
            uint32_t nanosec)
    {
        writer_attr_.times.heartbeat_period.nanosec = nanosec;
        return *this;
    }

    RTPSAsSocketWriter& disable_positive_acks_seconds(
            bool disable,
            int32_t sec)
    {
        writer_attr_.disable_positive_acks = disable;
        writer_attr_.keep_duration = eprosima::fastdds::dds::Duration_t(sec, 0);
        return *this;
    }

    /*** Access RTPSWriter functions ***/
    void update_attributes(
            const eprosima::fastdds::rtps::WriterAttributes& att)
    {
        writer_->update_attributes(att);
        return;
    }

    bool get_disable_positive_acks()
    {
        return writer_->get_disable_positive_acks();
    }

private:

    eprosima::fastdds::rtps::RTPSParticipant* participant_;
    eprosima::fastdds::rtps::RTPSWriter* writer_;
    eprosima::fastdds::rtps::RTPSParticipantAttributes participant_attr_;
    eprosima::fastdds::rtps::WriterAttributes writer_attr_;
    eprosima::fastdds::rtps::WriterHistory* history_;
    eprosima::fastdds::rtps::HistoryAttributes hattr_;
    bool initialized_;
    bool auto_remove_;
    std::string magicword_;
    type_support type_;
    std::string ip_;
    uint32_t port_;
};

#endif // _TEST_BLACKBOX_RTPSASSOCKETWRITER_HPP_
