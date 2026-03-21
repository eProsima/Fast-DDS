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

/*
 * Host.cpp
 *
 */

#include <utils/Host.hpp>

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

fastcdr::string_255 Host::compute_machine_id()
{
    #ifdef _WIN32
    char machine_id[255];
    DWORD BufferSize = sizeof(machine_id);
    LONG res = RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", "MachineGuid", RRF_RT_REG_SZ,
                    NULL, machine_id, &BufferSize);
    if (res == 0)
    {
        return machine_id;
    }
    return "";
    #elif defined(__APPLE__)
    io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMainPortDefault, "IOService:/");
    if (!ioRegistryRoot)
    {
        return "";
    }
    CFStringRef uuidCf = (CFStringRef) IORegistryEntryCreateCFProperty(ioRegistryRoot, CFSTR(
                        kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
    IOObjectRelease(ioRegistryRoot);
    if (!uuidCf)
    {
        return "";
    }

    char buf[255];
    if (!CFStringGetCString(uuidCf, buf, sizeof(buf), kCFStringEncodingUTF8))
    {
        return "";
    }
    CFRelease(uuidCf);
    return buf;
    #elif defined(_POSIX_SOURCE)
    int fd = open("/etc/machine-id", O_RDONLY);
    if (fd == -1)
    {
        return "";
    }

    char buffer[33] = {0};
    ssize_t bytes_read = read(fd, buffer, 32);
    close(fd);

    if (bytes_read < 32)
    {
        return "";
    }

    return buffer;
    #else
    return "";
    #endif // if defined(_WIN32)
}

} // eprosima
