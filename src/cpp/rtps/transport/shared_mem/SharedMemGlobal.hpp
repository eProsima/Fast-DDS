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

namespace eprosima{
namespace fastdds{
namespace rtps{

class SharedMemGlobal
{
public:

    SharedMemGlobal(
            const std::string& domain_name)
        : domain_name_(domain_name)
    {
        auto ports_mutex_name = domain_name + "ports_mutex";

        SharedMemSegment::named_mutex::remove(ports_mutex_name.c_str());

        ports_mutex_ = std::unique_ptr<SharedMemSegment::named_mutex>(
                new SharedMemSegment::named_mutex(boost::interprocess::open_or_create, ports_mutex_name.c_str()));
    }

    ~SharedMemGlobal()
    {
        ports_mutex_.reset();

        SharedMemSegment::named_mutex::remove((domain_name_ + "ports_mutex").c_str());
    }

    struct BufferDescriptor
    {
        SharedMemSegment::Id source_segment_id;
        SharedMemSegment::offset buffer_node_offset;
    };

    typedef MultiProducerConsumerRingBuffer<BufferDescriptor>::Listener Listener;
    typedef MultiProducerConsumerRingBuffer<BufferDescriptor>::Cell PortCell;

    struct PortNode
    {
        SharedMemSegment::condition_variable empty_cv;
        SharedMemSegment::mutex empty_cv_mutex;
        SharedMemSegment::condition_variable full_cv;
        SharedMemSegment::mutex full_cv_mutex;
        SharedMemSegment::offset buffer;
        SharedMemSegment::offset buffer_node;
        std::atomic<uint32_t> ref_counter;
    };

    class Port
    {
private:

        std::unique_ptr<SharedMemSegment> port_segment_;

        PortNode* node_;

        std::unique_ptr<MultiProducerConsumerRingBuffer<BufferDescriptor> > buffer_;

public:

        Port(
                std::unique_ptr<SharedMemSegment>&& port_segment,
                PortNode* node)
            : port_segment_(std::move(port_segment))
            , node_(node)
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
                port_segment_.reset();
                SharedMemSegment::remove(segment_name.c_str());
            }
        }

        bool push(
                const BufferDescriptor& buffer_descriptor)
        {
            bool ret;

            do
            {
                std::unique_lock<SharedMemSegment::mutex> lock_full(node_->full_cv_mutex);
                std::unique_lock<SharedMemSegment::mutex> lock_empty(node_->empty_cv_mutex);
                //boost::interprocess::scoped_lock<SharedMemSegment::mutex> lock_full(node_->full_cv_mutex);
                //boost::interprocess::scoped_lock<SharedMemSegment::mutex> lock_empty(node_->empty_cv_mutex);

                try
                {
                    ret = buffer_->push(buffer_descriptor);

                    lock_empty.unlock();
                    node_->empty_cv.notify_all();

                    break;
                }
                catch (const std::exception&)
                {
                    lock_empty.unlock();
                    node_->full_cv.wait(lock_full);
                }
            } while (1);

            return ret;
        }

        /**
         * Waits while the port is empty and listener is not closed
         */
        void wait_pop(
                Listener& listener,
                const std::atomic<bool>& is_listener_closed)
        {
            std::unique_lock<SharedMemSegment::mutex> lock(node_->empty_cv_mutex);
            //boost::interprocess::scoped_lock<SharedMemSegment::mutex> lock(node_->empty_cv_mutex);

            node_->empty_cv.wait(lock, [&] {
                return is_listener_closed.load() || listener.head() != nullptr;
            });
        }

        /**
         * Set the caller's 'is_closed' flag (protecting empty_cv_mutex) and
         * forzes wake-up all listeners on this port.
         * This function is used when destroying a listener waiting for messages
         * in the port.
         */
        void close_listener(
                std::atomic<bool>* is_listener_closed)
        {
            std::unique_lock<SharedMemSegment::mutex> lock(node_->empty_cv_mutex);
            is_listener_closed->exchange(true);
            lock.unlock();

            node_->empty_cv.notify_all();
        }

        bool pop(
                Listener& listener,
                bool& was_cell_freed)
        {
            //std::unique_lock<SharedMemSegment::mutex> lock(node_->full_cv_mutex);
            boost::interprocess::scoped_lock<SharedMemSegment::mutex> lock(node_->full_cv_mutex);

            // If a cell is freed because has been read by all listeners
            if ( (was_cell_freed = listener.pop()) )
            {
                // A writer is allowed to push
                lock.unlock();
                node_->full_cv.notify_one();
            }

            return true;
        }

        std::shared_ptr<Listener> create_listener()
        {
            return buffer_->register_listener();
        }

    }; // Port

    std::shared_ptr<Port> open_port(
            uint32_t port_id,
            uint32_t max_buffer_descriptors)
    {
        std::shared_ptr<Port> port;

        auto port_segment_name = domain_name_ + "_port" + std::to_string(port_id);

        std::lock_guard<SharedMemSegment::named_mutex> lock(*ports_mutex_);

        try
        {
            // Try to open
            auto port_segment = std::unique_ptr<SharedMemSegment>(
                    new SharedMemSegment(boost::interprocess::open_only, port_segment_name.c_str()));

            auto port_node = port_segment->get().find<PortNode>("port_node").first;
            port = std::make_shared<Port>(std::move(port_segment), port_node);
        }
        catch (std::exception&)
        {
            // Doesn't exist => create it
            // The segment will contain the node, the buffer and the internal allocator structures (512bytes estimated)
            uint32_t segment_size = sizeof(PortNode) + sizeof(PortCell) * max_buffer_descriptors  + 512;

            auto port_segment = std::unique_ptr<SharedMemSegment>(
                            new SharedMemSegment(boost::interprocess::create_only, port_segment_name.c_str(), segment_size));
                            
            port = init_port(port_segment, max_buffer_descriptors);
        }

        return port;
    }

private:

    std::string domain_name_;

    std::unique_ptr<SharedMemSegment::named_mutex> ports_mutex_;

    std::shared_ptr<Port> init_port(
            std::unique_ptr<SharedMemSegment>& segment,
            uint32_t max_buffer_descriptors)
    {
        std::shared_ptr<Port> port;
        PortNode* port_node = nullptr;
        MultiProducerConsumerRingBuffer<BufferDescriptor>::Node* buffer_node = nullptr;

        try
        {
            // Port node allocation
            port_node = segment->get().construct<PortNode>("port_node")();

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