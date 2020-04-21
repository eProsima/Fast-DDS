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

#ifndef _FASTDDS_SHAREDMEM_MANAGER_H_
#define _FASTDDS_SHAREDMEM_MANAGER_H_

#include <atomic>
#include <list>
#include <unordered_map>

#include <rtps/transport/shared_mem/SharedMemGlobal.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

/**
 *  Provides functionality for the application to:
 *
 *  Open shared-memory ports.
 *  Create shared memory segments.
 */
class SharedMemManager
{
private:

    struct BufferNode
    {        
        struct Status
        {
            // When buffers are enqued in a port this validity_id is copied to the BufferDescriptor in the port.
            // If the sender process needs to recover the buffer by force, just increment this validity_id, the
            // receiver process must check this validity_id vs the validity_id in the BufferDescriptor, if not equal
            // the buffer has been invalidated by the sender.
            uint64_t validity_id : 24;
            // This counter is incremented each time the buffer is enqueued in a port, an decremented when pop
            // from the port to be processed
            uint64_t enqueued_count : 20;
            // When listener processes start processing this buffer increments the processing_count. This way 
            // the sender can know whether the buffer is been processed or is just only enqueued in some ports.
            uint64_t processing_count : 20;
        };

        std::atomic<Status> status;
        uint32_t data_size;
        SharedMemSegment::Offset data_offset;
        
        /**
         * Atomically invalidates a buffer.
         */
        inline void invalidate_buffer()
        {
            auto s = status.load(std::memory_order_relaxed);
            while (!status.compare_exchange_weak(s,
                    { (uint64_t)s.validity_id+1, (uint64_t)0u, (uint64_t)0u },
                    std::memory_order_release,
                    std::memory_order_relaxed))
            {
            }
        }

        /**
         * Atomically invalidates a buffer, only, when the buffer is valid for the caller.
         * @return true when succedded, false when the buffer was invalid for the caller.
         */
        inline bool invalidate_buffer(
                uint32_t listener_validity_id)
        {
            auto s = status.load(std::memory_order_relaxed);
            while (listener_validity_id == s.validity_id &&
                    !status.compare_exchange_weak(s,
                    { (uint64_t)s.validity_id+1, (uint64_t)0u, (uint64_t)0u },
                    std::memory_order_release,
                    std::memory_order_relaxed))
            {
            }

            return (listener_validity_id == s.validity_id);
        }

        /**
         * Atomically invalidates a buffer, only, if the buffer is not being processed.
         * @return true when succedded, false otherwise.
         */
        bool invalidate_if_not_processing()
        {
            auto s = status.load(std::memory_order_relaxed);
            // If the buffer is not beeing processed by any listener => is invalidated
            while (s.processing_count == 0 &&
                    !status.compare_exchange_weak(s,
                    { (uint64_t)s.validity_id+1, (uint64_t)0u, (uint64_t)0u},
                    std::memory_order_release,
                    std::memory_order_relaxed))
            {
            }

            return (s.processing_count == 0);
        }

        /**
         * @return true if listener_validity_id == current buffer validity_id.
         */
        inline bool is_valid(
                uint32_t listener_validity_id) const
        {
            return (status.load(std::memory_order_relaxed).validity_id == listener_validity_id);
        }
        
        /**
         * Atomically decrease enqueued count & increase the buffer processing counts, only, if the buffer is valid.
         * @return true when succedded, false when the buffer has been invalidated.
         */
        inline bool dec_enqueued_inc_processing_counts(
                uint32_t listener_validity_id)
        {
            auto s = status.load(std::memory_order_relaxed);
            while (listener_validity_id == s.validity_id &&
                    !status.compare_exchange_weak(s,
                    { (uint64_t)s.validity_id, (uint64_t)s.enqueued_count-1, (uint64_t)s.processing_count+1 },
                    std::memory_order_release,
                    std::memory_order_relaxed))
            {
            }

            return (listener_validity_id == s.validity_id);
        }

