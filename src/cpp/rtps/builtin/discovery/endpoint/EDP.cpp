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
 * @file EDP.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/endpoint/EDP.h>

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>

#include "../../../participant/RTPSParticipantImpl.h"


#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/reader/ReaderListener.h>

#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/rtps/common/MatchingInfo.h>

#include <fastrtps/utils/StringMatching.h>
#include <fastrtps/log/Log.h>

#include <mutex>

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {


EDP::EDP(PDPSimple* p,RTPSParticipantImpl* part): mp_PDP(p),
    mp_RTPSParticipant(part) { }

EDP::~EDP()
{
    // TODO Auto-generated destructor stub
}

bool EDP::newLocalReaderProxyData(RTPSReader* reader, TopicAttributes& att, ReaderQos& rqos)
{
    logInfo(RTPS_EDP,"Adding " << reader->getGuid().entityId << " in topic "<<att.topicName);
    ReaderProxyData rpd;
    rpd.isAlive(true);
    rpd.m_expectsInlineQos = reader->expectsInlineQos();
    rpd.guid(reader->getGuid());
    rpd.key() = rpd.guid();
    rpd.multicastLocatorList(reader->getAttributes()->multicastLocatorList);
    rpd.unicastLocatorList(reader->getAttributes()->unicastLocatorList);
    rpd.RTPSParticipantKey() = mp_RTPSParticipant->getGuid();
    rpd.topicName(att.getTopicName());
    rpd.typeName(att.getTopicDataType());
    rpd.topicKind(att.getTopicKind());
    rpd.m_qos = rqos;
    rpd.userDefinedId(reader->getAttributes()->getUserDefinedID());
    reader->m_acceptMessagesFromUnkownWriters = false;

    //ADD IT TO THE LIST OF READERPROXYDATA
    ParticipantProxyData pdata;
    if(!this->mp_PDP->addReaderProxyData(&rpd, pdata))
    {
        return false;
    }

    //PAIRING
    pairing_reader_proxy_with_any_local_writer(&pdata, &rpd);
    pairingReader(reader, pdata, rpd);
    //DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
    processLocalReaderProxyData(reader, &rpd);
    return true;
}

bool EDP::newLocalWriterProxyData(RTPSWriter* writer,TopicAttributes& att, WriterQos& wqos)
{
    logInfo(RTPS_EDP,"Adding " << writer->getGuid().entityId << " in topic "<<att.topicName);
    WriterProxyData wpd;
    wpd.isAlive(true);
    wpd.guid(writer->getGuid());
    wpd.key() = wpd.guid();
    wpd.multicastLocatorList(writer->getAttributes()->multicastLocatorList);
    wpd.unicastLocatorList(writer->getAttributes()->unicastLocatorList);
    wpd.RTPSParticipantKey() = mp_RTPSParticipant->getGuid();
    wpd.topicName(att.getTopicName());
    wpd.typeName(att.getTopicDataType());
    wpd.topicKind(att.getTopicKind());
    wpd.typeMaxSerialized(writer->getTypeMaxSerialized());
    wpd.m_qos = wqos;
    wpd.userDefinedId(writer->getAttributes()->getUserDefinedID());
    wpd.persistence_guid(writer->getAttributes()->persistence_guid);

    //ADD IT TO THE LIST OF READERPROXYDATA
    ParticipantProxyData pdata;
    if(!this->mp_PDP->addWriterProxyData(&wpd, pdata))
    {
        return false;
    }

    //PAIRING
    pairing_writer_proxy_with_any_local_reader(&pdata, &wpd);
    pairingWriter(writer, pdata, wpd);
    //DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
    processLocalWriterProxyData(writer, &wpd);
    return true;
}

bool EDP::updatedLocalReader(RTPSReader* reader, ReaderQos& rqos)
{
    ParticipantProxyData pdata;
    ReaderProxyData rdata;
    rdata.m_qos.setQos(rqos, true);
    rdata.m_expectsInlineQos = reader->expectsInlineQos();

    if(this->mp_PDP->addReaderProxyData(&rdata, pdata))
    {
        processLocalReaderProxyData(reader, &rdata);
        //this->updatedReaderProxy(rdata);
        pairing_reader_proxy_with_any_local_writer(&pdata, &rdata);
        pairingReader(reader, pdata, rdata);
        return true;
    }
    return false;
}

bool EDP::updatedLocalWriter(RTPSWriter* writer, WriterQos& wqos)
{
    ParticipantProxyData pdata;
    WriterProxyData wdata;
    wdata.m_qos.setQos(wqos,true);

    if(this->mp_PDP->addWriterProxyData(&wdata, pdata))
    {
        processLocalWriterProxyData(writer, &wdata);
        //this->updatedWriterProxy(wdata);
        pairing_writer_proxy_with_any_local_reader(&pdata, &wdata);
        pairingWriter(writer, pdata, wdata);
        return true;
    }
    return false;
}

bool EDP::unpairWriterProxy(const GUID_t& participant_guid, const GUID_t& writer_guid)
{
    (void)participant_guid;

    logInfo(RTPS_EDP, writer_guid);

    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
            rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
    {
        RemoteWriterAttributes watt;
        watt.guid = writer_guid;
        if((*rit)->matched_writer_remove(watt))
        {
#if HAVE_SECURITY
            mp_RTPSParticipant->security_manager().remove_writer((*rit)->getGuid(), participant_guid, writer_guid);
#endif

            //MATCHED AND ADDED CORRECTLY:
            if((*rit)->getListener()!=nullptr)
            {
                MatchingInfo info;
                info.status = REMOVED_MATCHING;
                info.remoteEndpointGuid = writer_guid;
                (*rit)->getListener()->onReaderMatched((*rit),info);
            }
        }
    }
    return true;
}

bool EDP::unpairReaderProxy(const GUID_t& participant_guid, const GUID_t& reader_guid)
{
    (void)participant_guid;

    logInfo(RTPS_EDP, reader_guid);

    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
            wit!=mp_RTPSParticipant->userWritersListEnd();++wit)
    {
        RemoteReaderAttributes ratt;
        ratt.guid = reader_guid;
        if((*wit)->matched_reader_remove(ratt))
        {
#if HAVE_SECURITY
            mp_RTPSParticipant->security_manager().remove_reader((*wit)->getGuid(), participant_guid, reader_guid);
#endif
            //MATCHED AND ADDED CORRECTLY:
            if((*wit)->getListener()!=nullptr)
            {
                MatchingInfo info;
                info.status = REMOVED_MATCHING;
                info.remoteEndpointGuid = reader_guid;
                (*wit)->getListener()->onWriterMatched((*wit),info);
            }
        }
    }
    return true;
}


