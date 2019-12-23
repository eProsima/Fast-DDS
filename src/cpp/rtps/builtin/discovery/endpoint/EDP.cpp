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

#include <fastrtps/rtps/builtin/discovery/participant/PDP.h>

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
using namespace eprosima::fastrtps::types;

namespace eprosima {
namespace fastrtps{
namespace rtps {


EDP::EDP(
        PDP* p,
        RTPSParticipantImpl* part)
    : mp_PDP(p)
    , mp_RTPSParticipant(part)
    , temp_reader_proxy_data_(
            part->getRTPSParticipantAttributes().allocation.locators.max_unicast_locators,
            part->getRTPSParticipantAttributes().allocation.locators.max_multicast_locators,
            part->getRTPSParticipantAttributes().allocation.data_limits)
    , temp_writer_proxy_data_(
        part->getRTPSParticipantAttributes().allocation.locators.max_unicast_locators,
        part->getRTPSParticipantAttributes().allocation.locators.max_multicast_locators,
        part->getRTPSParticipantAttributes().allocation.data_limits)
{
}

EDP::~EDP()
{
    // TODO Auto-generated destructor stub
}

bool EDP::newLocalReaderProxyData(
    RTPSReader* reader,
    const TopicAttributes& att,
    const ReaderQos& rqos)
{
    logInfo(RTPS_EDP,"Adding " << reader->getGuid().entityId << " in topic " << att.topicName);

    auto init_fun = [this, reader, &att, &rqos](
            ReaderProxyData* rpd,
            bool updating,
            const ParticipantProxyData& participant_data)
    {
        if (updating)
        {
            logError(RTPS_EDP, "Adding already existent reader " << reader->getGuid().entityId << " in topic "
                << att.topicName);
            return false;
        }

        const NetworkFactory& network = mp_RTPSParticipant->network_factory();
        rpd->isAlive(true);
        rpd->m_expectsInlineQos = reader->expectsInlineQos();
        rpd->guid(reader->getGuid());
        rpd->key() = rpd->guid();
        if (reader->getAttributes().multicastLocatorList.empty() &&
            reader->getAttributes().unicastLocatorList.empty())
        {
            rpd->set_locators(participant_data.default_locators);
        }
        else
        {
            rpd->set_multicast_locators(reader->getAttributes().multicastLocatorList, network);
            rpd->set_announced_unicast_locators(reader->getAttributes().unicastLocatorList);
        }
        rpd->RTPSParticipantKey() = mp_RTPSParticipant->getGuid();
        rpd->topicName(att.getTopicName());
        rpd->typeName(att.getTopicDataType());
        rpd->topicKind(att.getTopicKind());
        rpd->topicDiscoveryKind(att.getTopicDiscoveryKind());
        rpd->m_qos = rqos;
        rpd->userDefinedId(reader->getAttributes().getUserDefinedID());
#if HAVE_SECURITY
        if (mp_RTPSParticipant->is_secure())
        {
            rpd->security_attributes_ = reader->getAttributes().security_attributes().mask();
            rpd->plugin_security_attributes_ = reader->getAttributes().security_attributes().plugin_endpoint_attributes;
        }
        else
        {
            rpd->security_attributes_ = 0UL;
            rpd->plugin_security_attributes_ = 0UL;
        }
#endif
        if (att.getTopicDiscoveryKind() != NO_CHECK)
        {
            if (att.type_id.m_type_identifier._d() == 0) // Not set
            {
                const TypeIdentifier* type_id = TypeObjectFactory::get_instance()->get_type_identifier(
                    rpd->typeName().to_string(), att.getTopicDiscoveryKind() == COMPLETE);
                if (type_id == nullptr)
                {
                    logError(EDP, "TopicDiscoveryKind isn't NO_CHECK, but type identifier " << rpd->typeName()
                        << " isn't registered.");
                }
                else
                {
                    rpd->type_id().m_type_identifier = *type_id;
                }
            }
            else
            {
                rpd->type_id(att.type_id);
            }

            if (att.type.m_type_object._d() == 0
                && (att.type_id.m_type_identifier._d() == EK_MINIMAL
                    || att.type_id.m_type_identifier._d() == EK_COMPLETE)) // Not set
            {
                //*rpd.type().m_type_object = *TypeObjectFactory::get_instance()->get_type_object(rpd.typeName());
                const TypeObject *type_obj = TypeObjectFactory::get_instance()->get_type_object(
                    rpd->typeName().to_string(), att.getTopicDiscoveryKind() == COMPLETE);
                if (type_obj == nullptr)
                {
                    logError(EDP, "TopicDiscoveryKind isn't NO_CHECK, but type object " << rpd->typeName() <<
                        " isn't registered.");
                }
                else
                {
                    rpd->type().m_type_object = *type_obj;
                }
            }
            else
            {
                rpd->type(att.type);
            }
        }

        return true;
    };

    //ADD IT TO THE LIST OF READERPROXYDATA
    GUID_t participant_guid;
    ReaderProxyData* reader_data = this->mp_PDP->addReaderProxyData(reader->getGuid(), participant_guid, init_fun);
    if(reader_data == nullptr)
    {
        return false;
    }

    //PAIRING
    pairing_reader_proxy_with_any_local_writer(participant_guid, reader_data);
    pairingReader(reader, participant_guid, *reader_data);
    //DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
    processLocalReaderProxyData(reader, reader_data);
    return true;
}

bool EDP::newLocalWriterProxyData(
    RTPSWriter* writer,
    const TopicAttributes& att,
    const WriterQos& wqos)
{
    logInfo(RTPS_EDP,"Adding " << writer->getGuid().entityId << " in topic "<<att.topicName);

    auto init_fun = [this, writer, &att, &wqos](
        WriterProxyData* wpd,
        bool updating,
        const ParticipantProxyData& participant_data)
    {
        if (updating)
        {
            logError(RTPS_EDP, "Adding already existent writer " << writer->getGuid().entityId << " in topic "
                << att.topicName);
            return false;
        }

        const NetworkFactory& network = mp_RTPSParticipant->network_factory();
        wpd->guid(writer->getGuid());
        wpd->key() = wpd->guid();
        if (writer->getAttributes().multicastLocatorList.empty() &&
            writer->getAttributes().unicastLocatorList.empty())
        {
            wpd->set_locators(participant_data.default_locators);
        }
        else
        {
            wpd->set_multicast_locators(writer->getAttributes().multicastLocatorList, network);
            wpd->set_announced_unicast_locators(writer->getAttributes().unicastLocatorList);
        }
        wpd->RTPSParticipantKey() = mp_RTPSParticipant->getGuid();
        wpd->topicName(att.getTopicName());
        wpd->typeName(att.getTopicDataType());
        wpd->topicKind(att.getTopicKind());
        wpd->topicDiscoveryKind(att.getTopicDiscoveryKind());
        wpd->typeMaxSerialized(writer->getTypeMaxSerialized());
        wpd->m_qos = wqos;
        wpd->userDefinedId(writer->getAttributes().getUserDefinedID());
        wpd->persistence_guid(writer->getAttributes().persistence_guid);
#if HAVE_SECURITY
        if (mp_RTPSParticipant->is_secure())
        {
            wpd->security_attributes_ = writer->getAttributes().security_attributes().mask();
            wpd->plugin_security_attributes_ = writer->getAttributes().security_attributes().plugin_endpoint_attributes;
        }
        else
        {
            wpd->security_attributes_ = 0UL;
            wpd->plugin_security_attributes_ = 0UL;
        }
#endif

        if (att.getTopicDiscoveryKind() != NO_CHECK)
        {
            if (att.type_id.m_type_identifier._d() == 0) // Not set
            {
                const TypeIdentifier* type_id = TypeObjectFactory::get_instance()->get_type_identifier(
                    wpd->typeName().to_string(), att.getTopicDiscoveryKind() == COMPLETE);
                if (type_id == nullptr)
                {
                    logError(EDP, "TopicDiscoveryKind isn't NO_CHECK, but type identifier " << wpd->typeName()
                        << " isn't registered.");
                }
                else
                {
                    wpd->type_id().m_type_identifier = *type_id;
                }
            }
            else
            {
                wpd->type_id(att.type_id);
            }

            if (att.type.m_type_object._d() == 0
                && (att.type_id.m_type_identifier._d() == EK_MINIMAL
                    || att.type_id.m_type_identifier._d() == EK_COMPLETE)) // Not set
            {
                const TypeObject *type_obj = TypeObjectFactory::get_instance()->get_type_object(
                    wpd->typeName().to_string(), att.getTopicDiscoveryKind() == COMPLETE);
                if (type_obj == nullptr)
                {
                    logError(EDP, "TopicDiscoveryKind isn't NO_CHECK, but type object " << wpd->typeName()
                        << " isn't registered.");
                }
                else
                {
                    wpd->type().m_type_object = *type_obj;
                }
            }
            else
            {
                wpd->type(att.type);
            }
        }

        return true;
    };

    //ADD IT TO THE LIST OF READERPROXYDATA
    GUID_t participant_guid;
    WriterProxyData* writer_data = this->mp_PDP->addWriterProxyData(writer->getGuid(), participant_guid, init_fun);
    if(writer_data == nullptr)
    {
        return false;
    }

    //PAIRING
    pairing_writer_proxy_with_any_local_reader(participant_guid, writer_data);
    pairingWriter(writer, participant_guid, *writer_data);
    //DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
    processLocalWriterProxyData(writer, writer_data);
    return true;
}

bool EDP::updatedLocalReader(
    RTPSReader* reader,
    const TopicAttributes& att,
    const ReaderQos& rqos)
{
    (void)att;

    auto init_fun = [this, reader, &rqos](
            ReaderProxyData* rdata,
            bool updating,
            const ParticipantProxyData& participant_data)
    {
        // Should only be called for existent data
        (void)updating;
        assert(updating);

        const NetworkFactory& network = mp_RTPSParticipant->network_factory();
        if (reader->getAttributes().multicastLocatorList.empty() &&
            reader->getAttributes().unicastLocatorList.empty())
        {
            rdata->set_locators(participant_data.default_locators);
        }
        else
        {
            rdata->set_multicast_locators(reader->getAttributes().multicastLocatorList, network);
            rdata->set_announced_unicast_locators(reader->getAttributes().unicastLocatorList);
        }
        rdata->m_qos.setQos(rqos, false);
        rdata->isAlive(true);
        rdata->m_expectsInlineQos = reader->expectsInlineQos();
        return true;
    };

    GUID_t participant_guid;
    ReaderProxyData* reader_data = this->mp_PDP->addReaderProxyData(reader->getGuid(), participant_guid, init_fun);
    if(reader_data != nullptr)
    {
        processLocalReaderProxyData(reader, reader_data);
        pairing_reader_proxy_with_any_local_writer(participant_guid, reader_data);
        pairingReader(reader, participant_guid, *reader_data);
        return true;
    }
    return false;
}

bool EDP::updatedLocalWriter(
    RTPSWriter* writer,
    const TopicAttributes& att,
    const WriterQos& wqos)
{
    (void)att;

    auto init_fun = [this, writer, &wqos](
        WriterProxyData* wdata,
        bool updating,
        const ParticipantProxyData& participant_data)
    {
        // Should only be called for existent data
        (void)updating;
        assert(updating);

        const NetworkFactory& network = mp_RTPSParticipant->network_factory();
        if (writer->getAttributes().multicastLocatorList.empty() &&
            writer->getAttributes().unicastLocatorList.empty())
        {
            wdata->set_locators(participant_data.default_locators);
        }
        else
        {
            wdata->set_multicast_locators(writer->getAttributes().multicastLocatorList, network);
            wdata->set_announced_unicast_locators(writer->getAttributes().unicastLocatorList);
        }
        wdata->m_qos.setQos(wqos, false);
        return true;
    };

    GUID_t participant_guid;
    WriterProxyData* writer_data = this->mp_PDP->addWriterProxyData(writer->getGuid(), participant_guid, init_fun);
    if (writer_data != nullptr)
    {
        processLocalWriterProxyData(writer, writer_data);
        pairing_writer_proxy_with_any_local_reader(participant_guid, writer_data);
        pairingWriter(writer, participant_guid, *writer_data);
        return true;
    }
    return false;
}

bool EDP::unpairWriterProxy(
    const GUID_t& participant_guid,
    const GUID_t& writer_guid)
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
            mp_RTPSParticipant->security_manager().remove_writer((*rit)->getGuid(),
                participant_guid, writer_guid);
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

bool EDP::unpairReaderProxy(
    const GUID_t& participant_guid,
    const GUID_t& reader_guid)
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
            mp_RTPSParticipant->security_manager().remove_reader((*wit)->getGuid(),
                participant_guid, reader_guid);
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


bool EDP::validMatching(
    const WriterProxyData* wdata,
    const ReaderProxyData* rdata)
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
            && rdata->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS) 
        //Means our writer is BE but the reader wants RE
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
    if(wdata->m_qos.m_deadline.period > rdata->m_qos.m_deadline.period)
    {
        logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->topicName() <<"):Remote reader "
                << rdata->guid() << " has smaller DEADLINE period");
        return false;
    }
    if (!wdata->m_qos.m_disablePositiveACKs.enabled && rdata->m_qos.m_disablePositiveACKs.enabled)
    {
        logWarning(RTPS_EDP, "Incompatible Disable Positive Acks QoS: writer is enabled but reader is not");
        return false;
    }
    if (wdata->m_qos.m_liveliness.lease_duration > rdata->m_qos.m_liveliness.lease_duration)
     {
         logWarning(RTPS_EDP, "Incompatible liveliness lease durations: offered lease duration "
                    << wdata->m_qos.m_liveliness.lease_duration << " must be <= requested lease duration "
                    << rdata->m_qos.m_liveliness.lease_duration);
         return false;
     }
    if (wdata->m_qos.m_liveliness.kind < rdata->m_qos.m_liveliness.kind)
    {
        logWarning(RTPS_EDP, "Incompatible liveliness kinds: offered kind is < requested kind");
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

bool EDP::validMatching(
    const ReaderProxyData* rdata,
    const WriterProxyData* wdata)
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
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS:Remote Writer " << wdata->guid() << 
            " is publishing in topic " << wdata->topicName() << "(keyed:" << wdata->topicKind() <<
                "), local reader subscribes as keyed: " << rdata->topicKind())
            return false;
    }
    if(!checkTypeIdentifier(wdata, rdata))
    {
        logInfo(RTPS_EDP, "Matching failed on checkTypeIdentifier.");
        return false;
    }
    if(rdata->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS
            && wdata->m_qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS) 
        //Means our reader is reliable but hte writer is not
    {
        logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< wdata->topicName() << "): Remote Writer " 
            << wdata->guid() << " is Best Effort and local reader is RELIABLE " << endl;);
        return false;
    }
    if(rdata->m_qos.m_durability.kind > wdata->m_qos.m_durability.kind)
    {
        // TODO (MCC) Change log message
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << wdata->topicName() << "):RemoteWriter " 
            << wdata->guid() << " has VOLATILE DURABILITY and we want TRANSIENT_LOCAL" << endl;);
        return false;
    }
    if(rdata->m_qos.m_ownership.kind != wdata->m_qos.m_ownership.kind)
    {
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " << wdata->topicName() << "):Remote Writer "
            << wdata->guid() << " has different Ownership Kind" << endl;);
        return false;
    }
    if(rdata->m_qos.m_deadline.period < wdata->m_qos.m_deadline.period)
    {
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: "
                   << wdata->topicName() << "):RemoteWriter "
                   << wdata->guid() << "has smaller DEADLINE period");
        return false;
    }
    if (rdata->m_qos.m_disablePositiveACKs.enabled && !wdata->m_qos.m_disablePositiveACKs.enabled)
    {
        logWarning(RTPS_EDP, "Incompatible Disable Positive Acks QoS: writer is enabled but reader is not");
        return false;
    }
    if (wdata->m_qos.m_liveliness.lease_duration > rdata->m_qos.m_liveliness.lease_duration)
    {
        logWarning(RTPS_EDP, "Incompatible liveliness lease durations: offered lease duration "
                   << wdata->m_qos.m_liveliness.lease_duration << " must be <= requested lease duration "
                   << rdata->m_qos.m_liveliness.lease_duration);
        return false;
    }
    if (wdata->m_qos.m_liveliness.kind < rdata->m_qos.m_liveliness.kind)
    {
        logWarning(RTPS_EDP, "Incompatible liveliness kinds: offered kind is < than requested kind");
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
        logWarning(RTPS_EDP, "INCOMPATIBLE QOS (topic: " <<  wdata->topicName() <<
            "): Different Partitions");

    return matched;

}

