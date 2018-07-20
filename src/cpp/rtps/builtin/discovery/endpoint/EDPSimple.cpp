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
 * @file EDPSimple.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include "EDPSimpleListeners.h"
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include "../../../participant/RTPSParticipantImpl.h"
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/BuiltinProtocols.h>


#include <fastrtps/log/Log.h>

#include <mutex>

namespace eprosima {
namespace fastrtps{
namespace rtps {


EDPSimple::EDPSimple(PDPSimple* p,RTPSParticipantImpl* part):
    EDP(p,part),
    mp_pubListen(nullptr),
    mp_subListen(nullptr)

    {
        // TODO Auto-generated constructor stub

    }

EDPSimple::~EDPSimple()
{
#if HAVE_SECURITY
    if(this->sedp_builtin_publications_secure_writer_.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(sedp_builtin_publications_secure_writer_.first);
        delete(sedp_builtin_publications_secure_writer_.second);
    }

    if(this->sedp_builtin_publications_secure_reader_.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(sedp_builtin_publications_secure_reader_.first);
        delete(sedp_builtin_publications_secure_reader_.second);
    }

    if(this->sedp_builtin_subscriptions_secure_writer_.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(sedp_builtin_subscriptions_secure_writer_.first);
        delete(sedp_builtin_subscriptions_secure_writer_.second);
    }

    if(this->sedp_builtin_subscriptions_secure_reader_.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(sedp_builtin_subscriptions_secure_reader_.first);
        delete(sedp_builtin_subscriptions_secure_reader_.second);
    }
#endif

    if(this->mp_PubReader.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(mp_PubReader.first);
        delete(mp_PubReader.second);
    }
    if(this->mp_SubReader.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(mp_SubReader.first);
        delete(mp_SubReader.second);
    }
    if(this->mp_PubWriter.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(mp_PubWriter.first);
        delete(mp_PubWriter.second);
    }
    if(this->mp_SubWriter.first !=nullptr)
    {
        this->mp_RTPSParticipant->deleteUserEndpoint(mp_SubWriter.first);
        delete(mp_SubWriter.second);
    }
    if(mp_pubListen!=nullptr)
        delete(mp_pubListen);
    if(mp_subListen !=nullptr)
        delete(mp_subListen);
}


bool EDPSimple::initEDP(BuiltinAttributes& attributes)
{
    logInfo(RTPS_EDP,"Beginning Simple Endpoint Discovery Protocol");
    m_discovery = attributes;

    if(!createSEDPEndpoints())
    {
        logError(RTPS_EDP,"Problem creation SimpleEDP endpoints");
        return false;
    }

#if HAVE_SECURITY
    if(!create_sedp_secure_endpoints())
    {
        logError(RTPS_EDP,"Problem creation SimpleEDP endpoints");
        return false;
    }
#endif

    return true;
}


bool EDPSimple::createSEDPEndpoints()
{
    WriterAttributes watt;
    ReaderAttributes ratt;
    HistoryAttributes hatt;
    bool created = true;
    RTPSReader* raux = nullptr;
    RTPSWriter* waux = nullptr;
    if(m_discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
    {
        hatt.initialReservedCaches = 100;
        hatt.maximumReservedCaches = 5000;
        hatt.payloadMaxSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.writerHistoryMemoryPolicy;
        mp_PubWriter.second = new WriterHistory(hatt);
        //Wparam.pushMode = true;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.times.nackResponseDelay.seconds = 0;
        watt.times.nackResponseDelay.fraction = 0;
        watt.times.initialHeartbeatDelay.seconds = 0;
        watt.times.initialHeartbeatDelay.fraction = 0;
        if(mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
                mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
            watt.mode = ASYNCHRONOUS_WRITER;
        created &=this->mp_RTPSParticipant->createWriter(&waux,watt,mp_PubWriter.second,nullptr,c_EntityId_SEDPPubWriter,true);
        if(created)
        {
#if HAVE_SECURITY
            this->mp_RTPSParticipant->set_endpoint_rtps_protection_supports(waux, false);
#endif
            mp_PubWriter.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP,"SEDP Publication Writer created");
        }
        else
        {
            delete(mp_PubWriter.second);
            mp_PubWriter.second = nullptr;
        }
        hatt.initialReservedCaches = 100;
        hatt.maximumReservedCaches = 1000000;
        hatt.payloadMaxSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.readerHistoryMemoryPolicy;
        mp_SubReader.second = new ReaderHistory(hatt);
        //Rparam.historyMaxSize = 100;
        ratt.expectsInlineQos = false;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.topicKind = WITH_KEY;
        ratt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.times.heartbeatResponseDelay.seconds = 0;
        ratt.times.heartbeatResponseDelay.fraction = 0;
        ratt.times.initialAcknackDelay.seconds = 0;
        ratt.times.initialAcknackDelay.fraction = 0;
        this->mp_subListen = new EDPSimpleSUBListener(this);
        created &=this->mp_RTPSParticipant->createReader(&raux,ratt,mp_SubReader.second,mp_subListen,c_EntityId_SEDPSubReader,true);
        if(created)
        {
#if HAVE_SECURITY
            this->mp_RTPSParticipant->set_endpoint_rtps_protection_supports(raux, false);
#endif
            mp_SubReader.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP,"SEDP Subscription Reader created");
        }
        else
        {
            delete(mp_SubReader.second);
            mp_SubReader.second = nullptr;
            delete(mp_subListen);
            mp_subListen = nullptr;
        }
    }
    if(m_discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
    {
        hatt.initialReservedCaches = 100;
        hatt.maximumReservedCaches = 1000000;
        hatt.payloadMaxSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.readerHistoryMemoryPolicy;
        mp_PubReader.second = new ReaderHistory(hatt);
        //Rparam.historyMaxSize = 100;
        ratt.expectsInlineQos = false;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.topicKind = WITH_KEY;
        ratt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.times.heartbeatResponseDelay.seconds = 0;
        ratt.times.heartbeatResponseDelay.fraction = 0;
        ratt.times.initialAcknackDelay.seconds = 0;
        ratt.times.initialAcknackDelay.fraction = 0;
        this->mp_pubListen = new EDPSimplePUBListener(this);
        created &=this->mp_RTPSParticipant->createReader(&raux,ratt,mp_PubReader.second,mp_pubListen,c_EntityId_SEDPPubReader,true);
        if(created)
        {
#if HAVE_SECURITY
            this->mp_RTPSParticipant->set_endpoint_rtps_protection_supports(raux, false);
#endif
            mp_PubReader.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP,"SEDP Publication Reader created");

        }
        else
        {
            delete(mp_PubReader.second);
            mp_PubReader.second = nullptr;
            delete(mp_pubListen);
            mp_pubListen = nullptr;
        }
        hatt.initialReservedCaches = 100;
        hatt.maximumReservedCaches = 5000;
        hatt.payloadMaxSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.writerHistoryMemoryPolicy;
        mp_SubWriter.second = new WriterHistory(hatt);
        //Wparam.pushMode = true;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.times.nackResponseDelay.seconds = 0;
        watt.times.nackResponseDelay.fraction = 0;
        watt.times.initialHeartbeatDelay.seconds = 0;
        watt.times.initialHeartbeatDelay.fraction = 0;
        if(mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
                mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
            watt.mode = ASYNCHRONOUS_WRITER;
        created &=this->mp_RTPSParticipant->createWriter(&waux, watt, mp_SubWriter.second, nullptr,
                c_EntityId_SEDPSubWriter, true);
        if(created)
        {
#if HAVE_SECURITY
            this->mp_RTPSParticipant->set_endpoint_rtps_protection_supports(waux, false);
#endif
            mp_SubWriter.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP,"SEDP Subscription Writer created");

        }
        else
        {
            delete(mp_SubWriter.second);
            mp_SubWriter.second = nullptr;
        }
    }
    logInfo(RTPS_EDP,"Creation finished");
    return created;
}

#if HAVE_SECURITY
bool EDPSimple::create_sedp_secure_endpoints()
{
    WriterAttributes watt;
    ReaderAttributes ratt;
    HistoryAttributes hatt;
    bool created = true;
    RTPSReader* raux = nullptr;
    RTPSWriter* waux = nullptr;
    if(m_discovery.m_simpleEDP.enable_builtin_secure_publications_writer_and_subscriptions_reader)
    {
        hatt.initialReservedCaches = 100;
        hatt.maximumReservedCaches = 5000;
        hatt.payloadMaxSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.writerHistoryMemoryPolicy;
        sedp_builtin_publications_secure_writer_.second = new WriterHistory(hatt);
        //Wparam.pushMode = true;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.times.nackResponseDelay.seconds = 0;
        watt.times.nackResponseDelay.fraction = 0;
        watt.times.initialHeartbeatDelay.seconds = 0;
        watt.times.initialHeartbeatDelay.fraction = 0;
        watt.endpoint.security_attributes().is_submessage_protected =
            mp_RTPSParticipant->security_attributes().is_discovered_protected;
        if(mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
                mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
            watt.mode = ASYNCHRONOUS_WRITER;
        created &=this->mp_RTPSParticipant->createWriter(&waux, watt, sedp_builtin_publications_secure_writer_.second,
                nullptr, sedp_builtin_publications_secure_writer, true);
        if(created)
        {
            this->mp_RTPSParticipant->set_endpoint_rtps_protection_supports(waux, false);
            sedp_builtin_publications_secure_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP,"SEDP Publication Writer created");
        }
        else
        {
            delete(sedp_builtin_publications_secure_writer_.second);
            sedp_builtin_publications_secure_writer_.second = nullptr;
        }
        hatt.initialReservedCaches = 100;
        hatt.maximumReservedCaches = 1000000;
        hatt.payloadMaxSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.readerHistoryMemoryPolicy;
        sedp_builtin_subscriptions_secure_reader_.second = new ReaderHistory(hatt);
        //Rparam.historyMaxSize = 100;
        ratt.expectsInlineQos = false;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.topicKind = WITH_KEY;
        ratt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.times.heartbeatResponseDelay.seconds = 0;
        ratt.times.heartbeatResponseDelay.fraction = 0;
        ratt.times.initialAcknackDelay.seconds = 0;
        ratt.times.initialAcknackDelay.fraction = 0;
        ratt.endpoint.security_attributes().is_submessage_protected =
            mp_RTPSParticipant->security_attributes().is_discovered_protected;
        created &=this->mp_RTPSParticipant->createReader(&raux, ratt, sedp_builtin_subscriptions_secure_reader_.second,
                mp_subListen, sedp_builtin_subscriptions_secure_reader, true);
        if(created)
        {
            this->mp_RTPSParticipant->set_endpoint_rtps_protection_supports(raux, false);
            sedp_builtin_subscriptions_secure_reader_.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP,"SEDP Subscription Reader created");
        }
        else
        {
            delete(sedp_builtin_subscriptions_secure_reader_.second);
            sedp_builtin_subscriptions_secure_reader_.second = nullptr;
        }
    }

