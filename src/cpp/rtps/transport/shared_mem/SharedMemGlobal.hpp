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

#ifndef _FASTDDS_SHAREDMEM_GLOBAL_H_
#define _FASTDDS_SHAREDMEM_GLOBAL_H_

#include <vector>
#include <mutex>
#include <memory>

#include <rtps/transport/shared_mem/SharedMemSegment.hpp>
#include <rtps/transport/shared_mem/MultiProducerConsumerRingBuffer.hpp>

#define THREADID "(ID:" << std::this_thread::get_id() <<") "

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * This class defines the global resources for shared-memory communication.
 * Mainly the shared-memory ports and its operations.
 */
class SharedMemGlobal
{
public:

    SharedMemGlobal(
            const std::string& domain_name)
        : domain_name_(domain_name)
    {

    }

    ~SharedMemGlobal()
    {

    }

    /**
     * Identifies a data buffer given its segment_id (shared-memory segment global_name)
     * and offset inside the segment
     */
    struct BufferDescriptor
    {
        SharedMemSegment::Id source_segment_id;
        SharedMemSegment::offset buffer_node_offset;
    };

    typedef MultiProducerConsumerRingBuffer<BufferDescriptor>::Listener Listener;
    typedef MultiProducerConsumerRingBuffer<BufferDescriptor>::Cell PortCell;

    struct PortNode
    {
        UUID<8> uuid;
        uint32_t port_id;
        SharedMemSegment::condition_variable empty_cv;
        SharedMemSegment::mutex empty_cv_mutex;
        SharedMemSegment::offset buffer;
        SharedMemSegment::offset buffer_node;
        std::atomic<uint32_t> ref_counter;
        uint32_t waiting_count;
        uint32_t check_awaken_count;
        uint32_t check_id;
        bool is_port_ok;
        bool is_opened_read_exclusive;
        bool is_opened_for_reading;
    };

    /**
     * A shared-memory port is a communication channel where data can be written / read.
     * A port has a port_id and a global name derived from the port_id and the domain.
     * System processes can open a port by knowing its name.
     */
    class Port
    {
        friend class MockPortSharedMemGlobal;

    private:

        std::unique_ptr<SharedMemSegment> port_segment_;

        PortNode* node_;

        std::unique_ptr<MultiProducerConsumerRingBuffer<BufferDescriptor> > buffer_;

        uint64_t overflows_count_;

        bool was_check_thread_detached_;

        bool check_all_waiting_threads_alive(
                uint32_t time_out_ms)
        {
            bool is_check_ok = false;

            {
                std::lock_guard<SharedMemSegment::mutex> lock_empty(node_->empty_cv_mutex);

                if (!node_->is_port_ok)
                {
                    throw std::runtime_error("Previous check failed");
                }

                node_->check_id++;
                node_->check_awaken_count = node_->waiting_count;

                node_->empty_cv.notify_all();
            }

            auto start = std::chrono::high_resolution_clock::now();

            do
            {
                std::this_thread::yield();
                is_check_ok = node_->check_awaken_count == 0;
            }
            while (!is_check_ok &&
                    std::chrono::high_resolution_clock::now() < start + std::chrono::milliseconds(time_out_ms));

            return is_check_ok;
        }

        inline void notify_unicast(
                bool was_buffer_empty_before_push)
        {
            if (was_buffer_empty_before_push)
            {
                node_->empty_cv.notify_one();
            }
        }

        inline void notify_multicast()
        {
            node_->empty_cv.notify_all();
        }

    public:

        /**
         * Defines open sharing mode of a shared-memory port:
         *
         * ReadShared (multiple listeners / multiple writers): Once a port is opened ReadShared cannot be opened ReadExclusive.
         *
         * ReadExclusive (one listener / multipler writers): Once a port is opened ReadExclusive cannot be opened ReadShared.
         *
         * Write (multiple writers): A port can always be opened for writing.
         */
        enum class OpenMode 
        { 
            ReadShared, 
            ReadExclusive, 
            Write 
        };

        Port(
                std::unique_ptr<SharedMemSegment>&& port_segment,
                PortNode* node)
            : port_segment_(std::move(port_segment))
            , node_(node)
            , overflows_count_(0)
        {
            auto buffer_base = static_cast<MultiProducerConsumerRingBuffer<BufferDescriptor>::Cell*>(
                port_segment_->get_address_from_offset(node_->buffer));

            auto buffer_node = static_cast<MultiProducerConsumerRingBuffer<BufferDescriptor>::Node*>(
                port_segment_->get_address_from_offset(node_->buffer_node));

            buffer_ = std::unique_ptr<MultiProducerConsumerRingBuffer<BufferDescriptor>>(
                new MultiProducerConsumerRingBuffer<BufferDescriptor>(buffer_base, buffer_node));

            node_->ref_counter.fetch_add(1);
        }

