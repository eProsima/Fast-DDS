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
#include <rtps/transport/shared_mem/RobustExclusiveLock.hpp>
#include <rtps/transport/shared_mem/RobustSharedLock.hpp>

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

    struct BufferDescriptor;

    // Long names for SHM files could cause problems on some platforms
    static constexpr uint32_t MAX_DOMAIN_NAME_LENGTH = 16;

    SharedMemGlobal(
            const std::string& domain_name)
        : domain_name_(domain_name)
    {
        if (domain_name.length() > MAX_DOMAIN_NAME_LENGTH)
        {
            throw std::runtime_error(
                      domain_name +
                      " too long for domain name (max " +
                      std::to_string(MAX_DOMAIN_NAME_LENGTH) +
                      " characters");
        }
    }

    /**
     * Identifies a data buffer given its segment_id (shared-memory segment global_name)
     * and offset inside the segment
     */
    struct BufferDescriptor
    {
        SharedMemSegment::Id source_segment_id;
        SharedMemSegment::Offset buffer_node_offset;
        uint32_t validity_id;
    };

    typedef MultiProducerConsumerRingBuffer<BufferDescriptor>::Listener Listener;
    typedef MultiProducerConsumerRingBuffer<BufferDescriptor>::Cell PortCell;

    static const uint32_t CURRENT_ABI_VERSION = 6;

    struct PortNode
    {
        alignas(8) std::atomic<uint32_t> ref_counter;

        SharedMemSegment::Offset buffer;
        SharedMemSegment::Offset buffer_node;

        uint32_t port_id;
        uint32_t num_listeners;
        uint32_t healthy_check_timeout_ms;
        uint32_t max_buffer_descriptors;

        uint32_t is_port_ok : 1;
        uint32_t is_opened_read_exclusive : 1;
        uint32_t is_opened_for_reading : 1;
        uint32_t pad : 29;

        UUID<8> uuid;

        SharedMemSegment::condition_variable empty_cv;
        SharedMemSegment::mutex empty_cv_mutex;

        static constexpr size_t LISTENERS_STATUS_SIZE = 1024;
        struct ListenerStatus
        {
            uint32_t is_in_use : 1;
            uint32_t instance_id : 31;
            uint32_t read_p;
        };
        ListenerStatus listeners_status[LISTENERS_STATUS_SIZE];

        char domain_name[MAX_DOMAIN_NAME_LENGTH+1];
    };

    /**
     * A shared-memory port is a communication channel where data can be written / read.
     * A port has a port_id and a global name derived from the port_id and the domain.
     * System processes can open a port by knowing its name.
     */
    class Port
    {
        friend class MockPortSharedMemGlobal;

    private: // Port

        std::shared_ptr<SharedMemSegment> port_segment_;

        PortNode* node_;

        std::unique_ptr<MultiProducerConsumerRingBuffer<BufferDescriptor> > buffer_;

        std::unique_ptr<RobustExclusiveLock> read_exclusive_lock_;
        std::unique_ptr<RobustSharedLock> read_shared_lock_;

        inline void listener_status_update_read_p(
                PortNode::ListenerStatus& status,
                uint32_t read_p) const
        {
                status.read_p = read_p;
        }

        inline void listener_status_remove(
                PortNode::ListenerStatus& status)
        {
            status.instance_id++;
            status.is_in_use = false;
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

        /**
         * Remove a listener by force
         * @pre empty_cv_mutex has to locked before calling this function
         * @throw std::exception when port is not ok
         */
        inline void remove_listener(
                PortNode::ListenerStatus& status)
        {
            // Is operation is not completed the port will become not ok.
            node_->is_port_ok = false;

            listener_status_remove(status);

            // This listener will perform the unregistering in the destructor.
            Listener impersonating_listener(*buffer_, status.read_p);

            node_->num_listeners--;

            node_->is_port_ok = true;
        }

        void remove_slowest_listener()
        {
            if(node_->num_listeners > 0)
            {
                uint32_t num_listeners = 0;
                uint32_t slow_listeners = 0;
                uint32_t slowest_listener = 0;
                uint32_t max_distance = 0;

                uint32_t write_p = buffer_->write_p();

                // Find the slowest listener
                for (uint32_t i = 0; i < PortNode::LISTENERS_STATUS_SIZE; i++)
                {
                    auto& status = node_->listeners_status[i];

                    if(status.is_in_use)
                    {
                        auto d = buffer_->distance(status.read_p, write_p);
                        if (d > max_distance)
                        {
                            max_distance = d;
                            slowest_listener = i;
                            slow_listeners = 1;
                        }
                        else if(d == max_distance)
                        {
                            slow_listeners++;
                        }

                        if(++num_listeners == node_->num_listeners)
                        {
                            break;
                        }
                    }
                }

                // Wrong number of listeners is a problem
                if(num_listeners != node_->num_listeners)
                {
                    node_->is_port_ok = false;
                    throw std::runtime_error("wrong number of listeners");
                }

                // If not all the listeners are slow
                if(slow_listeners != num_listeners)
                {
                    // Remove the slowest
                    remove_listener(node_->listeners_status[slowest_listener]);
                }
            }
        }

        inline bool is_listener_valid_no_lock(
                const PortNode::ListenerStatus& status,
                uint32_t listener_instance_id) const
        {
            return listener_instance_id == status.instance_id;
        }

    public: // Port

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

        static std::string open_mode_to_string(
                OpenMode open_mode)
        {
            switch (open_mode)
            {
            case OpenMode::ReadShared: return "ReadShared";
            case OpenMode::ReadExclusive: return "ReadExclusive";
            case OpenMode::Write: return "Write";
            }

            return "";
        }

        Port(
                std::shared_ptr<SharedMemSegment>&& port_segment,
                PortNode* node,
                std::unique_ptr<RobustExclusiveLock>&& read_exclusive_lock = std::unique_ptr<RobustExclusiveLock>())
            : port_segment_(std::move(port_segment))
            , node_(node)
            , read_exclusive_lock_(std::move(read_exclusive_lock))
        {
            auto buffer_base = static_cast<MultiProducerConsumerRingBuffer<BufferDescriptor>::Cell*>(
                port_segment_->get_address_from_offset(node_->buffer));

            auto buffer_node = static_cast<MultiProducerConsumerRingBuffer<BufferDescriptor>::Node*>(
                port_segment_->get_address_from_offset(node_->buffer_node));

            buffer_ = std::unique_ptr<MultiProducerConsumerRingBuffer<BufferDescriptor> >(
                new MultiProducerConsumerRingBuffer<BufferDescriptor>(buffer_base, buffer_node));

            node_->ref_counter.fetch_add(1);
        }

        ~Port()
        {
            if (node_->ref_counter.fetch_sub(1) == 1)
            {
                auto segment_name = port_segment_->name();

                try
                {
                    // This check avoid locking port_mutex when the port is not OK, also avoid
                    // recursive lock of port_mutex in create_port()
                    if(is_port_ok())
                    {
                        std::unique_ptr<SharedMemSegment::named_mutex> port_mutex =
                                SharedMemSegment::try_open_and_lock_named_mutex(segment_name + "_mutex");

                        std::unique_lock<SharedMemSegment::named_mutex> port_lock(*port_mutex, std::adopt_lock);
                        
                        // Check again to ensure nobody has re-opened the port while we were locking port_mutex
                        if (node_->ref_counter.load(std::memory_order_relaxed) == 0
                                && is_port_ok())
                        {
                            node_->is_port_ok = false;
                            node_ = nullptr;
                            port_segment_.reset();

                            SharedMemSegment::remove(segment_name.c_str());
                            SharedMemSegment::named_mutex::remove((segment_name + "_mutex").c_str());
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    if (node_)
                    {
                        node_->is_port_ok = false;
                    }

                    logWarning(RTPS_TRANSPORT_SHM, THREADID << segment_name.c_str()
                                                            << e.what());
                }
            }
        }

        /**
         * Try to enqueue a buffer descriptor in the port.
         * If the port queue is full returns inmediatelly with false value.
         * @param[in] buffer_descriptor buffer descriptor to be enqueued
         * @param[out] listeners_active false if no active listeners => buffer not enqueued
         * @return false in overflow case, true otherwise.
         * @throw std::exception when port is not ok.
         */
        bool try_push(
                const BufferDescriptor& buffer_descriptor,
                bool* listeners_active)
        {
            std::unique_lock<SharedMemSegment::mutex> lock_empty(node_->empty_cv_mutex);

            if (!node_->is_port_ok)
            {
                throw std::runtime_error("the port is marked as not ok!");
            }

            bool was_buffer_empty_before_push = buffer_->is_buffer_empty();

            if(buffer_->push(buffer_descriptor, listeners_active))
            {
                lock_empty.unlock();

                if (node_->is_opened_read_exclusive)
                {
                    notify_unicast(was_buffer_empty_before_push);
                }
                else
                {
                    notify_multicast();
                }
            }
            else // Overflow
            {
                if (!node_->is_opened_read_exclusive)
                {
                    // Overflow in ReadShared ports could mean a listener is blocked, so to prevent listeners
                    // starvation the slow (on blocked) listeners are removed by force.
                    remove_slowest_listener();
                    
                    // Notify the change
                    notify_multicast();
                }

                return false;
            }

            return true;
        }

        /**
         * Waits while the port is empty and listener is not closed
         * @param[in] listener reference to the listener that will wait for an incoming buffer descriptor.
         * @param[in] is_listener_closed this reference can become true in the middle of the waiting process.
         * @param[in] listener_index to check the port's listener_status
         * @param[in] listener_instance_id id given to the listeners when it was created.
         * @throw std::exception when the port is not ok.
         */
        BufferDescriptor wait_pop(
                Listener& listener,
                const std::atomic<bool>& is_listener_closed,
                uint32_t listener_index,
                uint32_t listener_instance_id)
        {
            std::unique_lock<SharedMemSegment::mutex> lock(node_->empty_cv_mutex);

            auto error = [&]
                {
                    return is_listener_closed.load() ||
                        !node_->is_port_ok ||
                        !is_listener_valid_no_lock(node_->listeners_status[listener_index], 
                            listener_instance_id);
                };

            auto condition_met = [&]
                {
                    return error() || (listener.head() != nullptr);
                };

            do
            {
                boost::system_time const timeout =
                        boost::get_system_time()+ boost::posix_time::milliseconds(node_->healthy_check_timeout_ms);

                node_->empty_cv.timed_wait(lock, timeout, condition_met);
            }while(!error() && listener.head() == nullptr);

            if (listener.head() == nullptr)
            {
                throw std::runtime_error("error");
            }

            return listener.head()->data();
        }

        inline bool is_port_ok() const
        {
            try
            {
                std::unique_lock<SharedMemSegment::mutex> lock(node_->empty_cv_mutex);
                return node_->is_port_ok;
            }
            catch(std::exception&)
            {
            }

            return false;
        }

        inline uint32_t port_id() const
        {
            return node_->port_id;
        }

        inline OpenMode open_mode() const
        {
            if (node_->is_opened_for_reading)
            {
                return node_->is_opened_read_exclusive ? OpenMode::ReadExclusive : OpenMode::ReadShared;
            }

            return OpenMode::Write;
        }

        inline uint32_t healthy_check_timeout_ms() const
        {
            return node_->healthy_check_timeout_ms;
        }

        inline uint32_t max_buffer_descriptors() const
        {
            return node_->max_buffer_descriptors;
        }

        /**
         * Set the caller's 'is_closed' flag (protecting empty_cv_mutex) and
         * forces wake-up all listeners on this port.
         * This function is used when destroying a listener waiting for messages
         * in the port.
         * @param is_listener_closed pointer to the atomic is_closed flag of the Listener object.
         * @throw std::exception on error
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
                uint32_t listener_index,
                uint32_t listener_instance_id,
                bool* was_cell_freed)
        {
            if(node_->is_opened_read_exclusive)
            {
                *was_cell_freed = listener.pop();
            }
            else 
            {
                // Shared ports allows remote listener removal, so pop operations need to hold the mutex
                // to assure listener validity during the pop operation
                std::unique_lock<SharedMemSegment::mutex> lock_empty(node_->empty_cv_mutex);

                auto& status = node_->listeners_status[listener_index];
                if(!is_listener_valid_no_lock(status, listener_instance_id))
                {
                    // Listener has been remotelly removed
                    throw std::runtime_error("invalid listener");
                }

                *was_cell_freed = listener.pop();
                
                listener_status_update_read_p(status, listener.read_p());
            }
        }

        /**
         * Check if a listener instance is still valid
         * @param [in] listener_index
         * @param [in] listener_instance_id
         * @return true when valid, false otherwise.
         * @throw std::runtime_error when port is not ok
         */
        inline bool is_listener_valid(
                uint32_t listener_index,
                uint32_t listener_instance_id) const
        {
            try
            {
                std::unique_lock<SharedMemSegment::mutex> lock(node_->empty_cv_mutex);
                return listener_instance_id == node_->listeners_status[listener_index].instance_id;
            }
            catch(std::exception&)
            {
                node_->is_port_ok = false;
                throw;
            }

            return false;
        }

        /**
         * Register a new listener
         * The new listener's read pointer is equal to the ring-buffer write pointer at the registering moment.
         * @param [out] listener_index pointer to where the index of the listener is returned. This index is
         * used to reference the elements from the listeners_status array.
         * @param [out] instance_id used to verify whether the listener is valid or it has been removed.
         * @return A shared_ptr to the listener.
         * The listener will be unregistered when shared_ptr is destroyed.
         * @throw std::exception on failure
         */
        std::unique_ptr<Listener> create_listener(
                uint32_t* listener_index,
                uint32_t* instance_id)
        {
            std::unique_ptr<Listener> listener;

            try
            {
                std::lock_guard<SharedMemSegment::mutex> lock(node_->empty_cv_mutex);

                uint32_t i;
                // Find a free listener_status
                for (i = 0; i<PortNode::LISTENERS_STATUS_SIZE; i++)
                {
                    if (!node_->listeners_status[i].is_in_use)
                    {
                        break;
                    }
                }

                if (i < PortNode::LISTENERS_STATUS_SIZE)
                {
                    listener = buffer_->register_listener();

                    *listener_index = i;

                    auto& status = node_->listeners_status[*listener_index];
                    *instance_id = status.instance_id;
                    status.is_in_use = true;
                    status.read_p = listener->read_p();

                    node_->num_listeners++;
                }
                else
                {
                    throw std::runtime_error("max listeners reached");
                }
            }
            catch(std::exception&)
            {
                // The port is not OK
                node_->is_port_ok = false;
                throw;
            }

            return listener;
        }

        /**
         * Remove the listener from the port, the cells still not processed by the listener are freed
         * @throw std::exception on failure
         */
        void unregister_listener(
                std::unique_ptr<Listener>* listener,
                uint32_t listener_index,
                uint32_t instance_id)
        {
            try
            {
                std::lock_guard<SharedMemSegment::mutex> lock(node_->empty_cv_mutex);

                if(!node_->is_port_ok)
                {
                    throw std::runtime_error("Port marked as not OK");
                }

                auto& status = node_->listeners_status[listener_index];
                if (instance_id == status.instance_id)
                {
                    remove_listener(status);
                }
                (*listener)->detach();
                (*listener).reset();
            }
            catch (const std::exception&)
            {
                // The port is not OK
                node_->is_port_ok = false;

                (*listener)->detach();
                (*listener).reset();

                throw;
            }
        }

        void lock_read_exclusive()
        {
            std::string lock_name = std::string(node_->domain_name) + "_port" + std::to_string(node_->port_id) + "_el";
            read_exclusive_lock_ = std::unique_ptr<RobustExclusiveLock>(new RobustExclusiveLock(lock_name));
        }

        void lock_read_shared()
        {
            std::string lock_name = std::string(node_->domain_name) + "_port" + std::to_string(node_->port_id) + "_sl";
            read_shared_lock_ = std::unique_ptr<RobustSharedLock>(new RobustSharedLock(lock_name));
        }

        void unlock_read_locks()
        {
            read_exclusive_lock_.reset();
            read_shared_lock_.reset();
        }

        static std::unique_ptr<RobustExclusiveLock> lock_read_exclusive(
                uint32_t port_id,
                const std::string& domain_name)
        {
            std::string lock_name = std::string(domain_name) + "_port" + std::to_string(port_id) + "_el";
            return std::unique_ptr<RobustExclusiveLock>(new RobustExclusiveLock(lock_name));
        }

        static bool is_zombie(
                uint32_t port_id,
                const std::string& domain_name)
        {
            bool was_lock_created;
            std::string lock_name;

            try
            {
                // An exclusive port is zombie when it has a "_el" file & the file is not locked
                lock_name = domain_name + "_port" + std::to_string(port_id) + "_el";
                RobustExclusiveLock zombie_test(lock_name, &was_lock_created);
                // Lock acquired, did the file exist before the acquire?
                if (!was_lock_created)
                {
                    // Yes, is zombie
                    return true;
                }
            }
            catch (const std::exception&)
            {
                // Resource locked => not zombie.
            }

            try
            {
                // A shared port is zombie when it has a "_sl" file & the file is not locked
                lock_name = domain_name + "_port" + std::to_string(port_id) + "_sl";
                bool was_lock_released;
                RobustSharedLock zombie_test(lock_name, &was_lock_created, &was_lock_released);
                // Lock acquired, did the file exist and was release before the acquire?
                if (!was_lock_created && was_lock_released)
                {
                    // Yes, is zombie
                    return true;
                }
            }
            catch (const std::exception&)
            {
                // Resource locked => not zombie.
            }

            return false;
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
     * @return A shared_ptr to the new port or nullptr if the open_mode is ReadExclusive and the port_id is already opened.
     * @remarks This function performs a test to validate whether the existing port is OK, if the test
     * goes wrong the existing port is removed from shared-memory and a new port is created.
     */
    std::shared_ptr<Port> open_port(
            uint32_t port_id,
            uint32_t max_buffer_descriptors,
            uint32_t healthy_check_timeout_ms,
            Port::OpenMode open_mode = Port::OpenMode::ReadShared)
    {
        return open_port_internal(port_id, max_buffer_descriptors, healthy_check_timeout_ms, open_mode, nullptr);
    }

    /**
     * Delete the port and open a new one with the same ID.
     * It is used when a port has been marked as not OK.
     * @return A shared_ptr to the new port
     * @throw std::exception on error
     */
    std::shared_ptr<Port> regenerate_port(
            std::shared_ptr<Port> port,
            SharedMemGlobal::Port::OpenMode open_mode)
    {
        return open_port_internal(
            port->port_id(),
            port->max_buffer_descriptors(),
            port->healthy_check_timeout_ms(),
            open_mode,
            port);
    }

    /**
     * Remove a port from the system.
     */
    void remove_port(
            uint32_t port_id)
    {
        auto port_segment_name = domain_name_ + "_port" + std::to_string(port_id);
        SharedMemSegment::remove(port_segment_name.c_str());
    }

    std::string domain_name()
    {
        return domain_name_;
    }

private:

    std::string domain_name_;

    /**
     * Try to open a port. If the port is not healthy is removed from the system. 
     * @return A pointer to the opened port.
     * @throw std::exception if the port is not found, is zombie or structures are corrupted.
     */
    std::shared_ptr<Port> open_existing_port(
            uint32_t port_id,
            const std::string& port_segment_name,
            SharedMemGlobal::PortNode** port_node)
    {
        std::shared_ptr<Port> port;

        if (Port::is_zombie(port_id, domain_name_))
        {
            logWarning(RTPS_TRANSPORT_SHM, THREADID << "Port "
                                                    << port_id << " Zombie. Reset the port");

            SharedMemSegment::remove(port_segment_name.c_str());

            throw std::runtime_error("zombie port");
        }

        // Try to open
        auto port_segment = std::shared_ptr<SharedMemSegment>(
            new SharedMemSegment(boost::interprocess::open_only, port_segment_name.c_str()));

        try
        {
            if (!port_segment->check_sanity())
            {
                throw std::runtime_error("check_sanity failed");
            }

            *port_node = port_segment->get().find<PortNode>(
                ("port_node_abi" + std::to_string(CURRENT_ABI_VERSION)).c_str()).first;

            if (*port_node)
            {
                port = std::make_shared<Port>(std::move(port_segment), *port_node);
            }
            else
            {
                throw std::runtime_error("port_abi not compatible");
            }
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

        return port;
    }

    /**
     * Checks if the port can be opened in the requested mode and whether the port is OK.
     * ReadExclusive ports are locked if the operation succeeds.
     * If the check fail, the port is removed from the system. 
     * @throw std::exception on fail.
     */
    void check_port(
            std::shared_ptr<Port>& port,
            PortNode* port_node,
            const std::string& port_segment_name,
            Port::OpenMode open_mode,
            std::string& err_reason)
    {
        auto port_id = port_node->port_id;
        auto port_uuid = port_node->uuid.to_string();

        try
        {
            if (open_mode == Port::OpenMode::ReadExclusive)
            {
                try
                {
                    port->lock_read_exclusive();
                }
                catch (const std::exception&)
                {
                    std::stringstream ss;
                    ss << port_node->port_id << " (" << port_node->uuid.to_string() <<
                        ") because is ReadExclusive locked";
                    err_reason = ss.str();

                    port.reset();
                }
            }

            if (port)
            {
                if (!port->is_port_ok())
                {
                    throw std::runtime_error("Port not ok");
                }

                port_node->is_opened_read_exclusive |= (open_mode == Port::OpenMode::ReadExclusive);
                port_node->is_opened_for_reading |= (open_mode != Port::OpenMode::Write);

                logInfo(RTPS_TRANSPORT_SHM, THREADID << "Port "
                                                     << port_node->port_id << " (" << port_node->uuid.to_string() <<
                        ") Opened" << Port::open_mode_to_string(open_mode));
            }
        }
        catch (std::exception&)
        {
            assert(port);

            port->unlock_read_locks();

            port_node->is_port_ok = false;

            auto port_uuid = port_node->uuid.to_string();

            logWarning(RTPS_TRANSPORT_SHM, THREADID << "Existing Port "
                                                    << port_id << " (" << port_uuid << ") NOT Healthy.");

            SharedMemSegment::remove(port_segment_name.c_str());

            logWarning(RTPS_TRANSPORT_SHM, THREADID << "Port "
                                                    << port_id << " (" << port_uuid << ") Removed.");

            port.reset();

            throw;
        }
    }

    /**
     * Create a new port from the scratch
     */
    std::shared_ptr<Port> create_port(
            uint32_t port_id,
            const std::string& port_segment_name,
            uint32_t max_buffer_descriptors,
            uint32_t healthy_check_timeout_ms,
            Port::OpenMode open_mode)
    {
        std::shared_ptr<Port> port;

        // Doesn't exist => create it
        // The segment will contain the node, the buffer and the internal allocator structures (512bytes estimated)
        uint32_t extra = 512;
        uint32_t segment_size = sizeof(PortNode) + sizeof(PortCell) * max_buffer_descriptors;

        std::unique_ptr<SharedMemSegment> port_segment;

        try
        {
            port_segment = std::unique_ptr<SharedMemSegment>(
                new SharedMemSegment(boost::interprocess::create_only, port_segment_name.c_str(),
                segment_size + extra));
        }
        catch (std::exception& e)
        {
            logWarning(RTPS_TRANSPORT_SHM, "Failed to create port segment " << port_segment_name
                                                                            << ": " << e.what());
        }

        if (port_segment)
        {
            try
            {
                // Memset the whole segment to zero in order to force physical map of the buffer
                auto payload = port_segment->get().allocate(segment_size);
                memset(payload, 0, segment_size);
                port_segment->get().deallocate(payload);

                port = init_port(port_id, port_segment, max_buffer_descriptors, open_mode,
                                healthy_check_timeout_ms);
            }
            catch (std::exception& e)
            {
                SharedMemSegment::remove(port_segment_name.c_str());

                logError(RTPS_TRANSPORT_SHM, "Failed init_port " << port_segment_name
                                                                 << ": " << e.what());

                throw;
            }
        }

        return port;
    }

    std::shared_ptr<Port> open_port_internal(
            uint32_t port_id,
            uint32_t max_buffer_descriptors,
            uint32_t healthy_check_timeout_ms,
            Port::OpenMode open_mode,
            std::shared_ptr<Port> regenerating_port)
    {
        std::string err_reason;
        std::shared_ptr<Port> port;

        auto port_segment_name = domain_name_ + "_port" + std::to_string(port_id);

        std::unique_ptr<SharedMemSegment::named_mutex> port_mutex =
                SharedMemSegment::open_or_create_and_lock_named_mutex(port_segment_name + "_mutex");
        std::unique_lock<SharedMemSegment::named_mutex> port_lock(*port_mutex, std::adopt_lock);

        if (regenerating_port)
        {
            try
            {
                regenerating_port->unlock_read_locks();
            }
            catch (std::exception& e)
            {
                logError(RTPS_TRANSPORT_SHM, THREADID << "Port "
                                                      << port_id << " failed unlock_read_locks " << e.what());
            }
        }

        try
        {
            SharedMemGlobal::PortNode* port_node;

            port = open_existing_port(port_id, port_segment_name, &port_node);

            check_port(port, port_node, port_segment_name, open_mode, err_reason);
        }
        catch (std::exception&)
        {
            port = create_port(port_id, port_segment_name, max_buffer_descriptors, healthy_check_timeout_ms, open_mode);
        }

        if (port == nullptr)
        {
            throw std::runtime_error("Couldn't open port " + err_reason);
        }

        return port;
    }

    std::shared_ptr<Port> init_port(
            uint32_t port_id,
            std::unique_ptr<SharedMemSegment>& segment,
            uint32_t max_buffer_descriptors,
            Port::OpenMode open_mode,
            uint32_t healthy_check_timeout_ms)
    {
        PortNode* port_node = nullptr;
        MultiProducerConsumerRingBuffer<BufferDescriptor>::Node* buffer_node = nullptr;

        std::unique_ptr<RobustExclusiveLock> lock_read_exclusive;
        if (open_mode == Port::OpenMode::ReadExclusive)
        {
            lock_read_exclusive = Port::lock_read_exclusive(port_id, domain_name_);
        }

        // Port node allocation
        port_node =
                segment->get().construct<PortNode>(("port_node_abi" +
                        std::to_string(CURRENT_ABI_VERSION)).c_str())();
        port_node->is_port_ok = false;
        port_node->port_id = port_id;
        UUID<8>::generate(port_node->uuid);
        port_node->is_opened_read_exclusive = (open_mode == Port::OpenMode::ReadExclusive);
        port_node->is_opened_for_reading = (open_mode != Port::OpenMode::Write);
        port_node->num_listeners = 0;
        port_node->healthy_check_timeout_ms = healthy_check_timeout_ms;
        port_node->max_buffer_descriptors = max_buffer_descriptors;
        memset(port_node->listeners_status, 0, sizeof(port_node->listeners_status));
#ifdef _MSC_VER
        strncpy_s(port_node->domain_name, sizeof(port_node->domain_name),
                domain_name_.c_str(), sizeof(port_node->domain_name)-1);
#else
        strncpy(port_node->domain_name, domain_name_.c_str(), sizeof(port_node->domain_name)-1);
#endif
        port_node->domain_name[sizeof(port_node->domain_name)-1] = 0;

        // Buffer cells allocation
        auto buffer = segment->get().construct<MultiProducerConsumerRingBuffer<BufferDescriptor>::Cell>(
            boost::interprocess::anonymous_instance)[max_buffer_descriptors]();
        port_node->buffer = segment->get_offset_from_address(buffer);

        // Buffer node allocation
        buffer_node = segment->get().construct<MultiProducerConsumerRingBuffer<BufferDescriptor>::Node>(
            boost::interprocess::anonymous_instance)();

        MultiProducerConsumerRingBuffer<BufferDescriptor>::init_node(buffer_node, max_buffer_descriptors);

        port_node->buffer_node = segment->get_offset_from_address(buffer_node);

        port_node->is_port_ok = true;

        std::shared_ptr<Port> port = std::make_shared<Port>(std::move(segment), port_node, std::move(lock_read_exclusive));

        if (open_mode == Port::OpenMode::ReadShared)
        {
            port->lock_read_shared();
        }

        logInfo(RTPS_TRANSPORT_SHM, THREADID << "Port "
                                             << port_node->port_id << " (" << port_node->uuid.to_string()
                                             << Port::open_mode_to_string(open_mode) << ") Created.");

        return port;
    }
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_GLOBAL_H_