    if(m_discovery.m_simpleEDP.enable_builtin_secure_subscriptions_writer_and_publications_reader)
    {
        hatt.initialReservedCaches = 100;
        hatt.maximumReservedCaches = 1000000;
        hatt.payloadMaxSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.readerHistoryMemoryPolicy;
        sedp_builtin_publications_secure_reader_.second = new ReaderHistory(hatt);
        //Rparam.historyMaxSize = 100;
        ratt.expectsInlineQos = false;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.topicKind = WITH_KEY;
        ratt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.times.heartbeatResponseDelay.seconds = 0;
        ratt.times.heartbeatResponseDelay.fraction = 0;
        ratt.times.initialAcknackDelay.seconds = 0;
        ratt.times.initialAcknackDelay.fraction = 0;
        ratt.endpoint.security_attributes().is_submessage_protected =
            mp_RTPSParticipant->security_attributes().is_discovered_protected;
        created &=this->mp_RTPSParticipant->createReader(&raux, ratt, sedp_builtin_publications_secure_reader_.second,
                mp_pubListen, sedp_builtin_publications_secure_reader, true);
        if(created)
        {
            this->mp_RTPSParticipant->set_endpoint_rtps_protection_supports(raux, false);
            sedp_builtin_publications_secure_reader_.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP,"SEDP Publication Reader created");

        }
        else
        {
            delete(sedp_builtin_publications_secure_reader_.second);
            sedp_builtin_publications_secure_reader_.second = nullptr;
        }
        hatt.initialReservedCaches = 100;
        hatt.maximumReservedCaches = 5000;
        hatt.payloadMaxSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
        hatt.memoryPolicy = mp_PDP->mp_builtin->m_att.writerHistoryMemoryPolicy;
        sedp_builtin_subscriptions_secure_writer_.second = new WriterHistory(hatt);
        //Wparam.pushMode = true;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.times.nackResponseDelay.seconds = 0;
        watt.times.nackResponseDelay.fraction = 0;
        watt.times.initialHeartbeatDelay.seconds = 0;
        watt.times.initialHeartbeatDelay.fraction = 0;
        watt.endpoint.security_attributes().is_submessage_protected =
            mp_RTPSParticipant->security_attributes().is_discovered_protected;
        if(mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
                mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
            watt.mode = ASYNCHRONOUS_WRITER;
        created &=this->mp_RTPSParticipant->createWriter(&waux, watt, sedp_builtin_subscriptions_secure_writer_.second,
                nullptr, sedp_builtin_subscriptions_secure_writer, true);
        if(created)
        {
            this->mp_RTPSParticipant->set_endpoint_rtps_protection_supports(waux, false);
            sedp_builtin_subscriptions_secure_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP,"SEDP Subscription Writer created");

        }
        else
        {
            delete(sedp_builtin_subscriptions_secure_writer_.second);
            sedp_builtin_subscriptions_secure_writer_.second = nullptr;
        }
    }
    logInfo(RTPS_EDP,"Creation finished");
    return created;
}
#endif

bool EDPSimple::processLocalReaderProxyData(RTPSReader* local_reader, ReaderProxyData* rdata)
{
    logInfo(RTPS_EDP,rdata->guid().entityId);
    (void)local_reader;

    auto* writer = &mp_SubWriter;
    auto* reader = &mp_SubReader;

#if HAVE_SECURITY
    if(local_reader->getAttributes()->security_attributes().is_discovered_protected)
    {
        writer = &sedp_builtin_subscriptions_secure_writer_;
        reader = &sedp_builtin_subscriptions_secure_reader_;
    }
#endif

    if(writer->first != nullptr)
    {
        // TODO(Ricardo) Write a getCdrSerializedPayload for ReaderProxyData.
        CacheChange_t* change = writer->first->new_change([]() -> uint32_t {return DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;},
                ALIVE,rdata->key());

        if(change !=nullptr)
        {
            rdata->toParameterList();

            CDRMessage_t aux_msg(0);
            aux_msg.wraps = true;
            aux_msg.buffer = change->serializedPayload.data;
            aux_msg.max_size = change->serializedPayload.max_size;

#if __BIG_ENDIAN__
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
            aux_msg.msg_endian = BIGEND;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
            aux_msg.msg_endian =  LITTLEEND;
#endif

            ParameterList_t parameter_list = rdata->toParameterList();
            ParameterList::writeParameterListToCDRMsg(&aux_msg, &parameter_list, true);
            change->serializedPayload.length = (uint16_t)aux_msg.length;

            {
                std::unique_lock<std::recursive_mutex> lock(*writer->second->getMutex());
                for(auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if((*ch)->instanceHandle == change->instanceHandle)
                    {
                        writer->second->remove_change(*ch);
                        break;
                    }
                }
            }

            if(this->mp_subListen->getAttachedListener() != nullptr)
            {
                this->mp_subListen->getAttachedListener()->onNewCacheChangeAdded(reader->first, change);
            }

            writer->second->add_change(change);

            return true;
        }
        return false;
    }
    return true;
}
bool EDPSimple::processLocalWriterProxyData(RTPSWriter* local_writer, WriterProxyData* wdata)
{
    logInfo(RTPS_EDP, wdata->guid().entityId);
    (void)local_writer;

    auto* writer = &mp_PubWriter;
    auto* reader = &mp_PubReader;

#if HAVE_SECURITY
    if(local_writer->getAttributes()->security_attributes().is_discovered_protected)
    {
        writer = &sedp_builtin_publications_secure_writer_;
        reader = &sedp_builtin_publications_secure_reader_;
    }
#endif

    if(writer->first !=nullptr)
    {
        CacheChange_t* change = writer->first->new_change([]() -> uint32_t {return DISCOVERY_PUBLICATION_DATA_MAX_SIZE;},
                ALIVE, wdata->key());
        if(change != nullptr)
        {
            wdata->toParameterList();

            CDRMessage_t aux_msg(0);
            aux_msg.wraps = true;
            aux_msg.buffer = change->serializedPayload.data;
            aux_msg.max_size = change->serializedPayload.max_size;

#if __BIG_ENDIAN__
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
            aux_msg.msg_endian = BIGEND;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
            aux_msg.msg_endian =  LITTLEEND;
#endif

            ParameterList_t parameter_list = wdata->toParameterList();
            ParameterList::writeParameterListToCDRMsg(&aux_msg, &parameter_list, true);
            change->serializedPayload.length = (uint16_t)aux_msg.length;

            {
                std::unique_lock<std::recursive_mutex> lock(*writer->second->getMutex());
                for(auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if((*ch)->instanceHandle == change->instanceHandle)
                    {
                        writer->second->remove_change(*ch);
                        break;
                    }
                }
            }

            if(this->mp_pubListen->getAttachedListener() != nullptr)
                this->mp_pubListen->getAttachedListener()->onNewCacheChangeAdded(reader->first, change);

            writer->second->add_change(change);

            return true;
        }
        return false;
    }
    return true;
}

bool EDPSimple::removeLocalWriter(RTPSWriter* W)
{
    logInfo(RTPS_EDP,W->getGuid().entityId);

    auto* writer = &mp_PubWriter;
    auto* reader = &mp_PubReader;

#if HAVE_SECURITY
    if(W->getAttributes()->security_attributes().is_discovered_protected)
    {
        writer = &sedp_builtin_publications_secure_writer_;
        reader = &sedp_builtin_publications_secure_reader_;
    }
#endif

    if(writer->first!=nullptr)
    {
        InstanceHandle_t iH;
        iH = W->getGuid();
        CacheChange_t* change = writer->first->new_change([]() -> uint32_t {return DISCOVERY_PUBLICATION_DATA_MAX_SIZE;},
                NOT_ALIVE_DISPOSED_UNREGISTERED,iH);
        if(change != nullptr)
        {
            {
                std::lock_guard<std::recursive_mutex> guard(*writer->second->getMutex());
                for(auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if((*ch)->instanceHandle == change->instanceHandle)
                    {
                        writer->second->remove_change(*ch);
                        break;
                    }
                }

            }

            if(this->mp_pubListen->getAttachedListener() != nullptr)
                this->mp_pubListen->getAttachedListener()->onNewCacheChangeAdded(reader->first, change);

            writer->second->add_change(change);
        }
    }
    return mp_PDP->removeWriterProxyData(W->getGuid());
}

bool EDPSimple::removeLocalReader(RTPSReader* R)
{
    logInfo(RTPS_EDP,R->getGuid().entityId);

    auto* writer = &mp_SubWriter;
    auto* reader = &mp_SubReader;

#if HAVE_SECURITY
    if(R->getAttributes()->security_attributes().is_discovered_protected)
    {
        writer = &sedp_builtin_subscriptions_secure_writer_;
        reader = &sedp_builtin_subscriptions_secure_reader_;
    }
#endif

    if(writer->first!=nullptr)
    {
        InstanceHandle_t iH;
        iH = (R->getGuid());
        CacheChange_t* change = writer->first->new_change([]() -> uint32_t {return DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;},
                NOT_ALIVE_DISPOSED_UNREGISTERED,iH);
        if(change != nullptr)
        {
            {
                std::lock_guard<std::recursive_mutex> guard(*writer->second->getMutex());
                for(auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if((*ch)->instanceHandle == change->instanceHandle)
                    {
                        writer->second->remove_change(*ch);
                        break;
                    }
                }
            }

            if(this->mp_subListen->getAttachedListener() != nullptr)
                this->mp_subListen->getAttachedListener()->onNewCacheChangeAdded(reader->first, change);

            writer->second->add_change(change);
        }
    }
    return mp_PDP->removeReaderProxyData(R->getGuid());
}



void EDPSimple::assignRemoteEndpoints(const ParticipantProxyData& pdata)
{
    logInfo(RTPS_EDP,"New DPD received, adding remote endpoints to our SimpleEDP endpoints");
    uint32_t endp = pdata.m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && mp_PubReader.first!=nullptr) //Exist Pub Writer and i have pub reader
    {
        logInfo(RTPS_EDP,"Adding SEDP Pub Writer to my Pub Reader");
        RemoteWriterAttributes watt(pdata.m_VendorId);
        watt.guid.guidPrefix = pdata.m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SEDPPubWriter;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata.m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata.m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        mp_PubReader.first->matched_writer_add(watt);
    }
    auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && mp_PubWriter.first!=nullptr) //Exist Pub Detector
    {
        logInfo(RTPS_EDP,"Adding SEDP Pub Reader to my Pub Writer");
        RemoteReaderAttributes ratt(pdata.m_VendorId);
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata.m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SEDPPubReader;
        ratt.endpoint.unicastLocatorList = pdata.m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata.m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.endpoint.reliabilityKind = RELIABLE;
        mp_PubWriter.first->matched_reader_add(ratt);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && mp_SubReader.first!=nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Writer to my Sub Reader");
        RemoteWriterAttributes watt(pdata.m_VendorId);
        watt.guid.guidPrefix = pdata.m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SEDPSubWriter;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata.m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata.m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        mp_SubReader.first->matched_writer_add(watt);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && mp_SubWriter.first!=nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Reader to my Sub Writer");
        RemoteReaderAttributes ratt(pdata.m_VendorId);
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata.m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SEDPSubReader;
        ratt.endpoint.unicastLocatorList = pdata.m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata.m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.endpoint.reliabilityKind = RELIABLE;
        mp_SubWriter.first->matched_reader_add(ratt);
    }