        ~Port()
        {
            if (node_->ref_counter.fetch_sub(1) == 1)
            {
                auto segment_name = port_segment_->name();

                logInfo(RTPS_TRANSPORT_SHM, THREADID << "Port " << node_->port_id
                        << segment_name.c_str() << " removed." << "overflows_count " 
                        << overflows_count_);

                if (overflows_count_)
                {
                    logWarning(RTPS_TRANSPORT_SHM, "Port " << node_->port_id
                        << segment_name.c_str() << " had overflows_count " 
                        << overflows_count_);
                }

                port_segment_.reset();

                SharedMemSegment::remove(segment_name.c_str());
                SharedMemSegment::named_mutex::remove((segment_name + "_mutex").c_str());
            }
        }

        /**
         * Try to enqueue a buffer descriptor in the port.
         * If the port queue is full returns inmediatelly with false value.
         * @param[in] buffer_descriptor buffer descriptor to be enqueued
         * @param[out] listeners_active false if no active listeners => buffer not enqueued
         * @return false in overflow case, true otherwise.
         */
        bool try_push(
                const BufferDescriptor& buffer_descriptor,
                bool* listeners_active)
        {
            std::unique_lock<SharedMemSegment::mutex> lock_empty(node_->empty_cv_mutex);

            try
            {
                bool was_opened_as_unicast_port = node_->is_opened_read_exclusive;
                bool was_buffer_empty_before_push = buffer_->is_buffer_empty();
                bool was_someone_listening = (node_->waiting_count > 0);

                *listeners_active = buffer_->push(buffer_descriptor);

                lock_empty.unlock();

                if (was_someone_listening)
                {
                    if (was_opened_as_unicast_port)
                    {
                        notify_unicast(was_buffer_empty_before_push);
                    }
                    else
                    {
                        notify_multicast();
                    }
                }

                return true;
            }
            catch (const std::exception&)
            {
                lock_empty.unlock();
                overflows_count_++;
            }
            return false;
        }

        /**
         * Waits while the port is empty and listener is not closed
         * @param[in] listener reference to the listener that will wait for an incoming buffer descriptor.
         * @param[in] is_listener_closed this reference can become true in the middle of the waiting process,
         * if that happens wait is aborted.
         */
        void wait_pop(
                Listener& listener,
                const std::atomic<bool>& is_listener_closed)
        {
            std::unique_lock<SharedMemSegment::mutex> lock(node_->empty_cv_mutex);

            uint32_t check_id = node_->check_id;

            node_->waiting_count++;

            do
            {
                node_->empty_cv.wait(lock, [&] {
                                return is_listener_closed.load() || listener.head() != nullptr || check_id != node_->check_id;
                            });

                if (check_id != node_->check_id)
                {
                    node_->check_awaken_count--;
                    check_id = node_->check_id;

                    if (listener.head())
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            } while (1);

            node_->waiting_count--;
        }

        /**
         * Set the caller's 'is_closed' flag (protecting empty_cv_mutex) and
         * forces wake-up all listeners on this port.
         * This function is used when destroying a listener waiting for messages
         * in the port.
         * @param is_listener_closed pointer to the atomic is_closed flag of the Listener object.
         */
        void close_listener(
                std::atomic<bool>* is_listener_closed)
        {
            {
                std::lock_guard<SharedMemSegment::mutex> lock(node_->empty_cv_mutex);
                is_listener_closed->exchange(true);
            }

            node_->empty_cv.notify_all();
        }

        /**
         * Removes the head buffer-descriptor from the listener's queue
         * @param [in] listener reference to the listener that will pop the buffer descriptor.
         * @param [out] was_cell_freed is true if the port's cell is freed because all listeners has poped the cell
         * @throw std::runtime_error if buffer is empty
         */
        void pop(
                Listener& listener,
                bool& was_cell_freed)
        {
            was_cell_freed = listener.pop();
        }

        /**
         * Register a new listener
         * The new listener's read pointer is equal to the ring-buffer write pointer at the registering moment.
         * @return A shared_ptr to the listener.
         * The listener will be unregistered when shared_ptr is destroyed.
         */
        std::shared_ptr<Listener> create_listener()
        {
            return buffer_->register_listener();
        }

        bool was_check_thread_detached()
        {
            return was_check_thread_detached_;
        }

        /**
         * Performs a check of the opened port.
         * When a process crashes with a port opened the port can be leave inoperative.
         * @param [in] healthy_check_timeout_ms max timeout (milliseconds) allowed for the whole 
         * healthy check operation.
         * @throw std::exception if the port is inoperative.
         */
        void healthy_check(
                uint32_t healthy_check_timeout_ms)
        {
            std::shared_ptr<bool> is_check_ok = std::make_shared<bool>(false);

            was_check_thread_detached_ = false;

            std::shared_ptr<std::mutex> notify_check_done_mutex = std::make_shared<std::mutex>();

            std::shared_ptr<std::condition_variable> notify_check_done_cv = 
                std::make_shared<std::condition_variable>();

            std::shared_ptr<bool> is_check_done_received = std::make_shared<bool>(false);

            std::thread check_thread([=]
                {
                    try
                    {
                        *is_check_ok = check_all_waiting_threads_alive(healthy_check_timeout_ms);

                        {
                            std::lock_guard<std::mutex> lock_received(*notify_check_done_mutex);
                            *is_check_done_received = true;
                        }

                        notify_check_done_cv->notify_one();
                    }
                    catch (std::exception&)
                    {
                        *is_check_ok = false;
                    }
                });

            std::unique_lock<std::mutex> lock(*notify_check_done_mutex);

            if (!notify_check_done_cv->wait_for(lock,
                    std::chrono::milliseconds(healthy_check_timeout_ms),
                    [&] { return *is_check_done_received; }))
            {
                node_->is_port_ok = false;
                was_check_thread_detached_ = true;
                check_thread.detach();
                throw std::runtime_error("healthy_check timeout");
            }

            check_thread.join();

            if (!(*is_check_ok))
            {
                node_->is_port_ok = false;
                throw std::runtime_error("healthy_check failed");
            }
        }

    }; // Port

