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

/**
 * @file IDataSharingNotifier.hpp
 */

#ifndef RTPS_DATASHARING_IDATASHARINGNOTIFIER_HPP
#define RTPS_DATASHARING_IDATASHARINGNOTIFIER_HPP


namespace eprosima {
namespace fastdds {
namespace rtps {

struct GUID_t;

class IDataSharingNotifier
{

public:

    IDataSharingNotifier() = default;

    virtual ~IDataSharingNotifier() = default;

    /**
     * Links the notifier to a reader that will be notified
     * @param reader_guid GUID of the reader to notify
     */
    virtual void enable(
            const GUID_t& reader_guid) = 0;

    /**
     * Unlinks the notifier and its reader
     */
    virtual void disable() = 0;

    /**
     * @return whether the notifier is linked to a reader or not
     */
    virtual bool is_enabled() = 0;

    /**
     * Notifies to the reader
     */
    virtual void notify() = 0;

};


}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_DATASHARING_IDATASHARINGNOTIFIER_HPP
