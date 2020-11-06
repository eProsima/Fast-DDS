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
#include <rtps/persistence/sqlite3.h>

#include <climits>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps::rtps;

class PersistenceTest : public ::testing::Test
{
protected:
    IPersistenceService * service = nullptr;

    virtual void SetUp()
    {
        std::remove(dbfile);
    }

    virtual void TearDown()
    {
        if (service != nullptr)
            delete service;

        std::remove(dbfile);
    }

    void create_database(sqlite3** db, int version)
    {
        const char* create_statement;
        switch (version)
        {
            case 0:
            case 1:
                create_statement = create_statement_v1;
                break;
            case 2:
                create_statement = create_statement_v2;
                break;
            default:
                FAIL() << "unsuppoerted database version " << version;
        }

        int rc;
        int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_SHAREDCACHE;

        rc = sqlite3_open_v2(dbfile, db, flags, 0);
        if (rc != SQLITE_OK)
        {
             FAIL() << sqlite3_errmsg(*db);
        }

        rc = sqlite3_exec(*db,create_statement,0,0,0);
        if (rc != SQLITE_OK)
        {
             FAIL() << sqlite3_errmsg(*db);
        }
    }

    void add_writer_data_v1(sqlite3* db, const char* persist_guid, int seq_num)
    {
        sqlite3_stmt* add_data_statement;
        sqlite3_prepare_v2(db,"INSERT INTO writers VALUES(?,?,?,?);",-1,&add_data_statement,NULL);

        sqlite3_bind_text(add_data_statement, 1, persist_guid, -1, SQLITE_STATIC);
        sqlite3_bind_int64(add_data_statement, 2, seq_num);
        sqlite3_bind_zeroblob(add_data_statement, 3, 16);
        sqlite3_bind_zeroblob(add_data_statement, 4, 128);

        if (sqlite3_step(add_data_statement) != SQLITE_DONE)
        {
            FAIL() << sqlite3_errmsg(db);
        }
        sqlite3_finalize(add_data_statement);
    }

    void add_writer_data_v2(sqlite3* db, const char* persist_guid, int seq_num)
    {
        sqlite3_stmt* add_last_seq_statement;
        sqlite3_prepare_v2(db,"INSERT INTO writers_states VALUES(?,?);",-1,&add_last_seq_statement,NULL);

        sqlite3_bind_text(add_last_seq_statement, 1, persist_guid, -1, SQLITE_STATIC);
        sqlite3_bind_int64(add_last_seq_statement, 2, seq_num);

        if (sqlite3_step(add_last_seq_statement) != SQLITE_DONE)
        {
            FAIL() << sqlite3_errmsg(db);
        }
        sqlite3_finalize(add_last_seq_statement);

        sqlite3_stmt* add_data_statement;
        sqlite3_prepare_v2(db,"INSERT INTO writers_histories VALUES(?,?,?,?);",-1,&add_data_statement,NULL);

        sqlite3_bind_text(add_data_statement, 1, persist_guid, -1, SQLITE_STATIC);
        sqlite3_bind_int64(add_data_statement, 2, seq_num);
        sqlite3_bind_zeroblob(add_data_statement, 3, 16);
        sqlite3_bind_zeroblob(add_data_statement, 4, 128);

        if (sqlite3_step(add_data_statement) != SQLITE_DONE)
        {
            FAIL() << sqlite3_errmsg(db);
        }
        sqlite3_finalize(add_data_statement);
    }

    void add_writer_data(sqlite3* db, int version, const char* persist_guid, int seq_num)
    {
        switch (version)
        {
            case 0:
            case 1:
                add_writer_data_v1(db, persist_guid, seq_num);
                break;
            case 2:
                add_writer_data_v2(db, persist_guid, seq_num);
                break;
            default:
                FAIL() << "unsuppoerted database version " << version;
        }
    }

    const char* dbfile = "text.db";
    const char* create_statement_v1 = R"(
CREATE TABLE IF NOT EXISTS writers(
    guid text,
    seq_num integer,
    instance binary(16),
    payload blob,
    PRIMARY KEY(guid, seq_num DESC)
) WITHOUT ROWID;

CREATE TABLE IF NOT EXISTS readers(
    guid text,
    writer_guid_prefix binary(12),
    writer_guid_entity binary(4),
    seq_num integer,
    PRIMARY KEY(guid, writer_guid_prefix, writer_guid_entity)
) WITHOUT ROWID;
)";
    const char* create_statement_v2 = R"(
PRAGMA user_version = 2;
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS writers_states(
    guid TEXT PRIMARY KEY, -- CHECK(guid REGEXP '([0-9a-fA-F]{1,2}\.){11}[0-9a-fA-F]{1,2}\|([0-9a-fA-F]{1,2}\.){3}[0-9a-fA-F]{1,2}'),
    last_seq_num INTEGER CHECK(last_seq_num > 0)
) WITHOUT ROWID;