        /**
         * Atomically increase the buffer processing count, only, if the buffer is valid.
         * @return true when succedded, false when the buffer has been invalidated.
         */
        inline bool inc_processing_count(
                uint32_t listener_validity_id)
        {
            auto s = status.load(std::memory_order_relaxed);
            while (listener_validity_id == s.validity_id &&
                    !status.compare_exchange_weak(s,
                    { (uint64_t)s.validity_id, (uint64_t)s.enqueued_count, (uint64_t)s.processing_count+1 },
                    std::memory_order_release,
                    std::memory_order_relaxed))
            {
            }

            return (listener_validity_id == s.validity_id);
        }

        /**
         * Atomically increase the buffer enqueued count, only, if the buffer is valid.
         * @return true when succedded, false when the buffer has been invalidated.
         */
        inline bool inc_enqueued_count(
                uint32_t listener_validity_id)
        {
            auto s = status.load(std::memory_order_relaxed);
            while (listener_validity_id == s.validity_id &&
                    !status.compare_exchange_weak(s,
                    { (uint64_t)s.validity_id, (uint64_t)s.enqueued_count+1, (uint64_t)s.processing_count },
                    std::memory_order_release,
                    std::memory_order_relaxed))
            {
            }

            return (listener_validity_id == s.validity_id);
        }

        /**
         * Atomically decrease the buffer enqueued count, only, if the buffer is valid.
         * @return true when succedded, false when the buffer has been invalidated.
         */
        inline bool dec_enqueued_count(
                uint32_t listener_validity_id)
        {
            auto s = status.load(std::memory_order_relaxed);
            while (listener_validity_id == s.validity_id &&
                    !status.compare_exchange_weak(s,
                    { (uint64_t)s.validity_id, (uint64_t)s.enqueued_count-1, (uint64_t)s.processing_count },
                    std::memory_order_release,
                    std::memory_order_relaxed))
            {
            }

            return (listener_validity_id == s.validity_id);
        }

        inline bool is_not_referenced() const
        {
            auto s = status.load(std::memory_order_relaxed);
            return (s.enqueued_count == 0) && (s.processing_count == 0);
        }

        /**
         * Atomically decrease the buffer processing count, only, if the buffer is valid.
         * @return true when succedded, false when the buffer has been invalidated.
         */
        inline bool dec_processing_count(
                uint32_t listener_validity_id)
        {
            auto s = status.load(std::memory_order_relaxed);
            while (listener_validity_id == s.validity_id &&
                    !status.compare_exchange_weak(s,
                    { (uint64_t)s.validity_id, (uint64_t)s.enqueued_count, (uint64_t)s.processing_count-1 },
                    std::memory_order_release,
                    std::memory_order_relaxed))
            {
            }

            return (listener_validity_id == s.validity_id);
        }        
    };

    struct SegmentNode
    {
        std::atomic<uint32_t> ref_count;
        uint32_t pad;
    };

public:

    SharedMemManager(
            const std::string& domain_name)
        : global_segment_(
            domain_name,
            []( const std::vector<const SharedMemGlobal::BufferDescriptor*>& buffer_descriptors,
                const std::string& domain_name)
            {
                on_failure_buffer_descriptor_handler(buffer_descriptors, domain_name);
            })
    {
        static_assert(std::alignment_of<BufferNode>::value % 8 == 0, "SharedMemManager::BufferNode bad alignment");

        if (domain_name.length() > SharedMemGlobal::MAX_DOMAIN_NAME_LENGTH)
        {
            throw std::runtime_error(
                      domain_name +
                      " too long for domain name (max " +
                      std::to_string(SharedMemGlobal::MAX_DOMAIN_NAME_LENGTH) +
                      " characters");
        }

        SharedMemGlobal::Port::on_failure_buffer_descriptors_handler(
            []( const std::vector<const SharedMemGlobal::BufferDescriptor*>& buffer_descriptors,
                const std::string& domain_name)
            {
                on_failure_buffer_descriptor_handler(buffer_descriptors, domain_name);
            });

        per_allocation_extra_size_ =
                SharedMemSegment::compute_per_allocation_extra_size(std::alignment_of<BufferNode>::value);
    }

    class Buffer
    {
    protected:

        virtual ~Buffer() = default;

    public:

        virtual void* data() = 0;
        virtual uint32_t size() = 0;
    };

    class SharedMemBuffer : public Buffer
    {
    public:

        SharedMemBuffer(
                std::shared_ptr<SharedMemSegment>& segment,
                const SharedMemSegment::Id& segment_id,
                BufferNode* buffer_node,
                uint32_t original_validity_id)
            : segment_(segment)
            , segment_id_(segment_id)
            , buffer_node_(buffer_node)
            , original_validity_id_(original_validity_id)
        {
            data_ = segment_->get_address_from_offset(buffer_node_->data_offset);
        }

        ~SharedMemBuffer() override
        {
            buffer_node_->dec_processing_count(original_validity_id_);
        }

        void* data() override
        {
            return data_;
        }

        uint32_t size() override
        {
            return buffer_node_->data_size;
        }

        SharedMemSegment::Offset node_offset() const
        {
            return segment_->get_offset_from_address(buffer_node_);
        }

        SharedMemSegment::Id segment_id() const
        {
            return segment_id_;
        }

        inline uint32_t validity_id() const
        {
            return original_validity_id_;
        }

        void inc_enqueued_count(uint32_t validity_id)
        {
            buffer_node_->inc_enqueued_count(validity_id);
        }

        void dec_enqueued_count(uint32_t validity_id)
        {
            buffer_node_->dec_enqueued_count(validity_id);
        }

    private:

        std::shared_ptr<SharedMemSegment> segment_;
        SharedMemSegment::Id segment_id_;
        BufferNode* buffer_node_;
        void* data_;
        uint32_t original_validity_id_;
    };

    /**
     * Handle a shared-memory segment
     * Allows buffer allocation / deallocation
     */
    class Segment
    {
    public:

        Segment(
                uint32_t size,
                uint32_t payload_size,
                uint32_t max_allocations)
            : segment_id_()
            , overflows_count_(0)
        {
            segment_id_.generate();

            auto segment_name = segment_id_.to_string();

            SharedMemSegment::remove(segment_name.c_str());

            try
            {
                segment_ = std::unique_ptr<SharedMemSegment>(
                    new SharedMemSegment(boost::interprocess::create_only, segment_name.c_str(), size));
            }
            catch (const std::exception& e)
            {
                logError(RTPS_TRANSPORT_SHM, "Failed to create segment " << segment_name
                                                                         << ": " << e.what());

                throw;
            }

            // Init the segment node
            segment_node_ = segment_->get().construct<SegmentNode>("segment_node")();
            segment_node_->ref_count.exchange(1);
            segment_node_->pad = 0;

            free_bytes_ = payload_size;

            // Alloc the buffer nodes
            auto buffers_nodes = segment_->get().construct<BufferNode>
                        (boost::interprocess::anonymous_instance)[max_allocations]();

            // All buffer nodes are free
            for (uint32_t i = 0; i<max_allocations; i++)
            {
                buffers_nodes[i].status.exchange({0, 0, 0});
                buffers_nodes[i].data_size = 0;
                buffers_nodes[i].data_offset = 0;
                free_buffers_.push_back(&buffers_nodes[i]);
            }
        }

        ~Segment()
        {
            if (segment_node_->ref_count.fetch_sub(1) == 1)
            {
                segment_.reset();
                SharedMemSegment::remove(segment_id_.to_string().c_str());
            }

            if (overflows_count_)
            {
                logWarning(RTPS_TRANSPORT_SHM,
                        "Segment " << segment_id_.to_string().c_str()
                                   << " closed. It had " << "overflows_count "
                                   << overflows_count_);
            }
        }

        SharedMemSegment::Id id()
        {
            return segment_id_;
        }

