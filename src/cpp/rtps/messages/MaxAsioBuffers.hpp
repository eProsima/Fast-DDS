/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

/**
 * @file MaxAsioBuffers.hpp
 *
 * max_asio_buffers / max_boost_buffers, kept in their own header (rather than inline in
 * RTPSMessageGroup.cpp) so a unit test can read the exact production constants without
 * linking the whole RTPSMessageGroup translation unit.
 */

#ifndef FASTDDS_RTPS_MESSAGES__MAXASIOBUFFERS_HPP
#define FASTDDS_RTPS_MESSAGES__MAXASIOBUFFERS_HPP

#include <cstddef>

#ifndef _WIN32
// Same sources asio uses to determine IOV_MAX.
#include <limits.h>
#include <sys/uio.h>
#endif // ifndef _WIN32

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace detail {

/*
 * Maximum number of buffers accepted by a single gather-write.
 *
 * This must mirror asio::detail::buffer_sequence_adapter::max_buffers, which caps the number of
 * buffers of one send operation at min(64, IOV_MAX) and *silently discards* every buffer past
 * that cap. Exceeding it therefore does not fail: the transport sends a datagram holding only
 * the accepted prefix, and the receiver rejects the whole message as truncated.
 *
 * POSIX does not require IOV_MAX to be defined -- QNX explicitly undefines it in <limits.h> even
 * though its socket layer accepts UIO_MAXIOV entries -- and asio falls back to 16 in that case.
 * So derive the cap from the same headers and apply the same fallback instead of assuming 64.
 */
#if defined(_WIN32)
constexpr size_t max_asio_buffers = 64;
#elif defined(IOV_MAX)
constexpr size_t max_asio_buffers = 64 < IOV_MAX ? 64 : static_cast<size_t>(IOV_MAX);
#else
constexpr size_t max_asio_buffers = 16;
#endif // if defined(_WIN32)

// Buffers reserved for the SubMsg header, the SubMsg body, and optional padding.
#ifdef FASTDDS_STATISTICS
constexpr size_t reserved_submessage_buffers = 4;
#else
constexpr size_t reserved_submessage_buffers = 3;
#endif // ifdef FASTDDS_STATISTICS

static_assert(max_asio_buffers > reserved_submessage_buffers + 1,
        "gather-write limit too small to build an RTPS message");

constexpr size_t max_boost_buffers = max_asio_buffers - reserved_submessage_buffers;

} // namespace detail
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_MESSAGES__MAXASIOBUFFERS_HPP
