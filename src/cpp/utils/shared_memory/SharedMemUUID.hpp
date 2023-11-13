// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_SHAREDMEM_UUID_H_
#define _FASTDDS_SHAREDMEM_UUID_H_

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // ifdef _MSC_VER

#include <array>
#include <cstdint>
#include <chrono>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * UUIDs Generator Singleton
 */
class UUIDGen
{
public:

    UUIDGen()
        : last_gen_time_(0)
    {

    }

    static UUIDGen& instance()
    {
        static UUIDGen singleton_uuid_gen;
        return singleton_uuid_gen;
    }

    /**
     * Generates UUIDs guaranteed to be unique only inside the same machine.
     * @param uuid pointer to an array of bytes to store the uuid.
     * @param len lenght in bytes of the uuid.
     * @remarks Thread-safe.
     */
    void generate(
            uint8_t* uuid,
            size_t len)
    {
        uint64_t now = std::chrono::high_resolution_clock::now().time_since_epoch().count();

        {
            // Two IDs cannot be generated in this process at the same exact time
            std::lock_guard<std::mutex> lock(gen_mutex_);

            while (now == last_gen_time_)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            }

            last_gen_time_ = now;
        }

        // Mersenne twister generator
        std::mt19937 gen;

        // The seed is derived from the ProcessID and the steady clock
        std::array<uint32_t, 4> seq;
        uint64_t pid = this_process_pid();
        seq[0] = static_cast<uint32_t>(pid);
        seq[1] = static_cast<uint32_t>(pid >> 32);
        seq[2] = static_cast<uint32_t>(now);
        seq[3] = static_cast<uint32_t>(now >> 32);

        std::seed_seq seed_seq(seq.begin(), seq.end());
        gen.seed(seed_seq);

        std::uniform_int_distribution<> dis(0, 255);

        for (size_t i = 0; i < len; i++)
        {
            uuid[i] = static_cast<uint8_t>(dis(gen));
        }
    }

private:

    std::mutex gen_mutex_;
    uint64_t last_gen_time_;

    static uint64_t this_process_pid()
    {
#ifdef _MSC_VER
        return GetCurrentProcessId();
#else // POSIX
        return static_cast<uint64_t>(getpid());
#endif // ifdef _MSC_VER
    }

};

template <size_t SIZE>
class UUID
{
public:

    struct null_t {};
    static constexpr null_t null = null_t();

    UUID()
    {
    }

    UUID(
            const null_t&)
    {
        memset(uuid_, 0, sizeof(uuid_));
    }

    static void generate(
            UUID& other)
    {
        UUIDGen::instance().generate(other.uuid_, sizeof(other.uuid_));
    }

    uint8_t* get()
    {
        return uuid_;
    }

    size_t size()
    {
        return sizeof(uuid_);
    }

    size_t hash() const
    {
        static_assert(sizeof(uuid_) >= sizeof(size_t), "UUID size should accomodate a size_t");
        return *reinterpret_cast<const size_t*>(uuid_);
    }

    std::string to_string() const
    {
        // TODO(Adolfo): This function should be allocation free
        std::stringstream ss;

        for (size_t i = 0; i < sizeof(uuid_); i++ )
        {
            std::stringstream hex_ss;
            hex_ss << std::hex << static_cast<unsigned int>(uuid_[i]);
            std::string hex = hex_ss.str();
            ss << (hex.length() < 2 ? '0' + hex : hex);
        }

        return ss.str();
    }

    bool operator == (
            const UUID& other) const
    {
        return 0 == memcmp(uuid_, other.uuid_, sizeof(uuid_));
    }

private:

    uint8_t uuid_[SIZE];

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

namespace std {
template <>
struct hash<eprosima::fastdds::rtps::UUID<8>>
{
    std::size_t operator ()(
            const eprosima::fastdds::rtps::UUID<8>& k) const
    {
        return k.hash();
    }

};

} // namespace std


#endif // _FASTDDS_SHAREDMEM_UUID_H_