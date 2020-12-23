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

/**
 * @file FixedSizeQueue.hpp
 *
 */

#ifndef FASTRTPS_UTILS_COLLECTIONS_FIXEDSIZEQUEUE_HPP_
#define FASTRTPS_UTILS_COLLECTIONS_FIXEDSIZEQUEUE_HPP_

#include <assert.h>
#include <memory>
#include <type_traits>

namespace eprosima {
namespace fastrtps {

/**
 * A queue with a preallocated fixed size.
 *
 * This template class holds a circular queue of fixed size. Pushing a new element to a full queue
 * will result in an error.
 *
 * @tparam _Ty                 Element type.
 * @tparam _Alloc              Allocator to use on the underlying collection type, defaults to std::allocator<_Ty>.
 *
 * @ingroup UTILITIES_MODULE
 */
template <
    typename _Ty,
    typename _Alloc = std::allocator<_Ty>>
class FixedSizeQueue
{

public:

    using allocator_type = _Alloc;
    using value_type = _Ty;
    using pointer = _Ty*;
    using const_pointer = const _Ty*;
    using reference = _Ty&;
    using const_reference = const _Ty&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    class iterator
    {
    public:

        using iterator_category = std::forward_iterator_tag;

        /**
         * @brief Constructor using a pointer and a range of the managed buffer
         * @param ptr Pointer to be set
         * @param begin Pointer to the beginning of the managed buffer
         * @param end Pointer to the end of the managed buffer
         */
        iterator(
                pointer ptr,
                pointer begin,
                pointer end)
            : ptr_(ptr)
            , begin_(begin)
            , end_(end)
        {
        }

        /**
         * @brief Constructor of an unitialized iterator
         */
        iterator()
            : ptr_(nullptr)
            , begin_(nullptr)
            , end_(nullptr)
        {
        }

        iterator operator ++()
        {
            iterator i = *this;
            advance();
            return i;
        }

        iterator operator ++(
                int)
        {
            advance();
            return *this;
        }

        reference operator *()
        {
            return *ptr_;
        }

        pointer operator ->()
        {
            return ptr_;
        }

        bool operator ==(
                const iterator& rhs) const
        {
            return ptr_ == rhs.ptr_;
        }

        bool operator !=(
                const iterator& rhs) const
        {
            return ptr_ != rhs.ptr_;
        }

    protected:

        /**
         * @brief Shift the pointer to the next value
         */
        void advance()
        {
            ++ptr_;
            if (ptr_ == end_)
            {
                ptr_ = begin_;
            }
        }

    private:

        //! Current pointer
        pointer ptr_;
        //! Pointer to the begin of managed buffer
        pointer begin_;
        //! Pointer to the end of managed buffer
        pointer end_;
    };

    using const_iterator = const iterator;

    /**
     * Construct an unitialized FixedSizeQueue.
     *
     * The created queue is empty and has null capacity.
     * It cannot be used to store data until it is initialized
     */
    FixedSizeQueue(
            const allocator_type& alloc = allocator_type())
        : collection_(nullptr)
        , capacity_(0)
        , size_(0)
        , allocator_(alloc)
        , owns_(false)
    {
    }

    /**
     * Construct and initialize a FixedSizeQueue.
     *
     * This constructor receives the number of elements that will fit in the collection.
     *
     * @param capacity  Number of elements to reserve for the queue.
     * @param alloc     Allocator object. Forwarded to collection constructor.
     */
    FixedSizeQueue(
            size_type capacity,
            const allocator_type& alloc = allocator_type())
        : capacity_(capacity)
        , size_(0)
        , allocator_(alloc)
        , owns_(true)
    {
        collection_ = allocator_.allocate(capacity_);
        head_ = iterator(collection_, collection_, collection_ + capacity_);
        tail_ = head_;
    }

    /**
     * Construct and initialize a FixedSizeQueue on the given buffer.
     *
     * This constructor receives a buffer already allocated that will be used to back the queue.
     * The queue is not the owner of the buffer, so no deallocation will be done on destruction.
     * It is the responsibility of the caller to deallocate the buffer.
     *
     * \pre The buffer has capacity to hold \c capacity elements
     *
     * @param buffer    Buffer where to construct the queue.
     * @param capacity  Number of elements to reserve for the queue.
     */
    FixedSizeQueue(
            pointer buffer,
            size_type capacity)
        : capacity_(capacity)
        , size_(0)
        , owns_(false)
    {
        collection_ = buffer;
        head_ = iterator(collection_, collection_, collection_ + capacity_);
        tail_ = head_;
    }

    /**
     * Copy construct a FixedSizeQueue.
     *
     * This constructor receives another FixedSizeQueue that will be copied into the constructed one.
     * The new FixedSizeQueue will have the same content and capacity of the given argument.
     *
     * The new queue always owns the buffer and will reserve new memory for it.
     *
     * @param other   Queue to copy.
     */
    FixedSizeQueue(
            const FixedSizeQueue& other)
        : FixedSizeQueue(other.capacity(), other.get_allocator())
    {
        for (const_reference item : other)
        {
            push(item);
        }
    }

    virtual ~FixedSizeQueue ()
    {
        while (pop())
        {
        }

        if (owns_)
        {
            allocator_.deallocate(collection_, capacity_);
        }
    }

