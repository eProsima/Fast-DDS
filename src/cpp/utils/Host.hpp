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
 * This singleton generates a host_id based on system interfaces ip addresses
 */
class Host
{
public:

    inline uint16_t id() const
    {
        return id_;
    }

    static Host& get()
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
    }

    uint16_t id_;
};

} // eprosima

#endif // UTILS_HOST_HPP_