bool EDP::validMatching(const WriterProxyData* wdata, const ReaderProxyData* rdata)
{

    if(wdata->topicName() != rdata->topicName())
        return false;
    if(wdata->typeName() != rdata->typeName())
        return false;
    if(wdata->topicKind() != rdata->topicKind())
    {
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS:Remote Reader " << rdata->guid() << " is publishing in topic "
                << rdata->topicName() << "(keyed:"<< rdata->topicKind() <<
                "), local writer publishes as keyed: "<< wdata->topicKind())
            return false;
    }
    if(!rdata->isAlive()) //Matching
    {
        logWarning(RTPS_EDP,"ReaderProxyData object is NOT alive");
        return false;
    }
    if( wdata->m_qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS
            && rdata->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS) //Means our writer is BE but the reader wants RE
    {
        logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->topicName() <<"):Remote Reader "
                << rdata->guid() << " is Reliable and local writer is BE ");
        return false;
    }
    if(wdata->m_qos.m_durability.kind < rdata->m_qos.m_durability.kind)
    {
        // TODO (MCC) Change log message
        logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->topicName() <<"):RemoteReader "
                << rdata->guid() << " has TRANSIENT_LOCAL DURABILITY and we offer VOLATILE");
        return false;
    }
    if(wdata->m_qos.m_ownership.kind != rdata->m_qos.m_ownership.kind)
    {
        logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->topicName() <<"):Remote reader "
                << rdata->guid() << " has different Ownership Kind");
        return false;
    }
    //Partition check:
    bool matched = false;
    if(wdata->m_qos.m_partition.names.empty() && rdata->m_qos.m_partition.names.empty())
    {
        matched = true;
    }
    else if(wdata->m_qos.m_partition.names.empty() && rdata->m_qos.m_partition.names.size()>0)
    {
        for(std::vector<std::string>::const_iterator rnameit = rdata->m_qos.m_partition.names.begin();
                rnameit!=rdata->m_qos.m_partition.names.end();++rnameit)
        {
            if(rnameit->size()==0)
            {
                matched = true;
                break;
            }
        }
    }
    else if(wdata->m_qos.m_partition.names.size()>0 && rdata->m_qos.m_partition.names.empty() )
    {
        for(std::vector<std::string>::const_iterator wnameit = wdata->m_qos.m_partition.names.begin();
                wnameit !=  wdata->m_qos.m_partition.names.end();++wnameit)
        {
            if(wnameit->size()==0)
            {
                matched = true;
                break;
            }
        }
    }
    else
    {
        for(std::vector<std::string>::const_iterator wnameit = wdata->m_qos.m_partition.names.begin();
                wnameit !=  wdata->m_qos.m_partition.names.end();++wnameit)
        {
            for(std::vector<std::string>::const_iterator rnameit = rdata->m_qos.m_partition.names.begin();
                    rnameit!=rdata->m_qos.m_partition.names.end();++rnameit)
            {
                if(StringMatching::matchString(wnameit->c_str(),rnameit->c_str()))
                {
                    matched = true;
                    break;
                }
            }
            if(matched)
                break;
        }
    }
    if(!matched) //Different partitions
        logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->topicName() <<"): Different Partitions");
    return matched;
}

