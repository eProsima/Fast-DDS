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

#include <fastrtps/types/TypeObjectFactory.h>

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

bool EDP::newLocalReaderProxyData(RTPSReader* reader, const TopicAttributes& att, const ReaderQos& rqos)
{
    logInfo(RTPS_EDP,"Adding " << reader->getGuid().entityId << " in topic "<<att.topicName);
    ReaderProxyData rpd;
    rpd.isAlive(true);
    rpd.m_expectsInlineQos = reader->expectsInlineQos();
    rpd.guid(reader->getGuid());
    rpd.key() = rpd.guid();
    rpd.multicastLocatorList(reader->getAttributes().multicastLocatorList);
    rpd.unicastLocatorList(reader->getAttributes().unicastLocatorList);
    rpd.RTPSParticipantKey() = mp_RTPSParticipant->getGuid();
    rpd.topicName(att.getTopicName());
    rpd.typeName(att.getTopicDataType());
    rpd.topicKind(att.getTopicKind());
    rpd.topicDiscoveryKind(att.getTopicDiscoveryKind());
    rpd.m_qos = rqos;
    rpd.userDefinedId(reader->getAttributes().getUserDefinedID());
#if HAVE_SECURITY
    if (mp_RTPSParticipant->is_secure())
    {
        rpd.security_attributes_ = reader->getAttributes().security_attributes().mask();
        rpd.plugin_security_attributes_ = reader->getAttributes().security_attributes().plugin_endpoint_attributes;
    }
    else
    {
        rpd.security_attributes_ = 0UL;
        rpd.plugin_security_attributes_ = 0UL;
    }
#endif
    reader->m_acceptMessagesFromUnkownWriters = false;

    if (att.getTopicDiscoveryKind() != NO_CHECK)
    {
        if (att.type_id.m_type_identifier._d() == 0) // Not set
        {
            //*rpd.type_id().m_type_identifier = *TypeObjectFactory::GetInstance()->GetTypeIdentifier(rpd.typeName());
            const TypeIdentifier* type_id = TypeObjectFactory::GetInstance()->GetTypeIdentifier(
                    rpd.typeName().to_string(), att.getTopicDiscoveryKind() == COMPLETE);
            if (type_id == nullptr)
            {
                logError(EDP, "TopicDiscoveryKind isn't NO_CHECK, but type identifier " << rpd.typeName() << " isn't registered.");
            }
            else
            {
                rpd.type_id().m_type_identifier = *type_id;
            }
        }
        else
        {
            rpd.type_id(att.type_id);
        }

        if (att.type.m_type_object._d() == 0
            && (att.type_id.m_type_identifier._d() == EK_MINIMAL
                || att.type_id.m_type_identifier._d() == EK_COMPLETE)) // Not set
        {
            //*rpd.type().m_type_object = *TypeObjectFactory::GetInstance()->GetTypeObject(rpd.typeName());
            const TypeObject *type_obj = TypeObjectFactory::GetInstance()->GetTypeObject(
                    rpd.typeName().to_string(), att.getTopicDiscoveryKind() == COMPLETE);
            if (type_obj == nullptr)
            {
                logError(EDP, "TopicDiscoveryKind isn't NO_CHECK, but type object " << rpd.typeName() << " isn't registered.");
            }
            else
            {
                rpd.type().m_type_object = *type_obj;
            }
        }
        else
        {
            rpd.type(att.type);
        }
    }

    //ADD IT TO THE LIST OF READERPROXYDATA
    GUID_t participant_guid;
    if(!this->mp_PDP->addReaderProxyData(&rpd, participant_guid))
    {
        return false;
    }

    //PAIRING
    pairing_reader_proxy_with_any_local_writer(participant_guid, &rpd);
    pairingReader(reader, participant_guid, rpd);
    //DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
    processLocalReaderProxyData(reader, &rpd);
    return true;
}

