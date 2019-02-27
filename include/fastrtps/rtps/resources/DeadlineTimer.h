// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DeadlineTimer.h
 *
 */

#ifndef DeadlineTimer_H_
#define DeadlineTimer_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "TimedEvent.h"

#include <functional>

namespace eprosima {
namespace fastrtps{
namespace rtps {

/** A class that starts a timer and invokes a callback when the timer expires
 * @ingroup MANAGEMENT_MODULE
 */
class DeadlineTimer : public TimedEvent
{
public:

    /** Constructor
     * @param callback A callback to invoke when the timer expires
     * @param period Interval of the DeadlineTimer in milliseconds
     * @param service IO service to run the event
     * @param event_thread starting thread for identification.
     */
    DeadlineTimer(
            std::function<void()> callback,
            Duration_t period,
            asio::io_service &service,
            const std::thread& event_thread);

    /** Destructor
     */
    virtual ~DeadlineTimer();

    /** Method invoked when the event occurs
     * @param code Code representing the status of the event
     * @param msg Message associated to the event. It can be nullptr
     */
    virtual void event(EventCode code, const char* msg) override;

private:

    //! Callback invoked when the timer expires
    std::function<void()> callback_;
};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif
