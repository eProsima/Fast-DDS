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

namespace eprosima{
namespace fastdds{
namespace rtps{

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

private:

public:

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
            if (read_p_ == pointer.write_p )
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
         * @throw int if buffer is empty
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
                        { pointer.write_p, pointer.free_cells + 1 },
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

    struct Pointer
    {
        uint32_t write_p;
        uint32_t free_cells;
    };

    struct RegisterPushLock
    {
        uint32_t pushing_count;
        bool registering_flag;
    };

    struct Node
    {
        std::atomic<Pointer> pointer_;
        uint32_t total_cells_;
        uint32_t registered_listeners_;
        std::atomic<RegisterPushLock> register_push_lock_;
    };

    MultiProducerConsumerRingBuffer(
            Cell* cells_base,
            uint32_t total_cells)
        : cells_(cells_base)
        , is_node_owned_(true)
    {
        if (total_cells > (1 << 31))
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
        lock_registering();

        // If no listeners the buffer is dropped
        if (node_->registered_listeners_ == 0)
        {
            unlock_registering();
            return false;
        }

        auto pointer = node_->pointer_.load(std::memory_order_relaxed);

        // if free cells, increase the write pointer and decrease the free cells
        while (pointer.free_cells > 0 &&
                !node_->pointer_.compare_exchange_weak(pointer, {inc_pointer(pointer.write_p), pointer.free_cells-1},
                std::memory_order_release,
                std::memory_order_relaxed))
        {
        }

        unlock_registering();


        if (pointer.free_cells == 0)
        {
            unlock_registering();
            throw std::runtime_error("Buffer full");
        }

        auto& cell = cells_[get_pointer_value(pointer.write_p)];

        cell.data(data);
        cell.ref_counter_.store(node_->registered_listeners_, std::memory_order_release);

        unlock_registering();

        return true;
    }

    bool is_buffer_full()
    {
        return (node_->pointer_.load(std::memory_order_relaxed).free_cells == 0);
    }

    bool is_buffer_empty()
    {
        return (node_->pointer_.load(std::memory_order_relaxed).free_cells == node_->total_cells_);
    }

    std::shared_ptr<Listener> register_listener()
    {
        lock_pushing();

        // The new listener's read pointer is the current write pointer
        auto listener = std::make_shared<Listener>(*this, node_->pointer_.load(std::memory_order_relaxed).write_p);

        node_->registered_listeners_++;

        unlock_pushing();

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
        node->register_push_lock_.store({0, false});
        node->pointer_.store({0,total_cells}, std::memory_order_relaxed);
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
            const uint32_t pointer)
    {
        auto value = pointer & 0x7FFFFFFF;
        auto loop_flag = pointer >> 31;

        value = (value + 1) % node_->total_cells_;

        if (value == 0)
        {
            loop_flag ^= 1;
        }

        // Bit 31 is loop_flag, 0-30 are value
        return (loop_flag << 31) | value;
    }

    /**
     * Called by the writters to lock listener's registering while writer pushes
     */
    void lock_registering()
    {
        auto register_push = node_->register_push_lock_.load(std::memory_order_relaxed);
        // Increase the pushing_count (only possible if registering_flag == false)
        while (!node_->register_push_lock_.compare_exchange_weak(register_push, {register_push.pushing_count+1, false},
                std::memory_order_acquire,
                std::memory_order_relaxed));
    }

    /**
     * Called by the writters to unlock listener's registering
     */
    void unlock_registering()
    {
        auto register_push = node_->register_push_lock_.load(std::memory_order_relaxed);
        assert(!register_push.registering_flag); // Opssss.
        // Decrease the pushing_count
        while (!node_->register_push_lock_.compare_exchange_weak(register_push, {register_push.pushing_count-1, false},
                std::memory_order_release,
                std::memory_order_relaxed));
    }

    /**
     * Called by a listener when registering to lock writters pushing
     */
    void lock_pushing()
    {
        RegisterPushLock register_push = {0, false};
        // Registering a listener... pushing_count must be 0
        while (!node_->register_push_lock_.compare_exchange_weak(register_push, {0, true},
                std::memory_order_acquire,
                std::memory_order_relaxed));
    }

    /**
     * Called by a listener when registering finish to unlock writters pushing
     */
    void unlock_pushing()
    {
        RegisterPushLock register_push = {0, true};
        // Registering a listener... pushing_count must be 0
        while (!node_->register_push_lock_.compare_exchange_weak(register_push, {0, false},
                std::memory_order_release,
                std::memory_order_relaxed));
    }

    void unregister_listener(
            Listener& listener)
    {
        lock_pushing();

        try
        {
            // Forzes to decrement the ref_counters for cells available to the listener.
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

        unlock_pushing();
    }
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_MPC_RINGBUFFER_