bool EDP::newLocalWriterProxyData(RTPSWriter* writer, const TopicAttributes& att, const WriterQos& wqos)
{
    logInfo(RTPS_EDP,"Adding " << writer->getGuid().entityId << " in topic "<<att.topicName);
    WriterProxyData wpd;
    wpd.isAlive(true);
    wpd.guid(writer->getGuid());
    wpd.key() = wpd.guid();
    wpd.multicastLocatorList(writer->getAttributes().multicastLocatorList);
    wpd.unicastLocatorList(writer->getAttributes().unicastLocatorList);
    wpd.RTPSParticipantKey() = mp_RTPSParticipant->getGuid();
    wpd.topicName(att.getTopicName());
    wpd.typeName(att.getTopicDataType());
    wpd.topicKind(att.getTopicKind());
    wpd.topicDiscoveryKind(att.getTopicDiscoveryKind());
    wpd.typeMaxSerialized(writer->getTypeMaxSerialized());
    wpd.m_qos = wqos;
    wpd.userDefinedId(writer->getAttributes().getUserDefinedID());
    wpd.persistence_guid(writer->getAttributes().persistence_guid);
#if HAVE_SECURITY
    if (mp_RTPSParticipant->is_secure())
    {
        wpd.security_attributes_ = writer->getAttributes().security_attributes().mask();
        wpd.plugin_security_attributes_ = writer->getAttributes().security_attributes().plugin_endpoint_attributes;
    }
    else
    {
        wpd.security_attributes_ = 0UL;
        wpd.plugin_security_attributes_ = 0UL;
    }
#endif

    if (att.getTopicDiscoveryKind() != NO_CHECK)
    {
        if (att.type_id.m_type_identifier._d() == 0) // Not set
        {
            const TypeIdentifier* type_id = TypeObjectFactory::GetInstance()->GetTypeIdentifier(
                    wpd.typeName().to_string(), att.getTopicDiscoveryKind() == COMPLETE);
            if (type_id == nullptr)
            {
                logError(EDP, "TopicDiscoveryKind isn't NO_CHECK, but type identifier " << wpd.typeName() << " isn't registered.");
            }
            else
            {
                wpd.type_id().m_type_identifier = *type_id;
            }
        }
        else
        {
            wpd.type_id(att.type_id);
        }

        if (att.type.m_type_object._d() == 0
            && (att.type_id.m_type_identifier._d() == EK_MINIMAL
                || att.type_id.m_type_identifier._d() == EK_COMPLETE)) // Not set
        {
            const TypeObject *type_obj = TypeObjectFactory::GetInstance()->GetTypeObject(
                    wpd.typeName().to_string(), att.getTopicDiscoveryKind() == COMPLETE);
            if (type_obj == nullptr)
            {
                logError(EDP, "TopicDiscoveryKind isn't NO_CHECK, but type object " << wpd.typeName() << " isn't registered.");
            }
            else
            {
                wpd.type().m_type_object = *type_obj;
            }
        }
        else
        {
            wpd.type(att.type);
        }
    }

    //ADD IT TO THE LIST OF READERPROXYDATA
    GUID_t participant_guid;
    if(!this->mp_PDP->addWriterProxyData(&wpd, participant_guid))
    {
        return false;
    }

    //PAIRING
    pairing_writer_proxy_with_any_local_reader(participant_guid, &wpd);
    pairingWriter(writer, participant_guid, wpd);
    //DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
    processLocalWriterProxyData(writer, &wpd);
    return true;
}

bool EDP::updatedLocalReader(RTPSReader* reader, const TopicAttributes& att, const ReaderQos& rqos)
{
    GUID_t participant_guid;
    ReaderProxyData rdata;
    rdata.isAlive(true);
    rdata.m_expectsInlineQos = reader->expectsInlineQos();
    rdata.guid(reader->getGuid());
    rdata.key() = rdata.guid();
    rdata.multicastLocatorList(reader->getAttributes().multicastLocatorList);
    rdata.unicastLocatorList(reader->getAttributes().unicastLocatorList);
    rdata.RTPSParticipantKey() = mp_RTPSParticipant->getGuid();
    rdata.topicName(att.getTopicName());
    rdata.typeName(att.getTopicDataType());
    rdata.topicKind(att.getTopicKind());
    rdata.topicDiscoveryKind(att.getTopicDiscoveryKind());
    rdata.m_qos.setQos(rqos, true);
    rdata.userDefinedId(reader->getAttributes().getUserDefinedID());
#if HAVE_SECURITY
    if (mp_RTPSParticipant->is_secure())
    {
        rdata.security_attributes_ = reader->getAttributes().security_attributes().mask();
        rdata.plugin_security_attributes_ = reader->getAttributes().security_attributes().plugin_endpoint_attributes;
    }
    else
    {
        rdata.security_attributes_ = 0UL;
        rdata.plugin_security_attributes_ = 0UL;
    }
#endif

    if(this->mp_PDP->addReaderProxyData(&rdata, participant_guid))
    {
        processLocalReaderProxyData(reader, &rdata);
        //this->updatedReaderProxy(rdata);
        pairing_reader_proxy_with_any_local_writer(participant_guid, &rdata);
        pairingReader(reader, participant_guid, rdata);
        return true;
    }
    return false;
}

