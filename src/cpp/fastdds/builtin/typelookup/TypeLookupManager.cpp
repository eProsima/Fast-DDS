// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeLookupManager.cpp
 *
 */

#include <fastdds/dds/builtin/typelookup/TypeLookupManager.hpp>

#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/writer/StatefulWriter.h>
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/dds/topic/TypeSupport.hpp>
// TODO Uncomment if security is implemented.
//#include <fastdds/rtps/common/Guid.h>
//#include <fastdds/rtps/security/accesscontrol/ParticipantSecurityAttributes.h>

#include <fastdds/dds/log/Log.hpp>

#include <rtps/participant/RTPSParticipantImpl.h>

#include <algorithm>

namespace eprosima {

using namespace fastrtps::rtps;
using eprosima::fastdds::dds::Log;

namespace fastdds {
namespace dds {
namespace builtin {

const fastrtps::rtps::SampleIdentity INVALID_SAMPLE_IDENTITY;

TypeLookupManager::TypeLookupManager(
        BuiltinProtocols* prot)
    : participant_(nullptr)
    , builtin_protocols_(prot)
    , builtin_request_writer_(nullptr)
    , builtin_request_reader_(nullptr)
    , builtin_reply_writer_(nullptr)
    , builtin_reply_reader_(nullptr)
    , builtin_request_writer_history_(nullptr)
    , builtin_reply_writer_history_(nullptr)
    , builtin_request_reader_history_(nullptr)
    , builtin_reply_reader_history_(nullptr)
    , request_listener_(nullptr)
    , reply_listener_(nullptr)
    , temp_reader_proxy_data_(
        prot->mp_participantImpl->getRTPSParticipantAttributes().allocation.locators.max_unicast_locators,
        prot->mp_participantImpl->getRTPSParticipantAttributes().allocation.locators.max_multicast_locators)
    , temp_writer_proxy_data_(
        prot->mp_participantImpl->getRTPSParticipantAttributes().allocation.locators.max_unicast_locators,
        prot->mp_participantImpl->getRTPSParticipantAttributes().allocation.locators.max_multicast_locators)
    /* TODO Uncomment if security is implemented
     #if HAVE_SECURITY
        , builtin_request_writer_secure_(nullptr)
        , builtin_reply_writer_secure_(nullptr)
        , builtin_request_reader_secure_(nullptr)
        , builtin_reply_reader_secure_(nullptr)
        , builtin_request_writer_secure_history_(nullptr)
        , builtin_reply_writer_secure_history_(nullptr)
        , builtin_request_reader_secure_history_(nullptr)
        , builtin_reply_reader_secure_history_(nullptr)
     #endif
     */
{
}

TypeLookupManager::~TypeLookupManager()
{
    /* TODO Uncomment if security is implemented
     #if HAVE_SECURITY
        participant_->deleteUserEndpoint(builtin_request_writer_secure_);
        participant_->deleteUserEndpoint(builtin_reply_writer_secure_);
        participant_->deleteUserEndpoint(builtin_request_reader_secure_);
        participant_->deleteUserEndpoint(builtin_reply_reader_secure_);
        delete builtin_request_writer_secure_history_;
        delete builtin_reply_writer_secure_history_;
        delete builtin_request_reader_secure_history_;
        delete builtin_reply_reader_secure_history_;
     #endif
     */
    if (nullptr != builtin_reply_reader_)
    {
        participant_->deleteUserEndpoint(builtin_reply_reader_);
    }
    if (nullptr != builtin_reply_writer_)
    {
        participant_->deleteUserEndpoint(builtin_reply_writer_);
    }
    if (nullptr != builtin_request_reader_)
    {
        participant_->deleteUserEndpoint(builtin_request_reader_);
    }
    if (nullptr != builtin_request_writer_)
    {
        participant_->deleteUserEndpoint(builtin_request_writer_);
    }
    delete builtin_request_writer_history_;
    delete builtin_reply_writer_history_;
    delete builtin_request_reader_history_;
    delete builtin_reply_reader_history_;

    delete reply_listener_;
    delete request_listener_;
}

bool TypeLookupManager::init_typelookup_service(
        RTPSParticipantImpl* participant)
{
    logInfo(TYPELOOKUP_SERVICE, "Initializing TypeLookup Service");
    participant_ = participant;
    bool retVal = create_endpoints();
    /*
     #if HAVE_SECURITY
        if (retVal)
        {
            retVal = create_secure_endpoints();
        }
     #endif
     */
    return retVal;
}

bool TypeLookupManager::assign_remote_endpoints(
        const ParticipantProxyData& pdata)
{
    const NetworkFactory& network = participant_->network_factory();
    uint32_t endp = pdata.m_availableBuiltinEndpoints;
    uint32_t partdet = endp;
    uint32_t auxendp = endp;

    std::lock_guard<std::mutex> data_guard(temp_data_lock_);

    temp_writer_proxy_data_.guid().guidPrefix = pdata.m_guid.guidPrefix;
    temp_writer_proxy_data_.persistence_guid().guidPrefix = pdata.m_guid.guidPrefix;
    temp_writer_proxy_data_.set_remote_locators(pdata.metatraffic_locators, network, true);
    temp_writer_proxy_data_.topicKind(NO_KEY);
    temp_writer_proxy_data_.m_qos.m_durability.kind = fastrtps::VOLATILE_DURABILITY_QOS;
    temp_writer_proxy_data_.m_qos.m_reliability.kind = fastrtps::RELIABLE_RELIABILITY_QOS;

    temp_reader_proxy_data_.clear();
    temp_reader_proxy_data_.m_expectsInlineQos = false;
    temp_reader_proxy_data_.guid().guidPrefix = pdata.m_guid.guidPrefix;
    temp_reader_proxy_data_.set_remote_locators(pdata.metatraffic_locators, network, true);
    temp_reader_proxy_data_.topicKind(NO_KEY);
    temp_reader_proxy_data_.m_qos.m_durability.kind = fastrtps::VOLATILE_DURABILITY_QOS;
    temp_reader_proxy_data_.m_qos.m_reliability.kind = fastrtps::RELIABLE_RELIABILITY_QOS;

    logInfo(TYPELOOKUP_SERVICE, "for RTPSParticipant: " << pdata.m_guid);

    partdet &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR; //Habria que quitar esta linea que comprueba si tiene PDP.
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_WRITER;

    if ((auxendp != 0 || partdet != 0) && builtin_request_reader_ != nullptr)
    {
        logInfo(TYPELOOKUP_SERVICE, "Adding remote writer to the local Builtin Request Reader");
        temp_writer_proxy_data_.guid().entityId = fastrtps::rtps::c_EntityId_TypeLookup_request_writer;
        temp_writer_proxy_data_.persistence_guid().entityId = fastrtps::rtps::c_EntityId_TypeLookup_request_writer;
        builtin_request_reader_->matched_writer_add(temp_writer_proxy_data_);
    }

    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_WRITER;

    if ((auxendp != 0 || partdet != 0) && builtin_reply_reader_ != nullptr)
    {
        logInfo(TYPELOOKUP_SERVICE, "Adding remote writer to the local Builtin Reply Reader");
        temp_writer_proxy_data_.guid().entityId = fastrtps::rtps::c_EntityId_TypeLookup_reply_writer;
        temp_writer_proxy_data_.persistence_guid().entityId = fastrtps::rtps::c_EntityId_TypeLookup_reply_writer;
        builtin_reply_reader_->matched_writer_add(temp_writer_proxy_data_);
    }

    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_READER;

    if ((auxendp != 0 || partdet != 0) && builtin_request_writer_ != nullptr)
    {
        logInfo(TYPELOOKUP_SERVICE, "Adding remote reader to the local Builtin Request Writer");
        temp_reader_proxy_data_.guid().entityId = fastrtps::rtps::c_EntityId_TypeLookup_request_reader;
        builtin_request_writer_->matched_reader_add(temp_reader_proxy_data_);
    }

    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_READER;

    if ((auxendp != 0 || partdet != 0) && builtin_reply_writer_ != nullptr)
    {
        logInfo(TYPELOOKUP_SERVICE, "Adding remote reader to the local Builtin Reply Writer");
        temp_reader_proxy_data_.guid().entityId = fastrtps::rtps::c_EntityId_TypeLookup_reply_reader;
        builtin_reply_writer_->matched_reader_add(temp_reader_proxy_data_);
    }

    return true;
}

void TypeLookupManager::remove_remote_endpoints(
        fastrtps::rtps::ParticipantProxyData* pdata)
{
    GUID_t tmp_guid;
    tmp_guid.guidPrefix = pdata->m_guid.guidPrefix;

    logInfo(TYPELOOKUP_SERVICE, "for RTPSParticipant: " << pdata->m_guid);
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t partdet = endp;
    uint32_t auxendp = endp;
    partdet &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR; //Habria que quitar esta linea que comprueba si tiene PDP.
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_WRITER;

    if ((auxendp != 0 || partdet != 0) && builtin_request_reader_ != nullptr)
    {
        logInfo(TYPELOOKUP_SERVICE, "Removing remote writer from the local Builtin Request Reader");
        tmp_guid.entityId = fastrtps::rtps::c_EntityId_TypeLookup_request_writer;
        builtin_request_reader_->matched_writer_remove(tmp_guid);
    }

    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_WRITER;

    if ((auxendp != 0 || partdet != 0) && builtin_reply_reader_ != nullptr)
    {
        logInfo(TYPELOOKUP_SERVICE, "Removing remote writer from the local Builtin Reply Reader");
        tmp_guid.entityId = fastrtps::rtps::c_EntityId_TypeLookup_reply_writer;
        builtin_reply_reader_->matched_writer_remove(tmp_guid);
    }

    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_READER;

    if ((auxendp != 0 || partdet != 0) && builtin_request_writer_ != nullptr)
    {
        logInfo(TYPELOOKUP_SERVICE, "Removing remote reader from the local Builtin Request Writer");
        tmp_guid.entityId = fastrtps::rtps::c_EntityId_TypeLookup_request_reader;
        builtin_request_writer_->matched_reader_remove(tmp_guid);
    }

    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_READER;

    if ((auxendp != 0 || partdet != 0) && builtin_reply_writer_ != nullptr)
    {
        logInfo(TYPELOOKUP_SERVICE, "Removing remote reader from the local Builtin Reply Writer");
        tmp_guid.entityId = fastrtps::rtps::c_EntityId_TypeLookup_reply_reader;
        builtin_reply_writer_->matched_reader_remove(tmp_guid);
    }
}

StatefulWriter* TypeLookupManager::get_builtin_request_writer()
{
    return builtin_request_writer_;
}

StatefulWriter* TypeLookupManager::get_builtin_reply_writer()
{
    return builtin_reply_writer_;
}

WriterHistory* TypeLookupManager::get_builtin_request_writer_history()
{
    return builtin_request_writer_history_;
}

WriterHistory* TypeLookupManager::get_builtin_reply_writer_history()
{
    return builtin_reply_writer_history_;
}

StatefulReader* TypeLookupManager::get_builtin_request_reader()
{
    return builtin_request_reader_;
}

StatefulReader* TypeLookupManager::get_builtin_reply_reader()
{
    return builtin_reply_reader_;
}

ReaderHistory* TypeLookupManager::get_builtin_request_reader_history()
{
    return builtin_request_reader_history_;
}

ReaderHistory* TypeLookupManager::get_builtin_reply_reader_history()
{
    return builtin_reply_reader_history_;
}

/* TODO Implement if security is needed.
 #if HAVE_SECURITY
   bool TypeLookupManager::pairing_remote_reader_with_local_writer_after_security(
        const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
   {

   }

   bool TypeLookupManager::pairing_remote_writer_with_local_reader_after_security(
        const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data)
   {

   }
 #endif
 */
bool TypeLookupManager::create_endpoints()
{
    // Built-in history attributes.
    HistoryAttributes hatt;
    hatt.initialReservedCaches = 20;
    hatt.maximumReservedCaches = 1000;
    hatt.payloadMaxSize = TYPELOOKUP_DATA_MAX_SIZE;

    WriterAttributes watt;
    watt.endpoint.unicastLocatorList = builtin_protocols_->m_att.metatrafficUnicastLocatorList;
    watt.endpoint.multicastLocatorList = builtin_protocols_->m_att.metatrafficMulticastLocatorList;
    watt.endpoint.remoteLocatorList = builtin_protocols_->m_att.initialPeersList;
    watt.matched_readers_allocation = participant_->getRTPSParticipantAttributes().allocation.participants;
    watt.endpoint.topicKind = fastrtps::rtps::NO_KEY;
    watt.endpoint.reliabilityKind = fastrtps::rtps::RELIABLE;
    watt.endpoint.durabilityKind = fastrtps::rtps::VOLATILE;

    // Built-in request writer
    if (builtin_protocols_->m_att.typelookup_config.use_client)
    {
        builtin_request_writer_history_ = new WriterHistory(hatt);

        RTPSWriter* req_writer;
        if (participant_->createWriter(
                    &req_writer,
                    watt,
                    builtin_request_writer_history_,
                    nullptr,
                    fastrtps::rtps::c_EntityId_TypeLookup_request_writer,
                    true))
        {
            builtin_request_writer_ = dynamic_cast<StatefulWriter*>(req_writer);
            logInfo(TYPELOOKUP_SERVICE, "Builtin Typelookup request writer created.");
        }
        else
        {
            logError(TYPELOOKUP_SERVICE, "Typelookup request writer creation failed.");
            delete builtin_request_writer_history_;
            builtin_request_writer_history_ = nullptr;
            return false;
        }
    }

    // Built-in reply writer
    if (builtin_protocols_->m_att.typelookup_config.use_server)
    {
        builtin_reply_writer_history_ = new WriterHistory(hatt);

        RTPSWriter* rep_writer;
        if (participant_->createWriter(
                    &rep_writer,
                    watt,
                    builtin_reply_writer_history_,
                    nullptr,
                    fastrtps::rtps::c_EntityId_TypeLookup_reply_writer,
                    true))
        {
            builtin_reply_writer_ = dynamic_cast<StatefulWriter*>(rep_writer);
            logInfo(TYPELOOKUP_SERVICE, "Builtin Typelookup reply writer created.");
        }
        else
        {
            logError(TYPELOOKUP_SERVICE, "Typelookup reply writer creation failed.");
            delete builtin_reply_writer_history_;
            builtin_reply_writer_history_ = nullptr;
            return false;
        }
    }

    ReaderAttributes ratt;
    ratt.endpoint.unicastLocatorList = builtin_protocols_->m_att.metatrafficUnicastLocatorList;
    ratt.endpoint.multicastLocatorList = builtin_protocols_->m_att.metatrafficMulticastLocatorList;
    ratt.endpoint.remoteLocatorList = builtin_protocols_->m_att.initialPeersList;
    ratt.matched_writers_allocation = participant_->getRTPSParticipantAttributes().allocation.participants;
    ratt.expectsInlineQos = true;
    ratt.endpoint.topicKind = fastrtps::rtps::NO_KEY;
    ratt.endpoint.reliabilityKind = fastrtps::rtps::RELIABLE;
    ratt.endpoint.durabilityKind = fastrtps::rtps::VOLATILE;

    // Built-in request reader
    if (builtin_protocols_->m_att.typelookup_config.use_server)
    {
        request_listener_ = new TypeLookupRequestListener(this);
        builtin_request_reader_history_ = new ReaderHistory(hatt);

        RTPSReader* req_reader;
        if (participant_->createReader(
                    &req_reader,
                    ratt,
                    builtin_request_reader_history_,
                    request_listener_,
                    fastrtps::rtps::c_EntityId_TypeLookup_request_reader,
                    true))
        {
            builtin_request_reader_ = dynamic_cast<StatefulReader*>(req_reader);
            logInfo(TYPELOOKUP_SERVICE, "Builtin Typelookup request reader created.");
        }
        else
        {
            logError(TYPELOOKUP_SERVICE, "Typelookup request reader creation failed.");
            delete builtin_request_reader_history_;
            builtin_request_reader_history_ = nullptr;
            delete request_listener_;
            request_listener_ = nullptr;
            return false;
        }
    }

    // Built-in reply reader
    if (builtin_protocols_->m_att.typelookup_config.use_client)
    {
        reply_listener_ = new TypeLookupReplyListener(this);
        builtin_reply_reader_history_ = new ReaderHistory(hatt);

        RTPSReader* rep_reader;
        if (participant_->createReader(
                    &rep_reader,
                    ratt,
                    builtin_reply_reader_history_,
                    reply_listener_,
                    fastrtps::rtps::c_EntityId_TypeLookup_reply_reader,
                    true))
        {
            builtin_reply_reader_ = dynamic_cast<StatefulReader*>(rep_reader);
            logInfo(TYPELOOKUP_SERVICE, "Builtin Typelookup reply reader created.");
        }
        else
        {
            logError(TYPELOOKUP_SERVICE, "Typelookup reply reader creation failed.");
            delete builtin_reply_reader_history_;
            builtin_reply_reader_history_ = nullptr;
            delete reply_listener_;
            reply_listener_ = nullptr;
            return false;
        }
    }

    return true;
}

/* TODO Implement if security is needed.
 #if HAVE_SECURITY
   bool TypeLookupManager::create_secure_endpoints()
   {
   }
 #endif
 */

SampleIdentity TypeLookupManager::get_type_dependencies(
        const fastrtps::types::TypeIdentifierSeq& id_seq) const
{
    SampleIdentity id = INVALID_SAMPLE_IDENTITY;
    if (builtin_protocols_->m_att.typelookup_config.use_client)
    {
        TypeLookup_getTypeDependencies_In in;
        in.type_ids = id_seq;
        TypeLookup_RequestTypeSupport type;
        TypeLookup_Request* request = static_cast<TypeLookup_Request*>(type.create_data());
        request->data.getTypeDependencies(in);

        if (send_request(*request))
        {
            id = request->header.requestId;
        }
        type.delete_data(request);
    }
    return id;
}

SampleIdentity TypeLookupManager::get_types(
        const fastrtps::types::TypeIdentifierSeq& id_seq) const
{
    SampleIdentity id = INVALID_SAMPLE_IDENTITY;
    if (builtin_protocols_->m_att.typelookup_config.use_client)
    {
        TypeLookup_getTypes_In in;
        in.type_ids = id_seq;
        TypeLookup_RequestTypeSupport type;
        TypeLookup_Request* request = static_cast<TypeLookup_Request*>(type.create_data());
        request->data.getTypes(in);

        if (send_request(*request))
        {
            id = request->header.requestId;
        }
        type.delete_data(request);
    }
    return id;
}

std::string TypeLookupManager::get_instanceName() const
{
    std::stringstream ss;
    ss << participant_->getGuid();
    std::string str = ss.str();
    std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c)
            {
                return static_cast<unsigned char>(std::tolower(c));
            });
    str.erase(std::remove(str.begin(), str.end(), '.'), str.end());
    return "dds.builtin.TOS." + str;
}

bool TypeLookupManager::send_request(
        TypeLookup_Request& req) const
{
    req.header.instanceName = get_instanceName();
    req.header.requestId.writer_guid(builtin_request_writer_->getGuid());
    req.header.requestId.sequence_number(request_seq_number_);
    ++request_seq_number_;

    CacheChange_t* change = builtin_request_writer_->new_change(
        [&req]()
        {
            return static_cast<uint32_t>(TypeLookup_Request::getCdrSerializedSize(req) + 4);
        },
        ALIVE);

    if (change != nullptr)
    {
        CDRMessage_t msg(change->serializedPayload);

        bool valid = CDRMessage::addOctet(&msg, 0);
        change->serializedPayload.encapsulation = static_cast<uint16_t>(PL_DEFAULT_ENCAPSULATION);
        msg.msg_endian = DEFAULT_ENDIAN;
        valid &= CDRMessage::addOctet(&msg, PL_DEFAULT_ENCAPSULATION);
        valid &= CDRMessage::addUInt16(&msg, 0);

        change->serializedPayload.pos = msg.pos;
        change->serializedPayload.length = msg.length;

        SerializedPayload_t payload;
        payload.max_size = change->serializedPayload.max_size - 4;
        payload.data = change->serializedPayload.data + 4;
        if (valid && request_type_.serialize(&req, &payload))
        {
            change->serializedPayload.length += payload.length;
            change->serializedPayload.pos += payload.pos;
            payload.data = nullptr;
            return builtin_request_writer_history_->add_change(change);
        }
    }
    builtin_request_writer_history_->remove_change(change);
    return false;

}

bool TypeLookupManager::send_reply(
        TypeLookup_Reply& rep) const
{
    rep.header.instanceName = get_instanceName();

    CacheChange_t* change = builtin_reply_writer_->new_change(
        [&rep]()
        {
            return static_cast<uint32_t>(TypeLookup_Reply::getCdrSerializedSize(rep) + 4);
        },
        ALIVE);

    if (change != nullptr)
    {
        CDRMessage_t msg(change->serializedPayload);

        bool valid = CDRMessage::addOctet(&msg, 0);
        change->serializedPayload.encapsulation = static_cast<uint16_t>(PL_DEFAULT_ENCAPSULATION);
        msg.msg_endian = DEFAULT_ENDIAN;
        valid &= CDRMessage::addOctet(&msg, PL_DEFAULT_ENCAPSULATION);
        valid &= CDRMessage::addUInt16(&msg, 0);

        change->serializedPayload.pos = msg.pos;
        change->serializedPayload.length = msg.length;

        SerializedPayload_t payload;
        payload.max_size = change->serializedPayload.max_size - 4;
        payload.data = change->serializedPayload.data + 4;
        if (valid && reply_type_.serialize(&rep, &payload))
        {
            change->serializedPayload.length += payload.length;
            change->serializedPayload.pos += payload.pos;
            payload.data = nullptr;
            return builtin_reply_writer_history_->add_change(change);
        }
    }
    builtin_request_writer_history_->remove_change(change);
    return false;
}

bool TypeLookupManager::recv_request(
        fastrtps::rtps::CacheChange_t& change,
        TypeLookup_Request& req) const
{
    CDRMessage_t msg(change.serializedPayload);
    msg.pos += 1;
    octet encapsulation = 0;
    CDRMessage::readOctet(&msg, &encapsulation);
    if (encapsulation == PL_CDR_BE)
    {
        msg.msg_endian = BIGEND;
    }
    else if (encapsulation == PL_CDR_LE)
    {
        msg.msg_endian = LITTLEEND;
    }
    else
    {
        return false;
    }
    change.serializedPayload.encapsulation = static_cast<uint16_t>(encapsulation);
    msg.pos += 2; // Skip encapsulation options.

    SerializedPayload_t payload;
    payload.max_size = change.serializedPayload.max_size - 4;
    payload.length = change.serializedPayload.length - 4;
    payload.data = change.serializedPayload.data + 4;
    bool result = request_type_.deserialize(&payload, &req);
    payload.data = nullptr;
    return result;
}

bool TypeLookupManager::recv_reply(
        fastrtps::rtps::CacheChange_t& change,
        TypeLookup_Reply& rep) const
{
    CDRMessage_t msg(change.serializedPayload);
    msg.pos += 1;
    octet encapsulation = 0;
    CDRMessage::readOctet(&msg, &encapsulation);
    if (encapsulation == PL_CDR_BE)
    {
        msg.msg_endian = BIGEND;
    }
    else if (encapsulation == PL_CDR_LE)
    {
        msg.msg_endian = LITTLEEND;
    }
    else
    {
        return false;
    }
    change.serializedPayload.encapsulation = static_cast<uint16_t>(encapsulation);
    msg.pos += 2; // Skip encapsulation options.

    SerializedPayload_t payload;
    payload.max_size = change.serializedPayload.max_size - 4;
    payload.length = change.serializedPayload.length - 4;
    payload.data = change.serializedPayload.data + 4;
    bool result = reply_type_.deserialize(&payload, &rep);
    payload.data = nullptr;
    return result;
}

const fastrtps::rtps::GUID_t& TypeLookupManager::get_builtin_request_writer_guid() const
{
    if (nullptr != builtin_request_writer_)
    {
        return builtin_request_writer_->getGuid();
    }
    return c_Guid_Unknown;
}

} // namespace builtin
} // namespace dds
} // namespace fastdds
} // namespace eprosima