bool EDP::validMatching(const ReaderProxyData* rdata, const WriterProxyData* wdata)
{

    if(rdata->topicName() != wdata->topicName())
        return false;
    if( rdata->typeName() != wdata->typeName())
        return false;
    if(rdata->topicKind() != wdata->topicKind())
    {
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS:Remote Writer " << wdata->guid() << " is publishing in topic " << wdata->topicName() << "(keyed:" << wdata->topicKind() <<
                "), local reader subscribes as keyed: " << rdata->topicKind())
            return false;
    }
    if(!wdata->isAlive()) //Matching
    {
        logWarning(RTPS_EDP, "WriterProxyData " << wdata->guid() << " is NOT alive");
        return false;
    }
    if(rdata->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS
            && wdata->m_qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS) //Means our reader is reliable but hte writer is not
    {
        logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< wdata->topicName() << "): Remote Writer " << wdata->guid() << " is Best Effort and local reader is RELIABLE " << endl;);
        return false;
    }
    if(rdata->m_qos.m_durability.kind > wdata->m_qos.m_durability.kind)
    {
        // TODO (MCC) Change log message
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << wdata->topicName() << "):RemoteWriter " << wdata->guid() << " has VOLATILE DURABILITY and we want TRANSIENT_LOCAL" << endl;);
        return false;
    }
    if(rdata->m_qos.m_ownership.kind != wdata->m_qos.m_ownership.kind)
    {
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << wdata->topicName() << "):Remote Writer " << wdata->guid() << " has different Ownership Kind" << endl;);
        return false;
    }
    //Partition check:
    bool matched = false;
    if(rdata->m_qos.m_partition.names.empty() && wdata->m_qos.m_partition.names.empty())
    {
        matched = true;
    }
    else if(rdata->m_qos.m_partition.names.empty() && wdata->m_qos.m_partition.names.size()>0)
    {
        for(std::vector<std::string>::const_iterator rnameit = wdata->m_qos.m_partition.names.begin();
                rnameit!=wdata->m_qos.m_partition.names.end();++rnameit)
        {
            if(rnameit->size()==0)
            {
                matched = true;
                break;
            }
        }
    }
    else if(rdata->m_qos.m_partition.names.size()>0 && wdata->m_qos.m_partition.names.empty() )
    {
        for(std::vector<std::string>::const_iterator wnameit = rdata->m_qos.m_partition.names.begin();
                wnameit !=  rdata->m_qos.m_partition.names.end();++wnameit)
        {
            if(wnameit->size()==0)
            {
                matched = true;
                break;
            }
        }
    }
    else
    {
        for(std::vector<std::string>::const_iterator wnameit = rdata->m_qos.m_partition.names.begin();
                wnameit !=  rdata->m_qos.m_partition.names.end();++wnameit)
        {
            for(std::vector<std::string>::const_iterator rnameit = wdata->m_qos.m_partition.names.begin();
                    rnameit!=wdata->m_qos.m_partition.names.end();++rnameit)
            {
                if(StringMatching::matchString(wnameit->c_str(),rnameit->c_str()))
                {
                    matched = true;
                    break;
                }
            }
            if(matched)
                break;
        }
    }
    if(!matched) //Different partitions
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " <<  wdata->topicName() << "): Different Partitions");

    return matched;

}

