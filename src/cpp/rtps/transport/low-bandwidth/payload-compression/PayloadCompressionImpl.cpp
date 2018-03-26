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


#include "PayloadCompressionImpl.h"

#if HAVE_ZLIB || HAVE_BZIP2

#if HAVE_ZLIB
#include "zlib.h"
#endif

#if HAVE_BZIP2
#include "bzlib.h"
#endif

#include <stdlib.h>
#include <string.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

uint32_t PayloadCompression_buffer_alloc(
        fastrtps::rtps::octet*& buffer,
        uint32_t data_len,
        bool has_auto)
{

    uint32_t size = data_len + 1;
    uint32_t max_size = size;
    uint32_t num_libs = 0;
#if HAVE_ZLIB
    num_libs++;
    size = compressBound(data_len) + 1;
    if (size > max_size)
    {
        max_size = size;
    }
#endif

#if HAVE_BZIP2
    num_libs++;
    size = data_len + (uint32_t) (data_len / 100) + 600 + 1;
    if (size > max_size)
    {
        max_size = size;
    }
#endif

    buffer = (fastrtps::rtps::octet*) malloc(has_auto ? max_size * num_libs : max_size);
    return max_size;
}

bool PayloadCompression_compress(
        fastrtps::rtps::octet* dest_buffer,
        uint32_t& dest_size,
        const fastrtps::rtps::octet* src_buffer,
        const uint32_t buffer_length,
        const PayloadCompressionLevel& options)
{
    if (options.library == COMPRESS_LIB_AUTO)
    {
        fastrtps::rtps::octet* aux_buf = &dest_buffer[1];
        fastrtps::rtps::octet* min_buf = nullptr;
        uint32_t min_compress_size = UINT32_MAX;
#if HAVE_ZLIB
        uLongf zlib_size = dest_size - 1;
        if ((Z_OK == compress2(aux_buf, &zlib_size, src_buffer, buffer_length, options.level)) &&
                (zlib_size < min_compress_size))
        {
            min_compress_size = zlib_size;
            dest_buffer[0] = (fastrtps::rtps::octet) 'Z';
            min_buf = aux_buf;
            aux_buf = (aux_buf == &dest_buffer[1]) ? &dest_buffer[dest_size] : &dest_buffer[1];
        }
#endif
#if HAVE_BZIP2
        unsigned int bzip2_size = dest_size - 1;
        if ((BZ_OK ==
                BZ2_bzBuffToBuffCompress((char*)aux_buf, &bzip2_size, (char*)src_buffer, buffer_length,
                options.level, 0, 0)) &&
                (bzip2_size < min_compress_size) )
        {
            min_compress_size = bzip2_size;
            dest_buffer[0] = (fastrtps::rtps::octet) 'B';
            min_buf = aux_buf;
            aux_buf = (aux_buf == &dest_buffer[1]) ? &dest_buffer[dest_size] : &dest_buffer[1];
        }
#endif

        if (min_buf != nullptr)
        {
            if (min_buf != &dest_buffer[1])
            {
                memcpy(&dest_buffer[1], min_buf, min_compress_size);
            }

            dest_size = min_compress_size + 1;
            return true;
        }
    }
#if HAVE_ZLIB
    else if (options.library == COMPRESS_LIB_ZLIB)
    {
        uLongf out_size = dest_size - 1;
        if (Z_OK == compress2(&dest_buffer[1], &out_size, src_buffer, buffer_length, options.level))
        {
            dest_buffer[0] = (fastrtps::rtps::octet) 'Z';
            dest_size = out_size + 1;
            return true;
        }
    }
#endif
#if HAVE_BZIP2
    else if (options.library == COMPRESS_LIB_BZIP2)
    {
        unsigned int out_size = dest_size - 1;
        if (BZ_OK ==
                BZ2_bzBuffToBuffCompress((char*)&dest_buffer[1], &out_size, (char*)src_buffer, buffer_length,
                options.level, 0, 0))
        {
            dest_buffer[0] = (fastrtps::rtps::octet) 'B';
            dest_size = out_size + 1;
            return true;
        }
    }
#endif

    return false;
}

bool PayloadCompression_uncompress(
        fastrtps::rtps::octet* dest_buffer,
        uint32_t& dest_size,
        const fastrtps::rtps::octet* src_buffer,
        const uint32_t src_length)
{
    if (src_length <= 0)
    {
        return false;
    }

#if HAVE_ZLIB
    if (src_buffer[0] == (fastrtps::rtps::octet) 'Z')
    {
        uLongf out_size = dest_size;
        if (Z_OK == uncompress(dest_buffer, &out_size, &src_buffer[1], src_length - 1))
        {
            dest_size = out_size;
            return true;
        }

        return false;
    }
#endif
#if HAVE_BZIP2
    if (src_buffer[0] == (fastrtps::rtps::octet) 'B')
    {
        unsigned int out_size = dest_size;
        if (BZ_OK ==
                BZ2_bzBuffToBuffDecompress((char*) dest_buffer, &out_size, (char*) &src_buffer[1], src_length - 1,
                0, 0))
        {
            dest_size = out_size;
            return true;
        }

        return false;
    }
#endif

    return false;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
