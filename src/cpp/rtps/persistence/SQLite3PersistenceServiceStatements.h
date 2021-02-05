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
 * @file SQLite3PersistenceServiceStatements.h
 */

#ifndef SQLITE3PERSISTENCESERVICESTATEMENTS_H_
#define SQLITE3PERSISTENCESERVICESTATEMENTS_H_

#include <string>
#include <sstream>

namespace eprosima {
namespace fastrtps {
namespace rtps {


class SQLite3PersistenceServiceSchemaV1
{
public:

    static constexpr const char* const writers_table =
            "guid TEXT,"
            "seq_num INTEGER CHECK(seq_num > 0),"
            "instance BLOB CHECK(length(instance)=16),"
            "payload BLOB,"
            "PRIMARY KEY(guid, seq_num DESC)";

    static constexpr const char* const readers_table =
            "guid text,"
            "writer_guid_prefix binary(12),"
            "writer_guid_entity binary(4),"
            "seq_num integer,"
            "PRIMARY KEY(guid, writer_guid_prefix, writer_guid_entity)";

    inline static std::string& writers_table_create_statement()
    {
        static std::string statement =
                std::string("CREATE TABLE IF NOT EXISTS writers(")
                + writers_table
                + ") WITHOUT ROWID;";
        return statement;
    }

    inline static std::string& readers_table_create_statement()
    {
        static std::string statement =
                std::string("CREATE TABLE IF NOT EXISTS readers(")
                + readers_table
                + ") WITHOUT ROWID;";
        return statement;
    }

    inline static std::string& database_create_statement()
    {
        static std::string statement =
                writers_table_create_statement()
                + readers_table_create_statement();
        return statement;
    }

};

class SQLite3PersistenceServiceSchemaV2
{
public:

    static constexpr const char* const writer_histories_table =
            SQLite3PersistenceServiceSchemaV1::writers_table;

    static constexpr const char* const writer_states_table =
            "guid TEXT PRIMARY KEY,"
            "last_seq_num INTEGER CHECK(last_seq_num > 0)";

    static constexpr const char* const readers_table =
            SQLite3PersistenceServiceSchemaV1::readers_table;

    inline static std::string& writers_histories_table_create_statement()
    {
        static std::string statement =
                std::string("CREATE TABLE IF NOT EXISTS writers_histories(")
                + writer_histories_table
                + ") WITHOUT ROWID;";
        return statement;
    }

    inline static std::string& writers_states_table_create_statement()
    {
        static std::string statement =
                std::string("CREATE TABLE IF NOT EXISTS writers_states(")
                + writer_states_table
                + ") WITHOUT ROWID;";
        return statement;
    }

    inline static std::string& readers_table_create_statement()
    {
        static std::string statement =
                std::string("CREATE TABLE IF NOT EXISTS readers(")
                + readers_table
                + ") WITHOUT ROWID;";
        return statement;
    }

    inline static std::string& database_create_statement()
    {
        static std::string statement =
                std::string("PRAGMA user_version = 2;")
                + "PRAGMA foreign_keys = ON;"
                + writers_histories_table_create_statement()
                + writers_states_table_create_statement()
                + readers_table_create_statement();
        return statement;
    }

    inline static std::string& update_from_v1_statement()
    {
        static std::string statement =
                std::string("PRAGMA foreign_keys = ON;")
                + writers_states_table_create_statement()
                // Populate the writers_states with information available in the old writers table. Note the assumption that
                // last_seq_num would be the largest seq_num associated to a history cache is not always true
                + "INSERT INTO writers_states SELECT guid, MAX(seq_num) FROM writers GROUP BY guid;"
                + writers_histories_table_create_statement()
                // Copy the contents of writers to the new table
                + "INSERT INTO writers_histories SELECT guid, seq_num, instance, payload FROM writers;"
                // Remove the old table, since it is no longer required
                + "DROP TABLE writers;"
                // Once the upgrade has succeded add the version number
                + "PRAGMA user_version = 2;";
        return statement;
    }

};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* SQLITE3PERSISTENCESERVICESTATEMENTS_H_ */