//TODO Estas cuatro funciones comparten codigo comun (2 a 2) y se podrían seguramente combinar.

bool EDP::pairingReader(RTPSReader* R, const ParticipantProxyData& pdata, const ReaderProxyData& rdata)
{
    (void)pdata;

    logInfo(RTPS_EDP, rdata.guid() <<" in topic: \"" << rdata.topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());

    for(std::vector<ParticipantProxyData*>::const_iterator pit = mp_PDP->ParticipantProxiesBegin();
            pit!=mp_PDP->ParticipantProxiesEnd(); ++pit)
    {
        for(std::vector<WriterProxyData*>::iterator wdatait = (*pit)->m_writers.begin();
                wdatait != (*pit)->m_writers.end(); ++wdatait)
        {
            bool valid = validMatching(&rdata, *wdatait);

            if(valid)
            {
#if HAVE_SECURITY
                if(!mp_RTPSParticipant->security_manager().discovered_writer(R->m_guid, (*pit)->m_guid,
                            **wdatait, R->getAttributes()->security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for reader " << R->getGuid());
                }
#else
                if(R->matched_writer_add((*wdatait)->toRemoteWriterAttributes()))
                {
                    logInfo(RTPS_EDP, "Valid Matching to writerProxy: " << (*wdatait)->guid());
                    //MATCHED AND ADDED CORRECTLY:
                    if(R->getListener()!=nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = (*wdatait)->guid();
                        R->getListener()->onReaderMatched(R,info);
                    }
                }
#endif
            }
            else
            {
                //logInfo(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<(*wdatait)->m_guid<<RTPS_DEF<<endl);
                if(R->matched_writer_is_matched((*wdatait)->toRemoteWriterAttributes())
                        && R->matched_writer_remove((*wdatait)->toRemoteWriterAttributes()))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_writer(R->getGuid(), pdata.m_guid, (*wdatait)->guid());
#endif

                    //MATCHED AND ADDED CORRECTLY:
                    if(R->getListener()!=nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = (*wdatait)->guid();
                        R->getListener()->onReaderMatched(R,info);
                    }
                }
            }
        }
    }
    return true;
}

