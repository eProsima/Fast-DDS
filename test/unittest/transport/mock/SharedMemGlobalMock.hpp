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

#ifndef _FASTDDS_MOCKSHAREDMEM_GLOBAL_H_
#define _FASTDDS_MOCKSHAREDMEM_GLOBAL_H_

#include <rtps/transport/shared_mem/SharedMemGlobal.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class MockPortSharedMemGlobal
{
public:

    static void remove_port_mutex(
            const std::string& domain_name,
            uint32_t port_id)
    {
        auto port_segment_name = domain_name + "_port" + std::to_string(port_id);

        SharedMemSegment::named_mutex::remove(port_segment_name.c_str());
    }

    static deleted_unique_ptr<SharedMemSegment::named_mutex> get_port_mutex(
            const std::string& domain_name,
            uint32_t port_id,
            bool open_only = true)
    {
        auto port_segment_name = domain_name + "_port" + std::to_string(port_id);

        if (open_only)
        {
            return deleted_unique_ptr<SharedMemSegment::named_mutex>(SharedMemSegment::open_named_mutex(
                               port_segment_name + "_mutex"));
        }
        else
        {
            return deleted_unique_ptr<SharedMemSegment::named_mutex>(SharedMemSegment::open_or_create_named_mutex(
                               port_segment_name + "_mutex"));
        }
    }

    static bool lock_empty_cv_mutex(
            SharedMemGlobal::Port& port)
    {
        return port.node_->empty_cv_mutex.try_lock();
    }

    /**
     * Simulates a deadlocked wait_pop.
     * Deadlock until is_listener_closed is true
     */
    static void wait_pop_deadlock(
            SharedMemGlobal::Port& port,
            SharedMemGlobal::Listener& listener,
            const std::atomic<bool>& is_listener_closed,
            uint32_t listener_index)
    {
        (void)listener;

        std::unique_lock<SharedMemSegment::mutex> lock(port.node_->empty_cv_mutex);

        port.node_->waiting_count++;
        auto& status = port.node_->listeners_status[listener_index];
        status.is_waiting = 1;

        port.node_->empty_cv.wait(lock, [&]
                {
                    return is_listener_closed.load();
                });

        status.is_waiting = 0;
        port.node_->waiting_count--;
    }

    static void unblock_wait_pop(
            SharedMemGlobal::Port& port,
            std::atomic<bool>& is_listener_closed)
    {
        is_listener_closed.exchange(true);
        port.node_->empty_cv.notify_all();
    }

    static void set_port_not_ok(
            SharedMemGlobal::Port& port)
    {
        port.node_->is_port_ok = false;
    }

    static void forze_listener_leak(
            SharedMemGlobal::Port& port)
    {
        port.node_->ref_counter.fetch_add(1);
        auto listener = port.buffer_->register_listener();
        listener.release();
        port.node_->num_listeners++;
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_MOCKSHAREDMEM_GLOBAL_H_
