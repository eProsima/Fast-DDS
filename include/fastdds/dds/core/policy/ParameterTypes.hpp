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
 * @file ParameterTypes.hpp
 */

#ifndef _FASTDDS_DDS_QOS_PARAMETERTYPES_HPP_
#define _FASTDDS_DDS_QOS_PARAMETERTYPES_HPP_

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/all_common.h>
#include <fastdds/rtps/common/Token.h>

#include <fastrtps/utils/fixed_size_string.hpp>

#if HAVE_SECURITY
#include <fastdds/rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#include <fastdds/rtps/security/accesscontrol/EndpointSecurityAttributes.h>
#endif

#include <string>
#include <vector>

namespace eprosima {

namespace fastrtps {
namespace rtps {
struct CDRMessage_t;
}
}

namespace fastdds {
namespace dds {

/**
 * @addtogroup PARAMETER_MODULE
 * @{
 */

enum ParameterId_t : uint16_t
{
    PID_PAD = 0x0000,
    PID_SENTINEL = 0x0001,
    PID_USER_DATA = 0x002c,
    PID_TOPIC_NAME = 0x0005,
    PID_TYPE_NAME = 0x0007,
    PID_GROUP_DATA =0x002d,
    PID_TOPIC_DATA =0x002e,
    PID_DURABILITY =0x001d,
    PID_DURABILITY_SERVICE =0x001e,
    PID_DEADLINE =0x0023,
    PID_LATENCY_BUDGET =0x0027,
    PID_LIVELINESS =0x001b,
    PID_RELIABILITY =0x001A,
    PID_LIFESPAN =0x002b,
    PID_DESTINATION_ORDER =0x0025,
    PID_HISTORY =0x0040,
    PID_RESOURCE_LIMITS =0x0041,
    PID_OWNERSHIP =0x001f,
    PID_OWNERSHIP_STRENGTH =0x0006,
    PID_PRESENTATION =0x0021,
    PID_PARTITION =0x0029,
    PID_TIME_BASED_FILTER =0x0004,
    PID_TRANSPORT_PRIORITY =0x0049,
    PID_PROTOCOL_VERSION = 0x0015,
    PID_VENDORID = 0x0016,
    PID_UNICAST_LOCATOR = 0x002f,
    PID_MULTICAST_LOCATOR = 0x0030,
    PID_MULTICAST_IPADDRESS =0x0011,
    PID_DEFAULT_UNICAST_LOCATOR = 0x0031,
    PID_DEFAULT_MULTICAST_LOCATOR = 0x0048,
    PID_METATRAFFIC_UNICAST_LOCATOR = 0x0032,
    PID_METATRAFFIC_MULTICAST_LOCATOR = 0x0033,
    PID_DEFAULT_UNICAST_IPADDRESS =0x000c,
    PID_DEFAULT_UNICAST_PORT = 0x000e,
    PID_METATRAFFIC_UNICAST_IPADDRESS =0x0045,
    PID_METATRAFFIC_UNICAST_PORT = 0x000d,
    PID_METATRAFFIC_MULTICAST_IPADDRESS =0x000b,
    PID_METATRAFFIC_MULTICAST_PORT = 0x0046,
    PID_EXPECTS_INLINE_QOS =0x0043,
    PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT =0x0034,
    PID_PARTICIPANT_BUILTIN_ENDPOINTS = 0x0044,
    PID_PARTICIPANT_LEASE_DURATION = 0x0002,
    PID_CONTENT_FILTER_PROPERTY =0x0035,
    PID_PARTICIPANT_GUID = 0x0050,
    PID_PARTICIPANT_ENTITYID =0x0051,
    PID_GROUP_GUID =0x0052,
    PID_GROUP_ENTITYID =0x0053,
    PID_BUILTIN_ENDPOINT_SET = 0x0058,
    PID_PROPERTY_LIST = 0x0059,
    PID_TYPE_MAX_SIZE_SERIALIZED =0x0060,
    PID_ENTITY_NAME = 0x0062,
    PID_TYPE_IDV1 = 0x0069,
    PID_KEY_HASH = 0x0070,
    PID_STATUS_INFO = 0x0071,
    PID_TYPE_OBJECTV1 = 0x0072,
    PID_ENDPOINT_GUID = 0x005a,
    //PID_RELATED_SAMPLE_IDENTITY = 0x0083
    PID_IDENTITY_TOKEN = 0x1001,
    PID_PERMISSIONS_TOKEN = 0x1002,
    PID_DATA_TAGS = 0x1003,
    PID_ENDPOINT_SECURITY_INFO = 0x1004,
    PID_PARTICIPANT_SECURITY_INFO = 0x1005,
    PID_IDENTITY_STATUS_TOKEN = 0x1006,
    PID_PERSISTENCE_GUID = 0x8002,
    PID_RELATED_SAMPLE_IDENTITY = 0x800f,
    PID_DATA_REPRESENTATION = 0x0073,
    PID_TYPE_CONSISTENCY_ENFORCEMENT = 0x0074,
    PID_TYPE_INFORMATION = 0x0075,
    PID_DISABLE_POSITIVE_ACKS = 0x8005,
};

//!Base Parameter class with parameter PID and parameter length in bytes.
//!@ingroup PARAMETER_MODULE
class Parameter_t
{
public:

    RTPS_DllAPI Parameter_t()
        : Pid(PID_PAD)
        , length(0)
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param length Its associated length
     */
    RTPS_DllAPI Parameter_t(
            ParameterId_t pid,
            uint16_t length)
        : Pid(pid)
        , length(length)
    {
    }

    virtual RTPS_DllAPI ~Parameter_t()
    {
    }

    bool operator ==(
            const Parameter_t& b) const
    {
        return (this->Pid == b.Pid) &&
               (this->length == b.length);
    }

    /**
     * Virtual method used to add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    RTPS_DllAPI virtual bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) = 0;

    /**
     * Virtual method used to get the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    RTPS_DllAPI virtual bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) = 0;

public:

    //!Parameter ID
    ParameterId_t Pid;
    //!Parameter length
    uint16_t length;
};

/**
 *@ingroup PARAMETER_MODULE
 */
class ParameterKey_t : public Parameter_t
{
public:

    fastrtps::rtps::InstanceHandle_t key;

    ParameterKey_t()
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterKey_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
    {
    }

    ParameterKey_t(
            ParameterId_t pid,
            uint16_t in_length,
            fastrtps::rtps::InstanceHandle_t& ke)
        : Parameter_t(pid, in_length)
        , key(ke)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_KEY_HASH_LENGTH 16

/**
 *
 */
class ParameterLocator_t : public Parameter_t
{
public:

    fastrtps::rtps::Locator_t locator;

    ParameterLocator_t()
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterLocator_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
    {
    }

    ParameterLocator_t(
            ParameterId_t pid,
            uint16_t in_length,
            const fastrtps::rtps::Locator_t& loc)
        : Parameter_t(pid, in_length)
        , locator(loc)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};
#define PARAMETER_LOCATOR_LENGTH 24


/**
 *
 */
class ParameterString_t : public Parameter_t
{
public:

    ParameterString_t()
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterString_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
    {
    }

    ParameterString_t(
            ParameterId_t pid,
            uint16_t in_length,
            const fastrtps::string_255& strin)
        : Parameter_t(pid, in_length)
        , string_(strin)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;
    inline const char* getName() const
    {
        return string_.c_str();
    }

    inline void setName(
            const char* name)
    {
        string_ = name;
    }

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

private:

    fastrtps::string_255 string_;
};

/**
 *
 */
class ParameterPort_t : public Parameter_t
{
public:

    uint32_t port;
    ParameterPort_t()
        : port(0)
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterPort_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
        , port(0)
    {
    }

