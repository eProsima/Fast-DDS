
#ifndef RTCP_HEADER_H
#define RTCP_HEADER_H

#include <fastrtps/TopicDataType.h>
#include <fastrtps/rtps/common/Types.h>
#include <cstring>
#include <fastcdr/FastCdr.h>
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

// TCP Header structs and enums.
struct TCPHeader
{
    const char rtcp[4];
    uint32_t length;
    uint32_t crc;
    uint16_t logicalPort;

    TCPHeader() :
		rtcp{'R','T','C','P'}
        , length(sizeof(TCPHeader))
        , crc(0)
        , logicalPort(0)
    {
        //memcpy((char*)rtcp, "RTCP", sizeof(char) * 4);
    }

    const octet* getAddress() const
    {
        return (const octet*)this;
    }
    
    /*!
     * @brief This function returns the maximum serialized size of an object
     * depending on the buffer alignment.
     * @param current_alignment Buffer alignment.
     * @return Maximum serialized size.
     */
    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0)
    {
        size_t initial_alignment = current_alignment;
                
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);

        return current_alignment - initial_alignment;
    }

    /*!
     * @brief This function returns the serialized size of a data depending on the buffer alignment.
     * @param data Data which is calculated its serialized size.
     * @param current_alignment Buffer alignment.
     * @return Serialized size.
     */
    static size_t getCdrSerializedSize(const TCPHeader&, size_t current_alignment = 0)
    {
        return TCPHeader::getMaxCdrSerializedSize(current_alignment);
    }


    /*!
     * @brief This function serializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    void serialize(eprosima::fastcdr::Cdr &cdr) const
    {
        for (int i = 0; i < 4; ++i)
        {
            cdr << rtcp[i];
        }
        cdr << length;
        cdr << crc;
        cdr << logicalPort;        
    }

    /*!
     * @brief This function deserializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    void deserialize(eprosima::fastcdr::Cdr &cdr)
    {
        char temp[4];
        for (int i = 0; i < 4; ++i)
        {
            cdr >> temp[i];
        }
        memcpy((char*)rtcp, temp, 4);
        cdr >> length;
        cdr >> crc;
        cdr >> logicalPort;   
    }



    /*!
     * @brief This function returns the maximum serialized size of the Key of an object
     * depending on the buffer alignment.
     * @param current_alignment Buffer alignment.
     * @return Maximum serialized size.
     */
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0)
    {
        return current_alignment;
    }

    /*!
     * @brief This function tells you if the Key has been defined for this type
     */
    static bool isKeyDefined() { return false; }

    /*!
     * @brief This function serializes the key members of an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    void serializeKey(eprosima::fastcdr::Cdr &) const {}

    bool serialize(SerializedPayload_t *payload)
    {
        TCPHeader *p_type = this;
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload->data, payload->max_size); // Object that manages the raw buffer.
        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        // Serialize encapsulation
        ser.serialize_encapsulation();

        try
        {
            p_type->serialize(ser); // Serialize the object:
        }
        catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            return false;
        }

        payload->length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        return true;
    }

    bool deserialize(SerializedPayload_t *payload)
    {
        TCPHeader* p_type = this; 	//Convert DATA to pointer of your type
        eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length); // Object that manages the raw buffer.
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.
        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        try
        {
            p_type->deserialize(deser); //Deserialize the object:
        }
        catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            return false;
        }

        return true;
    }
};

union TCPTransactionId
{
    uint32_t ints[3];
    octet octets[12];

    TCPTransactionId()
    {
        memset(ints, 0, 3 * sizeof(uint32_t));
    }

    TCPTransactionId(const TCPTransactionId& t)
    {
        memcpy(ints, t.ints, 3 * sizeof(uint32_t));
    }

    TCPTransactionId& operator++()
    {
        if (ints[0] == 0xffffffff)
        {
            if (ints[1] == 0xffffffff)
            {
                if (ints[2] == 0xffffffff)
                {
                    memset(ints, 0, 3 * sizeof(uint32_t));
                }
                else
                {
                    ints[2] += 1;
                }
            }
            else
            {
                ints[1] += 1;
            }
        }
        else
        {
            ints[0] += 1;
        }
        return *this;
    }

    TCPTransactionId operator++(int)
    {
        TCPTransactionId prev = *this;
        ++(*this);
        return prev;
    }


    TCPTransactionId& operator=(const TCPTransactionId& t)
    {
        memcpy(ints, t.ints, 3 * sizeof(uint32_t));
        return *this;
    }

    TCPTransactionId& operator=(const octet* id)
    {
        memcpy(octets, id, 12 * sizeof(octet));
        return *this;
    }

    TCPTransactionId& operator=(const char* id)
    {
        memcpy(octets, id, 12 * sizeof(octet));
        return *this;
    }

    TCPTransactionId& operator=(const uint32_t* id)
    {
        memcpy(ints, id, 3 * sizeof(uint32_t));
        return *this;
    }

    TCPTransactionId& operator=(uint32_t id)
    {
        ints[0] = id;
        ints[1] = 0;
        ints[2] = 0;
        return *this;
    }

    TCPTransactionId& operator=(uint64_t id)
    {
        memset(ints, 0, sizeof(uint32_t) * 3);
        memcpy(ints, &id, sizeof(uint64_t));
        return *this;
    }

    bool operator==(const TCPTransactionId& t) const
    {
        return memcmp(ints, t.ints, 3 * sizeof(uint32_t)) == 0;
    }

    bool operator<(const TCPTransactionId& t) const
    {
        return memcmp(ints, t.ints, 3 * sizeof(uint32_t)) < 0;
    }
};

inline std::ostream& operator<<(std::ostream& output,const TCPTransactionId& t)
{
    bool printed = false; // Don't skip cases like 99 0 34
    for (int i = 2; i >= 0; --i)
    {
        if (printed || t.ints[i] > 0)
        {
            output << t.ints[i]; 
            printed = true;
        }
    }
    return output;
}

enum TCPCPMKind : octet
{
    BIND_CONNECTION_REQUEST =           0xD1,
    BIND_CONNECTION_RESPONSE =          0xE1,
    OPEN_LOGICAL_PORT_REQUEST =         0xD2,
    OPEN_LOGICAL_PORT_RESPONSE =        0xE2,
    CHECK_LOGICAL_PORT_REQUEST =        0xD3,
    CHECK_LOGICAL_PORT_RESPONSE =       0xE3,
    KEEP_ALIVE_REQUEST =                0xD4,
    KEEP_ALIVE_RESPONSE =               0xE4,
    LOGICAL_PORT_IS_CLOSED_REQUEST =    0xD5,
    UNBIND_CONNECTION_REQUEST =         0xD6
};

struct TCPControlMsgHeader
{
    TCPCPMKind kind; // 1 byte
    octet flags; // 1 byte
    uint16_t length; // 2 bytes
    TCPTransactionId transactionId; // 12 bytes

    void setFlags(bool endianess, bool hasPayload, bool requiresResponse)
    {
        octet e = (endianess) ? BIT(1) : 0x00;
        octet p = (hasPayload) ? BIT(2) : 0x00;
        octet r = (requiresResponse) ? BIT(3) : 0x00;
        flags = e | p | r;
    }

    void setEndianess(Endianness_t endianess)
    {
        // Endianess flag has inverse logic than Endianness_t :-/
        if (endianess == Endianness_t::BIGEND)
        {
            flags &= 0xFE;
        }
        else
        {
            flags |= BIT(1);
        }
    }

    void setHasPayload(bool hasPayload)
    {
        if (hasPayload)
        {
            flags |= BIT(2);
        }
        else
        {
            flags &= 0xFD;
        }
    }

    void setRequiresResponse(bool requiresResponse)
    {
        if (requiresResponse)
        {
            flags |= BIT(3);
        }
        else
        {
            flags &= 0xFB;
        }
    }

    bool getEndianess()
    {
        return (flags & BIT(1)) != 0;
    }

    bool getHasPayload()
    {
        return (flags & BIT(2)) != 0;
    }

    bool getRequiresResponse()
    {
        return (flags & BIT(3)) != 0;
    }
    
    /*!
     * @brief This function returns the maximum serialized size of an object
     * depending on the buffer alignment.
     * @param current_alignment Buffer alignment.
     * @return Maximum serialized size.
     */
    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0)
    {
        size_t initial_alignment = current_alignment;
                
        current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
        current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
        current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
        current_alignment += 12 + eprosima::fastcdr::Cdr::alignment(current_alignment, 12);

        return current_alignment - initial_alignment;
    }

    /*!
     * @brief This function returns the serialized size of a data depending on the buffer alignment.
     * @param data Data which is calculated its serialized size.
     * @param current_alignment Buffer alignment.
     * @return Serialized size.
     */
    static size_t getCdrSerializedSize(const TCPControlMsgHeader&, size_t current_alignment = 0)
    {
        return TCPControlMsgHeader::getMaxCdrSerializedSize(current_alignment);
    }


    /*!
     * @brief This function serializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    void serialize(eprosima::fastcdr::Cdr &cdr) const
    {
        cdr << (octet)kind;
        cdr << flags;
        cdr << length;     
        for (int i = 0; i < 12; ++i)
        {
            cdr << transactionId.octets[i];
        }   
    }

    /*!
     * @brief This function deserializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    void deserialize(eprosima::fastcdr::Cdr &cdr)
    {
        cdr >> *((octet*)&kind);
        cdr >> flags;
        cdr >> length;     
        for (int i = 0; i < 12; ++i)
        {
            cdr >> transactionId.octets[i];
        }     
    }



    /*!
     * @brief This function returns the maximum serialized size of the Key of an object
     * depending on the buffer alignment.
     * @param current_alignment Buffer alignment.
     * @return Maximum serialized size.
     */
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0)
    {
        return current_alignment;
    }

    /*!
     * @brief This function tells you if the Key has been defined for this type
     */
    static bool isKeyDefined() { return false; }

    /*!
     * @brief This function serializes the key members of an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    void serializeKey(eprosima::fastcdr::Cdr &) const {}

    bool serialize(SerializedPayload_t *payload)
    {
        TCPControlMsgHeader *p_type = this;
        eprosima::fastcdr::FastBuffer fastbuffer((char*) payload->data, payload->max_size); // Object that manages the raw buffer.
        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
        payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        // Serialize encapsulation
        ser.serialize_encapsulation();

        try
        {
            p_type->serialize(ser); // Serialize the object:
        }
        catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            return false;
        }

        payload->length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
        return true;
    }

    bool deserialize(SerializedPayload_t *payload)
    {
        TCPControlMsgHeader* p_type = this; 	//Convert DATA to pointer of your type
        eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length); // Object that manages the raw buffer.
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.
        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        try
        {
            p_type->deserialize(deser); //Deserialize the object:
        }
        catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            return false;
        }

        return true;
    }
};


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTCP_HEADER_H