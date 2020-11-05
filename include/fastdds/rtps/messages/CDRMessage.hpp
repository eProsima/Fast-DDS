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

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * @file CDRMessage.hpp
 *
 */

#include <cassert>
#include <cstring>
#include <algorithm>
#include <vector>

#include <fastdds/dds/core/policy/ParameterTypes.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

inline bool CDRMessage::initCDRMsg(
        CDRMessage_t* msg,
        uint32_t payload_size)
{
    if (msg->buffer == nullptr)
    {
        msg->buffer = (octet*)malloc(payload_size + RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE);
        msg->max_size = payload_size + RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE;
    }
    msg->pos = 0;
    msg->length = 0;
    msg->msg_endian = DEFAULT_ENDIAN;
    return true;
}

inline bool CDRMessage::wrapVector(
        CDRMessage_t* msg,
        std::vector<octet>& vectorToWrap)
{
    if (msg->buffer && !msg->wraps)
    {
        free(msg->buffer);
    }

    msg->wraps = true;
    msg->buffer = vectorToWrap.data();
    msg->length = (uint32_t)vectorToWrap.size();
    msg->max_size = (uint32_t)vectorToWrap.capacity();
    msg->msg_endian = DEFAULT_ENDIAN;
    return true;
}

inline bool CDRMessage::appendMsg(
        CDRMessage_t* first,
        CDRMessage_t* second)
{
    return(CDRMessage::addData(first, second->buffer, second->length));
}

inline bool CDRMessage::readEntityId(
        CDRMessage_t* msg,
        EntityId_t* id)
{
    if (msg->pos + 4 > msg->length)
    {
        return false;
    }
    memcpy(id->value, &msg->buffer[msg->pos], id->size);
    msg->pos += 4;
    return true;
}

inline bool CDRMessage::readData(
        CDRMessage_t* msg,
        octet* o,
        uint32_t length)
{
    if (msg->pos + length > msg->length)
    {
        return false;
    }
    memcpy(o, &msg->buffer[msg->pos], length);
    msg->pos += length;
    return true;
}

inline bool CDRMessage::read_array_with_max_size(
        CDRMessage_t* msg,
        octet* arr,
        size_t max_size)
{
    if (msg->pos + 4 > msg->length)
    {
        return false;
    }
    uint32_t datasize;
    bool valid = CDRMessage::readUInt32(msg, &datasize);
    if (max_size < datasize)
    {
        return false;
    }
    valid &= CDRMessage::readData(msg, arr, datasize);
    msg->pos = (msg->pos + 3u) & ~3u;
    return valid;
}

inline bool CDRMessage::readDataReversed(
        CDRMessage_t* msg,
        octet* o,
        uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        *(o + i) = *(msg->buffer + msg->pos + length - 1 - i);
    }
    msg->pos += length;
    return true;
}

inline bool CDRMessage::readInt32(
        CDRMessage_t* msg,
        int32_t* lo)
{
    if (msg->pos + 4 > msg->length)
    {
        return false;
    }
    octet* dest = (octet*)lo;
    if (msg->msg_endian == DEFAULT_ENDIAN)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            dest[i] = msg->buffer[msg->pos + i];
        }
        msg->pos += 4;
    }
    else
    {
        readDataReversed(msg, dest, 4);
    }
    return true;
}

inline bool CDRMessage::readUInt32(
        CDRMessage_t* msg,
        uint32_t* ulo)
{
    if (msg->pos + 4 > msg->length)
    {
        return false;
    }
    octet* dest = (octet*)ulo;
    if (msg->msg_endian == DEFAULT_ENDIAN)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            dest[i] = msg->buffer[msg->pos + i];
        }
        msg->pos += 4;
    }
    else
    {
        readDataReversed(msg, dest, 4);
    }
    return true;
}

inline bool CDRMessage::readInt64(
        CDRMessage_t* msg,
        int64_t* lolo)
{
    if (msg->pos + 8 > msg->length)
    {
        return false;
    }

    octet* dest = (octet*)lolo;
    if (msg->msg_endian == DEFAULT_ENDIAN)
    {
        for (uint8_t i = 0; i < 8; ++i)
        {
            dest[i] = msg->buffer[msg->pos + i];
        }

        msg->pos += 8;
    }
    else
    {
        readDataReversed(msg, dest, 8);
    }

    return true;
}