#if HAVE_SECURITY
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && sedp_builtin_publications_secure_reader_.first != nullptr)
    {
        WriterProxyData watt;
        watt.guid().guidPrefix = pdata.m_guid.guidPrefix;
        watt.guid().entityId = sedp_builtin_publications_secure_writer;
        watt.persistence_guid(watt.guid());
        watt.unicastLocatorList(pdata.m_metatrafficUnicastLocatorList);
        watt.multicastLocatorList(pdata.m_metatrafficMulticastLocatorList);
        watt.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        watt.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        if(!mp_RTPSParticipant->security_manager().discovered_builtin_writer(
                    sedp_builtin_publications_secure_reader_.first->getGuid(), pdata.m_guid, watt,
                    sedp_builtin_publications_secure_reader_.first->getAttributes()->security_attributes()))
        {
            logError(RTPS_EDP, "Security manager returns an error for writer " <<
                    sedp_builtin_publications_secure_reader_.first->getGuid());
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && sedp_builtin_publications_secure_writer_.first!=nullptr)
    {
        ReaderProxyData ratt;
        ratt.m_expectsInlineQos = false;
        ratt.guid().guidPrefix = pdata.m_guid.guidPrefix;
        ratt.guid().entityId = sedp_builtin_publications_secure_reader;
        ratt.unicastLocatorList(pdata.m_metatrafficUnicastLocatorList);
        ratt.multicastLocatorList(pdata.m_metatrafficMulticastLocatorList);
        ratt.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        ratt.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        if(!mp_RTPSParticipant->security_manager().discovered_builtin_reader(
                    sedp_builtin_publications_secure_writer_.first->getGuid(), pdata.m_guid, ratt,
                    sedp_builtin_publications_secure_writer_.first->getAttributes()->security_attributes()))
        {
            logError(RTPS_EDP, "Security manager returns an error for writer " <<
                    sedp_builtin_publications_secure_writer_.first->getGuid());
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && sedp_builtin_subscriptions_secure_reader_.first != nullptr)
    {
        WriterProxyData watt;
        watt.guid().guidPrefix = pdata.m_guid.guidPrefix;
        watt.guid().entityId = sedp_builtin_subscriptions_secure_writer;
        watt.persistence_guid(watt.guid());
        watt.unicastLocatorList(pdata.m_metatrafficUnicastLocatorList);
        watt.multicastLocatorList(pdata.m_metatrafficMulticastLocatorList);
        watt.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        watt.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        if(!mp_RTPSParticipant->security_manager().discovered_builtin_writer(
                    sedp_builtin_subscriptions_secure_reader_.first->getGuid(), pdata.m_guid, watt,
                    sedp_builtin_subscriptions_secure_reader_.first->getAttributes()->security_attributes()))
        {
            logError(RTPS_EDP, "Security manager returns an error for writer " <<
                    sedp_builtin_subscriptions_secure_reader_.first->getGuid());
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && sedp_builtin_subscriptions_secure_writer_.first!=nullptr)
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Reader to my Sub Writer");
        ReaderProxyData ratt;
        ratt.m_expectsInlineQos = false;
        ratt.guid().guidPrefix = pdata.m_guid.guidPrefix;
        ratt.guid().entityId = sedp_builtin_subscriptions_secure_reader;
        ratt.unicastLocatorList(pdata.m_metatrafficUnicastLocatorList);
        ratt.multicastLocatorList(pdata.m_metatrafficMulticastLocatorList);
        ratt.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        ratt.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        if(!mp_RTPSParticipant->security_manager().discovered_builtin_reader(
                    sedp_builtin_subscriptions_secure_writer_.first->getGuid(), pdata.m_guid, ratt,
                    sedp_builtin_subscriptions_secure_writer_.first->getAttributes()->security_attributes()))
        {
            logError(RTPS_EDP, "Security manager returns an error for writer " <<
                    sedp_builtin_subscriptions_secure_writer_.first->getGuid());
        }
    }
#endif
}


void EDPSimple::removeRemoteEndpoints(ParticipantProxyData* pdata)
{
    logInfo(RTPS_EDP,"For RTPSParticipant: "<<pdata->m_guid);

    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && mp_PubReader.first!=nullptr) //Exist Pub Writer and i have pub reader
    {
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SEDPPubWriter;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        mp_PubReader.first->matched_writer_remove(watt);
    }
    auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && mp_PubWriter.first!=nullptr) //Exist Pub Detector
    {
        RemoteReaderAttributes ratt;
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SEDPPubReader;
        ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.endpoint.reliabilityKind = RELIABLE;
        mp_PubWriter.first->matched_reader_remove(ratt);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && mp_SubReader.first!=nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Writer to my Sub Reader");
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SEDPSubWriter;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        mp_SubReader.first->matched_writer_remove(watt);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && mp_SubWriter.first!=nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Reader to my Sub Writer");
        RemoteReaderAttributes ratt;
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SEDPSubReader;
        ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.endpoint.reliabilityKind = RELIABLE;
        mp_SubWriter.first->matched_reader_remove(ratt);
    }

