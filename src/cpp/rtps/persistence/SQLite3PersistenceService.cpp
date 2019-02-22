// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SQLite3PersistenceService.cpp
 *
 */

#include "SQLite3PersistenceService.h"
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/history/CacheChangePool.h>

#include "sqlite3.h"

#include <string.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static sqlite3* open_or_create_database(const char* filename)
{
    sqlite3* db = NULL;
    int rc;

    // Open database
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | 
                SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_SHAREDCACHE;
    rc = sqlite3_open_v2(filename, &db, flags, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db);
        return NULL;
    }

    // Create tables if they don't exist
    const char* create_statement = R"(
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
    rc = sqlite3_exec(db,create_statement,0,0,0);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db);
        return NULL;
    }

    return db;
}

static void finalize_statement(sqlite3_stmt*& stmt)
{
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }
}

IPersistenceService* create_SQLite3_persistence_service(const char* filename)
{
    sqlite3* db = open_or_create_database(filename);
    return (db == NULL) ? nullptr : new SQLite3PersistenceService(db);
}

SQLite3PersistenceService::SQLite3PersistenceService(sqlite3* db):
    db_(db),
    load_writer_stmt_(NULL),
    add_writer_change_stmt_(NULL),
    remove_writer_change_stmt_(NULL),
    load_reader_stmt_(NULL),
    update_reader_stmt_(NULL)
{
    // Prepare writer statements
    sqlite3_prepare_v3(db_,"SELECT seq_num,instance,payload FROM writers WHERE guid=?;",-1,SQLITE_PREPARE_PERSISTENT,&load_writer_stmt_,NULL);
    sqlite3_prepare_v3(db_,"INSERT INTO writers VALUES(?,?,?,?);",-1,SQLITE_PREPARE_PERSISTENT,&add_writer_change_stmt_,NULL);
    sqlite3_prepare_v3(db_,"DELETE FROM writers WHERE guid=? AND seq_num=?;",-1,SQLITE_PREPARE_PERSISTENT,&remove_writer_change_stmt_,NULL);

    // Prepare reader statements
    sqlite3_prepare_v3(db_, "SELECT writer_guid_prefix,writer_guid_entity,seq_num FROM readers WHERE guid=?;", -1, SQLITE_PREPARE_PERSISTENT, &load_reader_stmt_, NULL);
    sqlite3_prepare_v3(db_, "INSERT OR REPLACE INTO readers VALUES(?,?,?,?);", -1, SQLITE_PREPARE_PERSISTENT, &update_reader_stmt_, NULL);
}

SQLite3PersistenceService::~SQLite3PersistenceService()
{
    // Finalize writer statements
    finalize_statement(load_writer_stmt_);
    finalize_statement(add_writer_change_stmt_);
    finalize_statement(remove_writer_change_stmt_);

    // Finalize reader statements
    finalize_statement(load_reader_stmt_);
    finalize_statement(update_reader_stmt_);

    sqlite3_close(db_);
    db_ = NULL;
}

/**
* Get all data stored for a writer.
* @param writer_guid GUID of the writer to load.
* @return True if operation was successful.
*/
bool SQLite3PersistenceService::load_writer_from_storage(
        const std::string& persistence_guid,
        const GUID_t& writer_guid,
        std::vector<CacheChange_t*>& changes,
        CacheChangePool* pool)
{
    logInfo(RTPS_PERSISTENCE, "Loading writer " << writer_guid);

    if (load_writer_stmt_ != NULL)
    {
        sqlite3_reset(load_writer_stmt_);
        sqlite3_bind_text(load_writer_stmt_,1,persistence_guid.c_str(),-1,SQLITE_STATIC);

        while (SQLITE_ROW == sqlite3_step(load_writer_stmt_))
        {
            CacheChange_t* change = nullptr;
            int size = sqlite3_column_bytes(load_writer_stmt_, 2);
            if (pool->reserve_Cache(&change, size))
            {
                sqlite3_int64 sn = sqlite3_column_int64(load_writer_stmt_, 0);
                int instance_size = sqlite3_column_bytes(load_writer_stmt_, 1);
                instance_size = (instance_size > 16) ? 16 : instance_size;
                change->kind = ALIVE;
                change->writerGUID = writer_guid;
                memcpy(change->instanceHandle.value, sqlite3_column_blob(load_writer_stmt_, 1), instance_size);
                change->sequenceNumber.high = (int32_t)((sn >> 32) & 0xFFFFFFFF);
                change->sequenceNumber.low = (int32_t)(sn & 0xFFFFFFFF);
                change->serializedPayload.length = size;
                memcpy(change->serializedPayload.data, sqlite3_column_blob(load_writer_stmt_, 2), size);

                changes.insert(changes.begin(), change);
            }
        }
    }

    return true;
}

