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

#ifndef _FASTDDS_SHAREDMEM_SEGMENT_H_
#define _FASTDDS_SHAREDMEM_SEGMENT_H_

#include <boostconfig.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/thread/thread_time.hpp>

#include "SharedMemUUID.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

/**
 * Provides shared memory functionallity abstrating from
 * lower level layers
 */
class SharedMemSegment
{
public:

    typedef std::ptrdiff_t offset;
    typedef boost::interprocess::interprocess_condition condition_variable;
    typedef boost::interprocess::interprocess_mutex mutex;
    typedef boost::interprocess::named_mutex named_mutex;

    static constexpr boost::interprocess::open_only_t open_only = boost::interprocess::open_only_t();
    static constexpr boost::interprocess::create_only_t create_only = boost::interprocess::create_only_t();
    static constexpr boost::interprocess::open_or_create_t open_or_create = boost::interprocess::open_or_create_t();

    // Boost memory manager needs extra memory to maintain its structures,
    // as these structures are shared they are stored in the segment.
    // TODO(Adolfo): Further analysis to determine the perfect value for this extra segment size
    static constexpr uint32_t EXTRA_SEGMENT_SIZE = 512;

    SharedMemSegment(
            boost::interprocess::create_only_t,
            const std::string& name,
            size_t size)
        : segment_(boost::interprocess::create_only, name.c_str(), size + EXTRA_SEGMENT_SIZE)
        , name_(name)
    {
    }

    SharedMemSegment(
            boost::interprocess::open_only_t,
            const std::string& name)
        : segment_(boost::interprocess::open_only, name.c_str())
        , name_(name)
    {
    }

    SharedMemSegment(
            boost::interprocess::open_or_create_t,
            const std::string& name,
            size_t size)
        : segment_(boost::interprocess::create_only, name.c_str(), size)
        , name_(name)
    {
    }

    void* get_address_from_offset(
            SharedMemSegment::offset offset) const
    {
        return segment_.get_address_from_handle(offset);
    }

    SharedMemSegment::offset get_offset_from_address(
            void* address) const
    {
        return segment_.get_handle_from_address(address);
    }

    boost::interprocess::managed_shared_memory& get() { return segment_;}

    static void remove(
            const std::string& name)
    {
        boost::interprocess::shared_memory_object::remove(name.c_str());
    }

    std::string name()
    {
        return name_;
    }

    /**
     * Estimates the extra segment' space required for an allocation
     */
    static uint32_t compute_per_allocation_extra_size(size_t allocation_alignment)
    {
        Id uuid;
        uuid.generate();
            
        try
        {
            static uint32_t extra_size = 0;
                    
            if (extra_size == 0)
            {
                {
                    boost::interprocess::managed_shared_memory
                        test_segment(boost::interprocess::create_only, uuid.to_string().c_str(),
                        (std::max)((size_t)1024, allocation_alignment * 4));

                    auto m1 = test_segment.get_free_memory();
                    test_segment.allocate_aligned(1, allocation_alignment);
                    auto m2 = test_segment.get_free_memory();
                    extra_size = static_cast<uint32_t>(m1 - m2);
                }

                boost::interprocess::shared_memory_object::remove(uuid.to_string().c_str());
            }

            return extra_size;

        }
        catch(const std::exception& e)
        {
            logError(RTPS_TRANSPORT_SHM, "Failed to create segment " << uuid.to_string()
                        << ": " << e.what());

            throw;
        }
    }

    static std::unique_ptr<SharedMemSegment::named_mutex> open_or_create_and_lock_named_mutex(
        const std::string& mutex_name)
    {
        std::unique_ptr<SharedMemSegment::named_mutex> named_mutex;

        // Todo(Adolfo) : Dataraces could occur, this algorithm has to be improved

        named_mutex = std::unique_ptr<SharedMemSegment::named_mutex>(
            new SharedMemSegment::named_mutex(boost::interprocess::open_or_create, mutex_name.c_str()));

        boost::posix_time::ptime wait_time
            = boost::posix_time::microsec_clock::universal_time()
            + boost::posix_time::milliseconds(BOOST_INTERPROCESS_TIMEOUT_WHEN_LOCKING_DURATION_MS*2);
        if (!named_mutex->timed_lock(wait_time))
        {
            // Interprocess mutex timeout when locking. Possible deadlock: owner died without unlocking?
            // try to remove and create again
            SharedMemSegment::named_mutex::remove(mutex_name.c_str());

            named_mutex = std::unique_ptr<SharedMemSegment::named_mutex>(
            new SharedMemSegment::named_mutex(boost::interprocess::open_or_create, mutex_name.c_str()));

            if (!named_mutex->try_lock())
                throw std::runtime_error("Couldn't create name_mutex: " + mutex_name);
        }
        
        return named_mutex;
    }

    static std::unique_ptr<SharedMemSegment::named_mutex> open_named_mutex(
        const std::string& mutex_name)
    {
        std::unique_ptr<SharedMemSegment::named_mutex> named_mutex;

        // Todo(Adolfo) : Dataraces could occur, this algorithm has to be improved

        named_mutex = std::unique_ptr<SharedMemSegment::named_mutex>(
            new SharedMemSegment::named_mutex(boost::interprocess::open_only, mutex_name.c_str()));
        
        return named_mutex;
    }

    /**
     * Unique ID of the memory segment
     */
    class Id
    {
    public:

        typedef UUID<16> type;

        Id()
        {
        }

		Id(const Id& other)
		{
			uuid_ = other.uuid_;
		}

        Id(const type& uuid)
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

        bool operator == (const Id& other) const
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

    boost::interprocess::managed_shared_memory segment_;

    std::string name_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_SEGMENT_H_

