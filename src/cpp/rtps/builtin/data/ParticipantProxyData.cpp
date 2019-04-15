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
    m_protocolVersion(c_ProtocolVersion),
    m_VendorId(c_VendorId_Unknown),
    m_expectsInlineQos(false),
    m_availableBuiltinEndpoints(0),
    m_manualLivelinessCount(0),
#if HAVE_SECURITY
    security_attributes_(0UL),
    plugin_security_attributes_(0UL),
#endif
    isAlive(false),
    mp_leaseDurationTimer(nullptr)
    {
    }

ParticipantProxyData::ParticipantProxyData(const ParticipantProxyData& pdata) :
    m_protocolVersion(pdata.m_protocolVersion),
    m_guid(pdata.m_guid),
    m_VendorId(pdata.m_VendorId),
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
#if HAVE_SECURITY
    identity_token_(pdata.identity_token_),
    permissions_token_(pdata.permissions_token_),
    security_attributes_(pdata.security_attributes_),
    plugin_security_attributes_(pdata.plugin_security_attributes_),
#endif
    isAlive(pdata.isAlive),
    m_properties(pdata.m_properties),
    m_userData(pdata.m_userData),
    mp_leaseDurationTimer(nullptr)
    {
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

bool ParticipantProxyData::writeToCDRMessage(CDRMessage_t* msg, bool write_encapsulation)
{
    if (write_encapsulation)
    {
        if (!ParameterList::writeEncapsulationToCDRMsg(msg)) return false;
    }

    {
        ParameterProtocolVersion_t p(PID_PROTOCOL_VERSION,4);
        p.protocolVersion = this->m_protocolVersion;
        if (!p.addToCDRMessage(msg)) return false;
    }
    {
        ParameterVendorId_t p(PID_VENDORID,4);
        p.vendorId[0] = this->m_VendorId[0];
        p.vendorId[1] = this->m_VendorId[1];
        if (!p.addToCDRMessage(msg)) return false;
    }
    if(this->m_expectsInlineQos)
    {
        ParameterBool_t p(PID_EXPECTS_INLINE_QOS, PARAMETER_BOOL_LENGTH, m_expectsInlineQos);
        if (!p.addToCDRMessage(msg)) return false;
    }
    {
        ParameterGuid_t p(PID_PARTICIPANT_GUID, PARAMETER_GUID_LENGTH, m_guid);
        if (!p.addToCDRMessage(msg)) return false;
    }
    for(std::vector<Locator_t>::iterator it=this->m_metatrafficMulticastLocatorList.begin();
            it!=this->m_metatrafficMulticastLocatorList.end();++it)
    {
        ParameterLocator_t p(PID_METATRAFFIC_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, *it);
        if (!p.addToCDRMessage(msg)) return false;
    }
    for(std::vector<Locator_t>::iterator it=this->m_metatrafficUnicastLocatorList.begin();
            it!=this->m_metatrafficUnicastLocatorList.end();++it)
    {
        ParameterLocator_t p(PID_METATRAFFIC_UNICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, *it);
        if (!p.addToCDRMessage(msg)) return false;
    }
    for(std::vector<Locator_t>::iterator it=this->m_defaultUnicastLocatorList.begin();
            it!=this->m_defaultUnicastLocatorList.end();++it)
    {
        ParameterLocator_t p(PID_DEFAULT_UNICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, *it);
        if (!p.addToCDRMessage(msg)) return false;
    }
    for(std::vector<Locator_t>::iterator it=this->m_defaultMulticastLocatorList.begin();
            it!=this->m_defaultMulticastLocatorList.end();++it)
    {
        ParameterLocator_t p(PID_DEFAULT_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, *it);
        if (!p.addToCDRMessage(msg)) return false;
    }
    {
        ParameterTime_t p(PID_PARTICIPANT_LEASE_DURATION, PARAMETER_TIME_LENGTH);
        p.time = m_leaseDuration;
        if (!p.addToCDRMessage(msg)) return false;
    }
    {
        ParameterBuiltinEndpointSet_t p(PID_BUILTIN_ENDPOINT_SET, PARAMETER_BUILTINENDPOINTSET_LENGTH);
        p.endpointSet = m_availableBuiltinEndpoints;
        if (!p.addToCDRMessage(msg)) return false;
    }

    if(m_participantName.size() > 0)
    {
        ParameterString_t p(PID_ENTITY_NAME, 0, m_participantName);
        if (!p.addToCDRMessage(msg)) return false;
    }

    if(this->m_userData.size()>0)
    {
        UserDataQosPolicy p;
        p.setDataVec(m_userData);
        if (!p.addToCDRMessage(msg)) return false;
    }

    if(this->m_properties.properties.size()>0)
    {
        ParameterPropertyList_t p(m_properties);
        if (!p.addToCDRMessage(msg)) return false;
    }

#if HAVE_SECURITY
    if(!this->identity_token_.class_id().empty())
    {
        ParameterToken_t p(PID_IDENTITY_TOKEN, 0);
        p.token = identity_token_;
        if (!p.addToCDRMessage(msg)) return false;
    }

    if(!this->permissions_token_.class_id().empty())
    {
        ParameterToken_t p(PID_PERMISSIONS_TOKEN, 0);
        p.token = permissions_token_;
        if (!p.addToCDRMessage(msg)) return false;
    }

    if ((this->security_attributes_ != 0UL) || (this->plugin_security_attributes_ != 0UL))
    {
        ParameterParticipantSecurityInfo_t p;
        p.security_attributes = this->security_attributes_;
        p.plugin_security_attributes = this->plugin_security_attributes_;
        if (!p.addToCDRMessage(msg)) return false;
    }
#endif

    return CDRMessage::addParameterSentinel(msg);
}

bool ParticipantProxyData::readFromCDRMessage(CDRMessage_t* msg, bool use_encapsulation)
{
    auto param_process = [this](const Parameter_t* param)
    {
        switch (param->Pid)
        {
            case PID_KEY_HASH:
            {
                const ParameterKey_t* p = dynamic_cast<const ParameterKey_t*>(param);
                assert(p != nullptr);
                GUID_t guid;
                iHandle2GUID(guid, p->key);
                this->m_guid = guid;
                this->m_key = p->key;
                break;
            }
            case PID_PROTOCOL_VERSION:
            {
                const ParameterProtocolVersion_t* p = dynamic_cast<const ParameterProtocolVersion_t*>(param);
                assert(p != nullptr);
                if (p->protocolVersion.m_major < c_ProtocolVersion.m_major)
                {
                    return false;
                }
                this->m_protocolVersion = p->protocolVersion;
                break;
            }
            case PID_VENDORID:
            {
                const ParameterVendorId_t* p = dynamic_cast<const ParameterVendorId_t*>(param);
                assert(p != nullptr);
                this->m_VendorId[0] = p->vendorId[0];
                this->m_VendorId[1] = p->vendorId[1];
                break;
            }
            case PID_EXPECTS_INLINE_QOS:
            {
                const ParameterBool_t* p = dynamic_cast<const ParameterBool_t*>(param);
                assert(p != nullptr);
                this->m_expectsInlineQos = p->value;
                break;
            }
            case PID_PARTICIPANT_GUID:
            {
                const ParameterGuid_t* p = dynamic_cast<const ParameterGuid_t*>(param);
                assert(p != nullptr);
                this->m_guid = p->guid;
                this->m_key = p->guid;
                break;
            }
            case PID_METATRAFFIC_MULTICAST_LOCATOR:
            {
                const ParameterLocator_t* p = dynamic_cast<const ParameterLocator_t*>(param);
                assert(p != nullptr);
                this->m_metatrafficMulticastLocatorList.push_back(p->locator);
                break;
            }
            case PID_METATRAFFIC_UNICAST_LOCATOR:
            {
                const ParameterLocator_t* p = dynamic_cast<const ParameterLocator_t*>(param);
                assert(p != nullptr);
                this->m_metatrafficUnicastLocatorList.push_back(p->locator);
                break;
            }
            case PID_DEFAULT_UNICAST_LOCATOR:
            {
                const ParameterLocator_t* p = dynamic_cast<const ParameterLocator_t*>(param);
                assert(p != nullptr);
                this->m_defaultUnicastLocatorList.push_back(p->locator);
                break;
            }
            case PID_DEFAULT_MULTICAST_LOCATOR:
            {
                const ParameterLocator_t* p = dynamic_cast<const ParameterLocator_t*>(param);
                assert(p != nullptr);
                this->m_defaultMulticastLocatorList.push_back(p->locator);
                break;
            }
            case PID_PARTICIPANT_LEASE_DURATION:
            {
                const ParameterTime_t* p = dynamic_cast<const ParameterTime_t*>(param);
                assert(p != nullptr);
                this->m_leaseDuration = p->time.to_duration_t();
                break;
            }
            case PID_BUILTIN_ENDPOINT_SET:
            {
                const ParameterBuiltinEndpointSet_t* p = dynamic_cast<const ParameterBuiltinEndpointSet_t*>(param);
                assert(p != nullptr);
                this->m_availableBuiltinEndpoints = p->endpointSet;
                break;
            }
            case PID_ENTITY_NAME:
            {
                const ParameterString_t* p = dynamic_cast<const ParameterString_t*>(param);
                assert(p != nullptr);
                this->m_participantName = p->getName();
                break;
            }
            case PID_PROPERTY_LIST:
            {
                const ParameterPropertyList_t* p = dynamic_cast<const ParameterPropertyList_t*>(param);
                assert(p != nullptr);
                this->m_properties = *p;
                break;
            }
            case PID_USER_DATA:
            {
                const UserDataQosPolicy* p = dynamic_cast<const UserDataQosPolicy*>(param);
                assert(p != nullptr);
                this->m_userData = p->getDataVec();
                break;
            }
            case PID_IDENTITY_TOKEN:
            {
#if HAVE_SECURITY
                const ParameterToken_t* p = dynamic_cast<const ParameterToken_t*>(param);
                assert(p != nullptr);
                this->identity_token_ = std::move(p->token);
#else
                logWarning(RTPS_PARTICIPANT, "Received PID_IDENTITY_TOKEN but security is disabled");
#endif
                break;
            }
            case PID_PERMISSIONS_TOKEN:
            {
#if HAVE_SECURITY
                const ParameterToken_t* p = dynamic_cast<const ParameterToken_t*>(param);
                assert(p != nullptr);
                this->permissions_token_ = std::move(p->token);
#else
                logWarning(RTPS_PARTICIPANT, "Received PID_PERMISSIONS_TOKEN but security is disabled");
#endif
                break;
            }
            case PID_PARTICIPANT_SECURITY_INFO:
            {
#if HAVE_SECURITY
                const ParameterParticipantSecurityInfo_t* p =
                    dynamic_cast<const ParameterParticipantSecurityInfo_t*>(param);
                assert(p != nullptr);
                this->security_attributes_ = p->security_attributes;
                this->plugin_security_attributes_ = p->plugin_security_attributes;
#else
                logWarning(RTPS_PARTICIPANT, "Received PID_PARTICIPANT_SECURITY_INFO but security is disabled");
#endif
                break;
            }

            default: break;
        }

        return true;
    };

    uint32_t qos_size;
    return ParameterList::readParameterListfromCDRMsg(*msg, param_process, use_encapsulation, qos_size);
}


void ParticipantProxyData::clear()
{
    m_protocolVersion = ProtocolVersion_t();
    m_guid = GUID_t();
    //set_VendorId_Unknown(m_VendorId);
    m_VendorId = c_VendorId_Unknown;
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
#if HAVE_SECURITY
    identity_token_ = IdentityToken();
    permissions_token_ = PermissionsToken();
    security_attributes_ = 0UL;
    plugin_security_attributes_ = 0UL;
#endif
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
#if HAVE_SECURITY
    identity_token_ = pdata.identity_token_;
    permissions_token_ = pdata.permissions_token_;
    security_attributes_ = pdata.security_attributes_;
    plugin_security_attributes_ = pdata.plugin_security_attributes_;
#endif
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
#if HAVE_SECURITY
    identity_token_ = pdata.identity_token_;
    permissions_token_ = pdata.permissions_token_;
    security_attributes_ = pdata.security_attributes_;
    plugin_security_attributes_ = pdata.plugin_security_attributes_;
#endif
    if (this->mp_leaseDurationTimer != nullptr)
    {
        mp_leaseDurationTimer->cancel_timer();
        mp_leaseDurationTimer->update_interval(m_leaseDuration);
        mp_leaseDurationTimer->restart_timer();
    }
    return true;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