inline bool CDRMessage::readSequenceNumber(
        CDRMessage_t* msg,
        SequenceNumber_t* sn)
{
    if (msg->pos + 8 > msg->length)
    {
        return false;
    }
    bool valid = readInt32(msg, &sn->high);
    valid &= readUInt32(msg, &sn->low);
    return true;
}

inline SequenceNumberSet_t CDRMessage::readSequenceNumberSet(
        CDRMessage_t* msg)
{
    bool valid = true;
    SequenceNumberSet_t sns(c_SequenceNumber_Unknown);

    SequenceNumber_t seqNum;
    valid &= CDRMessage::readSequenceNumber(msg, &seqNum);
    uint32_t numBits = 0;
    valid &= CDRMessage::readUInt32(msg, &numBits);
    valid &= (numBits <= 256u);

    uint32_t n_longs = (numBits + 31u) / 32u;
    uint32_t bitmap[8];
    for (uint32_t i = 0; valid && (i < n_longs); ++i)
    {
        valid &= CDRMessage::readUInt32(msg, &bitmap[i]);
    }

    if (valid)
    {
        sns.base(seqNum);
        sns.bitmap_set(numBits, bitmap);
    }

    return sns;
}

inline bool CDRMessage::readFragmentNumberSet(
        CDRMessage_t* msg,
        FragmentNumberSet_t* fns)
{
    bool valid = true;

    FragmentNumber_t base = 0ul;
    valid &= CDRMessage::readUInt32(msg, &base);
    uint32_t numBits = 0;
    valid &= CDRMessage::readUInt32(msg, &numBits);
    valid &= (numBits <= 256u);

    uint32_t n_longs = (numBits + 31u) / 32u;
    uint32_t bitmap[8];
    for (uint32_t i = 0; valid && (i < n_longs); ++i)
    {
        valid &= CDRMessage::readUInt32(msg, &bitmap[i]);
    }

    if (valid)
    {
        fns->base(base);
        fns->bitmap_set(numBits, bitmap);
    }

    return valid;
}

inline bool CDRMessage::readTimestamp(
        CDRMessage_t* msg,
        rtps::Time_t* ts)
{
    bool valid = true;
    valid &= CDRMessage::readInt32(msg, &ts->seconds());
    uint32_t frac(0);
    valid &= CDRMessage::readUInt32(msg, &frac);
    ts->fraction(frac);
    return valid;
}

inline bool CDRMessage::readLocator(
        CDRMessage_t* msg,
        Locator_t* loc)
{
    if (msg->pos + 24 > msg->length)
    {
        return false;
    }

    bool valid = readInt32(msg, &loc->kind);
    valid &= readUInt32(msg, &loc->port);
    valid &= readData(msg, loc->address, 16);

    return valid;
}

inline bool CDRMessage::readInt16(
        CDRMessage_t* msg,
        int16_t* i16)
{
    if (msg->pos + 2 > msg->length)
    {
        return false;
    }
    octet* o = (octet*)i16;
    if (msg->msg_endian == DEFAULT_ENDIAN)
    {
        *o = msg->buffer[msg->pos];
        *(o + 1) = msg->buffer[msg->pos + 1];
    }
    else
    {
        *o = msg->buffer[msg->pos + 1];
        *(o + 1) = msg->buffer[msg->pos];
    }
    msg->pos += 2;
    return true;
}

inline bool CDRMessage::readUInt16(
        CDRMessage_t* msg,
        uint16_t* i16)
{
    if (msg->pos + 2 > msg->length)
    {
        return false;
    }
    octet* o = (octet*)i16;
    if (msg->msg_endian == DEFAULT_ENDIAN)
    {
        *o = msg->buffer[msg->pos];
        *(o + 1) = msg->buffer[msg->pos + 1];
    }
    else
    {
        *o = msg->buffer[msg->pos + 1];
        *(o + 1) = msg->buffer[msg->pos];
    }
    msg->pos += 2;
    return true;
}

