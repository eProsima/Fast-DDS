// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UTILS_HOST_HPP_
#define UTILS_HOST_HPP_

#include <fastcdr/cdr/fixed_size_string.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/utils/IPFinder.hpp>
#include <fastdds/utils/md5.hpp>

#if defined(_WIN32)
#include <WinSock2.h> // Avoid conflicts with WinSock of Windows.h
#include <windows.h>
#include <process.h>
#elif defined(__APPLE__)
#include <IOKit/IOKitLib.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif // if defined(_WIN32)


namespace eprosima {


/**
 * This singleton generates a host_id based on system interfaces
 * ip addresses, mac addresses or the machine UUID.
 */
class Host
{
public:

    static const size_t mac_id_length = 6;
    struct uint48
    {
        unsigned char value[mac_id_length];

        uint48()
        {
            memset(value, 0, mac_id_length);
        }

    };

    inline uint16_t id() const
    {
        return id_;
    }

    inline uint48 mac_id() const
    {
        return mac_id_;
    }

    inline fastcdr::string_255 machine_id() const
    {
        return machine_id_;
    }

    static Host& instance()
    {
        static Host singleton;
        return singleton;
    }

    static inline uint16_t compute_id(
            const fastdds::rtps::LocatorList& loc)
    {
        uint16_t ret_val = 0;

        if (loc.size() > 0)
        {
            fastdds::MD5 md5;
            for (auto& l : loc)
            {
                md5.update(l.address, sizeof(l.address));
            }
            md5.finalize();

            // Hash the 16-bytes md5.digest into a uint16_t
            ret_val = 0;
            for (size_t i = 0; i < sizeof(md5.digest); i += 2)
            {
                // Treat the next two bytes as a big-endian uint16_t and
                // hash them into ret_val.
                uint16_t tmp = static_cast<uint16_t>(md5.digest[i]);
                tmp = (tmp << 8) | static_cast<uint16_t>(md5.digest[i + 1]);
                ret_val ^= tmp;
            }
        }
        else
        {
            reinterpret_cast<uint8_t*>(&ret_val)[0] = 127;
            reinterpret_cast<uint8_t*>(&ret_val)[1] = 1;
        }

        return ret_val;
    }

private:

    Host()
    {
        // Compute the host id
        fastdds::rtps::LocatorList loc;
        fastdds::rtps::IPFinder::getIP4Address(&loc);
        id_ = compute_id(loc);

        // Compute the MAC id
        std::vector<fastdds::rtps::IPFinder::info_MAC> macs;
        if (fastdds::rtps::IPFinder::getAllMACAddress(&macs) &&
                macs.size() > 0)
        {
            fastdds::MD5 md5;
            for (auto& m : macs)
            {
                md5.update(m.address, sizeof(m.address));
            }
            md5.finalize();
            for (size_t i = 0, j = 0; i < sizeof(md5.digest); ++i, ++j)
            {
                if (j >= mac_id_length)
                {
                    j = 0;
                }
                mac_id_.value[j] ^= md5.digest[i];
            }
        }
        else
        {
            EPROSIMA_LOG_WARNING(UTILS, "Cannot get MAC addresses. Failing back to IP based ID");
            for (size_t i = 0; i < mac_id_length; i += 2)
            {
                mac_id_.value[i] = (id_ >> 8);
                mac_id_.value[i + 1] = (id_ & 0xFF);
            }
        }

        // Compute the machine id hash
        machine_id_ = compute_machine_id();
        if (machine_id_ == "")
        {
            EPROSIMA_LOG_WARNING(UTILS, "Cannot get machine id. Failing back to IP based ID");
            machine_id_ = std::to_string(id_);
        }
    }


    static std::string compute_machine_id()
    {
    #ifdef _WIN32
        char machine_id[255];
        DWORD BufferSize = sizeof(machine_id);
        LONG res = RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", "MachineGuid", RRF_RT_REG_SZ, NULL, machine_id, &BufferSize);
        if (res == 0)
        {
            return std::string(machine_id);
        }
        return "";
    #elif defined(__APPLE__)
        io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
        CFStringRef uuidCf = (CFStringRef) IORegistryEntryCreateCFProperty(ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
        IOObjectRelease(ioRegistryRoot);
        CFStringGetCString(uuidCf, buf, 255, kCFStringEncodingMacRoman);
        CFRelease(uuidCf);
        return std::string(buf, 255);
    #else
        int fd = open("/etc/machine-id", O_RDONLY);
        if (fd == -1) {
            return "";
        }

        char buffer[33] = {0};
        ssize_t bytes_read = read(fd, buffer, 32);
        close(fd);

        if (bytes_read < 32) {
            return "";
        }

        return std::string(buffer, 32);
    #endif // if defined(_WIN32)
    }

    uint16_t id_;
    uint48 mac_id_;
    fastcdr::string_255 machine_id_;
};

} // eprosima

#endif // UTILS_HOST_HPP_
