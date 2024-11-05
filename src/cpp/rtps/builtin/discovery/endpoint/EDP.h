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
 * @file EDP.h
 *
 */

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_ENDPOINT__EDP_H
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_ENDPOINT__EDP_H

#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>
#include <fastdds/rtps/common/Guid.hpp>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <utils/ProxyPool.hpp>

#define MATCH_FAILURE_REASON_COUNT size_t(16)

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

class TypeIdentifier;

} // namespace xtypes
} // namespace dds


namespace rtps {

class PDP;
class ParticipantProxyData;
class RTPSWriter;
class RTPSReader;
class WriterProxyData;
class RTPSParticipantImpl;
struct TopicDescription;

/**
 * Class EDP, base class for Endpoint Discovery Protocols. It contains generic methods used by the two EDP implemented (EDPSimple and EDPStatic), as well as abstract methods
 * definitions required by the specific implementations.
 * @ingroup DISCOVERY_MODULE
 */
class EDP
{
public:

    /**
     * Mask to hold the reasons why two endpoints do not match.
     */
    class MatchingFailureMask : public std::bitset<MATCH_FAILURE_REASON_COUNT>
    {
    public:

        //! Bit index for matching failing due to different topic
        static const uint32_t different_topic = 0u;

        //! Bit index for matching failing due to inconsistent topic (same topic name but different characteristics)
        static const uint32_t inconsistent_topic = 1u;

        //! Bit index for matching failing due to incompatible QoS
        static const uint32_t incompatible_qos = 2u;

        //! Bit index for matching failing due to inconsistent partitions
        static const uint32_t partitions = 3u;

        //! Bit index for matching failing due to incompatible TypeInformation
        static const uint32_t different_typeinfo = 4u;
    };

    /**
     * Constructor.
     * @param p Pointer to the PDPSimple
     * @param part Pointer to the RTPSParticipantImpl
     */
    EDP(
            PDP* p,
            RTPSParticipantImpl* part);
    virtual ~EDP();

    /**
     * Abstract method to initialize the EDP.
     * @param attributes DiscoveryAttributes structure.
     * @return True if correct.
     */
    virtual bool initEDP(
            BuiltinAttributes& attributes) = 0;
    /**
     * Abstract method that assigns remote endpoints when a new RTPSParticipantProxyData is discovered.
     * @param pdata Discovered ParticipantProxyData
     * @param assign_secure_endpoints Whether to try assigning secure endpoints
     */
    virtual void assignRemoteEndpoints(
            const ParticipantProxyData& pdata,
            bool assign_secure_endpoints) = 0;
    /**
     * Remove remote endpoints from the endpoint discovery protocol
     * @param pdata Pointer to the ParticipantProxyData to remove
     */
    virtual void removeRemoteEndpoints(
            ParticipantProxyData* pdata)
    {
        (void) pdata;
    }

    //! Verify whether the given participant EDP endpoints are matched with us
    virtual bool areRemoteEndpointsMatched(
            const ParticipantProxyData*)
    {
        return false;
    }

    /**
     * Abstract method that removes a local Reader from the discovery method
     * @param rtps_reader Pointer to the Reader to remove.
     * @return True if correctly removed.
     */
    virtual bool remove_reader(
            RTPSReader* rtps_reader) = 0;
    /**
     * Abstract method that removes a local Writer from the discovery method
     * @param rtps_writer Pointer to the Writer to remove.
     * @return True if correctly removed.
     */
    virtual bool remove_writer(
            RTPSWriter* rtps_writer) = 0;

    /**
     * After a new local ReaderProxyData has been created some processing is needed (depends on the implementation).
     * @param reader Pointer to the Reader object.
     * @param rdata Pointer to the ReaderProxyData object.
     * @return True if correct.
     */
    virtual bool process_reader_proxy_data(
            RTPSReader* reader,
            ReaderProxyData* rdata) = 0;

    /**
     * After a new local WriterProxyData has been created some processing is needed (depends on the implementation).
     * @param writer Pointer to the Writer object.
     * @param wdata Pointer to the Writer ProxyData object.
     * @return True if correct.
     */
    virtual bool process_writer_proxy_data(
            RTPSWriter* writer,
            WriterProxyData* wdata) = 0;

    /**
     * Create a new ReaderPD for a local Reader.
     *
     * @param rtps_reader     Pointer to the RTPSReader.
     * @param topic           Information regarding the topic where the writer is registering.
     * @param qos             QoS policies dictated by the subscriber.
     * @param content_filter  Optional content filtering information.
     *
     * @return True if correct.
     */
    bool new_reader_proxy_data(
            RTPSReader* rtps_reader,
            const TopicDescription& topic,
            const fastdds::dds::ReaderQos& qos,
            const fastdds::rtps::ContentFilterProperty* content_filter = nullptr);
    /**
     * Create a new WriterPD for a local Writer.
     *
     * @param rtps_writer  Pointer to the RTPSWriter.
     * @param topic        Information regarding the topic where the writer is registering.
     * @param qos          QoS policies dictated by the publisher.
     *
     * @return True if correct.
     */
    bool new_writer_proxy_data(
            RTPSWriter* rtps_writer,
            const TopicDescription& topic,
            const fastdds::dds::WriterQos& qos);
    /**
     * A previously created Reader has been updated
     * @param rtps_reader      Pointer to the RTPSReader.
     * @param qos              QoS policies dictated by the subscriber.
     * @param content_filter   Optional content filtering information.
     * @return True if correctly updated
     */
    bool update_reader(
            RTPSReader* rtps_reader,
            const fastdds::dds::ReaderQos& qos,
            const fastdds::rtps::ContentFilterProperty* content_filter = nullptr);
    /**
     * A previously created Writer has been updated
     * @param rtps_writer      Pointer to the RTPSWriter.
     * @param qos              QoS policies dictated by the publisher.
     * @return True if correctly updated
     */
    bool update_writer(
            RTPSWriter* rtps_writer,
            const fastdds::dds::WriterQos& qos);

