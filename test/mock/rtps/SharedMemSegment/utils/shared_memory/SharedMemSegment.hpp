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

#ifndef FASTDDS_UTILS_SHARED_MEMORY__SHAREDMEMSEGMENT_HPP
#define FASTDDS_UTILS_SHARED_MEMORY__SHAREDMEMSEGMENT_HPP

#include <boostconfig.hpp>

// For gcc-9 disable a new warning that warns about copy operator implicitly
// defined based on copy constructor.
#if defined(__GNUC__) && ( __GNUC__ >= 9)
#pragma GCC diagnostic error "-Wdeprecated-copy"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif // if defined(__GNUC__) && ( __GNUC__ >= 9)

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>

#if defined (__GNUC__) && ( __GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif // if defined (__GNUC__) && ( __GNUC__ >= 9)

#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/interprocess_sharable_mutex.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/spin/wait.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/thread/thread_time.hpp>

#include "../../../../../../src/cpp/utils/shared_memory/BoostAtExitRegistry.hpp"
#include "../../../../../../src/cpp/utils/shared_memory/RobustInterprocessCondition.hpp"
#include "../../../../../../src/cpp/utils/shared_memory/SharedMemUUID.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

template<typename T>
using deleted_unique_ptr = std::unique_ptr<T, std::function<void (T*)>>;

/**
 * Provides shared memory functionallity abstrating from
 * lower level layers
 */
class SharedSegmentBase
{
public:

    template <class M>
    using sharable_lock = boost::interprocess::sharable_lock<M>;
    using sharable_mutex = boost::interprocess::interprocess_sharable_mutex;

    using condition_variable = RobustInterprocessCondition;
    using mutex = boost::interprocess::interprocess_mutex;
    using named_mutex = boost::interprocess::named_mutex;
    using spin_wait = boost::interprocess::spin_wait;

    // Offset must be the same size for 32/64-bit versions, so no size_t used here.
    using Offset = std::uint32_t;

    static constexpr boost::interprocess::open_only_t open_only = boost::interprocess::open_only_t();
    static constexpr boost::interprocess::create_only_t create_only = boost::interprocess::create_only_t();
    static constexpr boost::interprocess::open_or_create_t open_or_create = boost::interprocess::open_or_create_t();

    // Boost memory manager needs extra memory to maintain its structures,
    // as these structures are shared they are stored in the segment.
    // TODO(Adolfo): Further analysis to determine the perfect value for this extra segment size
    static constexpr uint32_t EXTRA_SEGMENT_SIZE = 512;

    explicit SharedSegmentBase(
            const std::string& name)
        : name_(name)
    {
    }

    virtual ~SharedSegmentBase()
    {
    }

    virtual void remove() = 0;

    std::string name()
    {
        return name_;
    }

    virtual void* get_address_from_offset(
            SharedSegmentBase::Offset offset) const = 0;

    virtual SharedSegmentBase::Offset get_offset_from_address(
            void* address) const = 0;

    static deleted_unique_ptr<SharedSegmentBase::named_mutex> open_or_create_and_lock_named_mutex(
            const std::string& mutex_name)
    {
        deleted_unique_ptr<SharedSegmentBase::named_mutex> named_mutex;

        {
            std::lock_guard<std::mutex> lock(mtx_());

            named_mutex = deleted_unique_ptr<SharedSegmentBase::named_mutex>(
                new SharedSegmentBase::named_mutex(boost::interprocess::open_or_create, mutex_name.c_str()),
                [](SharedSegmentBase::named_mutex* p)
                {
                    std::lock_guard<std::mutex> lock(mtx_());
                    delete p;
                });
        }

        boost::posix_time::ptime wait_time
            = boost::posix_time::microsec_clock::universal_time()
                + boost::posix_time::milliseconds(BOOST_INTERPROCESS_TIMEOUT_WHEN_LOCKING_DURATION_MS * 2);
        if (!named_mutex->timed_lock(wait_time))
        {
            // Interprocess mutex timeout when locking. Possible deadlock: owner died without unlocking?
            // try to remove and create again
            SharedSegmentBase::named_mutex::remove(mutex_name.c_str());
            named_mutex.reset();
            {
                std::lock_guard<std::mutex> lock(mtx_());

                named_mutex = deleted_unique_ptr<SharedSegmentBase::named_mutex>(
                    new SharedSegmentBase::named_mutex(boost::interprocess::open_or_create, mutex_name.c_str()),
                    [](SharedSegmentBase::named_mutex* p)
                    {
                        std::lock_guard<std::mutex> lock(mtx_());
                        delete p;
                    });
            }

            if (!named_mutex->try_lock())
            {
                throw std::runtime_error("Couldn't create name_mutex: " + mutex_name);
            }
        }

        return named_mutex;
    }