//TODO Estas cuatro funciones comparten codigo comun (2 a 2) y se podrían seguramente combinar.

bool EDP::pairingReader(
        RTPSReader* R,
        const GUID_t& participant_guid,
        const ReaderProxyData& rdata)
{
    (void)participant_guid;

    logInfo(RTPS_EDP, rdata.guid() <<" in topic: \"" << rdata.topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());

    for(ResourceLimitedVector<ParticipantProxyData*>::const_iterator pit = mp_PDP->ParticipantProxiesBegin();
            pit!=mp_PDP->ParticipantProxiesEnd(); ++pit)
    {
        for(WriterProxyData* wdatait : (*pit)->m_writers)
        {
            bool valid = validMatching(&rdata, wdatait);

            if(valid)
            {
#if HAVE_SECURITY
                if(!mp_RTPSParticipant->security_manager().discovered_writer(R->m_guid, (*pit)->m_guid,
                            *wdatait, R->getAttributes().security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for reader " << R->getGuid());
                }
#else
                if(R->matched_writer_add(*wdatait))
                {
                    logInfo(RTPS_EDP, "Valid Matching to writerProxy: " << wdatait->guid());
                    //MATCHED AND ADDED CORRECTLY:
                    if(R->getListener()!=nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = wdatait->guid();
                        R->getListener()->onReaderMatched(R,info);
                    }
                }
#endif
            }
            else
            {
                //logInfo(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<wdatait->m_guid<<RTPS_DEF<<endl);
                if(R->matched_writer_is_matched(wdatait->guid())
                        && R->matched_writer_remove(wdatait->guid()))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_writer(R->getGuid(), participant_guid, wdatait->guid());
#endif

                    //MATCHED AND ADDED CORRECTLY:
                    if(R->getListener()!=nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = wdatait->guid();
                        R->getListener()->onReaderMatched(R,info);
                    }
                }
            }
        }
    }
    return true;
}

