// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef PAYLOAD_COMPRESSION_IMPL_H
#define PAYLOAD_COMPRESSION_IMPL_H

#include <fastrtps/rtps/common/Types.h>

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
    /// Maximum size of medium packets. Default value is COMPRESS_LOW_MARK_DEFAULT.
    uint32_t high_mark;
};

#define COMPRESSION_OPTIONS_DEFAULT {COMPRESSION_LEVEL_DEFAULT, COMPRESSION_LEVEL_DEFAULT, COMPRESSION_LEVEL_DEFAULT, \
                                     COMPRESS_LOW_MARK_DEFAULT, COMPRESS_HIGH_MARK_DEFAULT}

uint32_t PayloadCompression_buffer_alloc(
        fastrtps::rtps::octet*& buffer,
        uint32_t data_len,
        bool has_auto);

bool PayloadCompression_compress(
        fastrtps::rtps::octet* dest_buffer,
        uint32_t& dest_size,
        const fastrtps::rtps::octet* src_buffer,
        const uint32_t src_length,
        const PayloadCompressionLevel& options);

bool PayloadCompression_uncompress(
        fastrtps::rtps::octet* dest_buffer,
        uint32_t& dest_size,
        const fastrtps::rtps::octet* src_buffer,
        const uint32_t src_length);

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
