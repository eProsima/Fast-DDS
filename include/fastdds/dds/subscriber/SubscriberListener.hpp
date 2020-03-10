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
 * @file SubscriberListener.hpp
 */

#ifndef _FASTDDS_SUBLISTENER_HPP_
#define _FASTDDS_SUBLISTENER_HPP_

#include <fastrtps/fastrtps_dll.h>

#include <fastdds/dds/topic/DataReaderListener.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class Subscriber;

/**
 * Class SubscriberListener, it should be used by the end user to implement specific callbacks to certain actions.
 * @ingroup FASTDDS_MODULE
 */
class RTPS_DllAPI SubscriberListener : public DataReaderListener
{
public:

    SubscriberListener()
    {
    }

    virtual ~SubscriberListener()
    {
    }

    /**
     * Virtual method to be called when there is available data on the Subscriber
     * @param sub Subscriber
     */
    virtual void on_data_on_readers(
            Subscriber* sub)
    {
        (void) sub;
    }

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_SUBLISTENER_HPP_ */
