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

#include <fastrtps/rtps/network/NetworkFactory.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/rtps/resources/TimedEvent.h>
#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/utils/TimeConversion.h>

#include <mutex>
#include <chrono>

using namespace eprosima::fastrtps;


namespace eprosima {
namespace fastrtps{
namespace rtps {


ParticipantProxyData::ParticipantProxyData(const RTPSParticipantAllocationAttributes& allocation)
    : m_protocolVersion(c_ProtocolVersion)
    , m_VendorId(c_VendorId_Unknown)
    , m_expectsInlineQos(false)
    , m_availableBuiltinEndpoints(0)
    , metatraffic_locators(allocation.locators.max_unicast_locators, allocation.locators.max_multicast_locators)
    , default_locators(allocation.locators.max_unicast_locators, allocation.locators.max_multicast_locators)
#if HAVE_SECURITY
    , security_attributes_(0UL)
    , plugin_security_attributes_(0UL)
#endif
    , isAlive(false)
    , m_properties((uint32_t)allocation.data_limits.max_properties)
    , lease_duration_event(nullptr)
    , should_check_lease_duration(false)
    , m_readers(allocation.readers)
    , m_writers(allocation.writers)
    {
        m_userData.max_size((uint32_t)allocation.data_limits.max_user_data);
    }

ParticipantProxyData::ParticipantProxyData(const ParticipantProxyData& pdata)
    : m_protocolVersion(pdata.m_protocolVersion)
    , m_guid(pdata.m_guid)
    , m_VendorId(pdata.m_VendorId)
    , m_expectsInlineQos(pdata.m_expectsInlineQos)
    , m_availableBuiltinEndpoints(pdata.m_availableBuiltinEndpoints)
    , metatraffic_locators(pdata.metatraffic_locators)
    , default_locators(pdata.default_locators)
    , m_participantName(pdata.m_participantName)
    , m_key(pdata.m_key)
    , m_leaseDuration(pdata.m_leaseDuration)
#if HAVE_SECURITY
    , identity_token_(pdata.identity_token_)
    , permissions_token_(pdata.permissions_token_)
    , security_attributes_(pdata.security_attributes_)
    , plugin_security_attributes_(pdata.plugin_security_attributes_)
#endif
    , isAlive(pdata.isAlive)
    , m_properties(pdata.m_properties)
    , m_userData(pdata.m_userData)
    , lease_duration_event(nullptr)
    , should_check_lease_duration(false)
    , lease_duration_(std::chrono::microseconds(TimeConv::Duration_t2MicroSecondsInt64(pdata.m_leaseDuration)))