#if HAVE_SECURITY
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && sedp_builtin_publications_secure_reader_.first != nullptr)
    {
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = sedp_builtin_publications_secure_writer;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        if(sedp_builtin_publications_secure_reader_.first->matched_writer_remove(watt))
        {
            mp_RTPSParticipant->security_manager().remove_writer(
                    sedp_builtin_publications_secure_reader_.first->getGuid(), pdata->m_guid, watt.guid);
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && sedp_builtin_publications_secure_writer_.first != nullptr)
    {
        RemoteReaderAttributes ratt;
        ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        ratt.guid.entityId = sedp_builtin_publications_secure_reader;
        ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.endpoint.reliabilityKind = RELIABLE;
        if(sedp_builtin_publications_secure_writer_.first->matched_reader_remove(ratt))
        {
            mp_RTPSParticipant->security_manager().remove_reader(
                    sedp_builtin_publications_secure_writer_.first->getGuid(), pdata->m_guid, ratt.guid);
        }
    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && sedp_builtin_subscriptions_secure_reader_.first != nullptr)
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Writer to my Sub Reader");
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = sedp_builtin_subscriptions_secure_writer;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        if(sedp_builtin_subscriptions_secure_reader_.first->matched_writer_remove(watt))
        {
            mp_RTPSParticipant->security_manager().remove_writer(
                    sedp_builtin_subscriptions_secure_reader_.first->getGuid(), pdata->m_guid, watt.guid);
        }
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp != 0 && sedp_builtin_subscriptions_secure_writer_.first!=nullptr)
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Reader to my Sub Writer");
        RemoteReaderAttributes ratt;
        ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        ratt.guid.entityId = sedp_builtin_subscriptions_secure_reader;
        ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.endpoint.reliabilityKind = RELIABLE;
        if(sedp_builtin_subscriptions_secure_writer_.first->matched_reader_remove(ratt))
        {
            mp_RTPSParticipant->security_manager().remove_reader(
                    sedp_builtin_subscriptions_secure_writer_.first->getGuid(), pdata->m_guid, ratt.guid);
        }
    }
