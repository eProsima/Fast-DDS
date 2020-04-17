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
        struct
        {
            std::atomic<int> ref_count;
            uint32_t data_size;
        }header;
        uint8_t data[1];
    };

    struct SegmentNode
    {
        std::atomic<uint32_t> ref_count;
        std::atomic<uint32_t> free_bytes;
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
                }
        );
            
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
                SegmentNode* segment_node)
            : segment_(segment)
            , segment_id_(segment_id)
            , buffer_node_(buffer_node)
            , segment_node_(segment_node)
        {
            increase_ref();
        }

        ~SharedMemBuffer() override
        {
            decrease_ref();
        }

        void* data() override
        {
            return buffer_node_->data;
        }

        uint32_t size() override
        {
            return buffer_node_->header.data_size;
        }

        SharedMemSegment::Offset node_offset()
        {
            return segment_->get_offset_from_address(buffer_node_);
        }

        SharedMemSegment::Id segment_id()
        {
            return segment_id_;
        }

        void increase_ref()
        {
            buffer_node_->header.ref_count.fetch_add(1);
        }

        void decrease_ref()
        {
            int32_t buffer_size = buffer_node_->header.data_size;

            // Last reference to the buffer
            if (buffer_node_->header.ref_count.fetch_sub(1) == 1)
            {
                // Anotate the new free space
                segment_node_->free_bytes.fetch_add(buffer_size);
            }
        }

    private:

        std::shared_ptr<SharedMemSegment> segment_;
        SharedMemSegment::Id segment_id_;
        BufferNode* buffer_node_;
        SegmentNode* segment_node_;
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
                uint32_t payload_size)
            : segment_id_()
            , overflows_count_(0)
#ifdef SHM_SEGMENT_OVERFLOW_TIMEOUT
            , max_payload_size_(payload_size)
#endif
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
            segment_node_->free_bytes.exchange(payload_size);
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
            std::lock_guard<std::mutex> lock(alloc_mutex_);

            wait_for_avaible_space(size, max_blocking_time_point);

            release_unused_buffers();

            void* data = nullptr;
            BufferNode* buffer_node = nullptr;
            std::shared_ptr<SharedMemBuffer> new_buffer;

            try
            {
                buffer_node = static_cast<BufferNode*>(
                    segment_->get().allocate_aligned(sizeof(BufferNode) + size, 
                        std::alignment_of<BufferNode>::value));

                buffer_node->header.data_size = size;
                buffer_node->header.ref_count.store(0, std::memory_order_relaxed);

                // TODO(Adolfo) : Dynamic allocation. Use foonathan to convert it to static allocation
                new_buffer = std::make_shared<SharedMemBuffer>(segment_, segment_id_, buffer_node, segment_node_);

                // TODO(Adolfo) : Dynamic allocation. Use foonathan to convert it to static allocation
                allocated_nodes_.push_back(buffer_node);

                segment_node_->free_bytes.fetch_sub(size);
            }
            catch (const std::exception&)
            {
                if (buffer_node)
                {
                    release_buffer(buffer_node);
                }
                else if (data)
                {
                    segment_->get().deallocate(data);
                }

                overflows_count_++;

                throw;
            }

            return new_buffer;
        }

    private:

        SegmentNode* segment_node_;
        std::list<BufferNode*> allocated_nodes_;
        std::mutex alloc_mutex_;
        std::shared_ptr<SharedMemSegment> segment_;
        SharedMemSegment::Id segment_id_;
        uint64_t overflows_count_;

#ifdef SHM_SEGMENT_OVERFLOW_TIMEOUT
        uint32_t max_payload_size_;
#endif

        void release_buffer(
                BufferNode* buffer_node)
        {
            segment_->get().deallocate(buffer_node);
        }

        void release_unused_buffers()
        {
            auto node_it = allocated_nodes_.begin();
            while (node_it != allocated_nodes_.end())
            {
                // This shouldn't normally be negative, but when processes crashes
                // due to fault-tolerance mecanishms it could happen.
                if ((*node_it)->header.ref_count.load() <= 0)
                {
                    release_buffer(*node_it);

                    allocated_nodes_.erase(node_it++);
                }
                else
                {
                    node_it++;
                }
            }
        }

        void wait_for_avaible_space(
                uint32_t size,
                const std::chrono::steady_clock::time_point& max_blocking_time_point)
        {
#ifdef SHM_SEGMENT_OVERFLOW_TIMEOUT
            SharedMemSegment::spin_wait spin_wait;

            // Not enough avaible space
            while ( segment_node_->free_bytes.load(std::memory_order_relaxed) < size &&
                    std::chrono::steady_clock::now() < max_blocking_time_point )
            {
                if (size > max_payload_size_)
                {
                    throw std::runtime_error("buffer bigger than whole segment size");
                }

                // Optimized active wait. We're in overflow with timeout case.
                // Much higher throughput is achive with active wait over shared-memory
                // vs interprocess_mutex + interprocess_cv.
                spin_wait.yield();
            }
#else
            (void)max_blocking_time_point;
#endif
            if (segment_node_->free_bytes.load(std::memory_order_relaxed) < size)
            {
                throw std::runtime_error("allocation timeout");
            }
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
                global_port_->unregister_listener();
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

            try
            {
                bool was_cell_freed;
                
                SharedMemGlobal::PortCell* head_cell = nullptr;

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
                buffer_ref = std::make_shared<SharedMemBuffer>(segment, buffer_descriptor.source_segment_id, buffer_node,
                                segment_node);

                // If the cell has been read by all listeners
                global_port_->pop(*global_listener_, was_cell_freed);

                if (was_cell_freed)
                {
                    buffer_node->header.ref_count.fetch_sub(1);
                }
            }
            catch(const std::exception& e)
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
            // Just in case a thread is blocked in pop() function
            global_port_->close_listener(&is_closed_);
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
            shared_mem_buffer->increase_ref();

            bool ret;
            bool are_listeners_active = false;

            try
            {
                ret = global_port_->try_push({shared_mem_buffer->segment_id(), shared_mem_buffer->node_offset()},
                                &are_listeners_active);

                if (!are_listeners_active)
                {
                    shared_mem_buffer->decrease_ref();
                }
            }
            catch (std::exception& e)
            {
                shared_mem_buffer->decrease_ref();

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
        // Every buffer allocated implies two internal allocations, node and payload.
        // Every internal allocation consumes 'per_allocation_extra_size_' bytes
        uint32_t allocation_extra_size = sizeof(SegmentNode) + per_allocation_extra_size_ +
                max_allocations * ((sizeof(BufferNode) + per_allocation_extra_size_) + per_allocation_extra_size_);

        return std::make_shared<Segment>(size + allocation_extra_size, size);
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

            if(!segment_node_)
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

                int32_t buffer_size = buffer_node->header.data_size;

                // Last reference to the buffer
                if (buffer_node->header.ref_count.fetch_sub(1) == 1)
                {
                    // Anotate the new free space
                    segment_node->free_bytes.fetch_add(buffer_size);
                }
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