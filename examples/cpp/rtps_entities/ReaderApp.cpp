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
 * @file ReaderApp.cpp
 *
 */

#include "ReaderApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/TopicAttributes.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>

#include "HelloWorld.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rtps_entities {

ReaderApp::ReaderApp(
        const CLIParser::rtps_entities_config& config,
        const std::string& topic_name)
    : samples_(config.samples)
    , rtps_participant_(nullptr)
    , rtps_reader_(nullptr)
    , reader_history_(nullptr)
    , stop_(false)
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

    // Reader History Attributes
    HistoryAttributes hatt;
    hatt.payloadMaxSize = 255;
    reader_history_ = new ReaderHistory(hatt);

    // Create RTPS Reader
    ReaderAttributes reader_att;
    rtps_reader_ = RTPSDomain::createRTPSReader(rtps_participant_, reader_att, reader_history_, this);

    if (rtps_reader_ == nullptr)
    {
        throw std::runtime_error("RTPS Reader creation failed");
    }

    if (!register_entity(topic_name))
    {
        throw std::runtime_error("Entity registration failed");
    }
}

ReaderApp::~ReaderApp()
{
    RTPSDomain::removeRTPSParticipant(rtps_participant_);
    delete(reader_history_);
}

bool ReaderApp::register_entity(std::string topic_name)
{
    std::cout << "Registering RTPS Reader" << std::endl;


    TopicAttributes topic_att;
    topic_att.topicKind = NO_KEY;

    topic_att.topicDataType = "HelloWorld";
    topic_att.topicName = topic_name;

    eprosima::fastdds::dds::ReaderQos reader_qos;
    return rtps_participant_->registerReader(rtps_reader_, topic_att, reader_qos);
}

void ReaderApp::on_reader_matched(
        eprosima::fastdds::rtps::RTPSReader*,
        const eprosima::fastdds::rtps::MatchingInfo& info)

{
    if (info.status == MATCHED_MATCHING)
    {
        std::cout << "Remote endpoint with GUID " << info.remoteEndpointGuid << " matched." << std::endl;
    }
    else if (info.status == REMOVED_MATCHING)
    {
        std::cout << "Remote endpoint with GUID " << info.remoteEndpointGuid << " unmatched." << std::endl;
    }
}



void ReaderApp::run()
{
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, [&]
            {
                return is_stopped();
            });
}

void ReaderApp::on_new_cache_change_added(
        RTPSReader* reader,
        const CacheChange_t* const change)
{
    if (!is_stopped())
    {
        unsigned long index;
        std::string message;

        index = static_cast<unsigned long>(change->serializedPayload.data[7]) << 24
                | static_cast<unsigned long>(change->serializedPayload.data[6]) << 16
                | static_cast<unsigned long>(change->serializedPayload.data[5]) << 8
                | static_cast<unsigned long>(change->serializedPayload.data[4]);

        std::cout << " Message:";
        for (uint8_t i = 0; i < change->serializedPayload.length; ++i)
        {
            std::cout <<change->serializedPayload.data[i] << " ";
        }
        std::cout << " with index " << index << " RECEIVED" << std::endl;

        reader->get_history()->remove_change((CacheChange_t*)change);
        samples_received_++;

        if ((samples_ > 0) && (samples_received_ >= samples_))
        {
            stop();
        }
    }
}



bool ReaderApp::is_stopped()
{
    return stop_.load();
}

void ReaderApp::stop()
{
    stop_.store(true);
    terminate_cv_.notify_all();
}





void ReaderApp::on_requested_incompatible_qos(
        RTPSReader* reader,
        eprosima::fastdds::dds::PolicyMask qos)
{
    static_cast<void>(reader);
    static_cast<void>(qos);
    std::cout << "on_requested_incompatible_qos " <<std::endl;

}

void ReaderApp::on_sample_lost(
        RTPSReader* reader,
        int32_t sample_lost_since_last_update)
{
    static_cast<void>(reader);
    static_cast<void>(sample_lost_since_last_update);
        std::cout << "on_sample_lost " <<std::endl;

}

void ReaderApp::on_writer_discovery(
        RTPSReader* reader,
        WriterDiscoveryInfo::DISCOVERY_STATUS reason,
        const GUID_t& writer_guid,
        const WriterProxyData* writer_info)
{
    static_cast<void>(reader);
    static_cast<void>(reason);
    static_cast<void>(writer_guid);
    static_cast<void>(writer_info);
    std::cout << "on_writer_discovery " <<std::endl;
}

void ReaderApp::on_sample_rejected(
        RTPSReader* reader,
        eprosima::fastdds::dds::SampleRejectedStatusKind reason,
        const CacheChange_t* const change)
{
    static_cast<void>(reader);
    static_cast<void>(reason);
    static_cast<void>(change);
    std::cout << "on_sample_rejected " <<std::endl;
}

void ReaderApp::on_incompatible_type(
        RTPSReader* reader)
{
    static_cast<void>(reader);
    std::cout << "on_incompatible_type " <<std::endl;
}

} // namespace rtps_entities
} // namespace examples
} // namespace fastdds
} // namespace eprosima
