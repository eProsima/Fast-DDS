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
 * @file CDRMessage.hpp
 */

#ifndef _FASTDDS_RTPS_CDRMESSAGE_H_
#define _FASTDDS_RTPS_CDRMESSAGE_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastcdr/cdr/fixed_size_string.hpp>

#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/Property.hpp>
#include <fastdds/rtps/common/BinaryProperty.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/FragmentNumber.hpp>
#include <fastdds/rtps/common/SampleIdentity.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>

#include <rtps/security/common/ParticipantGenericMessage.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {
class ParticipantGenericMessage;
} /* namespace security */

/**
 * Namespace CDRMessage, contains inline methods to initialize CDRMessage_t and add or read different data types.
   @ingroup COMMON_MODULE
 */
namespace CDRMessage {

/** @name Read from a CDRMessage_t.
 * Methods to read different data types from a CDR message. Pointers to the message and to the data types are provided.
 * The read position is updated in the message. It fails if you attempt to read outside the
 * boundaries of the message.
 * @param [in] msg Pointer to message.
 * @param [out] data_ptr Pointer to data.
 * @param [in] size Number of bytes (if necessary).
 * @return True if correct.
 */
/// @{
bool readEntityId(
        CDRMessage_t* msg,
        EntityId_t* id);

bool readData(
        CDRMessage_t* msg,
        octet* o,
        uint32_t length);

bool read_array_with_max_size(
        CDRMessage_t* msg,
        octet* arr,
        size_t max_size);

bool readDataReversed(
        CDRMessage_t* msg,
        octet* o,
        uint32_t length);

bool readInt32(
        CDRMessage_t* msg,
        int32_t* lo);

bool readUInt32(
        CDRMessage_t* msg,
        uint32_t* ulo);

bool readInt64(
        CDRMessage_t* msg,
        int64_t* lolo);

bool readUInt64(
        CDRMessage_t* msg,
        uint64_t* lolo);

bool readSequenceNumber(
        CDRMessage_t* msg,
        SequenceNumber_t* sn);

bool readInt16(
        CDRMessage_t* msg,
        int16_t* i16);

bool readUInt16(
        CDRMessage_t* msg,
        uint16_t* i16);

bool readLocator(
        CDRMessage_t* msg,
        Locator_t* loc);

bool readOctet(
        CDRMessage_t* msg,
        octet* o);

SequenceNumberSet_t readSequenceNumberSet(
        CDRMessage_t* msg);

bool readFragmentNumberSet(
        CDRMessage_t* msg,
        FragmentNumberSet_t* snset);

bool readTimestamp(
        CDRMessage_t* msg,
        Time_t* ts);

bool readString(
        CDRMessage_t* msg,
        std::string* p_str);

bool readString(
        CDRMessage_t* msg,
        fastcdr::string_255* stri);

bool readOctetVector(
        CDRMessage_t* msg,
        std::vector<octet>* ocvec);

bool readProperty(
        CDRMessage_t* msg,
        Property& property);

bool readBinaryProperty(
        CDRMessage_t* msg,
        BinaryProperty& binary_property);

bool readPropertySeq(
        CDRMessage_t* msg,
        PropertySeq& properties,
        const uint32_t parameter_length);

bool readBinaryPropertySeq(
        CDRMessage_t* msg,
        BinaryPropertySeq& binary_properties,
        const uint32_t parameter_length);

bool readDataHolder(
        CDRMessage_t* msg,
        DataHolder& data_holder,
        const uint32_t parameter_length);

bool readDataHolderSeq(
        CDRMessage_t* msg,
        DataHolderSeq& data_holders);

bool readMessageIdentity(
        CDRMessage_t* msg,
        security::MessageIdentity& message_identity);

bool readParticipantGenericMessage(
        CDRMessage_t* msg,
        security::ParticipantGenericMessage& message);

bool read_resource_limited_container_config(
        CDRMessage_t* msg,
        ResourceLimitedContainerConfig& config);

bool read_duration_t(
        CDRMessage_t* msg,
        dds::Duration_t& duration);

///@}


/**
 * Initialize given CDR message with default size. It frees the memory already allocated and reserves new one.
 * @param [in,out] msg Pointer to the message to initialize.
 * @param data_size Size of the data the message is suppose to carry
 * @return True if correct.
 */
bool initCDRMsg(
        CDRMessage_t* msg,
        uint32_t data_size = RTPSMESSAGE_COMMON_DATA_PAYLOAD_SIZE);

bool wrapVector(
        CDRMessage_t* msg,
        std::vector<octet>& vectorToWrap);

/**
 * Append given CDRMessage to existing CDR Message. Joins two messages into the first one if it has space.
 * @param [out] first Pointer to first message.
 * @param [in] second Pointer to second message.
 ** @return True if correct.
 */
bool appendMsg(
        CDRMessage_t* first,
        CDRMessage_t* second);


/** @name Add to a CDRMessage_t.
 * Methods to add different data types to a CDR message. Pointers to the message and to the data types are provided.
 * The write position is updated in the message. It fails if you attempt to write outside the
 * boundaries of the message.
 * @param [in,out] Pointer to message.
 * @param [in] data Data to add (might be a pointer).
 * @param [in] byteSize Number of bytes (if necessary).
 * @return True if correct.
 */
/// @{

template<typename T>
bool addPrimitive(
        CDRMessage_t* msg,
        T value);

bool hasSpace(
        CDRMessage_t* msg,
        const uint32_t length);

void copyToBuffer(
        CDRMessage_t* msg,
        const octet* data,
        const uint32_t length,
        bool reverse = false);

bool addData(
        CDRMessage_t*,
        const octet*,
        const uint32_t number_bytes);
bool addDataReversed(
        CDRMessage_t*,
        const octet*,
        const uint32_t byte_number);

bool addOctet(
        CDRMessage_t* msg,
        octet o);

bool addUInt16(
        CDRMessage_t* msg,
        uint16_t us);

bool addInt32(
        CDRMessage_t* msg,
        int32_t lo);

bool addUInt32(
        CDRMessage_t* msg,
        uint32_t lo);

bool addInt64(
        CDRMessage_t* msg,
        int64_t lo);

bool addUInt64(
        CDRMessage_t* msg,
        uint64_t lo);

bool addEntityId(
        CDRMessage_t* msg,
        const EntityId_t* id);

bool addSequenceNumber(
        CDRMessage_t* msg,
        const SequenceNumber_t* sn);

bool addSequenceNumberSet(
        CDRMessage_t* msg,
        const SequenceNumberSet_t* sns);

bool addFragmentNumberSet(
        CDRMessage_t* msg,
        FragmentNumberSet_t* fns);

bool addLocator(
        CDRMessage_t* msg,
        const Locator_t& loc);

bool add_string(
        CDRMessage_t* msg,
        const char* in_str);

bool add_string(
        CDRMessage_t* msg,
        const std::string& in_str);

bool add_string(
        CDRMessage_t* msg,
        const fastcdr::string_255& in_str);

bool addOctetVector(
        CDRMessage_t* msg,
        const std::vector<octet>* ocvec,
        bool add_final_padding = true);

bool addProperty(
        CDRMessage_t* msg,
        const Property& property);

bool addBinaryProperty(
        CDRMessage_t* msg,
        const BinaryProperty& binary_property,
        bool add_final_padding = true);

bool addPropertySeq(
        CDRMessage_t* msg,
        const PropertySeq& properties);

bool addBinaryPropertySeq(
        CDRMessage_t* msg,
        const BinaryPropertySeq& binary_properties,
        bool add_final_padding);

bool addBinaryPropertySeq(
        CDRMessage_t* msg,
        const BinaryPropertySeq& binary_properties,
        const std::string& name_start,
        bool add_final_padding);

bool addDataHolder(
        CDRMessage_t* msg,
        const DataHolder& data_holder);

bool addDataHolderSeq(
        CDRMessage_t* msg,
        const DataHolderSeq& data_holders);

bool addMessageIdentity(
        CDRMessage_t* msg,
        const security::MessageIdentity& message_identity);

bool addParticipantGenericMessage(
        CDRMessage_t* msg,
        const security::ParticipantGenericMessage& message);

bool add_resource_limited_container_config(
        CDRMessage_t* msg,
        const ResourceLimitedContainerConfig& config);

bool add_duration_t(
        CDRMessage_t* msg,
        const dds::Duration_t& duration);
///@}

/**
 * @brief Skip bytes in serialized buffer
 *
 * @param msg The CDR message
 * @param length The number of bytes to skip
 * @return true if skipped, false otherwise
 */
bool skip(
        CDRMessage_t* msg,
        uint32_t length);

} /* namespace CDRMessage */

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_CDRMESSAGE_H_ */