CREATE TABLE IF NOT EXISTS writers_histories(
    guid TEXT, -- CHECK(guid REGEXP '([0-9a-fA-F]{1,2}\.){11}[0-9a-fA-F]{1,2}\|([0-9a-fA-F]{1,2}\.){3}[0-9a-fA-F]{1,2}'),
    seq_num INTEGER CHECK(seq_num > 0),
    instance BLOB CHECK(length(instance)=16),
    payload BLOB,
    PRIMARY KEY(guid, seq_num DESC), 
    FOREIGN KEY (guid)
        REFERENCES writers_states(guid)
) WITHOUT ROWID;

CREATE TABLE IF NOT EXISTS readers(
    guid text,
    writer_guid_prefix binary(12),
    writer_guid_entity binary(4),
    seq_num integer,
    PRIMARY KEY(guid, writer_guid_prefix, writer_guid_entity)
) WITHOUT ROWID;
)";

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
    policy.properties().emplace_back("dds.persistence.sqlite3.filename", dbfile);

    // Get service from factory
    service = PersistenceFactory::create_persistence_service(policy);
    ASSERT_NE(service, nullptr);

    CacheChangePool pool(10, 128, 0, MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE);
    CacheChange_t change;
    GUID_t guid(GuidPrefix_t::unknown(), 1U);
    std::vector<CacheChange_t*> changes;
    SequenceNumber_t last_seq_number;
    change.kind = ALIVE;
    change.writerGUID = guid;
    change.serializedPayload.length = 0;

    // Initial load should return empty vector
    changes.clear();
    ASSERT_TRUE(service->load_writer_from_storage(persist_guid, guid, changes, &pool, &last_seq_number));
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
    ASSERT_TRUE(service->load_writer_from_storage(persist_guid, guid, changes, &pool, &last_seq_number));
    ASSERT_EQ(changes.size(), 2u);
    ASSERT_EQ(last_seq_number, SequenceNumber_t(0, 2u));
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
    ASSERT_TRUE(service->load_writer_from_storage(persist_guid, guid, changes, &pool, &last_seq_number));
    ASSERT_EQ(changes.size(), 1u);
    ASSERT_EQ((*changes.begin())->sequenceNumber, SequenceNumber_t(0, 2));
    ASSERT_EQ(last_seq_number, SequenceNumber_t(0, 2u));

    // Remove seq = 2, and check that load returns empty vector
    changes.clear();
    change.sequenceNumber.low = 2;
    ASSERT_TRUE(service->remove_writer_change_from_storage(persist_guid, change));
    ASSERT_TRUE(service->load_writer_from_storage(persist_guid, guid, changes, &pool, &last_seq_number));
    ASSERT_EQ(changes.size(), 0u);
    ASSERT_EQ(last_seq_number, SequenceNumber_t(0, 2u));
}


/*!
* @fn TEST_F(PersistenceTest, SchemaVersionMismatch)
* @brief This test checks that an error is issued if the database has an old schema.
*/
TEST_F(PersistenceTest, SchemaVersionMismatch)
{
    const char* persist_guid = "TEST_WRITER";
    sqlite3* db = nullptr;
    create_database(&db, 1);
    add_writer_data(db, 1, persist_guid, 1);
    add_writer_data(db, 1, persist_guid, 2);
    sqlite3_close(db);

    PropertyPolicy policy;
    policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    policy.properties().emplace_back("dds.persistence.sqlite3.filename", dbfile);

    // Loading an old schema version should fail
    service = PersistenceFactory::create_persistence_service(policy);
    ASSERT_EQ(service, nullptr);
}

/*!
* @fn TEST_F(PersistenceTest, SchemaVersionUpdateFrom1To2)
* @brief This test checks that the database is updated correctly.
*/
TEST_F(PersistenceTest, SchemaVersionUpdateFrom1To2)
{
    const char* persist_guid = "TEST_WRITER";
    sqlite3* db = nullptr;
    create_database(&db, 1);
    add_writer_data(db, 1, persist_guid, 1);
    add_writer_data(db, 1, persist_guid, 2);
    sqlite3_close(db);

    PropertyPolicy policy;
    policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    policy.properties().emplace_back("dds.persistence.sqlite3.filename", dbfile);
    policy.properties().emplace_back("dds.persistence.update_schema", "true");

    // Get service from factory
    service = PersistenceFactory::create_persistence_service(policy);
    ASSERT_NE(service, nullptr);

    CacheChangePool pool(10, 128, 0, MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE);
    CacheChange_t change;
    GUID_t guid(GuidPrefix_t::unknown(), 1U);
    std::vector<CacheChange_t*> changes;
    SequenceNumber_t last_seq_number;
    change.kind = ALIVE;
    change.writerGUID = guid;
    change.serializedPayload.length = 0;

    // Load data
    changes.clear();
    ASSERT_TRUE(service->load_writer_from_storage(persist_guid, guid, changes, &pool, &last_seq_number));
    ASSERT_EQ(changes.size(), 2u);
    ASSERT_EQ(last_seq_number, SequenceNumber_t(0, 2u));
    uint32_t i = 0;
    for (auto it : changes)
    {
        ++i;
        ASSERT_EQ(it->sequenceNumber, SequenceNumber_t(0, i));
    }
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
    policy.properties().emplace_back("dds.persistence.sqlite3.filename", dbfile);

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
