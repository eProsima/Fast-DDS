// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file LibrarySettings.hpp
 *
 */

#ifndef FASTDDS__LIBRARY_SETTINGS_HPP
#define FASTDDS__LIBRARY_SETTINGS_HPP

namespace eprosima {
namespace fastdds {

enum IntraprocessDeliveryType
{
    INTRAPROCESS_OFF,
    INTRAPROCESS_USER_DATA_ONLY,
    INTRAPROCESS_FULL
};

/**
 * Class LibraySettings, used by the user to define the Fast DDS library behaviour
 * @ingroup FASTDDS_MODULE
 */
class LibrarySettings
{
public:

    LibrarySettings()
    {
    }

    virtual ~LibrarySettings()
    {
    }

    bool operator ==(
            const LibrarySettings& b) const
    {
        return (intraprocess_delivery == b.intraprocess_delivery);
    }

    IntraprocessDeliveryType intraprocess_delivery =
#if HAVE_STRICT_REALTIME
            INTRAPROCESS_OFF;
#else
            INTRAPROCESS_FULL;
#endif // if HAVE_STRICT_REALTIME
};

}  // namespace fastdds
}  // namespace eprosima

#endif // FASTDDS__LIBRARY_SETTINGS_HPP
