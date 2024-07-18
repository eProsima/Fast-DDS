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
 * @file EDPStatic.h
 *
 */

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_ENDPOINT__EDPSTATIC_H
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_ENDPOINT__EDPSTATIC_H

#include <rtps/builtin/discovery/endpoint/EDP.h>

namespace eprosima {

namespace fastdds {
namespace xmlparser {
class XMLEndpointParser;
} // namespace xmlparser

namespace rtps {

/**
 * Class EDPStatic, implements a static endpoint discovery module.
 * @ingroup DISCOVERYMODULE
 */
class EDPStatic : public EDP
{
public:

    //! Different exchange formats supported by Static Discovery.
    enum class ExchangeFormat : uint32_t
    {
        v1, //! Standard exchange format for Static Discovery.
        v1_Reduced //! Exchange format that reduces the used network bandwidth.
    };

    /**
     * Constructor.
     * @param p Pointer to the PDPSimple.
     * @param part Pointer to the RTPSParticipantImpl.
     */
    EDPStatic(
            PDP* p,
            RTPSParticipantImpl* part);
    virtual ~EDPStatic();
    /**
     * Abstract method to initialize the EDP.
     * @param attributes DiscoveryAttributes structure.
     * @return True if correct.
     */
    bool initEDP(
            BuiltinAttributes& attributes) override;
    /**
     * Abstract method that assigns remote endpoints when a new RTPSParticipantProxyData is discovered.
     * @param pdata Pointer to the ParticipantProxyData.
     * @param assign_secure_endpoints Whether to try assigning secure endpoints
     */
    void assignRemoteEndpoints(
            const ParticipantProxyData& pdata,
            bool assign_secure_endpoints) override;
    /**
     * Abstract method that removes a local reader from the discovery method
     * @param rtps_reader Pointer to the Reader to remove.
     * @return True if correctly removed.
     */
    bool remove_reader(
            RTPSReader* rtps_reader) override;
    /**
     * Abstract method that removes a local Writer from the discovery method
     * @param rtps_writer Pointer to the writer to remove.
     * @return True if correctly removed.
     */
    bool remove_writer(
            RTPSWriter* rtps_writer) override;

    /**
     * After a new local ReaderProxyData has been created some processing is needed (depends on the implementation).
     * @param rtps_reader Pointer to the RTPSReader object.
     * @param rdata       Pointer to the ReaderProxyData object.
     * @return True if correct.
     */
    bool process_reader_proxy_data(
            RTPSReader* rtps_reader,
            ReaderProxyData* rdata) override;
    /**
     * After a new local WriterProxyData has been created some processing is needed (depends on the implementation).
     * @param rtps_writer Pointer to the RTPSWriter object.
     * @param wdata       Pointer to the Writer ProxyData object.
     * @return True if correct.
     */
    bool process_writer_proxy_data(
            RTPSWriter* rtps_writer,
            WriterProxyData* wdata) override;

    /**
     * New Remote Writer has been found and this method process it and calls the pairing methods.
     * @param participant_guid  GUID of the participant.
     * @param participant_name  Name of the participant.
     * @param user_id           User Id.
     * @param ent_id            Entity Id.
     * @param persistence_guid  GUID used for persistence.
     * @return True if correct.
     */
    bool newRemoteWriter(
            const GUID_t& participant_guid,
            const fastcdr::string_255& participant_name,
            uint16_t user_id,
            EntityId_t ent_id = c_EntityId_Unknown,
            const GUID_t& persistence_guid = GUID_t::unknown());
    /**
     * New Remote Reader has been found and this method process it and calls the pairing methods.
     * @param participant_guid  GUID of the participant.
     * @param participant_name  Name of the participant.
     * @param user_id           User Id.
     * @param ent_id            Entity Id.
     * @return true if correct.
     */
    bool newRemoteReader(
            const GUID_t& participant_guid,
            const fastcdr::string_255& participant_name,
            uint16_t user_id,
            EntityId_t ent_id = c_EntityId_Unknown);

    /**
     * This method checks the provided entityId against the topic type to see if it matches
     * @param rdata Pointer to the readerProxyData
     * @return True if its correct.
     **/
    bool checkEntityId(
            ReaderProxyData* rdata);
    /**
     * This method checks the provided entityId against the topic type to see if it matches
     * @param wdata Pointer to the writerProxyData
     * @return True if its correct.
     **/
    bool checkEntityId(
            WriterProxyData* wdata);

private:

    xmlparser::XMLEndpointParser* mp_edpXML;

    BuiltinAttributes m_attributes;

    ExchangeFormat exchange_format_ = ExchangeFormat::v1;
};

} // namespace rtps
} /* namespace rtps */
} /* namespace eprosima */

#endif /* FASTDDS_RTPS_BUILTIN_DISCOVERY_ENDPOINT__EDPSTATIC_H */