        std::shared_ptr<Buffer> alloc_buffer(
                uint32_t size,
                const std::chrono::steady_clock::time_point& max_blocking_time_point)
        {
            (void)max_blocking_time_point;

            std::lock_guard<std::mutex> lock(alloc_mutex_);

            if (!recover_buffers(size))
            {
                throw std::runtime_error("allocation overflow");
            }

            void* data = nullptr;
            BufferNode* buffer_node = nullptr;
            std::shared_ptr<SharedMemBuffer> new_buffer;

            try
            {
                buffer_node = pop_free_node();

                data = segment_->get().allocate(size);
                free_bytes_-= size;
                
                buffer_node->data_offset = segment_->get_offset_from_address(data);
                buffer_node->data_size = size;

                auto validity_id = buffer_node->status.load(std::memory_order_relaxed).validity_id;
                                
                // TODO(Adolfo) : Dynamic allocation. Use foonathan to convert it to static allocation
                new_buffer = std::make_shared<SharedMemBuffer>(segment_, segment_id_, buffer_node, 
                    static_cast<uint32_t>(validity_id));

                if (new_buffer)
                {
                    buffer_node->inc_processing_count(static_cast<uint32_t>(validity_id));
                }
                else
                {
                    throw std::runtime_error("alloc_buffer: out of memory");
                }
                
                // TODO(Adolfo) : Dynamic allocation. Use foonathan to convert it to static allocation
                allocated_buffers_.push_back(buffer_node);
            }
            catch (const std::exception&)
            {
                if (buffer_node)
                {
                    if (data)
                    {
                        release_buffer(buffer_node);
                    }

                    free_buffers_.push_back(buffer_node);
                }

                overflows_count_++;

                throw;
            }

            return new_buffer;
        }

    private:

        SegmentNode* segment_node_;

        // TODO(Adolfo) : Dynamic allocations. Use foonathan to convert it to static allocation
        std::list<BufferNode*> free_buffers_;
        std::list<BufferNode*> allocated_buffers_;

        std::mutex alloc_mutex_;
        std::shared_ptr<SharedMemSegment> segment_;
        SharedMemSegment::Id segment_id_;
        uint64_t overflows_count_;

        uint32_t free_bytes_;

        inline BufferNode* pop_free_node()
        {
            if (free_buffers_.empty())
            {
                throw std::runtime_error("BufferNodes overflow");
            }

            auto node = free_buffers_.back();
            free_buffers_.pop_back();
            return node;
        }

        void release_buffer(
                BufferNode* buffer_node)
        {            
            segment_->get().deallocate(
                segment_->get_address_from_offset(buffer_node->data_offset));

            free_bytes_ += buffer_node->data_size;
        }

        /**
         * Recover unreferenced buffers and, in case of overflow, also recovers the oldest buffers not being
         * processed by any listener (until enough free bytes is gathered to solve the overflow).
         * @return true if at least required_data_size bytes are free after the recovery, false otherwise.
         */
        bool recover_buffers(uint32_t required_data_size)
        {
            auto it = allocated_buffers_.begin();

            while(it != allocated_buffers_.end())
            {
                // There is enough space to allocate the buffer
                if (free_bytes_ >= required_data_size)
                {
                    if ((*it)->is_not_referenced())
                    {
                        (*it)->invalidate_buffer();

                        release_buffer(*it);

                        free_buffers_.push_back(*it);
                        it = allocated_buffers_.erase(it);
                    }
                    else
                    {
                        it++;
                    }
                }
                else // No enough space, try to recover oldest not processing buffers
                {
                    // Buffer is not beign processed by any listener
                    if ((*it)->invalidate_if_not_processing())
                    {
                        release_buffer(*it);

                        free_buffers_.push_back(*it);
                        it = allocated_buffers_.erase(it);
                    }
                    else
                    {
                        it++;
                    }
                }
            }

            return free_bytes_ >= required_data_size;
        }

    }; // Segment

    class Port;

    /**
     * Listen to descriptors pushed to a port.
     * Provides an interface to wait and access to the data referenced by the descriptors
     */
    class Listener
    {
    public:

        Listener(
                SharedMemManager* shared_mem_manager,
                std::shared_ptr<SharedMemGlobal::Port> port)
            : global_port_(port)
            , shared_mem_manager_(shared_mem_manager)
            , is_closed_(false)
        {
            global_listener_ = global_port_->create_listener(&listener_index_);
        }

        ~Listener()
        {
            global_listener_.reset();
            if (global_port_)
            {
                try
                {
                    global_port_->unregister_listener();
                }
                catch(const std::exception& e)
                {
                    logWarning(RTPS_TRANSPORT_SHM, e.what());
                }
            }
        }