inline bool CDRMessage::readOctet(
        CDRMessage_t* msg,
        octet* o)
{
    if (msg->pos + 1 > msg->length)
    {
        return false;
    }
    *o = msg->buffer[msg->pos];
    msg->pos++;
    return true;
}

inline bool CDRMessage::readOctetVector(
        CDRMessage_t* msg,
        std::vector<octet>* ocvec)
{
    if (msg->pos + 4 > msg->length)
    {
        return false;
    }
    uint32_t vecsize;
    bool valid = CDRMessage::readUInt32(msg, &vecsize);
    ocvec->resize(vecsize);
    valid &= CDRMessage::readData(msg, ocvec->data(), vecsize);
    msg->pos = (msg->pos + 3u) & ~3u;
    return valid;
}

inline bool CDRMessage::readString(
        CDRMessage_t* msg,
        std::string* stri)
{
    uint32_t str_size = 1;
    bool valid = true;
    valid &= CDRMessage::readUInt32(msg, &str_size);
    if (msg->pos + str_size > msg->length)
    {
        return false;
    }

    stri->clear();
    if (str_size > 1)
    {
        stri->resize(str_size - 1);
        for (uint32_t i = 0; i < str_size - 1; i++)
        {
            stri->at(i) = static_cast<char>(msg->buffer[msg->pos + i]);
        }
    }
    msg->pos += str_size;
    msg->pos = (msg->pos + 3u) & ~3u;

    return valid;
}

inline bool CDRMessage::readString(
        CDRMessage_t* msg,
        string_255* stri)
{
    uint32_t str_size = 1;
    bool valid = true;
    valid &= CDRMessage::readUInt32(msg, &str_size);
    if (msg->pos + str_size > msg->length)
    {
        return false;
    }

    *stri = "";
    if (str_size > 1)
    {
        *stri = (const char*) &(msg->buffer[msg->pos]);
    }
    msg->pos += str_size;
    msg->pos = (msg->pos + 3u) & ~3u;

    return valid;
}

inline bool CDRMessage::addData(
        CDRMessage_t* msg,
        const octet* data,
        const uint32_t length)
{
    if (msg->pos + length > msg->max_size)
    {
        return false;
    }

    memcpy(&msg->buffer[msg->pos], data, length);
    msg->pos += length;
    msg->length += length;
    return true;
}

inline bool CDRMessage::addDataReversed(
        CDRMessage_t* msg,
        const octet* data,
        const uint32_t length)
{
    if (msg->pos + length > msg->max_size)
    {
        return false;
    }
    for (uint32_t i = 0; i < length; i++)
    {
        msg->buffer[msg->pos + i] = *(data + length - 1 - i);
    }
    msg->pos += length;
    msg->length += length;
    return true;
}

inline bool CDRMessage::addOctet(
        CDRMessage_t* msg,
        octet O)
{
    if (msg->pos + 1 > msg->max_size)
    {
        return false;
    }
    //const void* d = (void*)&O;
    msg->buffer[msg->pos] = O;
    msg->pos++;
    msg->length++;
    return true;
}

inline bool CDRMessage::addUInt16(
        CDRMessage_t* msg,
        uint16_t us)
{
    if (msg->pos + 2 > msg->max_size)
    {
        return false;
    }
    octet* o = (octet*)&us;
    if (msg->msg_endian == DEFAULT_ENDIAN)
    {
        msg->buffer[msg->pos] = *(o);
        msg->buffer[msg->pos + 1] = *(o + 1);
    }
    else
    {
        msg->buffer[msg->pos] = *(o + 1);
        msg->buffer[msg->pos + 1] = *(o);
    }
    msg->pos += 2;
    msg->length += 2;
    return true;
}

inline bool CDRMessage::addInt32(
        CDRMessage_t* msg,
        int32_t lo)
{
    octet* o = (octet*)&lo;
    if (msg->pos + 4 > msg->max_size)
    {
        return false;
    }
    if (msg->msg_endian == DEFAULT_ENDIAN)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            msg->buffer[msg->pos + i] = *(o + i);
        }
    }
    else
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            msg->buffer[msg->pos + i] = *(o + 3 - i);
        }
    }
    msg->pos += 4;
    msg->length += 4;
    return true;
}

