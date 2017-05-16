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
 * @file ParticipantProxyData.cpp
 *
 */

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>

#include <fastrtps/rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h>
#include <fastrtps/rtps/builtin/BuiltinProtocols.h>

#include <rtps/participant/RTPSParticipantImpl.h>

#include <fastrtps/log/Log.h>

#include <fastrtps/qos/QosPolicies.h>

#include <mutex>

using namespace eprosima::fastrtps;


namespace eprosima {
namespace fastrtps{
namespace rtps {

ParticipantProxyData::ParticipantProxyData():
    m_expectsInlineQos(false),
    m_availableBuiltinEndpoints(0),
    m_manualLivelinessCount(0),
    isAlive(false),
    m_hasChanged(true),
    mp_leaseDurationTimer(nullptr),
    mp_mutex(new std::recursive_mutex())
    {
        set_VendorId_Unknown(m_VendorId);
    }

ParticipantProxyData::~ParticipantProxyData()
{
    logInfo(RTPS_PARTICIPANT,this->m_guid);
    for(std::vector<ReaderProxyData*>::iterator it = this->m_readers.begin();
            it!=this->m_readers.end();++it)
    {
        delete(*it);
    }
    for(std::vector<WriterProxyData*>::iterator it = this->m_writers.begin();
            it!=this->m_writers.end();++it)
    {
        delete(*it);
    }
    m_QosList.inlineQos.deleteParams();
    m_QosList.allQos.deleteParams();
    if(this->mp_leaseDurationTimer != nullptr)
        delete(mp_leaseDurationTimer);

    delete(mp_mutex);
}

bool ParticipantProxyData::initializeData(RTPSParticipantImpl* part,PDPSimple* pdp)
{
    this->m_leaseDuration = part->getAttributes().builtin.leaseDuration;
    set_VendorId_eProsima(this->m_VendorId);

    this->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    this->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    if(part->getAttributes().builtin.use_WriterLivelinessProtocol)
    {
        this->m_availableBuiltinEndpoints |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
        this->m_availableBuiltinEndpoints |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
    }
    if(part->getAttributes().builtin.use_SIMPLE_EndpointDiscoveryProtocol)
    {
        if(part->getAttributes().builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
        {
            this->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
            this->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
        }
        if(part->getAttributes().builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
        {
            this->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
            this->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
        }
    }

    this->m_defaultUnicastLocatorList = part->getAttributes().defaultUnicastLocatorList;
    // (Ricardo) Removed multicast by default in user endpoints.
    //this->m_defaultMulticastLocatorList = part->getAttributes().defaultMulticastLocatorList;
    this->m_expectsInlineQos = false;
    this->m_guid = part->getGuid();
    for(uint8_t i = 0; i<16; ++i)
    {
        if(i<12)
            this->m_key.value[i] = m_guid.guidPrefix.value[i];
        else
            this->m_key.value[i] = m_guid.entityId.value[i - 12];
    }


    this->m_metatrafficMulticastLocatorList = pdp->mp_builtin->m_metatrafficMulticastLocatorList;
    this->m_metatrafficUnicastLocatorList = pdp->mp_builtin->m_metatrafficUnicastLocatorList;

    this->m_participantName = std::string(part->getAttributes().getName());

    this->m_userData = part->getAttributes().userData;

    return true;
}

bool ParticipantProxyData::toParameterList()
{
    if(m_hasChanged)
    {
        m_QosList.allQos.deleteParams();
        m_QosList.allQos.resetList();
        m_QosList.inlineQos.deleteParams();
        m_QosList.inlineQos.resetList();
        bool valid = QosList::addQos(&m_QosList,PID_PROTOCOL_VERSION,this->m_protocolVersion);
        valid &=QosList::addQos(&m_QosList,PID_VENDORID,this->m_VendorId);
        if(this->m_expectsInlineQos)
            valid &=QosList::addQos(&m_QosList,PID_EXPECTS_INLINE_QOS,this->m_expectsInlineQos);
        valid &=QosList::addQos(&m_QosList,PID_PARTICIPANT_GUID,this->m_guid);
        for(std::vector<Locator_t>::iterator it=this->m_metatrafficMulticastLocatorList.begin();
                it!=this->m_metatrafficMulticastLocatorList.end();++it)
        {
            valid &=QosList::addQos(&m_QosList,PID_METATRAFFIC_MULTICAST_LOCATOR,*it);
        }
        for(std::vector<Locator_t>::iterator it=this->m_metatrafficUnicastLocatorList.begin();
                it!=this->m_metatrafficUnicastLocatorList.end();++it)
        {
            valid &=QosList::addQos(&m_QosList,PID_METATRAFFIC_UNICAST_LOCATOR,*it);
        }
        for(std::vector<Locator_t>::iterator it=this->m_defaultUnicastLocatorList.begin();
                it!=this->m_defaultUnicastLocatorList.end();++it)
        {
            valid &=QosList::addQos(&m_QosList,PID_DEFAULT_UNICAST_LOCATOR,*it);
        }
        for(std::vector<Locator_t>::iterator it=this->m_defaultMulticastLocatorList.begin();
                it!=this->m_defaultMulticastLocatorList.end();++it)
        {
            valid &=QosList::addQos(&m_QosList,PID_DEFAULT_MULTICAST_LOCATOR,*it);
        }
        valid &=QosList::addQos(&m_QosList,PID_PARTICIPANT_LEASE_DURATION,this->m_leaseDuration);
        valid &=QosList::addQos(&m_QosList,PID_BUILTIN_ENDPOINT_SET,(uint32_t)this->m_availableBuiltinEndpoints);
        valid &=QosList::addQos(&m_QosList,PID_ENTITY_NAME,this->m_participantName);

        if(this->m_userData.size()>0)
            valid &=QosList::addQos(&m_QosList,PID_USER_DATA,this->m_userData);

        if(this->m_properties.properties.size()>0)
            valid &= QosList::addQos(&m_QosList,PID_PROPERTY_LIST,this->m_properties);

        if(!this->identity_token_.class_id().empty())
            valid &= QosList::addQos(&m_QosList,PID_IDENTITY_TOKEN,this->identity_token_);

        //FIXME: ADD STATIC INFO.
        //		if(this.use_STATIC_EndpointDiscoveryProtocol)
        //			valid&= this->addStaticEDPInfo();

        if(valid)
            m_hasChanged = false;
        return valid;
    }
    return true;
}

bool ParticipantProxyData::readFromCDRMessage(CDRMessage_t* msg)
{
    if(ParameterList::readParameterListfromCDRMsg(msg, &m_QosList.allQos, NULL, true) > 0)
    {
        for(std::vector<Parameter_t*>::iterator it = m_QosList.allQos.m_parameters.begin();
                it!=m_QosList.allQos.m_parameters.end();++it)
        {
            switch((*it)->Pid)
            {
                case PID_KEY_HASH:
                    {
                        ParameterKey_t*p = (ParameterKey_t*)(*it);
                        GUID_t guid;
                        iHandle2GUID(guid,p->key);
                        this->m_guid = guid;
                        this->m_key = p->key;
                        break;
                    }
                case PID_PROTOCOL_VERSION:
                    {
                        ParameterProtocolVersion_t * p = (ParameterProtocolVersion_t*)(*it);
                        if(p->protocolVersion.m_major < c_ProtocolVersion.m_major)
                        {
                            return false;
                        }
                        this->m_protocolVersion = p->protocolVersion;
                        break;
                    }
                case PID_VENDORID:
                    {
                        ParameterVendorId_t * p = (ParameterVendorId_t*)(*it);
                        this->m_VendorId[0] = p->vendorId[0];
                        this->m_VendorId[1] = p->vendorId[1];
                        break;
                    }
                case PID_EXPECTS_INLINE_QOS:
                    {
                        ParameterBool_t * p = (ParameterBool_t*)(*it);
                        this->m_expectsInlineQos = p->value;
                        break;
                    }
                case PID_PARTICIPANT_GUID:
                    {
                        ParameterGuid_t * p = (ParameterGuid_t*)(*it);
                        this->m_guid = p->guid;
                        this->m_key = p->guid;
                        break;
                    }
                case PID_METATRAFFIC_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t* p = (ParameterLocator_t*)(*it);
                        this->m_metatrafficMulticastLocatorList.push_back(p->locator);
                        break;
                    }
                case PID_METATRAFFIC_UNICAST_LOCATOR:
                    {
                        ParameterLocator_t* p = (ParameterLocator_t*)(*it);
                        this->m_metatrafficUnicastLocatorList.push_back(p->locator);
                        break;
                    }
                case PID_DEFAULT_UNICAST_LOCATOR:
                    {
                        ParameterLocator_t* p = (ParameterLocator_t*)(*it);
                        this->m_defaultUnicastLocatorList.push_back(p->locator);
                        break;
                    }
                case PID_DEFAULT_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t* p = (ParameterLocator_t*)(*it);
                        this->m_defaultMulticastLocatorList.push_back(p->locator);
                        break;
                    }
                case PID_PARTICIPANT_LEASE_DURATION:
                    {
                        ParameterTime_t* p = (ParameterTime_t*)(*it);
                        this->m_leaseDuration = p->time;
                        break;
                    }
                case PID_BUILTIN_ENDPOINT_SET:
                    {
                        ParameterBuiltinEndpointSet_t* p = (ParameterBuiltinEndpointSet_t*)(*it);
                        this->m_availableBuiltinEndpoints = p->endpointSet;
                        break;
                    }
                case PID_ENTITY_NAME:
                    {

                        ParameterString_t* p = (ParameterString_t*)(*it);
                        //cout << "ENTITY NAME " << p->m_string<<endl;
                        this->m_participantName = std::string(p->getName());
                        break;
                    }
                case PID_PROPERTY_LIST:
                    {
                        ParameterPropertyList_t*p = (ParameterPropertyList_t*)(*it);
                        this->m_properties = *p;
                        break;
                        //FIXME: STATIC EDP. IN ASSIGN REMOTE ENDPOINTS
                        //				if(mp_SPDP->m_discovery.use_STATIC_EndpointDiscoveryProtocol)
                        //				{
                        //					ParameterPropertyList_t* p = (ParameterPropertyList_t*)(*it);
                        //					uint16_t userId;
                        //					EntityId_t entityId;
                        //					for(std::vector<std::pair<std::string,std::string>>::iterator it = p->properties.begin();
                        //							it != p->properties.end();++it)
                        //					{
                        //						std::stringstream ss;
                        //						std::string first_str = it->first.substr(0, it->first.find("_"));
                        //						if(first_str == "staticedp")
                        //						{
                        //							std::string type = it->first.substr(10, it->first.find("_"));
                        //							first_str = it->first.substr(17, it->first.find("_"));
                        //							ss.clear();	ss.str(std::string());
                        //							ss << first_str;ss >> userId;
                        //							ss.clear();ss.str(std::string());
                        //							ss << it->second;
                        //							int a,b,c,d;
                        //							char ch;
                        //							ss >> a >> ch >> b >> ch >> c >>ch >> d;
                        //							entityId.value[0] = (octet)a;entityId.value[1] = (octet)b;
                        //							entityId.value[2] = (octet)c;entityId.value[3] = (octet)d;
                        //							assignUserId(type,userId,entityId,Pdata);
                        //						}
                        //					}
                    }
                        case PID_USER_DATA:
                    {
                        UserDataQosPolicy*p = (UserDataQosPolicy*)(*it);
                        this->m_userData = p->getDataVec();
                        break;
                    }
                        case PID_IDENTITY_TOKEN:
                    {
                        ParameterToken_t* p = (ParameterToken_t*)(*it);
                        this->identity_token_ = std::move(p->token);
                        break;
                    }
                        default: break;
                    }
            }
            return true;
        }

        return false;
    }