//TODO Añadir WriterProxyData como argumento con nullptr como valor por defecto.
bool EDP::pairingWriter(RTPSWriter* W, const ParticipantProxyData& pdata, const WriterProxyData& wdata)
{
    (void) pdata;

    logInfo(RTPS_EDP, W->getGuid() << " in topic: \"" << wdata.topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());

    for(std::vector<ParticipantProxyData*>::const_iterator pit = mp_PDP->ParticipantProxiesBegin();
            pit!=mp_PDP->ParticipantProxiesEnd(); ++pit)
    {
        for(std::vector<ReaderProxyData*>::iterator rdatait = (*pit)->m_readers.begin();
                rdatait!=(*pit)->m_readers.end(); ++rdatait)
        {
            bool valid = validMatching(&wdata, *rdatait);

            if(valid)
            {
#if HAVE_SECURITY
                if(!mp_RTPSParticipant->security_manager().discovered_reader(W->getGuid(), (*pit)->m_guid,
                            **rdatait, W->getAttributes()->security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for writer " << W->getGuid());
                }
#else
                if(W->matched_reader_add((*rdatait)->toRemoteReaderAttributes()))
                {
                    logInfo(RTPS_EDP,"Valid Matching to readerProxy: " << (*rdatait)->guid());
                    //MATCHED AND ADDED CORRECTLY:
                    if(W->getListener()!=nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = (*rdatait)->guid();
                        W->getListener()->onWriterMatched(W,info);
                    }
                }
#endif
            }
            else
            {
                //logInfo(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<(*wdatait)->m_guid<<RTPS_DEF<<endl);
                if(W->matched_reader_is_matched((*rdatait)->toRemoteReaderAttributes()) &&
                        W->matched_reader_remove((*rdatait)->toRemoteReaderAttributes()))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_reader(W->getGuid(), pdata.m_guid, (*rdatait)->guid());
#endif
                    //MATCHED AND ADDED CORRECTLY:
                    if(W->getListener()!=nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = (*rdatait)->guid();
                        W->getListener()->onWriterMatched(W,info);
                    }
                }
            }
        }
    }
    return true;
}

bool EDP::pairing_reader_proxy_with_any_local_writer(ParticipantProxyData* pdata, ReaderProxyData* rdata)
{
    (void)pdata;

    logInfo(RTPS_EDP, rdata->guid() <<" in topic: \"" << rdata->topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
            wit!=mp_RTPSParticipant->userWritersListEnd();++wit)
    {
        (*wit)->getMutex()->lock();
        GUID_t writerGUID = (*wit)->getGuid();
        (*wit)->getMutex()->unlock();
        ParticipantProxyData wpdata;
        WriterProxyData wdata;
        if(mp_PDP->lookupWriterProxyData(writerGUID, wdata, wpdata))
        {
            bool valid = validMatching(&wdata, rdata);

            if(valid)
            {
#if HAVE_SECURITY
                if(!mp_RTPSParticipant->security_manager().discovered_reader(writerGUID, pdata->m_guid,
                            *rdata, (*wit)->getAttributes()->security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for writer " << writerGUID);
                }
#else
                if((*wit)->matched_reader_add(rdata->toRemoteReaderAttributes()))
                {
                    logInfo(RTPS_EDP, "Valid Matching to local writer: " << writerGUID.entityId);
                    //MATCHED AND ADDED CORRECTLY:
                    if((*wit)->getListener()!=nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = rdata->guid();
                        (*wit)->getListener()->onWriterMatched((*wit),info);
                    }
                }
#endif
            }
            else
            {
                if((*wit)->matched_reader_is_matched(rdata->toRemoteReaderAttributes())
                        && (*wit)->matched_reader_remove(rdata->toRemoteReaderAttributes()))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_reader((*wit)->getGuid(), pdata->m_guid, rdata->guid());
#endif
                    //MATCHED AND ADDED CORRECTLY:
                    if((*wit)->getListener()!=nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = rdata->guid();
                        (*wit)->getListener()->onWriterMatched((*wit),info);
                    }
                }
            }
        }
    }

    return true;
}