inline bool CDRMessage::addUInt32(
        CDRMessage_t* msg,
        uint32_t ulo)
{
    octet* o = (octet*)&ulo;
    if (msg->pos + 4 > msg->max_size)
    {
        return false;
    }
    if (msg->msg_endian == DEFAULT_ENDIAN)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            msg->buffer[msg->pos + i] = *(o + i);
        }
    }
    else
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            msg->buffer[msg->pos + i] = *(o + 3 - i);
        }
    }
    msg->pos += 4;
    msg->length += 4;
    return true;
}

inline bool CDRMessage::addInt64(
        CDRMessage_t* msg,
        int64_t lolo)
{
    octet* o = (octet*)&lolo;
    if (msg->pos + 8 > msg->max_size)
    {
        return false;
    }
    if (msg->msg_endian == DEFAULT_ENDIAN)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            msg->buffer[msg->pos + i] = *(o + i);
        }
    }
    else
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            msg->buffer[msg->pos + i] = *(o + 7 - i);
        }
    }
    msg->pos += 8;
    msg->length += 8;
    return true;
}

inline bool CDRMessage::addOctetVector(
        CDRMessage_t* msg,
        const std::vector<octet>* ocvec,
        bool add_final_padding)
{
    // TODO Calculate without padding
    auto final_size = msg->pos + ocvec->size();
    if (add_final_padding)
    {
        final_size += 4;
    }
    if (final_size >= msg->max_size)
    {
        return false;
    }
    bool valid = CDRMessage::addUInt32(msg, (uint32_t)ocvec->size());
    valid &= CDRMessage::addData(msg, (octet*)ocvec->data(), (uint32_t)ocvec->size());

    if (add_final_padding)
    {
        int rest = ocvec->size() % 4;
        if (rest != 0)
        {
            rest = 4 - rest; //how many you have to add

            octet oc = '\0';
            for (int i = 0; i < rest; i++)
            {
                valid &= CDRMessage::addOctet(msg, oc);
            }
        }
    }

    return valid;
}

inline bool CDRMessage::addEntityId(
        CDRMessage_t* msg,
        const EntityId_t* ID)
{
    if (msg->pos + 4 >= msg->max_size)
    {
        return false;
    }
    memcpy(&msg->buffer[msg->pos], ID->value, ID->size);
    msg->pos += 4;
    msg->length += 4;
    return true;
}

inline bool CDRMessage::addSequenceNumber(
        CDRMessage_t* msg,
        const SequenceNumber_t* sn)
{
    addInt32(msg, sn->high);
    addUInt32(msg, sn->low);

    return true;
}

inline bool CDRMessage::addSequenceNumberSet(
        CDRMessage_t* msg,
        const SequenceNumberSet_t* sns)
{
    SequenceNumber_t base = sns->base();
    CDRMessage::addSequenceNumber(msg, &base);

    //Add set
    if (sns->empty())
    {
        addUInt32(msg, 0); //numbits 0
        return true;
    }

    uint32_t numBits;
    uint32_t n_longs;
    std::array<uint32_t, 8> bitmap;
    sns->bitmap_get(numBits, bitmap, n_longs);

    addUInt32(msg, numBits);

    for (uint32_t i = 0; i < n_longs; i++)
    {
        addUInt32(msg, bitmap[i]);
    }

    return true;
}

inline bool CDRMessage::addFragmentNumberSet(
        CDRMessage_t* msg,
        FragmentNumberSet_t* fns)
{
    FragmentNumber_t base = fns->base();
    if (base == 0)
    {
        return false;
    }

    CDRMessage::addUInt32(msg, base);

    //Add set
    if (fns->empty())
    {
        addUInt32(msg, 0); //numbits 0
        return true;
    }

    uint32_t numBits;
    uint32_t n_longs;
    std::array<uint32_t, 8> bitmap;
    fns->bitmap_get(numBits, bitmap, n_longs);

    addUInt32(msg, numBits);

    for (uint32_t i = 0; i < n_longs; i++)
    {
        addUInt32(msg, bitmap[i]);
    }

    return true;
}

inline bool CDRMessage::addLocator(
        CDRMessage_t* msg,
        const Locator_t& loc)
{
    addInt32(msg, loc.kind);
    addUInt32(msg, loc.port);
    addData(msg, loc.address, 16);
    return true;
}

