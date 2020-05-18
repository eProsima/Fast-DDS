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

#ifndef _FASTDDS_PUBLISHERLISTENER_HPP_
#define _FASTDDS_PUBLISHERLISTENER_HPP_

#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastdds/dds/core/status/BaseStatus.hpp>
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
    RTPS_DllAPI PublisherListener()
    {
    }

    /**
     * @brief Destructor
     */
    RTPS_DllAPI virtual ~PublisherListener()
    {
    }

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_PUBLISHERLISTENER_HPP_ */