/**
* Add a change to storage.
* @param change The cache change to add.
* @return True if operation was successful.
*/
bool SQLite3PersistenceService::add_writer_change_to_storage(
        const std::string& persistence_guid,
        const CacheChange_t& change)
{
    logInfo(RTPS_PERSISTENCE, "Writer " << change.writerGUID << " storing change for seq " << change.sequenceNumber);

    if (add_writer_change_stmt_ != NULL)
    {
        sqlite3_reset(add_writer_change_stmt_);
        sqlite3_bind_text(add_writer_change_stmt_, 1, persistence_guid.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(add_writer_change_stmt_, 2, change.sequenceNumber.to64long());
        if (change.instanceHandle.isDefined())
        {
            sqlite3_bind_blob(add_writer_change_stmt_, 3, change.instanceHandle.value, 16, SQLITE_STATIC);
        }
        else
        {
            sqlite3_bind_zeroblob(add_writer_change_stmt_, 3, 16);
        }
        sqlite3_bind_blob(add_writer_change_stmt_, 4, change.serializedPayload.data, change.serializedPayload.length, SQLITE_STATIC);
        return sqlite3_step(add_writer_change_stmt_) == SQLITE_DONE;
    }

    return false;
}

/**
* Remove a change from storage.
* @param change The cache change to remove.
* @return True if operation was successful.
*/
bool SQLite3PersistenceService::remove_writer_change_from_storage(
        const std::string& persistence_guid,
        const CacheChange_t& change)
{
    logInfo(RTPS_PERSISTENCE, "Writer " << change.writerGUID << " removing change for seq " << change.sequenceNumber);

    if (remove_writer_change_stmt_ != NULL)
    {
        sqlite3_reset(remove_writer_change_stmt_);
        sqlite3_bind_text(remove_writer_change_stmt_, 1, persistence_guid.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(remove_writer_change_stmt_, 2, change.sequenceNumber.to64long());
        return sqlite3_step(remove_writer_change_stmt_) == SQLITE_DONE;
    }

    return false;
}

/**
* Get all data stored for a reader.
* @param reader_guid GUID of the reader to load.
* @return True if operation was successful.
*/
bool SQLite3PersistenceService::load_reader_from_storage(
        const std::string& reader_guid,
        foonathan::memory::map<GUID_t, SequenceNumber_t, IPersistenceService::map_allocator_t>& seq_map)
{
    logInfo(RTPS_PERSISTENCE, "Loading reader " << reader_guid);

    if (load_reader_stmt_ != NULL)
    {
        sqlite3_reset(load_reader_stmt_);
        sqlite3_bind_text(load_reader_stmt_, 1, reader_guid.c_str(), -1, SQLITE_STATIC);

        while (SQLITE_ROW == sqlite3_step(load_reader_stmt_))
        {
            GUID_t guid;
            memcpy(guid.guidPrefix.value, sqlite3_column_blob(load_reader_stmt_, 0), GuidPrefix_t::size);
            memcpy(guid.entityId.value, sqlite3_column_blob(load_reader_stmt_, 1), EntityId_t::size);
            sqlite3_int64 sn = sqlite3_column_int64(load_reader_stmt_, 2);
            SequenceNumber_t seq((int32_t)((sn >> 32) & 0xFFFFFFFF), (uint32_t)(sn & 0xFFFFFFFF));
            seq_map[guid] = seq;
        }
    }

    return true;
}

/**
* Update the sequence number associated to a writer on a reader.
* @param reader_guid GUID of the reader to update.
* @param writer_guid GUID of the associated writer to update.
* @param seq_number New sequence number value to set for the associated writer.
* @return True if operation was successful.
*/
bool SQLite3PersistenceService::update_writer_seq_on_storage(
        const std::string& reader_guid,
        const GUID_t& writer_guid,
        const SequenceNumber_t& seq_number) 
{
    logInfo(RTPS_PERSISTENCE, "Reader " << reader_guid << " setting seq for writer " << writer_guid << " to " << seq_number);

    if (update_reader_stmt_ != NULL)
    {
        sqlite3_reset(update_reader_stmt_);
        sqlite3_bind_text(update_reader_stmt_, 1, reader_guid.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_blob(update_reader_stmt_, 2, writer_guid.guidPrefix.value, GuidPrefix_t::size, SQLITE_STATIC);
        sqlite3_bind_blob(update_reader_stmt_, 3, writer_guid.entityId.value, EntityId_t::size, SQLITE_STATIC);
        sqlite3_bind_int64(update_reader_stmt_, 4, seq_number.to64long());
        return sqlite3_step(update_reader_stmt_) == SQLITE_DONE;
    }

    return false;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