    static deleted_unique_ptr<SharedSegmentBase::named_mutex> try_open_and_lock_named_mutex(
            const std::string& mutex_name)
    {
        deleted_unique_ptr<SharedSegmentBase::named_mutex> named_mutex;

        {
            std::lock_guard<std::mutex> lock(mtx_());

            named_mutex = deleted_unique_ptr<SharedSegmentBase::named_mutex>(
                new SharedSegmentBase::named_mutex(boost::interprocess::open_only, mutex_name.c_str()),
                [](SharedSegmentBase::named_mutex* p)
                {
                    std::lock_guard<std::mutex> lock(mtx_());
                    delete p;
                });
        }

        boost::posix_time::ptime wait_time
            = boost::posix_time::microsec_clock::universal_time()
                + boost::posix_time::milliseconds(BOOST_INTERPROCESS_TIMEOUT_WHEN_LOCKING_DURATION_MS * 2);
        if (!named_mutex->timed_lock(wait_time))
        {
            throw std::runtime_error("Couldn't lock name_mutex: " + mutex_name);
        }

        return named_mutex;
    }

    static deleted_unique_ptr<SharedSegmentBase::named_mutex> open_or_create_named_mutex(
            const std::string& mutex_name)
    {
        deleted_unique_ptr<SharedSegmentBase::named_mutex> named_mutex;

        // Todo(Adolfo) : Dataraces could occur, this algorithm has to be improved

        {
            std::lock_guard<std::mutex> lock(mtx_());

            named_mutex = deleted_unique_ptr<SharedSegmentBase::named_mutex>(
                new SharedSegmentBase::named_mutex(boost::interprocess::open_or_create, mutex_name.c_str()),
                [](SharedSegmentBase::named_mutex* p)
                {
                    std::lock_guard<std::mutex> lock(mtx_());
                    delete p;
                });
        }

        return named_mutex;
    }

    static deleted_unique_ptr<SharedSegmentBase::named_mutex> open_named_mutex(
            const std::string& mutex_name)
    {
        deleted_unique_ptr<SharedSegmentBase::named_mutex> named_mutex;

        // Todo(Adolfo) : Dataraces could occur, this algorithm has to be improved

        {
            std::lock_guard<std::mutex> lock(mtx_());

            named_mutex = deleted_unique_ptr<SharedSegmentBase::named_mutex>(
                new SharedSegmentBase::named_mutex(boost::interprocess::open_only, mutex_name.c_str()),
                [](SharedSegmentBase::named_mutex* p)
                {
                    std::lock_guard<std::mutex> lock(mtx_());
                    delete p;
                });
        }

        return named_mutex;
    }

    /**
     * Unique ID of the segment
     */
    class Id
    {
    public:

        typedef UUID<8> type;

        Id()
            : uuid_(type::null_t{})
        {
        }

        Id(
                const Id& other)
        {
            uuid_ = other.uuid_;
        }

        Id(
                const type& uuid)
        {
            uuid_ = uuid;
        }

        void generate()
        {
            type::generate(uuid_);
        }

        const type& get() const
        {
            return uuid_;
        }

        Id& operator = (
                const Id& other)
        {
            uuid_ = other.uuid_;
            return *this;
        }

        bool operator == (
                const Id& other) const
        {
            return uuid_ == other.uuid_;
        }

        std::string to_string()
        {
            return uuid_.to_string();
        }

        static const Id null()
        {
            return Id(type(type::null_t()));
        }

    private:

        type uuid_;

    }; // Id

private:

    class EnvironmentInitializer
    {
    public:

        EnvironmentInitializer()
        {
            SharedMemEnvironment::get().init();
        }

    }
    shared_mem_environment_initializer_;

    std::string name_;

    static std::mutex& mtx_()
    {
        static std::mutex mtx_;
        return mtx_;
    }

};

template<typename T, typename U>
class SharedSegment : public SharedSegmentBase
{
public:

    typedef T managed_shared_memory_type;
    typedef U managed_shared_object_type;

