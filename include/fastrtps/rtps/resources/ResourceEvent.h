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
 * @file ResourceEvent.h
 *
 */

#ifndef RESOURCEEVENT_H_
#define RESOURCEEVENT_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <asio.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps {

class TimedEventImpl;

/**
 * Class ResourceEvent used to manage the temporal events.
 *@ingroup MANAGEMENT_MODULE
 */
class ResourceEvent
{
    public:

        ResourceEvent();

        virtual ~ResourceEvent();

        /**
         * Method to initialize the thread.
         * @param p
         */
        void init_thread();

        void register_timer(TimedEventImpl* event);

        void unregister_timer(TimedEventImpl* event);

        void notify();

        void notify(const std::chrono::steady_clock::time_point& timeout);

        /**
         * Get the associated IO service
         * @return Associated IO service
         */
        asio::io_service& get_io_service() { return io_service_; }

        std::thread& get_thread() { return thread_; }

    private:

        std::atomic<bool> stop_;

        std::timed_mutex mutex_;

        std::condition_variable_any cv_;

        bool notified_;

        bool allow_to_delete_;

        TimedEventImpl* front_;

        TimedEventImpl* back_;

        //!Thread
        std::thread thread_;

        //!IO service
        asio::io_service io_service_;

        asio::steady_timer timer_;

        void event();

        //!Method to run the tasks
        void run_io_service();
};
}
}
} /* namespace eprosima */
#endif
#endif /* RESOURCEEVENT_H_ */
