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

#ifndef _FASTDDS_SHAREDMEM_MPC_RINGBUFFER_
#define _FASTDDS_SHAREDMEM_MPC_RINGBUFFER_

#include <atomic>
#include <memory>
#include <cstdlib>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Ring buffer capable of multiple producers / multiple consumers.
 * Read / Write operations are lock-free.
 * Data is organized in a fixed number of Cells of the same type.
 * Consumers (listeners) must be registered to access the cells.
 * When a Cell is pushed to the buffer, a counter for that cell is initialized with number
 * of listeners registered in the buffer in that moment. The Cell will be freed
 * when all listeners have poped the cell.
 */
template <class T>
class MultiProducerConsumerRingBuffer
{
public:

    class Cell
    {
    public:

        const T& data() const
        {
            return data_;
        }

        void data(
                const T& data)
        {
            data_ = data;
        }

        uint32_t ref_counter() const
        {
            return ref_counter_.load(std::memory_order_relaxed);
        }

        friend class MultiProducerConsumerRingBuffer<T>;

        std::atomic<uint32_t> ref_counter_;
        T data_;
    };

    class Listener
    {
    public:

        Listener(
                MultiProducerConsumerRingBuffer<T>& buffer,
                uint32_t write_p)
            : buffer_(buffer)
            , read_p_(write_p)
        {
        }

        ~Listener()
        {
            buffer_.unregister_listener(*this);
        }

        /**
         * @returns the Cell at the read pointer or nullptr if the buffer is empty
         */
        Cell* head()
        {
            auto pointer = buffer_.node_->pointer_.load(std::memory_order_relaxed);

            // If local read_pointer and write_pointer are equal => buffer is empty for this listener
            if (read_p_ == pointer.ptr.write_p )
            {
                return nullptr;
            }

            auto cell = &buffer_.cells_[get_pointer_value(read_p_)];

            return cell->ref_counter() != 0 ? cell : nullptr;
        }

        /**
         * Decreases the ref_counter of the head cell,
         * if the counter reaches 0 the cell becomes dirty
         * and free_cells are incremented
         * @return true if the cell ref_counter is 0 after pop
         * @throw std::exception if buffer is empty
         */
        bool pop()
        {
            auto cell = head();

            if (!cell)
            {
                throw std::runtime_error("Buffer empty");
            }

            auto counter = cell->ref_counter_.fetch_sub(1);
            assert(counter > 0);

            // If all the listeners have read the cell
            if (counter == 1)
            {
                // Increase the free cells => increase the global read pointer
                auto pointer = buffer_.node_->pointer_.load(std::memory_order_relaxed);
                while (!buffer_.node_->pointer_.compare_exchange_weak(pointer,
                        { { pointer.ptr.write_p, pointer.ptr.free_cells + 1 } },
                        std::memory_order_release,
                        std::memory_order_relaxed))
                {
                }
            }

            // Increase the local read pointer
            read_p_ = buffer_.inc_pointer(read_p_);

            return (counter == 1);
        }

    private:

        MultiProducerConsumerRingBuffer<T>& buffer_;
        uint32_t read_p_;
    };

    // std::atomic<> can only be used on lock-free primitive types with shared mem
    union PtrType
    {
        struct Pointer
        {
            uint32_t write_p;
            uint32_t free_cells;
        }
        ptr;
        uint64_t u;
    };

    struct Node
    {
        alignas(8) std::atomic<PtrType> pointer_;
        uint32_t total_cells_;

        uint32_t registered_listeners_;
    };

    MultiProducerConsumerRingBuffer(
            Cell* cells_base,
            uint32_t total_cells)
        : cells_(cells_base)
        , is_node_owned_(true)
    {
        if (total_cells > (1u << 31u))
        {
            throw std::runtime_error("total_cells out of range");
        }

        node_ = new Node();
        init_node(node_, total_cells);

        // Init cells
        for (Cell* cell = &cells_[0]; cell < &cells_[total_cells]; cell++)
        {
            cell->ref_counter_.store(0, std::memory_order_relaxed);
        }
    }

    MultiProducerConsumerRingBuffer(
            Cell* cells_base,
            Node* node)
        : cells_(cells_base)
        , is_node_owned_(false)
    {
        node_ = node;
    }

    ~MultiProducerConsumerRingBuffer()
    {
        if (is_node_owned_)
        {
            delete node_;
        }
    }

