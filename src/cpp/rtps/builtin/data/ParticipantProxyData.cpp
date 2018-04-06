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
    mp_leaseDurationTimer(nullptr)
    {
        set_VendorId_Unknown(m_VendorId);
    }

ParticipantProxyData::ParticipantProxyData(const ParticipantProxyData& pdata) :
    m_protocolVersion(pdata.m_protocolVersion),
    m_guid(pdata.m_guid),
    m_expectsInlineQos(pdata.m_expectsInlineQos),
    m_availableBuiltinEndpoints(pdata.m_availableBuiltinEndpoints),
    m_metatrafficUnicastLocatorList(pdata.m_metatrafficUnicastLocatorList),
    m_metatrafficMulticastLocatorList(pdata.m_metatrafficMulticastLocatorList),
    m_defaultUnicastLocatorList(pdata.m_defaultUnicastLocatorList),
    m_defaultMulticastLocatorList(pdata.m_defaultMulticastLocatorList),
    m_manualLivelinessCount(pdata.m_manualLivelinessCount),
    m_participantName(pdata.m_participantName),
    m_key(pdata.m_key),
    m_leaseDuration(pdata.m_leaseDuration),
    identity_token_(pdata.identity_token_),
    permissions_token_(pdata.permissions_token_),
    isAlive(pdata.isAlive),
    m_properties(pdata.m_properties),
    m_userData(pdata.m_userData),
    mp_leaseDurationTimer(nullptr)
    {
        m_VendorId[0] = pdata.m_VendorId[0];
        m_VendorId[1] = pdata.m_VendorId[1];
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
    if(this->mp_leaseDurationTimer != nullptr)
        delete(mp_leaseDurationTimer);
}

ParameterList_t ParticipantProxyData::AllQostoParameterList()
{
    ParameterList_t parameter_list;

    {
        ParameterProtocolVersion_t* p = new ParameterProtocolVersion_t(PID_PROTOCOL_VERSION,4);
        p->protocolVersion = this->m_protocolVersion;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    {
        ParameterVendorId_t*p = new ParameterVendorId_t(PID_VENDORID,4);
        p->vendorId[0] = this->m_VendorId[0];
        p->vendorId[1] = this->m_VendorId[1];
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(this->m_expectsInlineQos)
    {
        ParameterBool_t * p = new ParameterBool_t(PID_EXPECTS_INLINE_QOS, PARAMETER_BOOL_LENGTH, m_expectsInlineQos);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    {
        ParameterGuid_t* p = new ParameterGuid_t(PID_PARTICIPANT_GUID, PARAMETER_GUID_LENGTH, m_guid);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    for(std::vector<Locator_t>::iterator it=this->m_metatrafficMulticastLocatorList.begin();
            it!=this->m_metatrafficMulticastLocatorList.end();++it)
    {
        ParameterLocator_t* p = new ParameterLocator_t(PID_METATRAFFIC_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, *it);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    for(std::vector<Locator_t>::iterator it=this->m_metatrafficUnicastLocatorList.begin();
            it!=this->m_metatrafficUnicastLocatorList.end();++it)
    {
        ParameterLocator_t* p = new ParameterLocator_t(PID_METATRAFFIC_UNICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, *it);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    for(std::vector<Locator_t>::iterator it=this->m_defaultUnicastLocatorList.begin();
            it!=this->m_defaultUnicastLocatorList.end();++it)
    {
        ParameterLocator_t* p = new ParameterLocator_t(PID_DEFAULT_UNICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, *it);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    for(std::vector<Locator_t>::iterator it=this->m_defaultMulticastLocatorList.begin();
            it!=this->m_defaultMulticastLocatorList.end();++it)
    {
        ParameterLocator_t* p = new ParameterLocator_t(PID_DEFAULT_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, *it);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    {
        ParameterTime_t* p = new ParameterTime_t(PID_PARTICIPANT_LEASE_DURATION, PARAMETER_TIME_LENGTH);
        p->time = m_leaseDuration;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    {
        ParameterBuiltinEndpointSet_t* p = new ParameterBuiltinEndpointSet_t(PID_BUILTIN_ENDPOINT_SET, PARAMETER_BUILTINENDPOINTSET_LENGTH);
        p->endpointSet = m_availableBuiltinEndpoints;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }

    if(m_participantName.size() > 0)
    {
        ParameterString_t* p = new ParameterString_t(PID_ENTITY_NAME, 0, m_participantName);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }

    if(this->m_userData.size()>0)
    {
        UserDataQosPolicy* p = new UserDataQosPolicy();
        p->setDataVec(m_userData);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }

    if(this->m_properties.properties.size()>0)
    {
        ParameterPropertyList_t* p = new ParameterPropertyList_t(m_properties);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }

    if(!this->identity_token_.class_id().empty())
    {
        ParameterToken_t* p = new ParameterToken_t(PID_IDENTITY_TOKEN, 0);
        p->token = identity_token_;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }


    if(!this->permissions_token_.class_id().empty())
    {
        ParameterToken_t* p = new ParameterToken_t(PID_PERMISSIONS_TOKEN, 0);
        p->token = permissions_token_;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }

    return parameter_list;
}

bool ParticipantProxyData::readFromCDRMessage(CDRMessage_t* msg)
{
    ParameterList_t parameter_list;

    if(ParameterList::readParameterListfromCDRMsg(msg, &parameter_list, NULL, true) > 0)
    {
        for(std::vector<Parameter_t*>::iterator it = parameter_list.m_parameters.begin();
                it!=parameter_list.m_parameters.end();++it)
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
                        case PID_PERMISSIONS_TOKEN:
                    {
                        ParameterToken_t* p = (ParameterToken_t*)(*it);
                        this->permissions_token_ = std::move(p->token);
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
        permissions_token_ = PermissionsToken();
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
        permissions_token_ = pdata.permissions_token_;
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
        permissions_token_ = pdata.permissions_token_;
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