inline bool CDRMessage::add_string(
        CDRMessage_t* msg,
        const char* in_str)
{
    uint32_t str_siz = static_cast<uint32_t>(strlen(in_str) + 1);
    bool valid = CDRMessage::addUInt32(msg, str_siz);
    valid &= CDRMessage::addData(msg, (unsigned char*) in_str, str_siz);
    octet oc = '\0';
    for (; str_siz& 3; ++str_siz)
    {
        valid &= CDRMessage::addOctet(msg, oc);
    }
    return valid;
}

inline bool CDRMessage::add_string(
        CDRMessage_t* msg,
        const std::string& in_str)
{
    return add_string(msg, in_str.c_str());
}

inline bool CDRMessage::add_string(
        CDRMessage_t* msg,
        const string_255& in_str)
{
    return add_string(msg, in_str.c_str());
}

inline bool CDRMessage::addProperty(
        CDRMessage_t* msg,
        const Property& property)
{
    assert(msg);

    if (property.propagate())
    {
        if (!CDRMessage::add_string(msg, property.name()))
        {
            return false;
        }
        if (!CDRMessage::add_string(msg, property.value()))
        {
            return false;
        }
    }

    return true;
}

inline bool CDRMessage::readProperty(
        CDRMessage_t* msg,
        Property& property)
{
    assert(msg);

    if (!CDRMessage::readString(msg, &property.name()))
    {
        return false;
    }
    if (!CDRMessage::readString(msg, &property.value()))
    {
        return false;
    }

    return true;
}

inline bool CDRMessage::addBinaryProperty(
        CDRMessage_t* msg,
        const BinaryProperty& binary_property,
        bool add_final_padding)
{
    assert(msg);

    if (binary_property.propagate())
    {
        if (!CDRMessage::add_string(msg, binary_property.name()))
        {
            return false;
        }
        if (!CDRMessage::addOctetVector(msg, &binary_property.value(), add_final_padding))
        {
            return false;
        }
    }

    return true;
}

inline bool CDRMessage::readBinaryProperty(
        CDRMessage_t* msg,
        BinaryProperty& binary_property)
{
    assert(msg);

    if (!CDRMessage::readString(msg, &binary_property.name()))
    {
        return false;
    }
    if (!CDRMessage::readOctetVector(msg, &binary_property.value()))
    {
        return false;
    }
    binary_property.propagate(true);

    return true;
}

inline bool CDRMessage::addPropertySeq(
        CDRMessage_t* msg,
        const PropertySeq& properties)
{
    assert(msg);

    bool returnedValue = false;

    if (msg->pos + 4 <=  msg->max_size)
    {
        uint32_t number_to_serialize = 0;
        for (auto it = properties.begin(); it != properties.end(); ++it)
        {
            if (it->propagate())
            {
                ++number_to_serialize;
            }
        }

        if (CDRMessage::addUInt32(msg, number_to_serialize))
        {
            returnedValue = true;
            for (auto it = properties.begin(); returnedValue && it != properties.end(); ++it)
            {
                if (it->propagate())
                {
                    returnedValue = CDRMessage::addProperty(msg, *it);
                }
            }
        }
    }

    return returnedValue;
}

inline bool CDRMessage::readPropertySeq(
        CDRMessage_t* msg,
        PropertySeq& properties)
{
    assert(msg);

    uint32_t length = 0;
    if (!CDRMessage::readUInt32(msg, &length))
    {
        return false;
    }

    properties.resize(length);
    bool returnedValue = true;
    for (uint32_t i = 0; returnedValue && i < length; ++i)
    {
        returnedValue = CDRMessage::readProperty(msg, properties.at(i));
    }

    return returnedValue;

}

