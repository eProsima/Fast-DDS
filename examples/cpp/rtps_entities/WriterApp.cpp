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
#include <fastdds/rtps/attributes/TopicAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
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
namespace rtps_entities {

WriterApp::WriterApp(
        const CLIParser::rtps_entities_config& config,
        const std::string& topic_name)
    : samples_(config.samples)
    , samples_sent_(0)
    , rtps_participant_(nullptr)
    , rtps_writer_(nullptr)
    , writer_history_(nullptr)
    , matched_(0)
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
    writer_att.endpoint.reliabilityKind = BEST_EFFORT;
    rtps_writer_ = RTPSDomain::createRTPSWriter(rtps_participant_, writer_att, writer_history_, this);

    if (rtps_writer_ == nullptr)
    {
        throw std::runtime_error("RTPS Writer creation failed");
    }

    if (!register_entity(topic_name))
    {
        throw std::runtime_error("Entity registration failed");
    }
}

WriterApp::~WriterApp()
{
    RTPSDomain::removeRTPSParticipant(rtps_participant_);
    delete(writer_history_);
}

bool WriterApp::register_entity(std::string topic_name)
{
    std::cout << "Registering RTPS Writer" << std::endl;

    TopicAttributes topic_att;
    topic_att.topicKind = NO_KEY;
    topic_att.topicDataType = "HelloWorld";
    topic_att.topicName = topic_name;

    eprosima::fastdds::dds::WriterQos writer_qos;
    return rtps_participant_->registerWriter(rtps_writer_, topic_att, writer_qos);
}

void WriterApp::onWriterMatched(
        RTPSWriter*,
        MatchingInfo& info)
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
        add_change_to_history();

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
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::CdrVersion::XCDRv1);

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


void WriterApp::add_change_to_history()
{
    // Wait for the data endpoints discovery
    std::unique_lock<std::mutex> matched_lock(terminate_cv_mtx_);
    terminate_cv_.wait(matched_lock, [&]()
            {
                // at least one has been discovered
                return ((matched_ > 0) || is_stopped());
            });

    CacheChange_t* ch = writer_history_->create_change(255, ALIVE);

    // In the case history is full, remove some old changes
    if (!ch)
    {
        std::cout << "cleaning history...";
        writer_history_->remove_min_change();
        ch = writer_history_->create_change(255, ALIVE);
    }

    data_->message("Hello World");
    data_->index(data_->index()+1);

    if (serialize_payload(data_, ch->serializedPayload))
    {
        std::cout << "Message " << data_->message() << " with index " << data_->index() << " SENT" << std::endl;
        ++samples_sent_;
    }

    writer_history_->add_change(ch);
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

} // namespace rtps_entities
} // namespace examples
} // namespace fastdds
} // namespace eprosima