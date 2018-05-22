
#ifndef RTCP_HEADER_H
#define RTCP_HEADER_H

#include <fastrtps/rtps/common/Types.h>
#include <cstring>

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

    static inline size_t GetSize() { return 14; }
};

union TCPTransactionId
{
    uint32_t ints[3];
    octet octets[12];

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
};

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

    static inline size_t GetSize()
    {
        return 16;
    }

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
};


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTCP_HEADER_H