inline bool CDRMessage::addBinaryPropertySeq(
        CDRMessage_t* msg,
        const BinaryPropertySeq& binary_properties,
        bool add_final_padding)
{
    assert(msg);

    bool returnedValue = false;

    if (msg->pos + 4 <=  msg->max_size)
    {
        uint32_t number_to_serialize = 0;
        for (auto it = binary_properties.begin(); it != binary_properties.end(); ++it)
        {
            if (it->propagate())
            {
                ++number_to_serialize;
            }
        }

        if (CDRMessage::addUInt32(msg, number_to_serialize))
        {
            returnedValue = true;
            for (auto it = binary_properties.begin(); returnedValue && it != binary_properties.end(); ++it)
            {
                if (it->propagate())
                {
                    --number_to_serialize;
                    returnedValue =
                            CDRMessage::addBinaryProperty(msg, *it,
                                    add_final_padding || (number_to_serialize != 0));
                }
            }
        }
    }

    return returnedValue;
}

inline bool CDRMessage::addBinaryPropertySeq(
        CDRMessage_t* msg,
        const BinaryPropertySeq& binary_properties,
        const std::string& name_start,
        bool add_final_padding)
{
    assert(msg);

    bool returnedValue = false;

    if (msg->pos + 4 <=  msg->max_size)
    {
        uint32_t number_to_serialize = 0;
        for (auto it = binary_properties.begin(); it != binary_properties.end(); ++it)
        {
            if (it->name().find(name_start) == 0)
            {
                ++number_to_serialize;
            }
        }

        if (CDRMessage::addUInt32(msg, number_to_serialize))
        {
            returnedValue = true;
            for (auto it = binary_properties.begin(); returnedValue && it != binary_properties.end(); ++it)
            {
                if (it->name().find(name_start) == 0)
                {
                    --number_to_serialize;
                    returnedValue =
                            CDRMessage::addBinaryProperty(msg, *it,
                                    add_final_padding || (number_to_serialize != 0));
                }
            }
        }
    }

    return returnedValue;
}

inline bool CDRMessage::readBinaryPropertySeq(
        CDRMessage_t* msg,
        BinaryPropertySeq& binary_properties)
{
    assert(msg);

    uint32_t length = 0;
    if (!CDRMessage::readUInt32(msg, &length))
    {
        return false;
    }

    binary_properties.resize(length);
    bool returnedValue = true;
    for (uint32_t i = 0; returnedValue && i < length; ++i)
    {
        returnedValue = CDRMessage::readBinaryProperty(msg, binary_properties.at(i));
    }

    return returnedValue;
}

inline bool CDRMessage::addDataHolder(
        CDRMessage_t* msg,
        const DataHolder& data_holder)
{
    assert(msg);

    if (!CDRMessage::add_string(msg, data_holder.class_id()))
    {
        return false;
    }
    if (!CDRMessage::addPropertySeq(msg, data_holder.properties()))
    {
        return false;
    }
    if (!CDRMessage::addBinaryPropertySeq(msg, data_holder.binary_properties(), true))
    {
        return false;
    }

    return true;
}

inline bool CDRMessage::readDataHolder(
        CDRMessage_t* msg,
        DataHolder& data_holder)
{
    assert(msg);

    if (!CDRMessage::readString(msg, &data_holder.class_id()))
    {
        return false;
    }
    if (!CDRMessage::readPropertySeq(msg, data_holder.properties()))
    {
        return false;
    }
    if (!CDRMessage::readBinaryPropertySeq(msg, data_holder.binary_properties()))
    {
        return false;
    }

    return true;

}

inline bool CDRMessage::addDataHolderSeq(
        CDRMessage_t* msg,
        const DataHolderSeq& data_holders)
{
    assert(msg);

    bool returnedValue = false;

    if (msg->pos + 4 <=  msg->max_size)
    {
        if (CDRMessage::addUInt32(msg, static_cast<uint32_t>(data_holders.size())))
        {
            returnedValue = true;
            for (auto data_holder = data_holders.begin(); returnedValue && data_holder != data_holders.end();
                    ++data_holder)
            {
                returnedValue = CDRMessage::addDataHolder(msg, *data_holder);
            }
        }
    }

    return returnedValue;
}

inline bool CDRMessage::readDataHolderSeq(
        CDRMessage_t* msg,
        DataHolderSeq& data_holders)
{
    assert(msg);

    uint32_t length = 0;
    if (!CDRMessage::readUInt32(msg, &length))
    {
        return false;
    }

    data_holders.resize(length);
    bool returnedValue = true;
    for (uint32_t i = 0; returnedValue && i < length; ++i)
    {
        returnedValue = CDRMessage::readDataHolder(msg, data_holders.at(i));
    }

    return returnedValue;
}

