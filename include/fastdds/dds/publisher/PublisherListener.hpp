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
 * @file PublisherListener.hpp
 */

#ifndef FASTDDS_DDS_PUBLISHER__PUBLISHERLISTENER_HPP
#define FASTDDS_DDS_PUBLISHER__PUBLISHERLISTENER_HPP

#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class Publisher;

/**
 * Class PublisherListener, allows the end user to implement callbacks triggered by certain events.
 * It inherits all the DataWriterListener callbacks.
 * @ingroup FASTDDS_MODULE
 */
class PublisherListener : public DataWriterListener
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API PublisherListener()
    {
    }

    /**
     * @brief Destructor
     */
    FASTDDS_EXPORTED_API virtual ~PublisherListener()
    {
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_PUBLISHER__PUBLISHERLISTENER_HPP
