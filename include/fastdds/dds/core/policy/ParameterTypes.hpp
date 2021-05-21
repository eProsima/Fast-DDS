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
#include <fastdds/rtps/messages/CDRMessage.h>

#include <fastrtps/utils/fixed_size_string.hpp>

#if HAVE_SECURITY
#include <fastdds/rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#include <fastdds/rtps/security/accesscontrol/EndpointSecurityAttributes.h>
#endif // if HAVE_SECURITY

#include <string>
#include <vector>

namespace eprosima {

namespace fastrtps {
namespace rtps {
struct CDRMessage_t;
} // namespace rtps
} // namespace fastrtps

namespace fastdds {
namespace dds {

/**
 * @addtogroup PARAMETER_MODULE
 * @{
 */

/**
 * @brief Enum for the unique parameter identifier. Alias of uint16_t.
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
    PID_DATASHARING = 0x8006,
};

//!Base Parameter class with parameter PID and parameter length in bytes.
//!@ingroup PARAMETER_MODULE
class Parameter_t
{
public:

    /**
     * @brief Constructor without parameters
     */
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

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~Parameter_t()
    {
    }

    bool operator ==(
            const Parameter_t& b) const
    {
        return (this->Pid == b.Pid) &&
               (this->length == b.length);
    }

public:

    //!Parameter ID. <br> By default, PID_PAD.
    ParameterId_t Pid;
    //!Parameter length. <br> By default, 0.
    uint16_t length;
};

/**
 *@ingroup PARAMETER_MODULE
 */
class ParameterKey_t : public Parameter_t
{
public:

    //!Instance Handle. <br> By default, c_InstanceHandle_Unknown.
    fastrtps::rtps::InstanceHandle_t key;
    /**
     * @brief Constructor without parameters
     */
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

    /**
     * @brief Constructor using a parameter PID, parameter length and Instance Handle
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     * @param ke Instance Handle to be set
     */
    ParameterKey_t(
            ParameterId_t pid,
            uint16_t in_length,
            const fastrtps::rtps::InstanceHandle_t& ke)
        : Parameter_t(pid, in_length)
        , key(ke)
    {
    }

};

#define PARAMETER_KEY_HASH_LENGTH 16

/**
 *@ingroup PARAMETER_MODULE
 */
class ParameterLocator_t : public Parameter_t
{
public:

    //!Locator
    rtps::Locator locator;

    /**
     * @brief Constructor without parameters
     */
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

    /**
     * @brief Constructor using a parameter PID, the parameter length and a Locator
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     * @param loc Locator to be set
     */
    ParameterLocator_t(
            ParameterId_t pid,
            uint16_t in_length,
            const rtps::Locator& loc)
        : Parameter_t(pid, in_length)
        , locator(loc)
    {
    }

};
#define PARAMETER_LOCATOR_LENGTH 24


/**
 *@ingroup PARAMETER_MODULE
 */
class ParameterString_t : public Parameter_t
{
public:

    /**
     * @brief Constructor without parameters.
     */
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

    /**
     * @brief Constructor using a parameter PID, the parameter length and a string
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     * @param strin Parameter name
     */
    ParameterString_t(
            ParameterId_t pid,
            uint16_t in_length,
            const fastrtps::string_255& strin)
        : Parameter_t(pid, in_length)
        , string_(strin)
    {
    }

    /**
     * @brief Getter for the name
     * @return current name associated
     */
    inline const char* getName() const
    {
        return string_.c_str();
    }

    /**
     * @brief Setter for the name
     * @param name String to be set
     */
    inline void setName(
            const char* name)
    {
        string_ = name;
    }

    /**
     * @brief Getter for the name size
     * @return size_t
     */
    inline size_t size() const
    {
        return string_.size();
    }

private:

    //!Name. <br> By default, empty string.
    fastrtps::string_255 string_;
};

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterPort_t : public Parameter_t
{
public:

    //!Port. <br> By default, 0.
    uint32_t port;

    /**
     * @brief Constructor without parameters
     */
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

    /**
     * @brief Constructor using a parameter PID, the parameter length and a port
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     * @param po Port to be set
     */
    ParameterPort_t(
            ParameterId_t pid,
            uint16_t in_length,
            uint32_t po)
        : Parameter_t(pid, in_length)
        , port(po)
    {
    }

};

#define PARAMETER_PORT_LENGTH 4

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterGuid_t : public Parameter_t
{
public:

    //!GUID <br> By default, unknown GUID.
    fastrtps::rtps::GUID_t guid;

    /**
     * @brief Constructor without parameters
     */
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

    /**
     * @brief Constructor using a parameter PID, the parameter length and a GUID
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     * @param guidin GUID to be set
     */
    ParameterGuid_t(
            ParameterId_t pid,
            uint16_t in_length,
            const fastrtps::rtps::GUID_t& guidin)
        : Parameter_t(pid, in_length)
        , guid(guidin)
    {
    }

    /**
     * @brief Constructor using a parameter PID, the parameter length and a Instance Handle
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     * @param iH Instance Handle to be set as GUID
     */
    ParameterGuid_t(
            ParameterId_t pid,
            uint16_t in_length,
            const fastrtps::rtps::InstanceHandle_t& iH)
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

};

#define PARAMETER_GUID_LENGTH 16

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterProtocolVersion_t : public Parameter_t
{
public:

    //!Protocol Version. <br> By default, c_ProtocolVersion.
    fastrtps::rtps::ProtocolVersion_t protocolVersion;

    /**
     * @brief Constructor without parameters
     */
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

};

#define PARAMETER_PROTOCOL_LENGTH 4

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterVendorId_t : public Parameter_t
{
public:

    //!Vendor Id. <br> By default, c_VendorId_eProsima.
    fastrtps::rtps::VendorId_t vendorId;

    /**
     * @brief Constructor without parameters
     */
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

};

#define PARAMETER_VENDOR_LENGTH 4

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterIP4Address_t : public Parameter_t
{
public:

    //!Address <br> By default [0,0,0,0].
    fastrtps::rtps::octet address[4];

    /**
     * @brief Constructor without parameters
     */
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
     * @brief Setter for the address
     * @param o1 First octet
     * @param o2 Second octet
     * @param o3 Third octet
     * @param o4 Fourth octet
     */
    void setIP4Address(
            fastrtps::rtps::octet o1,
            fastrtps::rtps::octet o2,
            fastrtps::rtps::octet o3,
            fastrtps::rtps::octet o4)
    {
        address[0] = o1;
        address[1] = o2;
        address[2] = o3;
        address[3] = o4;
    }

};

#define PARAMETER_IP4_LENGTH 4

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterBool_t : public Parameter_t
{
public:

    //!Boolean <br> By default, false.
    bool value;

    /**
     * @brief Constructor without parameter
     */
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

    /**
     * @brief Constructor using a parameter PID, the parameter length and a boolean
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     * @param inbool Boolean to be set
     */
    ParameterBool_t(
            ParameterId_t pid,
            uint16_t in_length,
            bool inbool)
        : Parameter_t(pid, in_length)
        , value(inbool)
    {
    }

};

#define PARAMETER_BOOL_LENGTH 4

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterStatusInfo_t : public Parameter_t
{
public:

    //!Status <br> By default, 0.
    uint8_t status;

    /**
     * @brief Constructor without parameter
     */
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

    /**
     * @brief Constructor using a parameter PID, the parameter length and status value
     * @param pid Pid of the parameter
     * @param in_length Its associated length
     * @param instatus uint8_t to be set as status
     */
    ParameterStatusInfo_t(
            ParameterId_t pid,
            uint16_t in_length,
            uint8_t instatus)
        : Parameter_t(pid, in_length)
        , status(instatus)
    {
    }

};

#define PARAMETER_STATUS_INFO_LENGTH 4

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterCount_t : public Parameter_t
{
public:

    //!Count <br> By default, 0.
    fastrtps::rtps::Count_t count;

    /**
     * @brief Constructor without parameter
     */
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

};

#define PARAMETER_COUNT_LENGTH 4

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterEntityId_t : public Parameter_t
{
public:

    //!EntityId <br> By default, ENTITYID_UNKNOWN.
    fastrtps::rtps::EntityId_t entityId;

    /**
     * @brief Constructor without parameters
     */
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

};

#define PARAMETER_ENTITYID_LENGTH 4

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterTime_t : public Parameter_t
{
public:

    //!Time <br> By default, 0.
    fastrtps::rtps::Time_t time;

    /**
     * @brief Constructor without parameters
     */
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

};

#define PARAMETER_TIME_LENGTH 8

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterBuiltinEndpointSet_t : public Parameter_t
{
public:

    //!Builtin Endpoint Set <br> By default, 0.
    fastrtps::rtps::BuiltinEndpointSet_t endpointSet;

    /**
     * @brief Constructor without parameters
     */
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

};

#define PARAMETER_BUILTINENDPOINTSET_LENGTH 4


/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterProperty_t
{

    friend class ParameterPropertyList_t;

private:

    //!Data <br> By default, nullptr.
    fastrtps::rtps::octet* data;

public:

    /**
     * @brief Constructor without parameters
     */
    ParameterProperty_t()
    {
        data = nullptr;
    }

    /**
     * @brief Constructor using a pointer
     * @param ptr Pointer to be set as data
     */
    explicit ParameterProperty_t(
            void* ptr)
    {
        data = (fastrtps::rtps::octet*)ptr;
    }

    /**
     * @brief Getter for the first element in data
     * @return string with the data
     */
    std::string first() const
    {
        //Skip the size and return the string
        return std::string((char*)data + 4);
    }

    /**
     * @brief Getter for the second element in data
     * @return string with the data
     */
    std::string second() const
    {
        //Skip the first element
        uint32_t size1 = ParameterProperty_t::element_size(data);

        //Skip the size of the second element and return the string
        return std::string((char*)data + size1 + 4);
    }

    /**
     * @brief Setter using a pair of strings
     * @param new_value Pair of strings with the new values
     * @return true if the modification is done correctly and false if the size of the new_value is not valid
     */
    bool modify(
            const std::pair<std::string, std::string>& new_value)
    {
        uint32_t old_size = size();

        uint32_t first_size = (uint32_t)new_value.first.size() + 1;
        uint32_t first_alignment = ((first_size + 3u) & ~3u) - first_size;
        uint32_t second_size = (uint32_t)new_value.second.size() + 1;
        uint32_t second_alignment = ((second_size + 3u) & ~3u) - second_size;
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

    /**
     * @brief Getter that returns a pair of the first and second elements in data
     * @return Pair of strings with the first and second elements data
     */
    std::pair<const std::string, const std::string> pair() const
    {
        return std::make_pair(std::string(first()), std::string(second()));
    }

    /**
     * @brief Getter for data size
     * @return uint32_t with the size
     */
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

    /**
     * @brief Getter for the size of a specific octet pointer
     * @param ptr Octet pointer to measure
     * @return Size of the pointer data
     */
    static uint32_t element_size(
            const fastrtps::rtps::octet* ptr)
    {
        //Size of the element (with alignment)
        uint32_t size = *(uint32_t*)ptr;
        return (4u + ((size + 3u) & ~3u));
    }

};

/**
 * Parameter property ID for persistence GUID
 * @ingroup PARAMETER_MODULE
 */
const std::string parameter_property_persistence_guid = "PID_PERSISTENCE_GUID";

/**
 * Parameter property ID for participant type
 * @ingroup PARAMETER_MODULE
 */
const std::string parameter_property_participant_type = "PARTICIPANT_TYPE";

/**
 * Parameter property ID for Discovery Server version
 * @ingroup PARAMETER_MODULE
 */
const std::string parameter_property_ds_version = "DS_VERSION";

/**
 * Parameter property value for Discovery Server version
 * @ingroup PARAMETER_MODULE
 */
const std::string parameter_property_current_ds_version = "2.0";

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterPropertyList_t : public Parameter_t
{
private:

    //!Properties
    fastrtps::rtps::SerializedPayload_t properties_;
    //!Number of properties
    uint32_t Nproperties_ = 0;
    //!Maximum size
    bool limit_size_ = false;

public:

    class iterator
    {
    public:

        typedef iterator self_type;
        typedef ParameterProperty_t value_type;
        typedef ParameterProperty_t& reference;
        typedef ParameterProperty_t* pointer;
        typedef size_t difference_type;
        typedef std::forward_iterator_tag iterator_category;

        /**
         * @brief Constructor using an octet pointer
         * @param ptr Octet pointer to be set
         */
        iterator(
                fastrtps::rtps::octet* ptr)
            : ptr_(ptr)
            , value_(ptr)
        {
        }

        self_type operator ++()
        {
            self_type i = *this;
            advance();
            return i;
        }

        self_type operator ++(
                int)
        {
            advance();
            return *this;
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
                const self_type& rhs) const
        {
            return ptr_ == rhs.ptr_;
        }

        bool operator !=(
                const self_type& rhs) const
        {
            return ptr_ != rhs.ptr_;
        }

    protected:

        /**
         * @brief Shift the pointer to the next value
         */
        void advance()
        {
            ptr_ += value_.size();
            value_ = ParameterProperty_t(ptr_);
        }

        /**
         * @brief Getter for the pointer
         * @return the pointer
         */
        fastrtps::rtps::octet* address() const
        {
            return ptr_;
        }

    private:

        //!Pointer
        fastrtps::rtps::octet* ptr_;
        //!Parameter Property
        ParameterProperty_t value_;
    };

    class const_iterator
    {
    public:

        typedef const_iterator self_type;
        typedef const ParameterProperty_t value_type;
        typedef const ParameterProperty_t& reference;
        typedef const ParameterProperty_t* pointer;
        typedef size_t difference_type;
        typedef std::forward_iterator_tag iterator_category;

        /**
         * @brief Constructor using a pointer
         * @param ptr Pointer to be set
         */
        const_iterator(
                const fastrtps::rtps::octet* ptr)
            : ptr_(ptr)
            , value_(const_cast<fastrtps::rtps::octet*>(ptr))
        {
        }

        self_type operator ++()
        {
            self_type i = *this;
            advance();
            return i;
        }

        self_type operator ++(
                int)
        {
            advance();
            return *this;
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
                const self_type& rhs) const
        {
            return ptr_ == rhs.ptr_;
        }

        bool operator !=(
                const self_type& rhs) const
        {
            return ptr_ != rhs.ptr_;
        }

    protected:

        /**
         * @brief Shift the pointer to the next value
         */
        void advance()
        {
            ptr_ += value_.size();
            value_ = ParameterProperty_t(const_cast<fastrtps::rtps::octet*>(ptr_));
        }

        /**
         * @brief Getter for the pointer
         * @return the pointer
         */
        const fastrtps::rtps::octet* address() const
        {
            return ptr_;
        }

    private:

        //!Pointer
        const fastrtps::rtps::octet* ptr_;
        //!Parameter Property
        ParameterProperty_t value_;
    };

public:

    /**
     * @brief Constructor without parameters
     * Sets PID_PROPERTY_LIST as the PID of the parameter
     */
    ParameterPropertyList_t()
        : Parameter_t(PID_PROPERTY_LIST, 0)
        , Nproperties_ (0)
        , limit_size_ (false)
    {
    }

    /**
     * Constructor with a defined maximum size
     * @param size Size to be set as maximum
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

    /**
     * @brief Constructor using a Parameter Property List
     * @param parameter_properties Properties to be set
     */
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

    /**
     * @brief Getter for the first position of the ParameterPropertyList
     * @return iterator
     */
    iterator begin()
    {
        return iterator(properties_.data);
    }

    /**
     * @brief Getter for the end of the ParameterPropertyList
     * @return iterator
     */
    iterator end()
    {
        return iterator(properties_.data + properties_.length);
    }

    /**
     * @brief Getter for the first position of the ParameterPropertyList
     * @return const_iterator
     */
    const_iterator begin() const
    {
        return const_iterator(properties_.data);
    }

    /**
     * @brief Getter for the end of the ParameterPropertyList
     * @return const_iterator
     */
    const_iterator end() const
    {
        return const_iterator(properties_.data + properties_.length);
    }

    /**
     * @brief Introduce a new property in the ParameterPropertyList
     * @param p Pair with the values of the new property
     * @return true if it is introduced, false if not.
     */
    bool push_back(
            std::pair<std::string, std::string> p)
    {
        return push_back(p.first, p.second);
    }

    /**
     * @brief Introduce a new property in the ParameterPropertyList
     * @param key Key part of the new property
     * @param value Value part of the new property
     * @return true if it is introduced, false if not.
     */
    bool push_back(
            const std::string& key,
            const std::string& value)
    {
        auto str1 = reinterpret_cast<const unsigned char*>(key.c_str());
        uint32_t size1 = (uint32_t) key.length() + 1;
        auto str2 = reinterpret_cast<const unsigned char*>(value.c_str());
        uint32_t size2 = (uint32_t) value.length() + 1;

        return push_back(str1, size1, str2, size2);
    }

    /**
     * @brief Introduce a new property in the ParameterPropertyList
     * @param str1 Name of the property
     * @param str1_size Size of the first string
     * @param str2 Value of the property
     * @param str2_size Size of the second string
     * @return true if it is introduced, false if not.
     */
    bool push_back(
            const unsigned char* str1,
            uint32_t str1_size,
            const unsigned char* str2,
            uint32_t str2_size)
    {
        //Realloc if needed;
        uint32_t alignment1 = ((str1_size + 3u) & ~3u) - str1_size;
        uint32_t alignment2 = ((str2_size + 3u) & ~3u) - str2_size;

        if (limit_size_ && (properties_.max_size < properties_.length +
                str1_size + alignment1 + 4 +
                str2_size + alignment2 + 4))
        {
            return false;
        }
        properties_.reserve(properties_.length +
                str1_size + alignment1 + 4 +
                str2_size + alignment2 + 4);

        push_back_helper((fastrtps::rtps::octet*)str1, str1_size, alignment1);
        push_back_helper((fastrtps::rtps::octet*)str2, str2_size, alignment2);
        ++Nproperties_;
        return true;
    }

    /**
     * @brief Setter of a new property value on a specific position
     * @param pos Iterator with the position of the property to be changed
     * @param new_value Value to be set
     * @return true if changed, false if not
     */
    bool set_property (
            iterator pos,
            const std::pair<std::string, std::string>& new_value)
    {
        return pos->modify(new_value);
    }

    /**
     * @brief Clears the ParameterPropertyList
     */
    void clear()
    {
        properties_.length = 0;
        Nproperties_ = 0;
    }

    /**
     * @brief Getter for the size of the ParameterPropertyList
     * @return uint32_t with the size
     */
    uint32_t size() const
    {
        return Nproperties_;
    }

    /**
     * @brief Setter for the maximum size of the ParameterPropertyList
     */
    void set_max_size (
            uint32_t size)
    {
        properties_.reserve(size);
        limit_size_ = true;
    }

    /**
     * @brief Getter for the maximum size of the ParameterPropertyList
     * @return uint32_t with the size
     */
    uint32_t max_size ()
    {
        return (limit_size_ ? properties_.max_size : 0);
    }

protected:

    void push_back_helper (
            const fastrtps::rtps::octet* data,
            uint32_t size,
            uint32_t alignment)
    {
        fastrtps::rtps::octet* o = (fastrtps::rtps::octet*)&size;
        memcpy(properties_.data + properties_.length, o, 4);
        properties_.length += 4;

        memcpy(properties_.data + properties_.length, data, size);
        properties_.length += size;

        for (uint32_t i = 0; i < alignment; ++i)
        {
            properties_.data[properties_.length + i] = '\0';
        }
        properties_.length += alignment;
    }

};


/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterSampleIdentity_t : public Parameter_t
{
public:

    //!Sample Identity <br> By default, unknown.
    fastrtps::rtps::SampleIdentity sample_id;

    /**
     * @brief Constructor without parameters
     */
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
            fastrtps::rtps::CDRMessage_t* msg) const;

    /**
     * Read the parameter from a CDRMessage_t message.
     * @param[in,out] msg Pointer to the message from where the parameter should be taken.
     * @param size Size of the parameter field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg,
            uint16_t size);

};

#define PARAMETER_SAMPLEIDENTITY_LENGTH 24


#if HAVE_SECURITY

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterToken_t : public Parameter_t
{
public:

    //!Token
    fastrtps::rtps::Token token;

    /**
     * @brief Constructor without parameters
     */
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

};

//!Default value for the ParameterParticipantSecurityInfo_t length
#define PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH 8

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterParticipantSecurityInfo_t : public Parameter_t
{
public:

    //!Participant Security Attributes Mask <br> By default, 0.
    fastrtps::rtps::security::ParticipantSecurityAttributesMask security_attributes = 0;
    //!Plugin Participant Security Attributes Mask <br> By default, 0.
    fastrtps::rtps::security::PluginParticipantSecurityAttributesMask plugin_security_attributes = 0;

    /**
     * @brief Constructor without parameters. <br>
     * Sets the value PID_PARTICIPANT_SECURITY_INFO for the parameter PID and PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH
     * for the length.
     */
    ParameterParticipantSecurityInfo_t()
        : Parameter_t(PID_PARTICIPANT_SECURITY_INFO, PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH)
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

};

//!Default value for the ParameterEndpointSecurityInfo_t length
#define PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH 8

/**
 * @ingroup PARAMETER_MODULE
 */
class ParameterEndpointSecurityInfo_t : public Parameter_t
{
public:

