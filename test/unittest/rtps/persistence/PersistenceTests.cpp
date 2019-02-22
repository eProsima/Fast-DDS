// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "rtps/persistence/PersistenceService.h"
#include <fastrtps/rtps/attributes/PropertyPolicy.h>
#include <fastrtps/rtps/history/CacheChangePool.h>

#include <climits>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps::rtps;

class PersistenceTest : public ::testing::Test
{
protected:
    IPersistenceService * service = nullptr;

    virtual void SetUp()
    {
        std::remove("test.db");
    }

    virtual void TearDown()
    {
        if (service != nullptr)
            delete service;

        std::remove("test.db");
    }
};

/*!
* @fn TEST_F(PersistenceTest, Writer)
* @brief This test checks the writer persistence interface of the persistence service.
*/
TEST_F(PersistenceTest, Writer)
{
    const std::string persist_guid("TEST_WRITER");

    PropertyPolicy policy;
    policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    policy.properties().emplace_back("dds.persistence.sqlite3.filename", "test.db");

    // Get service from factory
    service = PersistenceFactory::create_persistence_service(policy);
    ASSERT_NE(service, nullptr);

    CacheChangePool pool(10, 128, 0, MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE);
    CacheChange_t change;
    GUID_t guid(GuidPrefix_t::unknown(), 1U);
    std::vector<CacheChange_t*> changes;
    change.kind = ALIVE;
    change.writerGUID = guid;
    change.serializedPayload.length = 0;

    // Initial load should return empty vector
    changes.clear();
    ASSERT_TRUE(service->load_writer_from_storage(persist_guid, guid, changes, &pool));
    ASSERT_EQ(changes.size(), 0u);

    // Add two changes
    change.sequenceNumber.low = 1;
    ASSERT_TRUE(service->add_writer_change_to_storage(persist_guid, change));
    change.sequenceNumber.low = 2;
    ASSERT_TRUE(service->add_writer_change_to_storage(persist_guid, change));

    // Should not be able to add same sequence again
    change.sequenceNumber.low = 1;
    ASSERT_FALSE(service->add_writer_change_to_storage(persist_guid, change));
    change.sequenceNumber.low = 2;
    ASSERT_FALSE(service->add_writer_change_to_storage(persist_guid, change));

    // Loading should return two changes (seqs = 1, 2)
    changes.clear();
    ASSERT_TRUE(service->load_writer_from_storage(persist_guid, guid, changes, &pool));
    ASSERT_EQ(changes.size(), 2u);
    uint32_t i = 0;
    for (auto it : changes)
    {
        ++i;
        ASSERT_EQ(it->sequenceNumber, SequenceNumber_t(0, i));
    }

    // Remove seq = 1, and test it can be safely removed twice
    change.sequenceNumber.low = 1;
    ASSERT_TRUE(service->remove_writer_change_from_storage(persist_guid, change));
    ASSERT_TRUE(service->remove_writer_change_from_storage(persist_guid, change));

    // Loading should return one change (seq = 2)
    changes.clear();
    ASSERT_TRUE(service->load_writer_from_storage(persist_guid, guid, changes, &pool));
    ASSERT_EQ(changes.size(), 1u);
    ASSERT_EQ((*changes.begin())->sequenceNumber, SequenceNumber_t(0, 2));

    // Remove seq = 2, and check that load returns empty vector
    changes.clear();
    change.sequenceNumber.low = 2;
    ASSERT_TRUE(service->remove_writer_change_from_storage(persist_guid, change));
    ASSERT_TRUE(service->load_writer_from_storage(persist_guid, guid, changes, &pool));
    ASSERT_EQ(changes.size(), 0u);
}

/*!
* @fn TEST_F(PersistenceTest, Reader)
* @brief This test checks the reader persistence interface of the persistence service.
*/
TEST_F(PersistenceTest, Reader)
{
    const std::string persist_guid("TEST_READER");

    PropertyPolicy policy;
    policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    policy.properties().emplace_back("dds.persistence.sqlite3.filename", "test.db");

    // Get service from factory
    service = PersistenceFactory::create_persistence_service(policy);
    ASSERT_NE(service, nullptr);

    IPersistenceService::map_allocator_t pool(128, 1024);
    foonathan::memory::map<GUID_t, SequenceNumber_t, IPersistenceService::map_allocator_t> seq_map(pool);
    foonathan::memory::map<GUID_t, SequenceNumber_t, IPersistenceService::map_allocator_t> seq_map_loaded(pool);
    GUID_t guid_1(GuidPrefix_t::unknown(), 1U);
    SequenceNumber_t seq_1(0, 1);
    GUID_t guid_2(GuidPrefix_t::unknown(), 2U);
    SequenceNumber_t seq_2(0, 1);

    // Initial load should return empty map
    seq_map_loaded.clear();
    ASSERT_TRUE(service->load_reader_from_storage(persist_guid, seq_map_loaded));
    ASSERT_EQ(seq_map_loaded.size(), 0u);

    // Add two changes
    seq_map[guid_1] = seq_1;
    ASSERT_TRUE(service->update_writer_seq_on_storage(persist_guid, guid_1, seq_1));
    seq_map[guid_2] = seq_2;
    ASSERT_TRUE(service->update_writer_seq_on_storage(persist_guid, guid_2, seq_2));

    // Loading should return local map
    seq_map_loaded.clear();
    ASSERT_TRUE(service->load_reader_from_storage(persist_guid, seq_map_loaded));
    ASSERT_EQ(seq_map_loaded, seq_map);

    // Update previously added changes
    seq_1.low = 100;
    seq_map[guid_1] = seq_1;
    ASSERT_TRUE(service->update_writer_seq_on_storage(persist_guid, guid_1, seq_1));
    seq_2.low = 200;
    seq_map[guid_2] = seq_2;
    ASSERT_TRUE(service->update_writer_seq_on_storage(persist_guid, guid_2, seq_2));

    // Loading should return local map
    seq_map_loaded.clear();
    ASSERT_TRUE(service->load_reader_from_storage(persist_guid, seq_map_loaded));
    ASSERT_EQ(seq_map_loaded, seq_map);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
