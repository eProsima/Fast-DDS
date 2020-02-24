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

/**
 * @file LibrarySettingsAttributes.h
 *
 */

#ifndef LIBRARYSETTINGS_ATTRIBUTES_H_
#define LIBRARYSETTINGS_ATTRIBUTES_H_

namespace eprosima {
namespace fastrtps {

enum IntraprocessDeliveryType
{
    INTRAPROCESS_OFF,
    INTRAPROCESS_USER_DATA_ONLY,
    INTRAPROCESS_FULL
};

/**
 * Class LibraySettingsAttributes, used by the user to define the FastRTPS library behaviour
 * @ingroup FASTRTPS_ATTRIBUTES_MODULE
 */
class LibrarySettingsAttributes
{
public:

    LibrarySettingsAttributes() {
    }

    virtual ~LibrarySettingsAttributes() {
    }

    bool operator==(
            const LibrarySettingsAttributes& b) const
    {
        return (intraprocess_delivery == b.intraprocess_delivery);
    }

    IntraprocessDeliveryType intraprocess_delivery = INTRAPROCESS_FULL;
};

}  // namespace fastrtps
}  // namespace eprosima

#endif /* LIBRARYSETTINGS_ATTRIBUTES_H_ */
