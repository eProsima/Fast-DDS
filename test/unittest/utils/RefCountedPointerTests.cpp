// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include <utils/RefCountedPointer.hpp>

using namespace std;

namespace eprosima {
namespace fastdds {

struct EntityMock
{
    EntityMock()
        : local_pointer(std::make_shared<RefCountedPointer<EntityMock>>(this))
        , n_times_data_processed(0)
    {
    }

    std::shared_ptr<RefCountedPointer<EntityMock>> get_refcounter_pointer() const
    {
        return local_pointer;
    }

    void dummy_process_data(
            void*)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ++n_times_data_processed;
    }

    void destroy()
    {
        local_pointer->deactivate();
    }

    std::shared_ptr<RefCountedPointer<EntityMock>> local_pointer;
    std::atomic<size_t> n_times_data_processed;
};

enum class RoutineStatus
{
    NON_INITIALIZED,
    SUCCESS,
    FAILURE
};

struct EntityOwner
{
    EntityOwner(
            const EntityMock& entity)
        : entity_ptr(entity.get_refcounter_pointer())
        , routine_status(RoutineStatus::NON_INITIALIZED)
    {
    }

    void spawn_routine()
    {
        th = std::thread([&]()
                        {
                            RefCountedPointer<EntityMock>::Instance entity_instance(entity_ptr);
                            if (entity_instance)
                            {
                                entity_instance->dummy_process_data(nullptr);
                                routine_status = RoutineStatus::SUCCESS;
                            }
                            else
                            {
                                routine_status = RoutineStatus::FAILURE;
                            }
                        });
    }

    void join()
    {
        th.join();
    }

    std::shared_ptr<RefCountedPointer<EntityMock>> entity_ptr;
    RoutineStatus routine_status;
    std::thread th;
};

class RefCountedPointerTests : public ::testing::Test
{
public:

    static constexpr std::size_t n_owners = 5;

    void SetUp() override
    {
        owners_.reserve(5);
        for (std::size_t i = 0; i < n_owners; ++i)
        {
            owners_.emplace_back(entity_);
        }
    }

    void TearDown() override
    {
        for (std::size_t i = 0; i < n_owners; ++i)
        {
            owners_[i].join();
        }
    }

protected:

    EntityMock entity_;
    std::vector<EntityOwner> owners_;
};

TEST_F(RefCountedPointerTests, refcountedpointer_inactive)
{
    // Make the first owner spawn a routine
    owners_[0].spawn_routine();

    // Wait for the routine to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_EQ(owners_[0].routine_status, RoutineStatus::SUCCESS);

    // Destroy the entity
    entity_.destroy();

    // Make the rest of the owners spawn a routine
    for (std::size_t i = 1; i < n_owners; ++i)
    {
        owners_[i].spawn_routine();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // The routine should fail
        ASSERT_EQ(owners_[i].routine_status, RoutineStatus::FAILURE);
    }

    // The entity should have been processed only once
    ASSERT_EQ(1, entity_.n_times_data_processed);
}

TEST_F(RefCountedPointerTests, refcounterpointer_deactivate_waits_for_no_references)
{
    // Spawn some routines
    for (std::size_t i = 0; i < n_owners; ++i)
    {
        owners_[i].spawn_routine();
    }

    // Ensure owners' routines have started
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    auto t0 = std::chrono::steady_clock::now();
    entity_.destroy();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();

    std::cout << "Elapsed time: " << elapsed << " ms" << std::endl;
    ASSERT_GT(elapsed, 50); // destroy should have taken at least 50 ms. Being strict it should be 80, but we allow some margin
    ASSERT_EQ(entity_.n_times_data_processed, 5);
}

} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