        Listener& operator = (
                Listener&& other)
        {
            global_listener_ = other.global_listener_;
            other.global_listener_.reset();
            global_port_ = other.global_port_;
            other.global_port_.reset();
            shared_mem_manager_ = other.shared_mem_manager_;
            is_closed_.exchange(other.is_closed_);

            return *this;
        }

        /**
         * Extract the first buffer enqued in the port.
         * If the queue is empty, blocks until a buffer is pushed
         * to the port.
         * @return A shared_ptr to the buffer, this shared_ptr can be nullptr if the
         * wait was interrupted because errors or close operations.
         * @remark Multithread not supported.
         */
        std::shared_ptr<Buffer> pop()
        {
            std::shared_ptr<Buffer> buffer_ref;
            bool is_buffer_valid = false;

            try
            {
                while(!is_buffer_valid)
                {
                    bool was_cell_freed;

                    SharedMemGlobal::PortCell* head_cell = nullptr;
                    buffer_ref.reset();

                    while ( !is_closed_.load() && nullptr == (head_cell = global_listener_->head()) )
                    {
                        // Wait until there's data to pop
                        global_port_->wait_pop(*global_listener_, is_closed_, listener_index_);
                    }

                    if (!head_cell)
                    {
                        return nullptr;
                    }

                    if (!global_port_->is_port_ok())
                    {
                        throw std::runtime_error("");
                    }

                    SharedMemGlobal::BufferDescriptor buffer_descriptor = head_cell->data();

                    SegmentNode* segment_node;
                    auto segment = shared_mem_manager_->find_segment(buffer_descriptor.source_segment_id, &segment_node);
                    auto buffer_node =
                            static_cast<BufferNode*>(segment->get_address_from_offset(buffer_descriptor.buffer_node_offset));

                    // TODO(Adolfo) : Dynamic allocation. Use foonathan to convert it to static allocation
                    buffer_ref = std::make_shared<SharedMemBuffer>(segment, buffer_descriptor.source_segment_id,
                                    buffer_node,
                                    buffer_descriptor.validity_id);

                    // If the cell has been read by all listeners
                    global_port_->pop(*global_listener_, was_cell_freed);

                    if (buffer_ref)
                    {
                        if (was_cell_freed)
                        {
                            // Atomically increase processing & decrease enqueued 
                            is_buffer_valid = buffer_node->dec_enqueued_inc_processing_counts(
                                buffer_descriptor.validity_id);
                        }
                        else
                        {
                            is_buffer_valid = buffer_node->inc_processing_count(buffer_descriptor.validity_id);
                        }
                    }
                    else
                    {
                        if (was_cell_freed)
                        {
                            buffer_node->dec_enqueued_count(buffer_descriptor.validity_id);
                        }
                        
                        throw std::runtime_error("pop() : out of memory");
                    }
                }
            }
            catch (const std::exception& e)
            {
                if (global_port_->is_port_ok())
                {
                    throw;
                }
                else
                {
                    logWarning(RTPS_TRANSPORT_SHM, "SHM Listener on port " << global_port_->port_id() << " failure: "
                                                                           << e.what());

                    regenerate_port();
                }
            }

            return buffer_ref;
        }

        void regenerate_port()
        {
            auto new_port = shared_mem_manager_->open_port(
                global_port_->port_id(),
                global_port_->max_buffer_descriptors(),
                global_port_->healthy_check_timeout_ms(),
                global_port_->open_mode()
                );

            auto new_listener = new_port->create_listener();

            *this = std::move(*new_listener);
        }

        /**
         * Unblock a thread blocked in pop() call, not allowing pop() to block again,
         */
        void close()
        {
            try
            {
                // Just in case a thread is blocked in pop() function
                global_port_->close_listener(&is_closed_);
            }
            catch(const std::exception& e)
            {
                logWarning(RTPS_TRANSPORT_SHM, e.what());
            }
        }

    private:

        std::shared_ptr<SharedMemGlobal::Port> global_port_;

        std::shared_ptr<SharedMemGlobal::Listener> global_listener_;
        uint32_t listener_index_;