    SharedSegment(
            boost::interprocess::create_only_t,
            const std::string& name,
            size_t size)
        : SharedSegmentBase(name)
    {
        segment_ = std::unique_ptr<managed_shared_memory_type>(
            new managed_shared_memory_type(boost::interprocess::create_only, name.c_str(),
            static_cast<Offset>(size + EXTRA_SEGMENT_SIZE)));
    }

    SharedSegment(
            boost::interprocess::open_only_t,
            const std::string& name)
        : SharedSegmentBase(name)
    {
        segment_ = std::unique_ptr<managed_shared_memory_type>(
            new managed_shared_memory_type(boost::interprocess::open_only, name.c_str()));
    }

    SharedSegment(
            boost::interprocess::open_read_only_t,
            const std::string& name)
        : SharedSegmentBase(name)
    {
        segment_ = std::unique_ptr<managed_shared_memory_type>(
            new managed_shared_memory_type(boost::interprocess::open_read_only, name.c_str()));
    }

    SharedSegment(
            boost::interprocess::open_or_create_t,
            const std::string& name,
            size_t size)
        : SharedSegmentBase(name)
    {
        segment_ = std::unique_ptr<managed_shared_memory_type>(
            new managed_shared_memory_type(boost::interprocess::create_only, name.c_str(), static_cast<Offset>(size)));
    }

    ~SharedSegment()
    {
        // no need of exception handling cause never throws
        segment_.reset();
    }

    static void remove(
            const std::string& name)
    {
        managed_shared_object_type::remove(name.c_str());
    }

    void remove() override
    {
        managed_shared_object_type::remove(name().c_str());
    }

    void* get_address_from_offset(
            SharedSegment::Offset offset) const override
    {
        return segment_->get_address_from_handle(offset);
    }

    SharedSegment::Offset get_offset_from_address(
            void* address) const override
    {
        return segment_->get_handle_from_address(address);
    }

    managed_shared_memory_type& get()
    {
        return *segment_;
    }

    /**
     * Estimates the extra segment space required for an allocation
     */
    static uint32_t compute_per_allocation_extra_size(
            size_t allocation_alignment,
            const std::string& domain_name)
    {
        Id uuid;

        try
        {
            static uint32_t extra_size = 0;

            if (extra_size == 0)
            {
                uuid.generate();

                // Additional invalid path characters to trigger the exception
                auto name = "///" + domain_name + "_" + uuid.to_string();

                SharedMemEnvironment::get().init();

                {
                    managed_shared_memory_type
                            test_segment(boost::interprocess::create_only, name.c_str(),
                            (std::max)((uint32_t)1024, static_cast<uint32_t>(allocation_alignment * 4)));

                    auto m1 = test_segment.get_free_memory();
                    test_segment.allocate_aligned(1, static_cast<uint32_t>(allocation_alignment));
                    auto m2 = test_segment.get_free_memory();
                    extra_size = static_cast<uint32_t>(m1 - m2);
                }

                managed_shared_object_type::remove(name.c_str());
            }

            return extra_size;

        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(RTPS_TRANSPORT_SHM, "Failed to create segment " << uuid.to_string()
                                                                               << ": " << e.what());

            throw;
        }
    }

    /**
     * Check the allocator internal structures
     * @return true if structures are ok, false otherwise
     */
    bool check_sanity()
    {
        return segment_->check_sanity();
    }

    /**
     * @return The segment's size in bytes, including internal structures overhead.
     */
    Offset mem_size() const
    {
        return segment_->get_size();
    }

private:

    std::unique_ptr<managed_shared_memory_type> segment_;
};

using SharedMemSegment = SharedSegment<
    boost::interprocess::basic_managed_shared_memory<
        char,
        boost::interprocess::rbtree_best_fit<boost::interprocess::mutex_family,
        boost::interprocess::offset_ptr<void, SharedSegmentBase::Offset, std::uint64_t>>,
        boost::interprocess::iset_index>,
    boost::interprocess::shared_memory_object>;

using SharedFileSegment = SharedSegment<
    boost::interprocess::basic_managed_mapped_file<
        char,
        boost::interprocess::rbtree_best_fit<boost::interprocess::mutex_family,
        boost::interprocess::offset_ptr<void, SharedSegmentBase::Offset, std::uint64_t>>,
        boost::interprocess::iset_index>,
    boost::interprocess::file_mapping>;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_UTILS_SHARED_MEMORY__SHAREDMEMSEGMENT_HPP