    ParameterPort_t(
            ParameterId_t pid,
            uint16_t in_length,
            uint32_t po)
        : Parameter_t(pid, in_length)
        , port(po)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_PORT_LENGTH 4

/**
 *
 */
class ParameterGuid_t : public Parameter_t
{
public:

    fastrtps::rtps::GUID_t guid;
    ParameterGuid_t()
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterGuid_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
    {
    }

    ParameterGuid_t(
            ParameterId_t pid,
            uint16_t in_length,
            fastrtps::rtps::GUID_t guidin)
        : Parameter_t(pid, in_length)
        , guid(guidin)
    {
    }

    ParameterGuid_t(
            ParameterId_t pid,
            uint16_t in_length,
            fastrtps::rtps::InstanceHandle_t& iH)
        : Parameter_t(pid, in_length)
    {
        for (uint8_t i = 0; i < 16; ++i)
        {
            if (i < 12)
            {
                guid.guidPrefix.value[i] = iH.value[i];
            }
            else
            {
                guid.entityId.value[i - 12] = iH.value[i];
            }
        }
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_GUID_LENGTH 16

/**
 *
 */
class ParameterProtocolVersion_t : public Parameter_t
{
public:

    fastrtps::rtps::ProtocolVersion_t protocolVersion;
    ParameterProtocolVersion_t()
    {
        protocolVersion = fastrtps::rtps::c_ProtocolVersion;
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterProtocolVersion_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
    {
        protocolVersion = fastrtps::rtps::c_ProtocolVersion;
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_PROTOCOL_LENGTH 4

/**
 *
 */
class ParameterVendorId_t : public Parameter_t
{
public:

    fastrtps::rtps::VendorId_t vendorId;
    ParameterVendorId_t()
        : vendorId(fastrtps::rtps::c_VendorId_eProsima)
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterVendorId_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
        , vendorId(fastrtps::rtps::c_VendorId_eProsima)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_VENDOR_LENGTH 4

/**
 *
 */
class ParameterIP4Address_t : public Parameter_t
{
public:

    fastrtps::rtps::octet address[4];
    ParameterIP4Address_t()
    {
        this->setIP4Address(0, 0, 0, 0);
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterIP4Address_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
    {
        this->setIP4Address(0, 0, 0, 0);
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

    void setIP4Address(
            fastrtps::rtps::octet o1,
            fastrtps::rtps::octet o2,
            fastrtps::rtps::octet o3,
            fastrtps::rtps::octet o4);
};

#define PARAMETER_IP4_LENGTH 4

/**
 *
 */
class ParameterBool_t : public Parameter_t
{
public:

    bool value;
    ParameterBool_t()
        : value(false)
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterBool_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
        , value(false)
    {
    }

    ParameterBool_t(
            ParameterId_t pid,
            uint16_t in_length,
            bool inbool)
        : Parameter_t(pid, in_length)
        , value(inbool)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_BOOL_LENGTH 4

/**
 *
 */
class ParameterStatusInfo_t : public Parameter_t
{
public:

    uint8_t status;
    ParameterStatusInfo_t()
        : status(0)
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterStatusInfo_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
        , status(0)
    {
    }

    ParameterStatusInfo_t(
            ParameterId_t pid,
            uint16_t in_length,
            uint8_t instatus)
        : Parameter_t(pid, in_length)
        , status(instatus)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_STATUS_INFO_LENGTH 4

/**
 *
 */
class ParameterCount_t : public Parameter_t
{
public:

    fastrtps::rtps::Count_t count;
    ParameterCount_t()
        : count(0)
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterCount_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
        , count(0)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_COUNT_LENGTH 4

/**
 *
 */
class ParameterEntityId_t : public Parameter_t
{
public:

    fastrtps::rtps::EntityId_t entityId;
    ParameterEntityId_t()
        : entityId(ENTITYID_UNKNOWN)
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterEntityId_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
        , entityId(ENTITYID_UNKNOWN)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_ENTITYID_LENGTH 4

/**
 *
 */
class ParameterTime_t : public Parameter_t
{
public:

    fastrtps::rtps::Time_t time;
    ParameterTime_t()
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterTime_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_TIME_LENGTH 8

/**
 *
 */
class ParameterBuiltinEndpointSet_t : public Parameter_t
{
public:

    fastrtps::rtps::BuiltinEndpointSet_t endpointSet;
    ParameterBuiltinEndpointSet_t()
        : endpointSet(0)
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterBuiltinEndpointSet_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
        , endpointSet(0)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_BUILTINENDPOINTSET_LENGTH 4


class ParameterProperty_t
{

    friend class ParameterPropertyList_t;

private:

    fastrtps::rtps::octet* data;

public:

    ParameterProperty_t()
    {
        data = nullptr;
    }

    explicit ParameterProperty_t(
            void* ptr)
    {
        data = (fastrtps::rtps::octet*)ptr;
    }

    std::string first() const
    {
        //Skip the size and return the string
        return std::string((char*)data + 4);
    }

    std::string second() const
    {
        //Skip the first element
        uint32_t size1 = ParameterProperty_t::element_size(data);

        //Skip the size of the second element and return the string
        return std::string((char*)data + size1 + 4);
    }

    bool modify(
            const std::pair<std::string, std::string>& new_value)
    {
        uint32_t old_size = size();

        uint32_t first_size = (uint32_t)new_value.first.size() + 1;
        uint32_t first_alignment = ((first_size + 3) & ~3) - first_size;
        uint32_t second_size = (uint32_t)new_value.second.size() + 1;
        uint32_t second_alignment = ((second_size + 3) & ~3) - second_size;
        uint32_t new_size = first_size + first_alignment + second_size + second_alignment + 8;

        if (old_size != new_size)
        {
            return false;
        }

        fastrtps::rtps::octet* current = data;
        memcpy(current, &first_size, 4);
        memcpy(current + 4, new_value.first.c_str(), first_size);
        memset(current + 4 + first_size, 0, first_alignment);

        current = data + 4 + first_size + first_alignment;
        memcpy(current, &second_size, 4);
        memcpy(current + 4, new_value.second.c_str(), second_size);
        memset(current + 4 + second_size, 0, second_alignment);

        return true;
    }

    std::pair<const std::string, const std::string> pair() const
    {
        return std::make_pair(std::string(first()), std::string(second()));
    }

    uint32_t size() const
    {
        //Size of the first element (with alignment)
        uint32_t size1 = ParameterProperty_t::element_size(data);

        //Size of the second element (with alignment)
        uint32_t size2 = ParameterProperty_t::element_size(data + size1);
        return size1 + size2;
    }

    bool operator ==(
            const ParameterProperty_t& b) const
    {
        return (first() == b.first()) &&
               (second() == b.second());
    }

    bool operator !=(
            const ParameterProperty_t& b) const
    {
        return !(*this == b);
    }

private:

    static uint32_t element_size(
            const fastrtps::rtps::octet* ptr)
    {
        //Size of the element (with alignment)
        uint32_t size = *(uint32_t*)ptr;
        return (4 + ((size + 3) & ~3));
    }

};

/**
 *
 */
class ParameterPropertyList_t : public Parameter_t
{
private:

    fastrtps::rtps::SerializedPayload_t properties_;
    uint32_t Nproperties_ = 0;
    bool limit_size_ = false;

public:

    class iterator
    {
public:

        typedef iterator self_type;
        typedef ParameterProperty_t value_type;
        typedef ParameterProperty_t reference;
        typedef ParameterProperty_t* pointer;
        typedef size_t difference_type;
        typedef std::forward_iterator_tag iterator_category;

        iterator(
                fastrtps::rtps::octet* ptr)
            : ptr_(ptr)
            , value_(ptr)
        {
        }

        self_type operator ++()
        {
            self_type i = *this; advance(); return i;
        }

        self_type operator ++(
                int)
        {
            advance(); return *this;
        }

        reference operator *()
        {
            return value_;
        }

        pointer operator ->()
        {
            return &value_;
        }

        bool operator ==(
                const self_type& rhs)
        {
            return ptr_ == rhs.ptr_;
        }

        bool operator !=(
                const self_type& rhs)
        {
            return ptr_ != rhs.ptr_;
        }

protected:

        void advance()
        {
            ptr_ += value_.size();
            value_ = ParameterProperty_t(ptr_);
        }

        fastrtps::rtps::octet* address() const
        {
            return ptr_;
        }

private:

        fastrtps::rtps::octet* ptr_;
        ParameterProperty_t value_;
    };

    class const_iterator
    {
public:

        typedef const_iterator self_type;
        typedef const ParameterProperty_t value_type;
        typedef const ParameterProperty_t reference;
        typedef const ParameterProperty_t* pointer;
        typedef size_t difference_type;
        typedef std::forward_iterator_tag iterator_category;

        const_iterator(
                const fastrtps::rtps::octet* ptr)
            : ptr_(ptr)
            , value_(const_cast<fastrtps::rtps::octet*>(ptr))
        {
        }

        self_type operator ++()
        {
            self_type i = *this; advance(); return i;
        }

        self_type operator ++(
                int)
        {
            advance(); return *this;
        }

        reference operator *()
        {
            return value_;
        }

        pointer operator ->()
        {
            return &value_;
        }

        bool operator ==(
                const self_type& rhs)
        {
            return ptr_ == rhs.ptr_;
        }

        bool operator !=(
                const self_type& rhs)
        {
            return ptr_ != rhs.ptr_;
        }

protected:

        void advance()
        {
            ptr_ += value_.size();
            value_ = ParameterProperty_t(const_cast<fastrtps::rtps::octet*>(ptr_));
        }

        const fastrtps::rtps::octet* address() const
        {
            return ptr_;
        }

private:

        const fastrtps::rtps::octet* ptr_;
        ParameterProperty_t value_;
    };

public:

    ParameterPropertyList_t()
        : Parameter_t(PID_PROPERTY_LIST, 0)
        , Nproperties_ (0)
        , limit_size_ (false)
    {
    }

    /**
     * Constructor with a defined maximum size
     */
    ParameterPropertyList_t(
            uint32_t size)
        : Parameter_t(PID_PROPERTY_LIST, 0)
        , properties_(size)
        , Nproperties_ (0)
        , limit_size_ (size == 0 ? false : true)
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param in_length Its associated length
     */
    ParameterPropertyList_t(
            ParameterId_t /*pid*/,
            uint16_t in_length)
        : Parameter_t(PID_PROPERTY_LIST, in_length)
        , Nproperties_ (0)
        , limit_size_ (false)
    {
    }

    ParameterPropertyList_t(
            const ParameterPropertyList_t& parameter_properties)
        : Parameter_t(PID_PROPERTY_LIST, parameter_properties.length)
        , properties_(parameter_properties.limit_size_ ?
                parameter_properties.properties_.max_size :
                parameter_properties.properties_.length)
        , Nproperties_ (parameter_properties.Nproperties_)
        , limit_size_ (parameter_properties.limit_size_)
    {
        properties_.copy(&parameter_properties.properties_, parameter_properties.limit_size_);
    }

    ParameterPropertyList_t& operator = (
            const ParameterPropertyList_t& parameter_properties)
    {
        length = parameter_properties.length;
        limit_size_ = parameter_properties.limit_size_;
        properties_.reserve(limit_size_ ?
                parameter_properties.properties_.max_size :
                parameter_properties.properties_.length);
        properties_.copy(&parameter_properties.properties_, parameter_properties.limit_size_);
        Nproperties_ = parameter_properties.Nproperties_;
        return *this;
    }

    iterator begin()
    {
        return iterator(properties_.data);
    }

    iterator end()
    {
        return iterator(properties_.data + properties_.length);
    }

    const_iterator begin() const
    {
        return const_iterator(properties_.data);
    }

    const_iterator end() const
    {
        return const_iterator(properties_.data + properties_.length);
    }

    bool push_back(
            std::pair<std::string, std::string> p)
    {

        //Realloc if needed;
        uint32_t size1 = (uint32_t) p.first.length() + 1;
        uint32_t alignment1 = ((size1 + 3) & ~3) - size1;

        uint32_t size2 = (uint32_t) p.second.length() + 1;
        uint32_t alignment2 = ((size2 + 3) & ~3) - size2;

        if (limit_size_ && (properties_.max_size < properties_.length +
                size1 + alignment1 + 4 +
                size2 + alignment2 + 4))
        {
            return false;
        }
        properties_.reserve(properties_.length +
                size1 + alignment1 + 4 +
                size2 + alignment2 + 4);

        push_back_helper((fastrtps::rtps::octet*)p.first.c_str(), size1, alignment1);
        push_back_helper((fastrtps::rtps::octet*)p.second.c_str(), size2, alignment2);
        ++Nproperties_;
        return true;
    }

    bool set_property (
            iterator pos,
            const std::pair<std::string, std::string>& new_value)
    {
        return pos->modify(new_value);
    }

    void clear()
    {
        properties_.length = 0;
        Nproperties_ = 0;
    }

    uint32_t size() const
    {
        return Nproperties_;
    }

    void set_max_size (
            uint32_t size)
    {
        properties_.reserve(size);
        limit_size_ = true;
    }

    uint32_t max_size ()
    {
        return (limit_size_ ? properties_.max_size : 0);
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

protected:

    void push_back_helper (
            const fastrtps::rtps::octet* data,
            uint32_t size,
            uint32_t alignment)
    {
        fastrtps::rtps::octet* o = (fastrtps::rtps::octet*)&size;
        memcpy(properties_.data + properties_.length,
                o, 4);
        properties_.length += 4;

        memcpy(properties_.data + properties_.length,
                data, size);
        properties_.length += size;

        for (uint32_t i = 0; i < alignment; ++i)
        {
            properties_.data[properties_.length + i] = '\0';
        }
        properties_.length += alignment;
    }

};


/**
 *
 */
class ParameterSampleIdentity_t : public Parameter_t
{
public:

    fastrtps::rtps::SampleIdentity sample_id;

    ParameterSampleIdentity_t()
        : sample_id(fastrtps::rtps::SampleIdentity::unknown())
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterSampleIdentity_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
        , sample_id(fastrtps::rtps::SampleIdentity::unknown())
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#if HAVE_SECURITY

/**
 *
 */
class ParameterToken_t : public Parameter_t
{
public:

    fastrtps::rtps::Token token;

    ParameterToken_t()
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterToken_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

class ParameterParticipantSecurityInfo_t : public Parameter_t
{
public:

    fastrtps::rtps::security::ParticipantSecurityAttributesMask security_attributes;
    fastrtps::rtps::security::PluginParticipantSecurityAttributesMask plugin_security_attributes;

    ParameterParticipantSecurityInfo_t()
        : Parameter_t(PID_PARTICIPANT_SECURITY_INFO, 0)
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterParticipantSecurityInfo_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH 8

class ParameterEndpointSecurityInfo_t : public Parameter_t
{
public:

    fastrtps::rtps::security::EndpointSecurityAttributesMask security_attributes;
    fastrtps::rtps::security::PluginEndpointSecurityAttributesMask plugin_security_attributes;

    ParameterEndpointSecurityInfo_t()
        : Parameter_t(PID_ENDPOINT_SECURITY_INFO, 0)
    {
    }

    /**
     * Constructor using a parameter PID and the parameter length
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     */
    ParameterEndpointSecurityInfo_t(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
    {
    }

    /**
     * Add the parameter to a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message where the parameter should be added.
     * @return True if the parameter was correctly added.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) override;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size) override;

};

#define PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH 8

#endif

///@}

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif
#endif // _FASTDDS_DDS_QOS_PARAMETERTYPES_HPP_