    fastrtps::rtps::security::EndpointSecurityAttributesMask security_attributes = 0;
    fastrtps::rtps::security::PluginEndpointSecurityAttributesMask plugin_security_attributes = 0;

    /**
     * @brief Constructor without parameters. <br>
     * Sets the value PID_ENDPOINT_SECURITY_INFO for the parameter PID and PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH
     * for the length.
     */
    ParameterEndpointSecurityInfo_t()
        : Parameter_t(PID_ENDPOINT_SECURITY_INFO, PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH)
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

};

#endif // if HAVE_SECURITY

///@}

template<class T, class PL>
void set_proxy_property(
        const T& p,
        const char* PID,
        PL& properties)
{
    // only valid values
    if (p == T::unknown())
    {
        return;
    }

    // generate pair
    std::pair<std::string, std::string> pair;
    pair.first = PID;

    std::ostringstream data;
    data << p;
    pair.second = data.str();

    // if exists replace
    auto it = std::find_if(
        properties.begin(),
        properties.end(),
        [&pair](const typename PL::const_iterator::reference p)
        {
            return pair.first == p.first();
        });

    if (it != properties.end())
    {
        // it->modify(pair);
        properties.set_property(it, pair);
    }
    else
    {
        // if not exists add
        properties.push_back(pair.first, pair.second);
    }
}

template<class T, class PL>
T get_proxy_property(
        const char* const PID,
        PL& properties)
{
    T property;

    auto it = std::find_if(
        properties.begin(),
        properties.end(),
        [PID](const typename PL::const_iterator::reference p)
        {
            return PID == p.first();
        });

    if (it != properties.end())
    {
        std::istringstream in(it->second());
        in >> property;
    }

    return property;
}

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // _FASTDDS_DDS_QOS_PARAMETERTYPES_HPP_