        SharedMemManager* shared_mem_manager_;

        std::atomic<bool> is_closed_;

    }; // Listener

    /**
     * Allows to push buffers and create listeners of a shared-memory port,
     */
    class Port
    {
    public:

        Port(
                SharedMemManager* shared_mem_manager,
                std::shared_ptr<SharedMemGlobal::Port> port,
                SharedMemGlobal::Port::OpenMode open_mode)
            : shared_mem_manager_(shared_mem_manager)
            , global_port_(port)
            , open_mode_(open_mode)
        {
        }

        Port& operator = (
                Port&& other)
        {
            shared_mem_manager_ = other.shared_mem_manager_;
            open_mode_ = other.open_mode_;
            global_port_ = other.global_port_;
            other.global_port_.reset();

            return *this;
        }

        /**
         * Try to enqueue a buffer in the port.
         * @returns false If the port's queue is full so buffer couldn't be enqueued.
         */
        bool try_push(
                const std::shared_ptr<Buffer>& buffer)
        {
            assert(std::dynamic_pointer_cast<SharedMemBuffer>(buffer));

            SharedMemBuffer* shared_mem_buffer = std::static_pointer_cast<SharedMemBuffer>(buffer).get();

            bool ret;
            bool are_listeners_active = false;
            auto validity_id = shared_mem_buffer->validity_id();

            shared_mem_buffer->inc_enqueued_count(validity_id);

            try
            {
                ret = global_port_->try_push( {shared_mem_buffer->segment_id(), shared_mem_buffer->node_offset(),
                                               validity_id},
                                &are_listeners_active);

                if (!are_listeners_active)
                {
                    shared_mem_buffer->dec_enqueued_count(validity_id);
                }
            }
            catch (std::exception& e)
            {
                shared_mem_buffer->dec_enqueued_count(validity_id);

                if (!global_port_->is_port_ok())
                {
                    logWarning(RTPS_TRANSPORT_SHM, "SHM Port " << global_port_->port_id() << " failure: "
                                                               << e.what());

                    regenerate_port();
                    ret = false;
                }
                else
                {
                    throw;
                }
            }

            return ret;
        }

        std::shared_ptr<Listener> create_listener()
        {
            return std::make_shared<Listener>(shared_mem_manager_, global_port_);
        }

    private:

        void regenerate_port()
        {
            auto new_port = shared_mem_manager_->open_port(
                global_port_->port_id(),
                global_port_->max_buffer_descriptors(),
                global_port_->healthy_check_timeout_ms(),
                open_mode_);

            *this = std::move(*new_port);
        }

        SharedMemManager* shared_mem_manager_;

        std::shared_ptr<SharedMemGlobal::Port> global_port_;

        SharedMemGlobal::Port::OpenMode open_mode_;

    }; // Port

    /**
     * Creates a shared-memory segment
     * @param size size of the segment
     * @param max_buffers maximum, at a time, allocated buffers
     * @return A shared_ptr to the segment
     */
    std::shared_ptr<Segment> create_segment(
            uint32_t size,
            uint32_t max_allocations)
    {
        // Every buffer allocation of 'n-bytes', consumes an extra 'per_allocation_extra_size_' bytes.
        // This is due to the allocator internal structures (also residing in the shared-memory segment)
        // used to manage the allocation algorithm. 
        // So with an estimation of 'max_allocations' user buffers, the total segment extra size is computed.
        uint32_t allocation_extra_size = 
            sizeof(SegmentNode) + per_allocation_extra_size_ +
            (max_allocations * sizeof(BufferNode)) + per_allocation_extra_size_ +
            max_allocations * per_allocation_extra_size_;

        return std::make_shared<Segment>(size + allocation_extra_size, size, max_allocations);
    }

    std::shared_ptr<Port> open_port(
            uint32_t port_id,
            uint32_t max_descriptors,
            uint32_t healthy_check_timeout_ms,
            SharedMemGlobal::Port::OpenMode open_mode = SharedMemGlobal::Port::OpenMode::ReadShared)
    {
        return std::make_shared<Port>(this,
                       global_segment_.open_port(port_id, max_descriptors, healthy_check_timeout_ms, open_mode),
                       open_mode);
    }