bool EDP::updatedLocalWriter(RTPSWriter* writer, const TopicAttributes& att, const WriterQos& wqos)
{
    GUID_t participant_guid;
    WriterProxyData wdata;
    wdata.isAlive(true);
    wdata.guid(writer->getGuid());
    wdata.key() = wdata.guid();
    wdata.multicastLocatorList(writer->getAttributes().multicastLocatorList);
    wdata.unicastLocatorList(writer->getAttributes().unicastLocatorList);
    wdata.RTPSParticipantKey() = mp_RTPSParticipant->getGuid();
    wdata.topicName(att.getTopicName());
    wdata.typeName(att.getTopicDataType());
    wdata.topicKind(att.getTopicKind());
    wdata.topicDiscoveryKind(att.getTopicDiscoveryKind());
    wdata.typeMaxSerialized(writer->getTypeMaxSerialized());
    wdata.m_qos.setQos(wqos,true);
    wdata.userDefinedId(writer->getAttributes().getUserDefinedID());
    wdata.persistence_guid(writer->getAttributes().persistence_guid);
#if HAVE_SECURITY
    if (mp_RTPSParticipant->is_secure())
    {
        wdata.security_attributes_ = writer->getAttributes().security_attributes().mask();
        wdata.plugin_security_attributes_ = writer->getAttributes().security_attributes().plugin_endpoint_attributes;
    }
    else
    {
        wdata.security_attributes_ = 0UL;
        wdata.plugin_security_attributes_ = 0UL;
    }
#endif

    if(this->mp_PDP->addWriterProxyData(&wdata, participant_guid))
    {
        processLocalWriterProxyData(writer, &wdata);
        //this->updatedWriterProxy(wdata);
        pairing_writer_proxy_with_any_local_reader(participant_guid, &wdata);
        pairingWriter(writer, participant_guid, wdata);
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
        if((*rit)->matched_writer_remove(writer_guid))
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
        if((*wit)->matched_reader_remove(reader_guid))
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
    if (wdata->topicName() != rdata->topicName())
    {
        return false;
    }
    if (wdata->typeName() != rdata->typeName())
    {
        return false;
    }
    if(wdata->topicKind() != rdata->topicKind())
    {
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS:Remote Reader " << rdata->guid() << " is publishing in topic "
                << rdata->topicName() << "(keyed:"<< rdata->topicKind() <<
                "), local writer publishes as keyed: "<< wdata->topicKind())
            return false;
    }
    if(!checkTypeIdentifier(wdata, rdata))
    {
        logInfo(RTPS_EDP, "Matching failed on checkTypeIdentifier.");
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

#if HAVE_SECURITY
    // TODO: Check EndpointSecurityInfo
#endif

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
    if (rdata->topicName() != wdata->topicName())
    {
        return false;
    }
    if (rdata->typeName() != wdata->typeName())
    {
        return false;
    }
    if(rdata->topicKind() != wdata->topicKind())
    {
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS:Remote Writer " << wdata->guid() << " is publishing in topic " << wdata->topicName() << "(keyed:" << wdata->topicKind() <<
                "), local reader subscribes as keyed: " << rdata->topicKind())
            return false;
    }
    if(!checkTypeIdentifier(wdata, rdata))
    {
        logInfo(RTPS_EDP, "Matching failed on checkTypeIdentifier.");
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
#if HAVE_SECURITY
    // TODO: Check EndpointSecurityInfo
#endif

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

bool EDP::pairingReader(RTPSReader* R, const GUID_t& participant_guid, const ReaderProxyData& rdata)
{
    (void)participant_guid;

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
                            **wdatait, R->getAttributes().security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for reader " << R->getGuid());
                }
#else
				RemoteWriterAttributes rwatt = (*wdatait)->toRemoteWriterAttributes();
                if(R->matched_writer_add(rwatt))
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
                if(R->matched_writer_is_matched((*wdatait)->guid())
                        && R->matched_writer_remove((*wdatait)->guid()))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_writer(R->getGuid(), participant_guid, (*wdatait)->guid());
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
bool EDP::pairingWriter(RTPSWriter* W, const GUID_t& participant_guid, const WriterProxyData& wdata)
{
    (void)participant_guid;

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
                            **rdatait, W->getAttributes().security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for writer " << W->getGuid());
                }
#else
				RemoteReaderAttributes rratt = (*rdatait)->toRemoteReaderAttributes();
				if(W->matched_reader_add(rratt))
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
                if(W->matched_reader_is_matched((*rdatait)->guid()) &&
                        W->matched_reader_remove((*rdatait)->guid()))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_reader(W->getGuid(), participant_guid, (*rdatait)->guid());
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

bool EDP::pairing_reader_proxy_with_any_local_writer(const GUID_t& participant_guid, ReaderProxyData* rdata)
{
    (void)participant_guid;

    logInfo(RTPS_EDP, rdata->guid() <<" in topic: \"" << rdata->topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
            wit!=mp_RTPSParticipant->userWritersListEnd();++wit)
    {
        (*wit)->getMutex()->lock();
        GUID_t writerGUID = (*wit)->getGuid();
        (*wit)->getMutex()->unlock();
        WriterProxyData wdata;
        if(mp_PDP->lookupWriterProxyData(writerGUID, wdata))
        {
            bool valid = validMatching(&wdata, rdata);

            if(valid)
            {
#if HAVE_SECURITY
                if(!mp_RTPSParticipant->security_manager().discovered_reader(writerGUID, participant_guid,
                            *rdata, (*wit)->getAttributes().security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for writer " << writerGUID);
                }
#else
                RemoteReaderAttributes rratt = rdata->toRemoteReaderAttributes();
                if((*wit)->matched_reader_add(rratt))
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
                if((*wit)->matched_reader_is_matched(rdata->guid())
                        && (*wit)->matched_reader_remove(rdata->guid()))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_reader((*wit)->getGuid(), participant_guid, rdata->guid());
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
        (*wit)->getMutex()->unlock();

        if(local_writer == writerGUID)
        {
            WriterProxyData wdata;
            if(mp_PDP->lookupWriterProxyData(writerGUID, wdata))
            {
                bool valid = validMatching(&wdata, &rdata);

                if(valid)
                {
                    if(!mp_RTPSParticipant->security_manager().discovered_reader(writerGUID,
                                remote_participant_guid, rdata, (*wit)->getAttributes().security_attributes()))
                    {
                        logError(RTPS_EDP, "Security manager returns an error for writer " << writerGUID);
                    }
                }
                else
                {
                    if((*wit)->matched_reader_is_matched(rdata.guid())
                            && (*wit)->matched_reader_remove(rdata.guid()))
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
            RemoteReaderAttributes rratt = remote_reader_data.toRemoteReaderAttributes();
            if((*wit)->matched_reader_add(rratt))
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

bool EDP::pairing_writer_proxy_with_any_local_reader(const GUID_t& participant_guid, WriterProxyData* wdata)
{
    (void)participant_guid;

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
        ReaderProxyData rdata;
        if(mp_PDP->lookupReaderProxyData(readerGUID, rdata))
        {
            bool valid = validMatching(&rdata, wdata);

            if(valid)
            {
#if HAVE_SECURITY
                if(!mp_RTPSParticipant->security_manager().discovered_writer(readerGUID, participant_guid,
                            *wdata, (*rit)->getAttributes().security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for reader " << readerGUID);
                }
#else
				RemoteWriterAttributes rwatt = wdata->toRemoteWriterAttributes();
                if((*rit)->matched_writer_add(rwatt))
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
                if((*rit)->matched_writer_is_matched(wdata->guid())
                        && (*rit)->matched_writer_remove(wdata->guid()))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_writer((*rit)->getGuid(), participant_guid, wdata->guid());
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
            ReaderProxyData rdata;
            if(mp_PDP->lookupReaderProxyData(readerGUID, rdata))
            {
                bool valid = validMatching(&rdata, &wdata);

                if(valid)
                {
                    if(!mp_RTPSParticipant->security_manager().discovered_writer(readerGUID,
                                remote_participant_guid, wdata, (*rit)->getAttributes().security_attributes()))
                    {
                        logError(RTPS_EDP, "Security manager returns an error for reader " << readerGUID);
                    }
                }
                else
                {
                    if((*rit)->matched_writer_is_matched(wdata.guid())
                            && (*rit)->matched_writer_remove(wdata.guid()))
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
            RemoteWriterAttributes rwatt = remote_writer_data.toRemoteWriterAttributes();
            if((*rit)->matched_writer_add(rwatt))
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
/*
bool EDP::checkTypeIdentifier(const TypeIdentifier * wti, const TypeIdentifier * rti) const
{
    if (wti->_d() != rti->_d())
    {
        return false;
    }

    switch (wti->_d())
    {
        case TI_STRING8_SMALL:
        case TI_STRING16_SMALL:
            return wti->string_sdefn().bound() == rti->string_sdefn().bound();

        case TI_STRING8_LARGE:
        case TI_STRING16_LARGE:
            return wti->string_ldefn().bound() == rti->string_ldefn().bound();

        case TI_PLAIN_SEQUENCE_SMALL:
            return wti->seq_sdefn().bound() == rti->seq_sdefn().bound()
                && checkTypeIdentifier(wti->seq_sdefn().element_identifier(), rti->seq_sdefn().element_identifier());

        case TI_PLAIN_SEQUENCE_LARGE:
            return wti->seq_ldefn().bound() == wti->seq_ldefn().bound()
                && checkTypeIdentifier(wti->seq_ldefn().element_identifier(), rti->seq_ldefn().element_identifier());

        case TI_PLAIN_ARRAY_SMALL:
            {
                if (wti->array_sdefn().array_bound_seq().size() != rti->array_sdefn().array_bound_seq().size())
                {
                    return false;
                }
                for (uint32_t idx = 0; idx < wti->array_sdefn().array_bound_seq().size(); ++idx)
                {
                    if (wti->array_sdefn().array_bound_seq()[idx] != rti->array_sdefn().array_bound_seq()[idx])
                    {
                        return false;
                    }
                }
                return checkTypeIdentifier(wti->array_sdefn().element_identifier(),
                                           rti->array_sdefn().element_identifier());
            }

        case TI_PLAIN_ARRAY_LARGE:
            {
                if (wti->array_ldefn().array_bound_seq().size() != rti->array_ldefn().array_bound_seq().size())
                {
                    return false;
                }
                for (uint32_t idx = 0; idx < wti->array_ldefn().array_bound_seq().size(); ++idx)
                {
                    if (wti->array_ldefn().array_bound_seq()[idx] != rti->array_ldefn().array_bound_seq()[idx])
                    {
                        return false;
                    }
                }
                return checkTypeIdentifier(wti->array_ldefn().element_identifier(),
                                           rti->array_ldefn().element_identifier());
            }

        case TI_PLAIN_MAP_SMALL:
            return wti->map_sdefn().bound() == wti->map_sdefn().bound()
                && checkTypeIdentifier(wti->map_sdefn().key_identifier(), rti->map_sdefn().key_identifier())
                && checkTypeIdentifier(wti->map_sdefn().element_identifier(), rti->map_sdefn().element_identifier());

        case TI_PLAIN_MAP_LARGE:
            return wti->map_ldefn().bound() == wti->map_ldefn().bound()
                && checkTypeIdentifier(wti->map_ldefn().key_identifier(), rti->map_ldefn().key_identifier())
                && checkTypeIdentifier(wti->map_ldefn().element_identifier(), rti->map_ldefn().element_identifier());

        case EK_MINIMAL:
        case EK_COMPLETE:
        {
            //return memcmp(wti->equivalence_hash(), rti->equivalence_hash(), 14) == 0;
            for (int i = 0; i < 14; ++i)
            {
                if (wti->equivalence_hash()[i] != rti->equivalence_hash()[i])
                {
                    return false;
                }
            }
            return true;
        }
        default:
            break;
    }
    return false;
}
*/
bool EDP::checkTypeIdentifier(const WriterProxyData* wdata, const ReaderProxyData* rdata) const
{
    if (wdata->topicDiscoveryKind() == NO_CHECK || rdata->topicDiscoveryKind() == NO_CHECK)
    {
        return true;
    }
    else if (wdata->topicDiscoveryKind() != rdata->topicDiscoveryKind())
    {
        logInfo(RTPS_EDP, "Matching failed due to DiscoveryKind mismatch.");
        return false;
    }

    return wdata->type_id().m_type_identifier == rdata->type_id().m_type_identifier;
}

}
} /* namespace rtps */
} /* namespace eprosima */