    /**
     * Initialize a FixedSizeQueue.
     *
     * It receives the number of elements that will fit in the collection.
     * Only queues that have not been initialized before can be initialized.
     *
     * @param capacity  Number of elements to reserve for the queue.
     *
     * \pre The queue is not initialized yet
     */
    void init(
            size_type capacity)
    {
        assert(capacity_ == 0);

        capacity_ = capacity;
        collection_ = allocator_.allocate(capacity_);
        head_ = iterator(collection_, collection_, collection_ + capacity_);
        tail_ = head_;
        owns_ = true;
    }

    /**
     * Initialize a FixedSizeQueue on the given buffer.
     *
     * It receives a buffer already allocated that will be used to back the queue.
     * The queue is not the owner of the buffer, so no deallocation will be done on destruction.
     * It is the responsibility of the caller to deallocate the buffer.
     *
     * \pre The buffer has capacity to hold \c capacity elements
     *
     * @param buffer    Buffer where to construct the queue.
     * @param capacity  Number of elements to reserve for the queue.
     */
    void init(
            pointer buffer,
            size_type capacity)
    {
        assert(capacity_ == 0);

        collection_ = buffer;
        capacity_ = capacity;
        head_ = iterator(collection_, collection_, collection_ + capacity_);
        tail_ = head_;
        owns_ = false;
    }

    /**
     * Copy the data of another FixedSizeQueue.
     *
     * This operator receives another FixedSizeQueue that will be copied into the current one.
     * The new FixedSizeQueue will retain its original capacity, and only the data of the other queue is copied.
     * No memory allocation will occur.
     * If the size of the other queue exceeds the capacity of the current one,
     * only the first capacity() elements are copied.
     *
     * @param other   Queue to copy.
     */
    FixedSizeQueue& operator = (
            const FixedSizeQueue& other)
    {
        if (this != &other)
        {
            // This also resets the pointers
            clear();
            for (const_reference item : other)
            {
                if (!push(item))
                {
                    break;
                }
            }
        }
        return *this;
    }

    /**
     * Enqueue elements at the end.
     * The content of val is copied to the queue.
     *
     * @param val   Value to be copied to the queue.
     *
     * @return true if the value was enqueued, false if queue limit is reached.
     */
    bool push(
            const value_type& val)
    {
        if (full())
        {
            return false;
        }
        allocator_.construct(&(*tail_), val);
        ++tail_;
        ++size_;
        return true;
    }

    /**
     * Enqueue elements and the end.
     * The content of val is moved to the queue.
     *
     * @param val   Value to be moved to the queue.
     *
     * @return true if the value was enqueued, false if queue limit is reached.
     */
    bool push(
            value_type&& val)
    {
        if (full())
        {
            return false;
        }
        return emplace(std::move(val));
    }

    /**
     * Construct and insert element at the end of the queue.
     * This new element is constructed in place using args as the arguments for its constructor.
     *
     * @param args   Arguments forwarded to construct the new element.
     *
     * @return pointer to the new element, nullptr if resource limit is reached.
     */
    template<typename ... Args>
    bool emplace(
            Args&& ... args)
    {
        if (full())
        {
            return false;
        }
        allocator_.construct(&(*tail_), std::forward<Args &&>(args)...);
        ++tail_;
        ++size_;
        return true;
    }

    /**
     * Dequeue elements at the beginning.
     * The first element is removed from the queue.
     *
     * @return true if something was dequeued, false if the queue is empty.
     */
    bool pop()
    {
        if (empty())
        {
            return false;
        }
        allocator_.destroy(&(*head_));
        ++head_;
        --size_;
        return true;
    }

    /**
     * Return the first element at the beginning of the queue.
     *
     * @return A reference to the first element at the beginning of the queue.
     *         Calling \c head on an empty queue is undefined.
     *
     * \pre The queue is not empty
     */
    reference front()
    {
        return *head_;
    }

    /**
     * Return the first element at the beginning of the queue.
     *
     * @return A const reference to the first element at the beginning of the queue.
     *         Calling \c head on an empty queue is undefined.
     *
     * \pre The queue is not empty
     */
    const_reference front() const
    {
        return *head_;
    }

    iterator begin() noexcept
    {
        return head_;
    }

    const_iterator begin() const noexcept
    {
        return head_;
    }

    const_iterator cbegin() const noexcept
    {
        return head_;
    }

    iterator end() noexcept
    {
        return tail_;
    }

    const_iterator end() const noexcept
    {
        return tail_;
    }

    const_iterator cend() const noexcept
    {
        return tail_;
    }

    bool empty() const noexcept
    {
        return size_ == 0;
    }

    bool full() const noexcept
    {
        return capacity_ == size_;
    }

    size_type size() const noexcept
    {
        return size_;
    }

    size_type capacity() const noexcept
    {
        return capacity_;
    }

    size_type max_size() const noexcept
    {
        return capacity();
    }

    void clear()
    {
        head_ = iterator(collection_, collection_, collection_ + capacity_);
        tail_ = head_;
        size_ = 0;
    }

    allocator_type& get_allocator()
    {
        return allocator_;
    }

protected:

    pointer collection_;
    size_type capacity_;
    size_type size_;
    iterator head_;
    iterator tail_;
    allocator_type allocator_;
    bool owns_;

};

}  // namespace fastrtps
}  // namespace eprosima

#endif /* FASTRTPS_UTILS_COLLECTIONS_FIXEDSIZEQUEUE_HPP_ */