#if HAVE_SECURITY
bool EDP::pairing_reader_proxy_with_local_writer(const GUID_t& local_writer, const GUID_t& remote_participant_guid,
        ReaderProxyData& rdata)
{
    logInfo(RTPS_EDP, rdata.guid() <<" in topic: \"" << rdata.topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
            wit!=mp_RTPSParticipant->userWritersListEnd();++wit)
    {
        (*wit)->getMutex()->lock();
        GUID_t writerGUID = (*wit)->getGuid();
        (*wit)->getMutex()->lock();

        if(local_writer == writerGUID)
        {
            ParticipantProxyData wpdata;
            WriterProxyData wdata;
            if(mp_PDP->lookupWriterProxyData(writerGUID, wdata, wpdata))
            {
                bool valid = validMatching(&wdata, &rdata);

                if(valid)
                {
                    if(!mp_RTPSParticipant->security_manager().discovered_reader(writerGUID,
                                remote_participant_guid, rdata, (*wit)->getAttributes()->security_attributes()))
                    {
                        logError(RTPS_EDP, "Security manager returns an error for writer " << writerGUID);
                    }
                }
                else
                {
                    if((*wit)->matched_reader_is_matched(rdata.toRemoteReaderAttributes())
                            && (*wit)->matched_reader_remove(rdata.toRemoteReaderAttributes()))
                    {
                        mp_RTPSParticipant->security_manager().remove_reader((*wit)->getGuid(),
                                remote_participant_guid, rdata.guid());
                        //MATCHED AND ADDED CORRECTLY:
                        if((*wit)->getListener()!=nullptr)
                        {
                            MatchingInfo info;
                            info.status = REMOVED_MATCHING;
                            info.remoteEndpointGuid = rdata.guid();
                            (*wit)->getListener()->onWriterMatched((*wit),info);
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool EDP::pairing_remote_reader_with_local_writer_after_security(const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
{
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
            wit!=mp_RTPSParticipant->userWritersListEnd();++wit)
    {
        (*wit)->getMutex()->lock();
        GUID_t writerGUID = (*wit)->getGuid();
        (*wit)->getMutex()->unlock();

        if(local_writer == writerGUID)
        {
            if((*wit)->matched_reader_add(remote_reader_data.toRemoteReaderAttributes()))
            {
                logInfo(RTPS_EDP, "Valid Matching to local writer: " << writerGUID.entityId);
                //MATCHED AND ADDED CORRECTLY:
                if((*wit)->getListener()!=nullptr)
                {
                    MatchingInfo info;
                    info.status = MATCHED_MATCHING;
                    info.remoteEndpointGuid = remote_reader_data.guid();
                    (*wit)->getListener()->onWriterMatched((*wit),info);
                }

                return true;
            }

            return false;
        }
    }

    return pairing_remote_reader_with_local_builtin_writer_after_security(local_writer, remote_reader_data);
}
#endif

bool EDP::pairing_writer_proxy_with_any_local_reader(ParticipantProxyData *pdata, WriterProxyData* wdata)
{
    (void)pdata;

    logInfo(RTPS_EDP, wdata->guid() <<" in topic: \"" << wdata->topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
            rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
    {
        GUID_t readerGUID;
        (*rit)->getMutex()->lock();
        readerGUID = (*rit)->getGuid();
        (*rit)->getMutex()->unlock();
        ParticipantProxyData rpdata;
        ReaderProxyData rdata;
        if(mp_PDP->lookupReaderProxyData(readerGUID, rdata, rpdata))
        {
            bool valid = validMatching(&rdata, wdata);

            if(valid)
            {
#if HAVE_SECURITY
                if(!mp_RTPSParticipant->security_manager().discovered_writer(readerGUID, pdata->m_guid,
                            *wdata, (*rit)->getAttributes()->security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for reader " << readerGUID);
                }
#else
                if((*rit)->matched_writer_add(wdata->toRemoteWriterAttributes()))
                {
                    logInfo(RTPS_EDP, "Valid Matching to local reader: " << readerGUID.entityId);
                    //MATCHED AND ADDED CORRECTLY:
                    if((*rit)->getListener()!=nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = wdata->guid();
                        (*rit)->getListener()->onReaderMatched((*rit),info);
                    }
                }
#endif
            }
            else
            {
                if((*rit)->matched_writer_is_matched(wdata->toRemoteWriterAttributes())
                        && (*rit)->matched_writer_remove(wdata->toRemoteWriterAttributes()))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_writer((*rit)->getGuid(), pdata->m_guid, wdata->guid());
#endif
                    //MATCHED AND ADDED CORRECTLY:
                    if((*rit)->getListener()!=nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = wdata->guid();
                        (*rit)->getListener()->onReaderMatched((*rit),info);
                    }
                }
            }
        }
    }
    return true;
}

#if HAVE_SECURITY
bool EDP::pairing_writer_proxy_with_local_reader(const GUID_t& local_reader, const GUID_t& remote_participant_guid,
        WriterProxyData& wdata)
{
    logInfo(RTPS_EDP, wdata.guid() <<" in topic: \"" << wdata.topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
            rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
    {
        GUID_t readerGUID;
        (*rit)->getMutex()->lock();
        readerGUID = (*rit)->getGuid();
        (*rit)->getMutex()->unlock();

        if(local_reader == readerGUID)
        {
            ParticipantProxyData rpdata;
            ReaderProxyData rdata;
            if(mp_PDP->lookupReaderProxyData(readerGUID, rdata, rpdata))
            {
                bool valid = validMatching(&rdata, &wdata);

                if(valid)
                {
                    if(!mp_RTPSParticipant->security_manager().discovered_writer(readerGUID,
                                remote_participant_guid, wdata, (*rit)->getAttributes()->security_attributes()))
                    {
                        logError(RTPS_EDP, "Security manager returns an error for reader " << readerGUID);
                    }
                }
                else
                {
                    if((*rit)->matched_writer_is_matched(wdata.toRemoteWriterAttributes())
                            && (*rit)->matched_writer_remove(wdata.toRemoteWriterAttributes()))
                    {
                        mp_RTPSParticipant->security_manager().remove_writer((*rit)->getGuid(),
                                remote_participant_guid, wdata.guid());
                        //MATCHED AND ADDED CORRECTLY:
                        if((*rit)->getListener()!=nullptr)
                        {
                            MatchingInfo info;
                            info.status = REMOVED_MATCHING;
                            info.remoteEndpointGuid = wdata.guid();
                            (*rit)->getListener()->onReaderMatched((*rit),info);
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool EDP::pairing_remote_writer_with_local_reader_after_security(const GUID_t& local_reader,
                const WriterProxyData& remote_writer_data)
{
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
            rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
    {
        GUID_t readerGUID;
        (*rit)->getMutex()->lock();
        readerGUID = (*rit)->getGuid();
        (*rit)->getMutex()->unlock();

        if(local_reader == readerGUID)
        {
            // TODO(richiware) Implement and use move with attributes
            if((*rit)->matched_writer_add(remote_writer_data.toRemoteWriterAttributes()))
            {
                logInfo(RTPS_EDP, "Valid Matching to local reader: " << readerGUID.entityId);
                //MATCHED AND ADDED CORRECTLY:
                if((*rit)->getListener()!=nullptr)
                {
                    MatchingInfo info;
                    info.status = MATCHED_MATCHING;
                    info.remoteEndpointGuid = remote_writer_data.guid();
                    (*rit)->getListener()->onReaderMatched((*rit),info);
                }

                return true;
            }

            return false;
        }
    }

    return pairing_remote_writer_with_local_builtin_reader_after_security(local_reader, remote_writer_data);
}
#endif

}
} /* namespace rtps */
} /* namespace eprosima */