#endif
}

#if HAVE_SECURITY
bool EDPSimple::pairing_remote_writer_with_local_builtin_reader_after_security(const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data)
{
    bool returned_value = false;

    if(local_reader.entityId == sedp_builtin_publications_secure_reader)
    {
        sedp_builtin_publications_secure_reader_.first->matched_writer_add(remote_writer_data.toRemoteWriterAttributes());
        returned_value = true;
    }
    else if(local_reader.entityId == sedp_builtin_subscriptions_secure_reader)
    {
        sedp_builtin_subscriptions_secure_reader_.first->matched_writer_add(remote_writer_data.toRemoteWriterAttributes());
        returned_value = true;
    }

    return returned_value;
}

bool EDPSimple::pairing_remote_reader_with_local_builtin_writer_after_security(const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
{
    bool returned_value = false;

    if(local_writer.entityId == sedp_builtin_publications_secure_writer)
    {
        sedp_builtin_publications_secure_writer_.first->matched_reader_add(remote_reader_data.toRemoteReaderAttributes());
        returned_value = true;
    }
    else if(local_writer.entityId == sedp_builtin_subscriptions_secure_writer)
    {
        sedp_builtin_subscriptions_secure_writer_.first->matched_reader_add(remote_reader_data.toRemoteReaderAttributes());
        returned_value = true;
    }

    return returned_value;
}
#endif

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
