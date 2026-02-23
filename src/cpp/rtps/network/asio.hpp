/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef RTPS_TRANSPORT_ASIO_HPP
#define RTPS_TRANSPORT_ASIO_HPP

#if defined(__GNUC__) && !defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wnull-dereference"
#elif defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wnull-dereference" // clang name differs by version; might be -Wnull-dereference or -Wnull-pointer-arithmetic etc.
#endif // if defined(__GNUC__) && !defined(__clang__)

#include <asio.hpp>

#if defined(__GNUC__) && !defined(__clang__)
#  pragma GCC diagnostic pop
#elif defined(__clang__)
#  pragma clang diagnostic pop
#endif // if defined(__GNUC__) && !defined(__clang__)

#endif // RTPS_TRANSPORT_ASIO_HPP
