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

#include <fastrtps/utils/md5.h>
#include <fastrtps/utils/IPFinder.h>

namespace eprosima {

/**
 * This singleton generates a host_id based on system interfaces
 * ip addresses or mac addresses
 */
class Host
{
public:

    static const size_t mac_id_length = 6;
    struct uint48
    {
        unsigned char value [mac_id_length];

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

    static Host& instance()
    {
        static Host singleton;
        return singleton;
    }

private:

    Host()
    {
        // Compute the host id
        fastrtps::rtps::LocatorList_t loc;
        fastrtps::rtps::IPFinder::getIP4Address(&loc);

        {
            if (loc.size() > 0)
            {
                MD5 md5;
                for (auto& l : loc)
                {
                    md5.update(l.address, sizeof(l.address));
                }
                md5.finalize();
                id_ = 0;
                for (size_t i = 0; i < sizeof(md5.digest); i += 2)
                {
                    id_ ^= ((md5.digest[i] << 8) | md5.digest[i + 1]);
                }
            }
            else
            {
                reinterpret_cast<uint8_t*>(&id_)[0] = 127;
                reinterpret_cast<uint8_t*>(&id_)[1] = 1;
            }
        }

        // Compute the MAC id
        std::vector<fastrtps::rtps::IPFinder::info_MAC> macs;
        if (fastrtps::rtps::IPFinder::getAllMACAddress(&macs) &&
                macs.size() > 0)
        {
            MD5 md5;
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
            printf("Cannot get MAC addresses. Failing back to IP based ID\n");
            for (size_t i = 0; i < mac_id_length; i += 2)
            {
                mac_id_.value[i] = (id_ >> 8);
                mac_id_.value[i + 1] = (id_ & 0xFF);
            }
        }
    }

    uint16_t id_;
    uint48 mac_id_;
};

} // eprosima

#endif // UTILS_HOST_HPP_
