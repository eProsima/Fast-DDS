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
namespace fastrtps {
namespace rtps {

class GUID_t;

class IDataSharingNotifier
{

public:

    IDataSharingNotifier() = default;

    ~IDataSharingNotifier() = default;

    /**
     * Adds a new reader to notify
     * 
     * @param reader_guid GUID of the reader to add to the notification list
     * @return Whether the initialization was  successful
     */
    virtual bool add_reader(
            const GUID_t& reader_guid) = 0;

    /**
     * Removes a reader from notifications
     * 
     * @param reader_guid GUID of the reader to remove from the notification list
     * @return Whether the stop was successful
     */
    virtual bool remove_reader(
            const GUID_t& reader_guid) = 0;

    /**
     * Checks if the reader with the given GUID is subscribed to data sharing notifications
     * 
     * @param reader_guid GUID of the reader to check
     * @return Whether the reader is subscribed or not
     */
    virtual bool reader_is_subscribed(
            const GUID_t& reader_guid) const = 0;

    /**
     * Notifies to all subscribed readers
     */
    virtual void notify() = 0;

    /**
     * Checks if there is any reader subscribed to notifications
     * 
     * @return True if there is no reader subscribed
     */
    virtual inline bool empty() const = 0;

};


}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_DATASHARING_IDATASHARINGNOTIFIER_HPP