    /**
     * Check the validity of a matching between a local RTPSWriter and a ReaderProxyData object.
     * @param wdata Pointer to the WriterProxyData object of the local RTPSWriter.
     * @param rdata Pointer to the ReaderProxyData object.
     * @param [out] reason On return will specify the reason of failed matching (if any).
     * @param [out] incompatible_qos On return will specify all the QoS values that were incompatible (if any).
     * @return True if the two can be matched.
     */
    bool valid_matching(
            const WriterProxyData* wdata,
            const ReaderProxyData* rdata,
            MatchingFailureMask& reason,
            fastdds::dds::PolicyMask& incompatible_qos);

    /**
     * Check the validity of a matching between a local RTPSReader and a WriterProxyData object.
     * @param rdata Pointer to the ReaderProxyData object of the local RTPSReader.
     * @param wdata Pointer to the WriterProxyData object.
     * @param [out] reason On return will specify the reason of failed matching (if any).
     * @param [out] incompatible_qos On return will specify all the QoS values that were incompatible (if any).
     * @return True if the two can be matched.
     */
    bool valid_matching(
            const ReaderProxyData* rdata,
            const WriterProxyData* wdata,
            MatchingFailureMask& reason,
            fastdds::dds::PolicyMask& incompatible_qos);

    /**
     * Unpair a WriterProxyData object from all local readers.
     * @param participant_guid GUID of the participant.
     * @param writer_guid GUID of the writer.
     * @param removed_by_lease Whether the writer is being unpaired due to a participant drop.
     * @return True if correct.
     */
    bool unpairWriterProxy(
            const GUID_t& participant_guid,
            const GUID_t& writer_guid,
            bool removed_by_lease);

    /**
     * Unpair a ReaderProxyData object from all local writers.
     * @param participant_guid GUID of the participant.
     * @param reader_guid GUID of the reader.
     * @return True if correct.
     */
    bool unpairReaderProxy(
            const GUID_t& participant_guid,
            const GUID_t& reader_guid);

    /**
     * Try to pair/unpair ReaderProxyData.
     * @param participant_guid Identifier of the participant.
     * @param rdata Pointer to the ReaderProxyData object.
     * @return True.
     */
    bool pairing_reader_proxy_with_any_local_writer(
            const GUID_t& participant_guid,
            ReaderProxyData* rdata);

#if HAVE_SECURITY
    bool pairing_reader_proxy_with_local_writer(
            const GUID_t& local_writer,
            const GUID_t& remote_participant_guid,
            ReaderProxyData& rdata);

    bool pairing_remote_reader_with_local_writer_after_security(
            const GUID_t& local_writer,
            const ReaderProxyData& remote_reader_data);
#endif // if HAVE_SECURITY

    /**
     * Try to pair/unpair WriterProxyData.
     * @param participant_guid Identifier of the participant.
     * @param wdata Pointer to the WriterProxyData.
     * @return True.
     */
    bool pairing_writer_proxy_with_any_local_reader(
            const GUID_t& participant_guid,
            WriterProxyData* wdata);

#if HAVE_SECURITY
    bool pairing_writer_proxy_with_local_reader(
            const GUID_t& local_reader,
            const GUID_t& remote_participant_guid,
            WriterProxyData& wdata);

    bool pairing_remote_writer_with_local_reader_after_security(
            const GUID_t& local_reader,
            const WriterProxyData& remote_writer_data);

    virtual bool pairing_remote_writer_with_local_builtin_reader_after_security(
            const GUID_t& /*local_reader*/,
            const WriterProxyData& /*remote_writer_data*/)
    {
        return false;
    }

    virtual bool pairing_remote_reader_with_local_builtin_writer_after_security(
            const GUID_t& /*local_writer*/,
            const ReaderProxyData& /*remote_reader_data*/)
    {
        return false;
    }

#endif // if HAVE_SECURITY

    //! Pointer to the PDP object that contains the endpoint discovery protocol.
    PDP* mp_PDP;
    //! Pointer to the RTPSParticipant.
    RTPSParticipantImpl* mp_RTPSParticipant;

    /**
     * Access the temporary proxy pool for reader proxies
     * @return pool reference
     */
    ProxyPool<ReaderProxyData>& get_temporary_reader_proxies_pool();

    /**
     * Access the temporary proxy pool for writer proxies
     * @return pool reference
     */
    ProxyPool<WriterProxyData>& get_temporary_writer_proxies_pool();

private:

    /**
     * Try to pair/unpair a local Reader against all possible writerProxy Data.
     * @param R Pointer to the Reader
     * @param participant_guid
     * @param rdata
     * @return True
     */
    bool pairingReader(
            RTPSReader* R,
            const GUID_t& participant_guid,
            const ReaderProxyData& rdata);
    /**
     * Try to pair/unpair a local Writer against all possible readerProxy Data.
     * @param W Pointer to the Writer
     * @param participant_guid
     * @param wdata
     * @return True
     */
    bool pairingWriter(
            RTPSWriter* W,
            const GUID_t& participant_guid,
            const WriterProxyData& wdata);

    bool checkDataRepresentationQos(
            const WriterProxyData* wdata,
            const ReaderProxyData* rdata) const;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DISCOVERY_ENDPOINT__EDP_H