    /**
     * Remove a port from the system.
     */
    void remove_port(uint32_t port_id)
    {
        global_segment_.remove_port(port_id);
    }

    /**
     * @return Pointer to the underlying global segment. The pointer is only valid
     * while this SharedMemManager is alive.
     */
    SharedMemGlobal* global_segment()
    {
        return &global_segment_;
    }

private:

    /**
     * Controls life-cycle of a remote segment
     */
    class SegmentWrapper
    {
    public:

        SegmentWrapper()
        {

        }

        SegmentWrapper(
                std::shared_ptr<SharedMemSegment> segment_,
                SharedMemSegment::Id segment_id)
            : segment_(segment_)
            , segment_id_(segment_id)
        {
            segment_node_ = segment_->get().find<SegmentNode>("segment_node").first;

            if (!segment_node_)
            {
                throw std::runtime_error("segment_node not found");
            }

            segment_node_->ref_count.fetch_add(1);
        }

        SegmentWrapper& operator=(
                SegmentWrapper&& other)
        {
            segment_ = other.segment_;
            segment_id_ = other.segment_id_;
            segment_node_ = other.segment_node_;

            other.segment_.reset();

            return *this;
        }

        ~SegmentWrapper()
        {
            if (segment_ != nullptr && segment_node_->ref_count.fetch_sub(1) == 1)
            {
                segment_.reset();
                SharedMemSegment::remove(segment_id_.to_string().c_str());
            }
        }

        std::shared_ptr<SharedMemSegment> segment() { return segment_; }
        SegmentNode* segment_node() { return segment_node_; }

    private:

        std::shared_ptr<SharedMemSegment> segment_;
        SharedMemSegment::Id segment_id_;
        SegmentNode* segment_node_;
    };

    uint32_t per_allocation_extra_size_;

    std::unordered_map<SharedMemSegment::Id::type, SegmentWrapper,
            std::hash<SharedMemSegment::Id::type> > ids_segments_;
    std::mutex ids_segments_mutex_;

    SharedMemGlobal global_segment_;

    std::shared_ptr<SharedMemSegment> find_segment(
            SharedMemSegment::Id id,
            SegmentNode** segment_node)
    {
        std::lock_guard<std::mutex> lock(ids_segments_mutex_);

        std::shared_ptr<SharedMemSegment> segment;

        // TODO (Adolfo): Garbage collector for opened but unused segments????

        try
        {
            SegmentWrapper& segment_wrapper = ids_segments_.at(id.get());
            segment = segment_wrapper.segment();
            *segment_node = segment_wrapper.segment_node();
        }
        catch (std::out_of_range&)
        {
            segment = std::make_shared<SharedMemSegment>(boost::interprocess::open_only, id.to_string());
            SegmentWrapper segment_wrapper(segment, id);

            *segment_node = segment_wrapper.segment_node();
            ids_segments_[id.get()] = std::move(segment_wrapper);
        }

        return segment;
    }

    /**
     * Called by PortWatchdog when a dead listener has been detected.
     * At this point the port is marked as not OK, and a vector of
     * the recovered descriptors, from the port, are passed to
     * this function that performs their release.
     */
    static void on_failure_buffer_descriptor_handler(
            const std::vector<const SharedMemGlobal::BufferDescriptor*>& buffer_descriptors,
            const std::string& domain_name)
    {
        try
        {
            SharedMemManager shared_mem_manager(domain_name);

            for (auto buffer_descriptor : buffer_descriptors)
            {
                SegmentNode* segment_node;
                auto segment = shared_mem_manager.find_segment(buffer_descriptor->source_segment_id, &segment_node);
                auto buffer_node =
                        static_cast<BufferNode*>(segment->get_address_from_offset(buffer_descriptor->buffer_node_offset));

                buffer_node->dec_enqueued_count(buffer_descriptor->validity_id);
            }
        }
        catch (const std::exception& e)
        {
            logError(RTPS_TRANSPORT_SHM, e.what());
        }
    }
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_MANAGER_H_