    /**
     * Open a shared-memory port. If the port doesn't exist in the system a port with port_id is created,
     * otherwise the existing port is opened.
     *
     * @param [in] port_id Identifies the port
     * @param [in] max_buffer_descriptors Capacity of the port (only used if the port is created)
     * @param [in] healthy_check_timeout_ms Timeout for healthy check test
     * @param [in] open_mode Can be ReadShared, ReadExclusive or Write (see Port::OpenMode enum).
     *
     * @remarks This function performs a test to validate whether the existing port is OK, if the test
     * goes wrong the existing port is removed from shared-memory and a new port is created.
     */
    std::shared_ptr<Port> open_port(
            uint32_t port_id,
            uint32_t max_buffer_descriptors,
            uint32_t healthy_check_timeout_ms,
            Port::OpenMode open_mode = Port::OpenMode::ReadShared)
    {
        std::string err_reason;
        std::shared_ptr<Port> port;

        auto port_segment_name = domain_name_ + "_port" + std::to_string(port_id);

        logInfo(RTPS_TRANSPORT_SHM, THREADID << "Opening " << port_segment_name);

        std::unique_ptr<SharedMemSegment::named_mutex> port_mutex =
                SharedMemSegment::open_or_create_and_lock_named_mutex(port_segment_name + "_mutex");

        std::unique_lock<SharedMemSegment::named_mutex> port_lock(*port_mutex, std::adopt_lock);

        try
        {
            // Try to open
            auto port_segment = std::unique_ptr<SharedMemSegment>(
                new SharedMemSegment(boost::interprocess::open_only, port_segment_name.c_str()));

            SharedMemGlobal::PortNode* port_node;

            try
            {
                port_node = port_segment->get().find<PortNode>("port_node").first;
                port = std::make_shared<Port>(std::move(port_segment), port_node);
            }
            catch (std::exception&)
            {
                logWarning(RTPS_TRANSPORT_SHM, THREADID << "Port "
                                                        << port_id << " Couldn't find port_node ");

                SharedMemSegment::remove(port_segment_name.c_str());

                logWarning(RTPS_TRANSPORT_SHM, THREADID << "Port "
                                                        << port_id << " Removed.");

                throw;
            }

            try
            {
                port->healthy_check(healthy_check_timeout_ms);

                if ( (port_node->is_opened_read_exclusive && open_mode != Port::OpenMode::Write) ||
                        (port_node->is_opened_for_reading && open_mode == Port::OpenMode::ReadExclusive))
                {
                    std::stringstream ss;

                    ss << port_node->port_id << " (" << port_node->uuid.to_string() <<
                                        ") because is already opened ReadExclusive";

                    err_reason = ss.str();
                    port.reset();
                }
                else
                {
                    port_node->is_opened_read_exclusive |= (open_mode == Port::OpenMode::ReadExclusive);
                    port_node->is_opened_for_reading |= (open_mode != Port::OpenMode::Write);

                    logInfo(RTPS_TRANSPORT_SHM, THREADID << "Port "
                                                         << port_node->port_id << " (" << port_node->uuid.to_string() <<
                                        ") Opened");
                }
            }
            catch (std::exception&)
            {
                auto port_uuid = port_node->uuid.to_string();

                // Healthy check left a thread blocked at port resources
                // So we leave port_segment unmanaged, better to leak memory than a crash
                if (port->was_check_thread_detached())
                {
                    // Release owership
                    port_segment.release();

                    logWarning(RTPS_TRANSPORT_SHM, THREADID << "Existing Port "
                                                            << port_id << " (" << port_uuid <<
                                        ") NOT Healthy (check_thread detached).");
                }
                else
                {
                    logWarning(RTPS_TRANSPORT_SHM, THREADID << "Existing Port "
                                                            << port_id << " (" << port_uuid << ") NOT Healthy.");
                }

                SharedMemSegment::remove(port_segment_name.c_str());

                logWarning(RTPS_TRANSPORT_SHM, THREADID << "Port "
                                                        << port_id << " (" << port_uuid << ") Removed.");

                throw;
            }
        }
        catch (std::exception&)
        {
            // Doesn't exist => create it
            // The segment will contain the node, the buffer and the internal allocator structures (512bytes estimated)
            uint32_t extra = 512;
            uint32_t segment_size = sizeof(PortNode) + sizeof(PortCell) * max_buffer_descriptors;

            try
            {
                auto port_segment = std::unique_ptr<SharedMemSegment>(
                    new SharedMemSegment(boost::interprocess::create_only, port_segment_name.c_str(),
                    segment_size + extra));

                // Memset the whole segment to zero in order to force physical map of the buffer
                auto payload = port_segment->get().allocate(segment_size);
                memset(payload, 0, segment_size);
                port_segment->get().deallocate(payload);

                port = init_port(port_id, port_segment, max_buffer_descriptors, open_mode);
            }
            catch (std::exception& e)
            {
                logError(RTPS_TRANSPORT_SHM, "Failed to create port segment " << port_segment_name
                                                                              << ": " << e.what());

                throw;
            }
        }

        if (port == nullptr)
        {
            throw std::runtime_error("Coulnd't open port " + err_reason);
        }

        return port;
    }

private:

