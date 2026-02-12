/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef HEADER_REDUCTION_IMPL_HPP
#define HEADER_REDUCTION_IMPL_HPP

#include <cstdint>

#include <fastdds/rtps/common/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/*!
 * \brief This structure stores the bytes reduction in RTPS Header.
 */
struct HRCONFIG_RTPS_Header
{
    //! If true, the Protocol field is eliminated.
    bool eliminate_protocol;
    //! If true, the Version field is eliminated.
    bool eliminate_version;
    //! If true, the VendorId field is eliminated.
    bool eliminate_vendorId;
    //! Array to reduce the GuidPrefix field. Each element of the array represents a 4-byte block of the GuidPrefix.
    uint32_t reduce_guidPrefix[3]; // Bit level.
};

#define HRCONFIG_RTPS_HEADER_PROTOCOL_SIZE 4
#define HRCONFIG_RTPS_HEADER_VERSION_SIZE 2
#define HRCONFIG_RTPS_HEADER_VENDORID_SIZE 2
#define HRCONFIG_RTPS_HEADER_GUIDPREFIX_SIZE 12
#define HRCONFIG_RTPS_HEADER_DEFAULT {false, false, false, {32, 32, 32 \
                                      } \
}

/*!
 * \brief This structure stores the bytes reduction in Submessage Header.
 */
struct HRCONFIG_Submessage_Header
{
    //! If true, the SubmessageId field is combined with the flags field.
    bool combine_submessageId_with_flags;
};

#define HRCONFIG_SUBMESSAGE_HEADER_ID_SIZE 1
#define HRCONFIG_SUBMESSAGE_HEADER_FLAGS_SIZE 1
#define HRCONFIG_SUBMESSAGE_HEADER_OCTETS_NEXT 2
#define HRCONFIG_SUBMESSAGE_HEADER_DEFAULT {false}

/*!
 * \brief This structure stores the bytes reduction in Submessage Body.
 */
struct HRCONFIG_Submessage_Body
{
    //! If true, the extraFlags field is eliminated.
    bool eliminate_extraFlags;
    //! If true, the octetsToInlineQos field is eliminated.
    uint32_t reduce_entitiesId[2];
    //! If true, the sequenceNumber field is eliminated.
    uint32_t reduce_sequenceNumber;
};

#define HRCONFIG_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE 2
#define HRCONFIG_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE 2
#define HRCONFIG_SUBMESSAGE_BODY_ENTITIESID_SIZE 8
#define HRCONFIG_SUBMESSAGE_BODY_SEQUENCENUMBER_SIZE 8
#define HRCONFIG_SUBMESSAGE_BODY_DEFAULT {false, {32, 32}, 64}

/**
 * \brief This structure stores the bytes reduction in the RTPS packet
 */
struct HeaderReductionOptions
{
    //! RTPS Header reduction options.
    HRCONFIG_RTPS_Header rtps_header;
    //! Submessage Header reduction options.
    HRCONFIG_Submessage_Header submessage_header;
    //! Submessage Body reduction options.
    HRCONFIG_Submessage_Body submessage_body;
};

#define HRCONFIG_RTPS_PACKET_DEFAULT {HRCONFIG_RTPS_HEADER_DEFAULT, HRCONFIG_SUBMESSAGE_HEADER_DEFAULT, \
                                      HRCONFIG_SUBMESSAGE_BODY_DEFAULT}

/*!
 * \brief This function reduces the size of an RTPS packet according to the specified reduction options.
 *
 * \param[out] dest_buffer Pointer to the destination buffer where the reduced packet will be stored.
 * \param[in] src_buffer Pointer to the source buffer containing the original RTPS packet.
 * \param[in,out] buffer_length On input, it represents the size of the source buffer. On output, it represents the size of the reduced packet.
 * \param[in] reductions The HeaderReductionOptions structure specifying which parts of the RTPS packet to reduce.
 * \return true if the reduction was successful, false otherwise.
 */
bool HeaderReduction_Reduce(octet * dest_buffer, const octet * src_buffer, uint32_t & buffer_length,
        const HeaderReductionOptions& reductions);

/*!
 * \brief This function recovers the original RTPS packet from a reduced packet according to the specified reduction options.
 *
 * \param[out] dest_buffer Pointer to the destination buffer where the recovered packet will be stored.
 * \param[in] src_buffer Pointer to the source buffer containing the reduced RTPS packet.
 * \param[in,out] buffer_length On input, it represents the size of the source buffer. On output, it represents the size of the recovered packet.
 * \param[in] reductions The HeaderReductionOptions structure specifying which parts of the RTPS packet were reduced.
 * \return true if the recovery was successful, false otherwise.
 */
bool HeaderReduction_Recover(octet * dest_buffer, const octet * src_buffer, uint32_t & buffer_length,
        const HeaderReductionOptions& reductions);

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // ifndef HEADER_REDUCTION_IMPL_HPP
