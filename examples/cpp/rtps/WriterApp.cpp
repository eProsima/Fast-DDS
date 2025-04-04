// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file WriterApp.cpp
 *
 */

#include "WriterApp.hpp"

#include <chrono>
#include <condition_variable>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>

#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include "HelloWorld.hpp"

template<>
void eprosima::fastcdr::serialize<HelloWorld>(
        eprosima::fastcdr::Cdr& scdr,
        const HelloWorld& data)
{
    eprosima::fastcdr::Cdr::state current_state(scdr);
    scdr.begin_serialize_type(current_state,
            eprosima::fastcdr::CdrVersion::XCDRv2 == scdr.get_cdr_version() ?
            eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR);

    scdr
        << eprosima::fastcdr::MemberId(0) << data.index()
        << eprosima::fastcdr::MemberId(1) << data.message();
    scdr.end_serialize_type(current_state);
}

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rtps {

WriterApp::WriterApp(
        const CLIParser::rtps_config& config,
        const std::string& topic_name)
    : samples_(config.samples)
    , samples_sent_(0)
    , rtps_participant_(nullptr)
    , rtps_writer_(nullptr)
    , writer_history_(nullptr)
    , matched_(0)
    , expected_matches_(config.matched)
    , stop_(false)
    , data_(new HelloWorld)
{
    // Create RTPS Participant
    RTPSParticipantAttributes part_attr;
    part_attr.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SIMPLE;
    part_attr.builtin.use_WriterLivelinessProtocol = true;
    rtps_participant_ = RTPSDomain::createParticipant(0, part_attr);

    if (rtps_participant_ == nullptr)
    {
        throw std::runtime_error("RTPS Participant creation failed");
    }

    // Writer History Attributes
    HistoryAttributes hatt;
    hatt.payloadMaxSize = 255;
    hatt.maximumReservedCaches = 50;
    writer_history_ = new WriterHistory(hatt);

    // Create RTPS Writer
    WriterAttributes writer_att;
    writer_att.endpoint.reliabilityKind = RELIABLE;
    writer_att.endpoint.durabilityKind = TRANSIENT_LOCAL;

    rtps_writer_ = RTPSDomain::createRTPSWriter(rtps_participant_, writer_att, writer_history_, this);

    if (rtps_writer_ == nullptr)
    {
        throw std::runtime_error("RTPS Writer creation failed");
    }

    std::cout << "Registering RTPS Writer" << std::endl;

    TopicDescription topic_desc;
    topic_desc.type_name = "HelloWorld";
    topic_desc.topic_name = topic_name;

    eprosima::fastdds::dds::WriterQos writer_qos;
    writer_qos.m_durability.kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    writer_qos.m_reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;

    // Register entity
    if (!rtps_participant_->register_writer(rtps_writer_, topic_desc, writer_qos))
    {
        throw std::runtime_error("Entity registration failed");
    }
}

WriterApp::~WriterApp()
{
    RTPSDomain::removeRTPSParticipant(rtps_participant_);
    delete(writer_history_);
}

void WriterApp::on_writer_matched(
        RTPSWriter*,
        const MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        ++matched_;
        std::cout << "Remote endpoint with GUID " << info.remoteEndpointGuid << " matched." << std::endl;

        terminate_cv_.notify_one();
    }
    else if (info.status == REMOVED_MATCHING)
    {
        --matched_;
        std::cout << "Remote endpoint with GUID " << info.remoteEndpointGuid << " unmatched." << std::endl;
    }
}

void WriterApp::run()
{
    while (!is_stopped() && ((samples_ == 0) || (samples_sent_ < samples_)))
    {
        if (add_change_to_history())
        {
            std::cout << "Message " << data_->message() << " with index " << data_->index() << " SENT" << std::endl;
        }

        if (data_->index() == 1u)
        {
            bool sample_acked = false;
            do
            {
                dds::Duration_t acked_wait{1, 0};
                sample_acked = rtps_writer_->wait_for_all_acked(acked_wait);
            }
            while (!sample_acked);
        }

        // Wait for period or stop event
        std::unique_lock<std::mutex> period_lock(terminate_cv_mtx_);
        terminate_cv_.wait_for(period_lock, std::chrono::milliseconds(period_ms_), [&]()
                {
                    return is_stopped();
                });
    }
}

bool WriterApp::serialize_payload(
        const HelloWorld* data,
        SerializedPayload_t& payload)
{
    const HelloWorld* p_type = data;

    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.max_size);

    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv1);

    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

#if FASTCDR_VERSION_MAJOR > 1
    ser.set_encoding_flag(eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR);
#endif // FASTCDR_VERSION_MAJOR > 1

    try
    {
        // Serialize encapsulation
        ser.serialize_encapsulation();

        // Serialize the object.
        ser << *p_type;
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        return false;
    }

    // Get the serialized length
#if FASTCDR_VERSION_MAJOR == 1
    payload.length = static_cast<uint32_t>(ser.getSerializedDataLength());
#else
    payload.length = static_cast<uint32_t>(ser.get_serialized_data_length());
#endif // FASTCDR_VERSION_MAJOR == 1
    return true;
}

bool WriterApp::add_change_to_history()
{
    // Wait for the data endpoints discovery
    std::unique_lock<std::mutex> matched_lock(terminate_cv_mtx_);
    terminate_cv_.wait(matched_lock, [&]()
            {
                // at least one has been discovered
                return ((matched_ >= expected_matches_) || is_stopped());
            });

    bool ret =  false;

    CacheChange_t* ch = writer_history_->create_change(255, ALIVE);

    // In the case history is full, remove some old changes
    if (writer_history_->isFull())
    {
        writer_history_->remove_min_change();
        ch = writer_history_->create_change(255, ALIVE);
    }

    data_->message("Hello World");
    data_->index(data_->index() + 1);

    if (serialize_payload(data_, ch->serializedPayload))
    {
        if (writer_history_->add_change(ch))
        {
            ++samples_sent_;
            ret = true;
        }
        else
        {
            std::cout << "Fail to add the change to the history!" << std::endl;
        }
    }
    else
    {
        std::cout << "Fail to serialize the payload!" << std::endl;
    }

    return ret;
}

bool WriterApp::is_stopped()
{
    return stop_.load();
}

void WriterApp::stop()
{
    stop_.store(true);
    terminate_cv_.notify_all();
}

} // namespace rtps
} // namespace examples
} // namespace fastdds
} // namespace eprosima