    std::string domain_name_;

    std::shared_ptr<Port> init_port(
            uint32_t port_id,
            std::unique_ptr<SharedMemSegment>& segment,
            uint32_t max_buffer_descriptors,
            Port::OpenMode open_mode)
    {
        std::shared_ptr<Port> port;
        PortNode* port_node = nullptr;
        MultiProducerConsumerRingBuffer<BufferDescriptor>::Node* buffer_node = nullptr;

        try
        {
            // Port node allocation
            port_node = segment->get().construct<PortNode>("port_node")();
            port_node->port_id = port_id;
            port_node->is_port_ok = true;
            UUID<8>::generate(port_node->uuid);
            port_node->waiting_count = 0;
            port_node->check_awaken_count = 0;
            port_node->check_id = 0;
            port_node->is_opened_read_exclusive = (open_mode == Port::OpenMode::ReadExclusive);
            port_node->is_opened_for_reading = (open_mode != Port::OpenMode::Write);

            // Buffer cells allocation
            port_node->buffer =
                    segment->get_offset_from_address(
                segment->get().construct<MultiProducerConsumerRingBuffer<BufferDescriptor>::Cell>(
                    boost::interprocess::anonymous_instance)[max_buffer_descriptors]());

            // Buffer node allocation
            buffer_node = segment->get().construct<MultiProducerConsumerRingBuffer<BufferDescriptor>::Node>(
                boost::interprocess::anonymous_instance)();

            MultiProducerConsumerRingBuffer<BufferDescriptor>::init_node(buffer_node, max_buffer_descriptors);

            port_node->buffer_node = segment->get_offset_from_address(buffer_node);

            port = std::make_shared<Port>(std::move(segment), port_node);

            logInfo(RTPS_TRANSPORT_SHM, THREADID << "Port "
                                                 << port_node->port_id << " (" << port_node->uuid.to_string() <<
                                ") Created.");
        }
        catch (const std::exception&)
        {
            if (port_node)
            {
                segment->get().destroy_ptr(port_node);
            }

            if (buffer_node)
            {
                segment->get().destroy_ptr(buffer_node);
            }

            throw;
        }

        return port;
    }
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_GLOBAL_H_