    void ParticipantProxyData::clear()
    {
        m_protocolVersion = ProtocolVersion_t();
        m_guid = GUID_t();
        set_VendorId_Unknown(m_VendorId);
        m_expectsInlineQos = false;
        m_availableBuiltinEndpoints = 0;
        m_metatrafficUnicastLocatorList.clear();
        m_metatrafficMulticastLocatorList.clear();
        m_defaultUnicastLocatorList.clear();
        m_defaultMulticastLocatorList.clear();
        m_manualLivelinessCount = 0;
        m_participantName = "";
        m_key = InstanceHandle_t();
        m_leaseDuration = Duration_t();
        isAlive = true;
        identity_token_ = IdentityToken();
        m_QosList.allQos.deleteParams();
        m_QosList.allQos.resetList();
        m_QosList.inlineQos.resetList();
        m_properties.properties.clear();
        m_properties.length = 0;
        m_userData.clear();
    }

    void ParticipantProxyData::copy(ParticipantProxyData& pdata)
    {
        m_protocolVersion = pdata.m_protocolVersion;
        m_guid = pdata.m_guid;
        m_VendorId[0] = pdata.m_VendorId[0];
        m_VendorId[1] = pdata.m_VendorId[1];
        m_availableBuiltinEndpoints = pdata.m_availableBuiltinEndpoints;
        m_metatrafficUnicastLocatorList = pdata.m_metatrafficUnicastLocatorList;
        m_metatrafficMulticastLocatorList = pdata.m_metatrafficMulticastLocatorList;
        m_defaultUnicastLocatorList = pdata.m_defaultUnicastLocatorList;
        m_defaultMulticastLocatorList = pdata.m_defaultMulticastLocatorList;
        m_manualLivelinessCount = pdata.m_manualLivelinessCount;
        m_participantName = pdata.m_participantName;
        m_leaseDuration = pdata.m_leaseDuration;
        m_key = pdata.m_key;
        isAlive = pdata.isAlive;
        m_properties = pdata.m_properties;
        m_userData = pdata.m_userData;
        identity_token_ = pdata.identity_token_;
    }

    bool ParticipantProxyData::updateData(ParticipantProxyData& pdata)
    {
        m_metatrafficUnicastLocatorList = pdata.m_metatrafficUnicastLocatorList;
        m_metatrafficMulticastLocatorList = pdata.m_metatrafficMulticastLocatorList;
        m_defaultUnicastLocatorList = pdata.m_defaultUnicastLocatorList;
        m_defaultMulticastLocatorList = pdata.m_defaultMulticastLocatorList;
        m_manualLivelinessCount = pdata.m_manualLivelinessCount;
        m_properties = pdata.m_properties;
        m_leaseDuration = pdata.m_leaseDuration;
        m_userData = pdata.m_userData;
        isAlive = true;
        identity_token_ = pdata.identity_token_;
        if(this->mp_leaseDurationTimer != nullptr)
        {
            mp_leaseDurationTimer->cancel_timer();
            mp_leaseDurationTimer->update_interval(m_leaseDuration);
            mp_leaseDurationTimer->restart_timer();
        }
        return true;
    }

} /* namespace rtps */
} /* namespace eprosima */
}