    // This method is only called from SecurityManager when a new participant is discovered and the
    // corresponding DiscoveredParticipantInfo struct is created. Only participant info is used,
    // so there is no need to copy m_readers and m_writers
    {
    }

ParticipantProxyData::~ParticipantProxyData()
{
    logInfo(RTPS_PARTICIPANT, m_guid);

    for (ReaderProxyData* it : m_readers)
    {
        delete it;
    }

    for (WriterProxyData* it : m_writers)
    {
        delete it;
    }

    if (lease_duration_event != nullptr)
    {
        delete lease_duration_event;
    }
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
    for(const Locator_t& it : metatraffic_locators.multicast)
    {
        ParameterLocator_t p(PID_METATRAFFIC_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, it);
        if (!p.addToCDRMessage(msg)) return false;
    }
    for(const Locator_t& it : metatraffic_locators.unicast)
    {
        ParameterLocator_t p(PID_METATRAFFIC_UNICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, it);
        if (!p.addToCDRMessage(msg)) return false;
    }
    for(const Locator_t& it : default_locators.unicast)
    {
        ParameterLocator_t p(PID_DEFAULT_UNICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, it);
        if (!p.addToCDRMessage(msg)) return false;
    }
    for(const Locator_t& it : default_locators.multicast)
    {
        ParameterLocator_t p(PID_DEFAULT_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, it);
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

    if(this->m_userData.dataVec().size()>0)
    {
        if (!m_userData.addToCDRMessage(msg)) return false;
    }

    if(this->m_properties.size()>0)
    {
        if (!m_properties.addToCDRMessage(msg)) return false;
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


bool ParticipantProxyData::readFromCDRMessage(
        CDRMessage_t* msg,
        bool use_encapsulation,
        const NetworkFactory& network)
{
    auto param_process = [this, &network](CDRMessage_t* msg, const ParameterId_t &pid, uint16_t plength)
        {
            try
            {
                switch (pid)
                {
                    case PID_DEFAULT_UNICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        Locator_t temp_locator;
                        if (network.transform_remote_locator(p.locator, temp_locator))
                        {
                            default_locators.add_unicast_locator(temp_locator);
                        }
                        break;
                    }
                    case PID_METATRAFFIC_UNICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        Locator_t temp_locator;
                        if (network.transform_remote_locator(p.locator, temp_locator))
                        {
                            metatraffic_locators.add_unicast_locator(temp_locator);
                        }
                        break;
                    }
                    case PID_DEFAULT_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        Locator_t temp_locator;
                        if (network.transform_remote_locator(p.locator, temp_locator))
                        {
                            default_locators.add_multicast_locator(temp_locator);
                        }
                        break;
                    }
                    case PID_METATRAFFIC_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        Locator_t temp_locator;
                        if (network.transform_remote_locator(p.locator, temp_locator))
                        {
                            metatraffic_locators.add_multicast_locator(temp_locator);
                        }
                        break;
                    }
                    case PID_PROTOCOL_VERSION:
                    {
                        ParameterProtocolVersion_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        if (p.protocolVersion.m_major < c_ProtocolVersion.m_major)
                        {
                            return false;
                        }
                        this->m_protocolVersion = p.protocolVersion;
                        break;
                    }
                    case PID_EXPECTS_INLINE_QOS:
                    {
                        ParameterBool_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        this->m_expectsInlineQos = p.value;
                        break;
                    }
                    case PID_VENDORID:
                    {
                        ParameterVendorId_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        this->m_VendorId[0] = p.vendorId[0];
                        this->m_VendorId[1] = p.vendorId[1];
                        break;
                    }
                    case PID_PARTICIPANT_GUID:
                    {
                        ParameterGuid_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        this->m_guid = p.guid;
                        this->m_key = p.guid;
                        break;
                    }
                    case PID_ENTITY_NAME:
                    {
                        ParameterString_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        this->m_participantName = p.getName();
                        break;
                    }
                    case PID_PROPERTY_LIST:
                    {
                        if (!m_properties.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case PID_KEY_HASH:
                    {
                        ParameterKey_t p(PID_KEY_HASH, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        GUID_t guid;
                        iHandle2GUID(guid, p.key);
                        this->m_guid = guid;
                        this->m_key = p.key;
                        break;
                    }
                    case PID_USER_DATA:
                    {
                        if (!m_userData.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case PID_BUILTIN_ENDPOINT_SET:
                    {
                        ParameterBuiltinEndpointSet_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        this->m_availableBuiltinEndpoints = p.endpointSet;
                        break;
                    }
                    case PID_PARTICIPANT_LEASE_DURATION:
                    {
                        ParameterTime_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        this->m_leaseDuration = p.time.to_duration_t();
                        lease_duration_ = std::chrono::microseconds(TimeConv::Duration_t2MicroSecondsInt64(m_leaseDuration));
                        break;
                    }
                    case PID_IDENTITY_TOKEN:
                    {
#if HAVE_SECURITY
                        ParameterToken_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        this->identity_token_ = std::move(p.token);
#else
                        logWarning(RTPS_PARTICIPANT, "Received PID_IDENTITY_TOKEN but security is disabled");
#endif
                        break;
                    }
                   case PID_PERMISSIONS_TOKEN:
                    {
#if HAVE_SECURITY
                        ParameterToken_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        this->permissions_token_ = std::move(p.token);
#else
                        logWarning(RTPS_PARTICIPANT, "Received PID_PERMISSIONS_TOKEN but security is disabled");
#endif
                        break;
                    }

                    case PID_PARTICIPANT_SECURITY_INFO:
                    {
#if HAVE_SECURITY
                        ParameterParticipantSecurityInfo_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        this->security_attributes_ = p.security_attributes;
                        this->plugin_security_attributes_ = p.plugin_security_attributes;
#else
                        logWarning(RTPS_PARTICIPANT, "Received PID_PARTICIPANT_SECURITY_INFO but security is disabled");
#endif
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            catch (std::bad_alloc& ba)
            {
                std::cerr << "bad_alloc caught: " << ba.what() << '\n';
                return false;
            }

            return true;
        };

    uint32_t qos_size;
    clear();
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
    metatraffic_locators.unicast.clear();
    metatraffic_locators.multicast.clear();
    default_locators.unicast.clear();
    default_locators.multicast.clear();
    m_participantName = "";
    m_key = InstanceHandle_t();
    m_leaseDuration = Duration_t();
    lease_duration_ = std::chrono::microseconds::zero();
    isAlive = true;
#if HAVE_SECURITY
    identity_token_ = IdentityToken();
    permissions_token_ = PermissionsToken();
    security_attributes_ = 0UL;
    plugin_security_attributes_ = 0UL;
#endif
    m_properties.clear();
    m_properties.length = 0;
    m_userData.clear();
    m_userData.length = 0;
}

void ParticipantProxyData::copy(const ParticipantProxyData& pdata)
{
    m_protocolVersion = pdata.m_protocolVersion;
    m_guid = pdata.m_guid;
    m_VendorId[0] = pdata.m_VendorId[0];
    m_VendorId[1] = pdata.m_VendorId[1];
    m_availableBuiltinEndpoints = pdata.m_availableBuiltinEndpoints;
    metatraffic_locators = pdata.metatraffic_locators;
    default_locators = pdata.default_locators;
    m_participantName = pdata.m_participantName;
    m_leaseDuration = pdata.m_leaseDuration;
    lease_duration_ = std::chrono::microseconds(TimeConv::Duration_t2MicroSecondsInt64(pdata.m_leaseDuration));
    m_key = pdata.m_key;
    isAlive = pdata.isAlive;
    m_userData = pdata.m_userData;
    m_properties = pdata.m_properties;

    // This method is only called when a new participant is discovered.The destination of the copy
    // will always be a new ParticipantProxyData or one from the pool, so there is no need for
    // m_readers and m_writers to be copied

#if HAVE_SECURITY
    identity_token_ = pdata.identity_token_;
    permissions_token_ = pdata.permissions_token_;
    security_attributes_ = pdata.security_attributes_;
    plugin_security_attributes_ = pdata.plugin_security_attributes_;
#endif
}

bool ParticipantProxyData::updateData(ParticipantProxyData& pdata)
{
    metatraffic_locators = pdata.metatraffic_locators;
    default_locators = pdata.default_locators;
    m_leaseDuration = pdata.m_leaseDuration;
    isAlive = true;
    m_userData = pdata.m_userData;
    m_properties = pdata.m_properties;

#if HAVE_SECURITY
    identity_token_ = pdata.identity_token_;
    permissions_token_ = pdata.permissions_token_;
    security_attributes_ = pdata.security_attributes_;
    plugin_security_attributes_ = pdata.plugin_security_attributes_;
#endif
    auto new_lease_duration = std::chrono::microseconds(TimeConv::Duration_t2MicroSecondsInt64(m_leaseDuration));
    if (this->lease_duration_event != nullptr)
    {
        if(new_lease_duration < lease_duration_)
        {
            // Calculate next trigger.
            auto real_lease_tm = last_received_message_tm_ + new_lease_duration;
            auto next_trigger = real_lease_tm - std::chrono::steady_clock::now();
            lease_duration_event->cancel_timer();
            lease_duration_event->update_interval_millisec(
                    (double)std::chrono::duration_cast<std::chrono::milliseconds>(next_trigger).count());
            lease_duration_event->restart_timer();
        }
    }
    lease_duration_ = new_lease_duration;
    return true;
}

void ParticipantProxyData::set_persistence_guid(const GUID_t& guid)
{
    // only valid values
    if (guid == c_Guid_Unknown)
    {
        return;
    }

    // generate pair
    std::pair<std::string, std::string> persistent_guid;
    persistent_guid.first = "PID_PERSISTENCE_GUID";

    std::ostringstream data;
    data << guid;
    persistent_guid.second = data.str();

    // if exists replace
    ParameterPropertyList_t::iterator it =
        std::find_if(
                m_properties.begin(),
                m_properties.end(),
                [&persistent_guid](const ParameterProperty_t p)
            {
                return persistent_guid.first == p.first();
            });

    if (it != m_properties.end())
    {
        if (!it->modify(persistent_guid))
        {
            logError(RTPS_PARTICIPANT,"Failed to change property <"
                    << it->first() << " | " << it->second() << "> to <"
                    << persistent_guid.first << " | " << persistent_guid.second << ">");
        }
    }
    else
    {
        // if not exists add
        m_properties.push_back(persistent_guid);
    }
}

GUID_t ParticipantProxyData::get_persistence_guid() const
{
    GUID_t persistent(c_Guid_Unknown);

    ParameterPropertyList_t::const_iterator it =
        std::find_if(
                m_properties.begin(),
                m_properties.end(),
                [](const ParameterProperty_t p)
            {
                return "PID_PERSISTENCE_GUID" == p.first();
            });

    if (it != m_properties.end())
    {
        std::istringstream in((*it).second());
        in >> persistent;
    }

    return persistent;
}

void ParticipantProxyData::assert_liveliness()
{
    last_received_message_tm_ = std::chrono::steady_clock::now();
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
