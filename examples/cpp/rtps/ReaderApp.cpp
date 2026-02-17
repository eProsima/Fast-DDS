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

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>

#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>

#include "HelloWorld.hpp"

template<>
void eprosima::fastcdr::deserialize(
        eprosima::fastcdr::Cdr& cdr,
        HelloWorld& data)
{
    cdr.deserialize_type(eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version() ?
            eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
            [&data](eprosima::fastcdr::Cdr& dcdr, const eprosima::fastcdr::MemberId& mid) -> bool
            {
                bool ret_value = true;
                switch (mid.id)
                {
                    case 0:
                        dcdr >> data.index();
                        break;

                    case 1:
                        dcdr >> data.message();
                        break;

                    default:
                        ret_value = false;
                        break;
                }
                return ret_value;
            });
}

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rtps {

ReaderApp::ReaderApp(
        const CLIParser::rtps_config& config,
        const std::string& topic_name)
    : samples_(config.samples)
    , samples_received_(0)
    , rtps_participant_(nullptr)
    , rtps_reader_(nullptr)
    , reader_history_(nullptr)
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

    // Reader History Attributes
    HistoryAttributes hatt;
    hatt.payloadMaxSize = 255;
    reader_history_ = new ReaderHistory(hatt);

    // Create RTPS Reader
    ReaderAttributes reader_att;
    reader_att.endpoint.reliabilityKind = RELIABLE;
    reader_att.endpoint.durabilityKind = TRANSIENT_LOCAL;

    rtps_reader_ = RTPSDomain::createRTPSReader(rtps_participant_, reader_att, reader_history_, this);

    if (rtps_reader_ == nullptr)
    {
        throw std::runtime_error("RTPS Reader creation failed");
    }

    std::cout << "Registering RTPS Reader" << std::endl;

    TopicDescription topic_desc;
    topic_desc.topic_name = topic_name;
    topic_desc.type_name = "HelloWorld";

    eprosima::fastdds::dds::ReaderQos reader_qos;
    reader_qos.m_durability.kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.m_reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;

    // Register entity
    if (!rtps_participant_->register_reader(rtps_reader_, topic_desc, reader_qos))
    {
        throw std::runtime_error("Entity registration failed");
    }
}

ReaderApp::~ReaderApp()
{
    RTPSDomain::removeRTPSParticipant(rtps_participant_);
    delete(reader_history_);
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
        if (deserialize_payload(change->serializedPayload, data_))
        {
            std::cout << "Message: " << data_->message() << " with index " <<  data_->index() << " RECEIVED" <<
                std::endl;
            samples_received_++;
        }
        else
        {
            std::cout << "Message: not deserialized" << std::endl;
        }

        reader->get_history()->remove_change((CacheChange_t*)change);

        if ((samples_ > 0) && (samples_received_ >= samples_))
        {
            stop();
        }
    }
}

bool ReaderApp::deserialize_payload(
        const SerializedPayload_t& payload,
        HelloWorld* data)
{
    try
    {
        HelloWorld* p_type = data;

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.length);

        // Object that deserializes the data.
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN
#if FASTCDR_VERSION_MAJOR == 1
                , eprosima::fastcdr::Cdr::CdrType::DDS_CDR
#endif // FASTCDR_VERSION_MAJOR == 1
                );

        // Deserialize encapsulation.
        deser.read_encapsulation();
        //payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        // Deserialize the object.
        deser >> *p_type;
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        return false;
    }

    return true;
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

} // namespace rtps
} // namespace examples
} // namespace fastdds
} // namespace eprosima