    /**
     * Push a new element into the buffer initializing the cell's ref_counter,
     * @return true if there are listeners registered, false if no listeners => buffer not enqueued
     * @throw std::runtime_error if the buffer is full
     */
    bool push(
            const T& data)
    {
        // If no listeners the buffer is dropped
        if (node_->registered_listeners_ == 0)
        {
            return false;
        }

        auto pointer = node_->pointer_.load(std::memory_order_relaxed);

        // if free cells, increase the write pointer and decrease the free cells
        while (pointer.ptr.free_cells > 0 &&
                !node_->pointer_.compare_exchange_weak(pointer,
                { { inc_pointer(pointer.ptr.write_p), pointer.ptr.free_cells - 1 } },
                std::memory_order_release,
                std::memory_order_relaxed))
        {
        }

        if (pointer.ptr.free_cells == 0)
        {
            throw std::runtime_error("Buffer full");
        }

        auto& cell = cells_[get_pointer_value(pointer.ptr.write_p)];

        cell.data(data);
        cell.ref_counter_.store(node_->registered_listeners_, std::memory_order_release);

        return true;
    }

    bool is_buffer_full()
    {
        return (node_->pointer_.load(std::memory_order_relaxed).ptr.free_cells == 0);
    }

    bool is_buffer_empty()
    {
        return (node_->pointer_.load(std::memory_order_relaxed).ptr.free_cells == node_->total_cells_);
    }

    /**
     * Register a new listener (consumer)
     * The new listener's read pointer is equal to the ring-buffer write pointer at the registering moment.
     * @return A unique_ptr to the listener.
     * @remarks This operation is not lock-free with push() / pop() operations, so the upper layer is responsible
     * for the mutual exclusion handling.
     * The listener will be unregistered when the unique_ptr is destroyed.
     */
    std::unique_ptr<Listener> register_listener()
    {
        // The new listener's read pointer is the current write pointer
        auto listener = std::unique_ptr<Listener>(
            new Listener(
                *this, node_->pointer_.load(std::memory_order_relaxed).ptr.write_p));

        node_->registered_listeners_++;

        return listener;
    }

    static void init_node(
            Node* node,
            uint32_t total_cells)
    {
        if (total_cells > static_cast<uint32_t>((1 << 31)))
        {
            throw std::runtime_error("total_cells out of range");
        }

        node->total_cells_ = total_cells;
        node->registered_listeners_ = 0;
        node->pointer_.store({ { 0, total_cells } }, std::memory_order_relaxed);
    }

    /**
     * Copies the currenty enqueued cells to a vector
     * @param [out] enqueued_cells pointers vector to where cells will be copied.
     * @remark This is an unsafe operation, that means the caller must assure
     * that no write operations are performed on the buffer while executing the copy.
     */
    void copy(
            std::vector<const T*>* enqueued_cells)
    {
        if (node_->registered_listeners_ > 0)
        {
            auto pointer = node_->pointer_.load(std::memory_order_relaxed);

            uint32_t p = pointer_to_head(pointer);

            while (p != pointer.ptr.write_p)
            {
                auto cell = &cells_[get_pointer_value(p)];

                // If the cell has not been read by any listener
                if (cell->ref_counter() > 0)
                {
                    enqueued_cells->push_back(&cell->data());
                }

                p = inc_pointer(p);
            }
        }
    }

private:

    Node* node_;
    Cell* cells_;
    bool is_node_owned_;

    static uint32_t get_pointer_value(
            uint32_t pointer)
    {
        // Bit 31 is loop_flag, 0-30 are value
        return pointer & 0x7FFFFFFF;
    }

    uint32_t inc_pointer(
            const uint32_t pointer) const
    {
        uint32_t value = pointer & 0x7FFFFFFF;
        uint32_t loop_flag = pointer >> 31;

        value = (value + 1) % node_->total_cells_;

        if (value == 0)
        {
            loop_flag ^= 1;
        }

        // Bit 31 is loop_flag, 0-30 are value
        return (loop_flag << 31) | value;
    }

    uint32_t pointer_to_head(
            const PtrType& pointer) const
    {
        // Init the head as write pointer in previous loop
        uint32_t head = pointer.ptr.write_p ^ 0x80000000;

        uint32_t value = head & 0x7FFFFFFF;
        uint32_t loop_flag = head >> 31;

        if (value +  pointer.ptr.free_cells >= node_->total_cells_)
        {
            loop_flag ^= 1;
        }

        // Skip the free cells
        value = (value + pointer.ptr.free_cells) % node_->total_cells_;

        // Bit 31 is loop_flag, 0-30 are value
        return (loop_flag << 31) | value;
    }

    /**
     * Unregister a listener.
     * @param listener Reference to the listener to unregister.
     * @remarks This operation is not lock-free with push() / pop() operations, so the upper layer is responsible.
     */
    void unregister_listener(
            Listener& listener)
    {
        try
        {
            // Forces to decrement the ref_counters for cells available to the listener.
            // A exception will break the loop
            while (1)
            {
                listener.pop();
            }

        }
        catch (const std::exception&)
        {
        }

        node_->registered_listeners_--;
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_MPC_RINGBUFFER_