// TODO Add a WriterProxyData argument with a nullptr default value
bool EDP::pairingWriter(
        RTPSWriter* W,
        const GUID_t& participant_guid,
        const WriterProxyData& wdata)
{
    (void)participant_guid;

    logInfo(RTPS_EDP, W->getGuid() << " in topic: \"" << wdata.topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());

    for(ResourceLimitedVector<ParticipantProxyData*>::const_iterator pit = mp_PDP->ParticipantProxiesBegin();
            pit!=mp_PDP->ParticipantProxiesEnd(); ++pit)
    {
        for(ReaderProxyData* rdatait : (*pit)->m_readers)
        {
            GUID_t reader_guid = rdatait->guid();
            if (reader_guid == c_Guid_Unknown)
            {
                continue;
            }

            bool valid = validMatching(&wdata, rdatait);

            if(valid)
            {
#if HAVE_SECURITY
                if(!mp_RTPSParticipant->security_manager().discovered_reader(W->getGuid(), (*pit)->m_guid,
                            *rdatait, W->getAttributes().security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for writer " << W->getGuid());
                }
#else
				if(W->matched_reader_add(*rdatait))
                {
                    logInfo(RTPS_EDP,"Valid Matching to readerProxy: " << reader_guid);
                    //MATCHED AND ADDED CORRECTLY:
                    if(W->getListener()!=nullptr)
                    {
                        MatchingInfo info;
                        info.status = MATCHED_MATCHING;
                        info.remoteEndpointGuid = reader_guid;
                        W->getListener()->onWriterMatched(W,info);
                    }
                }
#endif
            }
            else
            {
                //logInfo(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<wdatait->m_guid<<RTPS_DEF<<endl);
                if(W->matched_reader_is_matched(reader_guid) && W->matched_reader_remove(reader_guid))
                {
#if HAVE_SECURITY
                    mp_RTPSParticipant->security_manager().remove_reader(W->getGuid(), participant_guid, reader_guid);
#endif
                    //MATCHED AND ADDED CORRECTLY:
                    if(W->getListener()!=nullptr)
                    {
                        MatchingInfo info;
                        info.status = REMOVED_MATCHING;
                        info.remoteEndpointGuid = reader_guid;
                        W->getListener()->onWriterMatched(W,info);
                    }
                }
            }
        }
    }
    return true;
}

bool EDP::pairing_reader_proxy_with_any_local_writer(
        const GUID_t& participant_guid,
        ReaderProxyData* rdata)
{
    (void)participant_guid;

    logInfo(RTPS_EDP, rdata->guid() <<" in topic: \"" << rdata->topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
            wit!=mp_RTPSParticipant->userWritersListEnd();++wit)
    {
        (*wit)->getMutex().lock();
        GUID_t writerGUID = (*wit)->getGuid();
        (*wit)->getMutex().unlock();
        if(mp_PDP->lookupWriterProxyData(writerGUID, temp_writer_proxy_data_))
        {
            bool valid = validMatching(&temp_writer_proxy_data_, rdata);

            if(valid)
            {
#if HAVE_SECURITY
                if(!mp_RTPSParticipant->security_manager().discovered_reader(writerGUID, participant_guid,
                            *rdata, (*wit)->getAttributes().security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for writer " << writerGUID);
                }
#else
                if((*wit)->matched_reader_add(*rdata))
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
bool EDP::pairing_reader_proxy_with_local_writer(
    const GUID_t& local_writer,
    const GUID_t& remote_participant_guid,
    ReaderProxyData& rdata)
{
    logInfo(RTPS_EDP, rdata.guid() <<" in topic: \"" << rdata.topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
            wit!=mp_RTPSParticipant->userWritersListEnd();++wit)
    {
        (*wit)->getMutex().lock();
        GUID_t writerGUID = (*wit)->getGuid();
        (*wit)->getMutex().unlock();

        if(local_writer == writerGUID)
        {
            if(mp_PDP->lookupWriterProxyData(writerGUID, temp_writer_proxy_data_))
            {
                bool valid = validMatching(&temp_writer_proxy_data_, &rdata);

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

bool EDP::pairing_remote_reader_with_local_writer_after_security(
    const GUID_t& local_writer,
    const ReaderProxyData& remote_reader_data)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
            wit!=mp_RTPSParticipant->userWritersListEnd();++wit)
    {
        (*wit)->getMutex().lock();
        GUID_t writerGUID = (*wit)->getGuid();
        (*wit)->getMutex().unlock();

        if(local_writer == writerGUID)
        {
            if((*wit)->matched_reader_add(remote_reader_data))
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

bool EDP::pairing_writer_proxy_with_any_local_reader(
        const GUID_t& participant_guid,
        WriterProxyData* wdata)
{
    (void)participant_guid;

    logInfo(RTPS_EDP, wdata->guid() <<" in topic: \"" << wdata->topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
            rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
    {
        GUID_t readerGUID;
        (*rit)->getMutex().lock();
        readerGUID = (*rit)->getGuid();
        (*rit)->getMutex().unlock();
        if(mp_PDP->lookupReaderProxyData(readerGUID, temp_reader_proxy_data_))
        {
            bool valid = validMatching(&temp_reader_proxy_data_, wdata);

            if(valid)
            {
#if HAVE_SECURITY
                if(!mp_RTPSParticipant->security_manager().discovered_writer(readerGUID, participant_guid,
                            *wdata, (*rit)->getAttributes().security_attributes()))
                {
                    logError(RTPS_EDP, "Security manager returns an error for reader " << readerGUID);
                }
#else
                if((*rit)->matched_writer_add(*wdata))
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
bool EDP::pairing_writer_proxy_with_local_reader(
    const GUID_t& local_reader,
    const GUID_t& remote_participant_guid,
    WriterProxyData& wdata)
{
    logInfo(RTPS_EDP, wdata.guid() <<" in topic: \"" << wdata.topicName() <<"\"");
    std::lock_guard<std::recursive_mutex> pguard(*mp_PDP->getMutex());
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
            rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
    {
        GUID_t readerGUID;
        (*rit)->getMutex().lock();
        readerGUID = (*rit)->getGuid();
        (*rit)->getMutex().unlock();

        if(local_reader == readerGUID)
        {
            if(mp_PDP->lookupReaderProxyData(readerGUID, temp_reader_proxy_data_))
            {
                bool valid = validMatching(&temp_reader_proxy_data_, &wdata);

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

bool EDP::pairing_remote_writer_with_local_reader_after_security(
    const GUID_t& local_reader,
    const WriterProxyData& remote_writer_data)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
    for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
            rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
    {
        GUID_t readerGUID;
        (*rit)->getMutex().lock();
        readerGUID = (*rit)->getGuid();
        (*rit)->getMutex().unlock();

        if(local_reader == readerGUID)
        {
            // TODO(richiware) Implement and use move with attributes
            if((*rit)->matched_writer_add(remote_writer_data))
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
bool EDP::checkTypeIdentifier(
    const WriterProxyData* wdata,
    const ReaderProxyData* rdata) 
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
