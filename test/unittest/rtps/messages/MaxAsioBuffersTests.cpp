/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

/*
 * This test reads the actual production constants from MaxAsioBuffers.hpp -- not a copy of
 * them -- and checks each against an expected value derived independently, branching on the
 * same (_WIN32, IOV_MAX) macros that the header itself branches on. Since those macros are
 * resolved once per build, only the branch this platform actually takes can be exercised
 * here; there is no way to also cover the other platforms' branches in this binary.
 */

#include <cstddef>

#ifndef _WIN32
#include <limits.h>
#include <sys/uio.h>
#endif // ifndef _WIN32

#include <gtest/gtest.h>

#include <rtps/messages/MaxAsioBuffers.hpp>

using eprosima::fastdds::rtps::detail::max_asio_buffers;
using eprosima::fastdds::rtps::detail::max_boost_buffers;
using eprosima::fastdds::rtps::detail::reserved_submessage_buffers;

/*!
 * @fn TEST(MaxAsioBuffers, MaxAsioBuffersMatchesPlatformFormula)
 * @brief Checks max_asio_buffers against the min(64, IOV_MAX) rule (16 as a fallback when
 * IOV_MAX isn't defined, e.g. QNX; always 64 on Windows) for whatever platform this test
 * binary was built on.
 */
TEST(MaxAsioBuffers, MaxAsioBuffersMatchesPlatformFormula)
{
#if defined(_WIN32)
    EXPECT_EQ(max_asio_buffers, 64u);
#elif defined(IOV_MAX)
    EXPECT_EQ(max_asio_buffers, (64 < IOV_MAX ? 64u : static_cast<size_t>(IOV_MAX)));
#else
    EXPECT_EQ(max_asio_buffers, 16u);
#endif // if defined(_WIN32)
}

/*!
 * @fn TEST(MaxAsioBuffers, MaxBoostBuffersMatchesPlatformFormula)
 * @brief Checks max_boost_buffers reserves the right amount of room, on top of
 * max_asio_buffers, for the SubMsg header/body and optional padding.
 */
TEST(MaxAsioBuffers, MaxBoostBuffersMatchesPlatformFormula)
{
    EXPECT_EQ(max_boost_buffers, max_asio_buffers - reserved_submessage_buffers);
}

/*!
 * @fn TEST(MaxAsioBuffers, InvariantHoldsOnThisPlatform)
 * @brief Mirrors the `static_assert(max_asio_buffers > reserved_submessage_buffers + 1, ...)`
 * guard in MaxAsioBuffers.hpp at runtime: the gather-write limit must leave room to build a
 * minimal RTPS message on this platform.
 */
TEST(MaxAsioBuffers, InvariantHoldsOnThisPlatform)
{
    EXPECT_GT(max_asio_buffers, reserved_submessage_buffers + 1);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