inline bool CDRMessage::addMessageIdentity(
        CDRMessage_t* msg,
        const security::MessageIdentity& message_identity)
{
    assert(msg);

    if (!CDRMessage::addData(msg, message_identity.source_guid().guidPrefix.value, GuidPrefix_t::size))
    {
        return false;
    }
    if (!CDRMessage::addData(msg, message_identity.source_guid().entityId.value, EntityId_t::size))
    {
        return false;
    }
    if (!CDRMessage::addInt64(msg, message_identity.sequence_number()))
    {
        return false;
    }

    return true;
}

inline bool CDRMessage::readMessageIdentity(
        CDRMessage_t* msg,
        security::MessageIdentity& message_identity)
{
    assert(msg);

    if (!CDRMessage::readData(msg, message_identity.source_guid().guidPrefix.value, GuidPrefix_t::size))
    {
        return false;
    }
    if (!CDRMessage::readData(msg, message_identity.source_guid().entityId.value, EntityId_t::size))
    {
        return false;
    }
    if (!CDRMessage::readInt64(msg, &message_identity.sequence_number()))
    {
        return false;
    }

    return true;
}

inline bool CDRMessage::addParticipantGenericMessage(
        CDRMessage_t* msg,
        const security::ParticipantGenericMessage& message)
{
    assert(msg);

    if (!CDRMessage::addMessageIdentity(msg, message.message_identity()))
    {
        return false;
    }
    if (!CDRMessage::addMessageIdentity(msg, message.related_message_identity()))
    {
        return false;
    }
    if (!CDRMessage::addData(msg, message.destination_participant_key().guidPrefix.value, GuidPrefix_t::size))
    {
        return false;
    }
    if (!CDRMessage::addData(msg, message.destination_participant_key().entityId.value, EntityId_t::size))
    {
        return false;
    }
    if (!CDRMessage::addData(msg, message.destination_endpoint_key().guidPrefix.value, GuidPrefix_t::size))
    {
        return false;
    }
    if (!CDRMessage::addData(msg, message.destination_endpoint_key().entityId.value, EntityId_t::size))
    {
        return false;
    }
    if (!CDRMessage::addData(msg, message.source_endpoint_key().guidPrefix.value, GuidPrefix_t::size))
    {
        return false;
    }
    if (!CDRMessage::addData(msg, message.source_endpoint_key().entityId.value, EntityId_t::size))
    {
        return false;
    }
    if (!CDRMessage::add_string(msg, message.message_class_id()))
    {
        return false;
    }
    if (!CDRMessage::addDataHolderSeq(msg, message.message_data()))
    {
        return false;
    }

    return true;
}

inline bool CDRMessage::readParticipantGenericMessage(
        CDRMessage_t* msg,
        security::ParticipantGenericMessage& message)
{
    assert(msg);

    if (!CDRMessage::readMessageIdentity(msg, message.message_identity()))
    {
        return false;
    }
    if (!CDRMessage::readMessageIdentity(msg, message.related_message_identity()))
    {
        return false;
    }
    if (!CDRMessage::readData(msg, message.destination_participant_key().guidPrefix.value, GuidPrefix_t::size))
    {
        return false;
    }
    if (!CDRMessage::readData(msg, message.destination_participant_key().entityId.value, EntityId_t::size))
    {
        return false;
    }
    if (!CDRMessage::readData(msg, message.destination_endpoint_key().guidPrefix.value, GuidPrefix_t::size))
    {
        return false;
    }
    if (!CDRMessage::readData(msg, message.destination_endpoint_key().entityId.value, EntityId_t::size))
    {
        return false;
    }
    if (!CDRMessage::readData(msg, message.source_endpoint_key().guidPrefix.value, GuidPrefix_t::size))
    {
        return false;
    }
    if (!CDRMessage::readData(msg, message.source_endpoint_key().entityId.value, EntityId_t::size))
    {
        return false;
    }
    if (!CDRMessage::readString(msg, &message.message_class_id()))
    {
        return false;
    }
    if (!CDRMessage::readDataHolderSeq(msg, message.message_data()))
    {
        return false;
    }

    return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
