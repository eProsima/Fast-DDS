/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef PAYLOAD_COMPRESSION_IMPL_HPP
#define PAYLOAD_COMPRESSION_IMPL_HPP

#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/transport/low-bandwidth/config.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

#define COMPRESS_LOW_MARK_DEFAULT 128
#define COMPRESS_HIGH_MARK_DEFAULT 512

/**
 * \brief This enumeration identifies the compression libraries that can be used.
 */
enum PayloadCompressionLibrary
{
    /// The Zlib compression library. Used in gz files.
    COMPRESS_LIB_ZLIB = 0,
    /// The BZip2 compression library. Used in bz2 files.
    COMPRESS_LIB_BZIP2 = 1,
    /// Compress with all libraries and send smallest packet.
    COMPRESS_LIB_AUTO = -1
};

/**
 * \brief This structure stores the compression options of one size mark.
 */
struct PayloadCompressionLevel
{
    /// The library to use for this size mark. Default value COMPRESS_LIB_ZLIB.
    PayloadCompressionLibrary library;
    /// The compression level (1 to 9) for this size mark. Default value 9 (best compression).
    uint32_t level;
};

#define COMPRESSION_LEVEL_DEFAULT {COMPRESS_LIB_ZLIB, 9 \
}

/**
 * \brief This structure stores the compression options.
 */
struct PayloadCompressionOptions
{
    /// Compression library and level for small packets.
    PayloadCompressionLevel small_compression;
    /// Compression library and level for medium packets.
    PayloadCompressionLevel medium_compression;
    /// Compression library and level for large packets.
    PayloadCompressionLevel large_compression;
    /// Maximum size of a small packet. Default value is COMPRESS_LOW_MARK_DEFAULT.
    uint32_t low_mark;
    /// Maximum size of medium packets. Default value is COMPRESS_HIGH_MARK_DEFAULT.
    uint32_t high_mark;
};

#define COMPRESSION_OPTIONS_DEFAULT {COMPRESSION_LEVEL_DEFAULT, COMPRESSION_LEVEL_DEFAULT, COMPRESSION_LEVEL_DEFAULT, \
                                     COMPRESS_LOW_MARK_DEFAULT, COMPRESS_HIGH_MARK_DEFAULT}

/*!
 * Initializes the compression libraries.
 */
void PayloadCompression_init();

/*!
 * Allocates a buffer that can hold the compressed data.
 * If has_auto is true, the buffer is allocated to hold the worst case of all libraries.
 * If has_auto is false, the buffer is allocated to hold the worst case of one library.
 *
 * \param buffer Pointer to the allocated buffer.
 * \param data_len Length of the data to be compressed.
 * \param has_auto True if one of the compression levels is COMPRESS_LIB_AUTO.
 * \return The size of the allocated buffer.
 */
uint32_t PayloadCompression_buffer_alloc(
        fastdds::rtps::octet*& buffer,
        uint32_t data_len,
        bool has_auto);

/*!
 * Compresses the data in src_buffer into dest_buffer.
 *
 * \param[in] dest_buffer destination buffer
 * \param[in,out] dest_size Size of destination buffer and returns the size of the compressed data.
 * \param[in] src_buffer Source buffer
 * \param[in] src_length Length of source buffer
 * \param[in] options Compression options
 * \return true if compression was successful, false otherwise.
 */
bool PayloadCompression_compress(
        fastdds::rtps::octet* dest_buffer,
        uint32_t& dest_size,
        const fastdds::rtps::octet* src_buffer,
        const uint32_t src_length,
        const PayloadCompressionLevel& options);

/*!
 * Uncompress the data in src_buffer into dest_buffer.
 *
 * \param[in] dest_buffer destination buffer
 * \param[in,out] dest_size Size of destination buffer and returns the size of the uncompressed data.
 * \param[in] src_buffer Source buffer
 * \param[in] src_length Length of source buffer
 * \return true if uncompression was successful, false otherwise.
 */
bool PayloadCompression_uncompress(
        fastdds::rtps::octet* dest_buffer,
        uint32_t& dest_size,
        const fastdds::rtps::octet* src_buffer,
        const uint32_t src_length);

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // ifndef PAYLOAD_COMPRESSION_IMPL_HPP
