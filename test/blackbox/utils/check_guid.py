# Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sqlite3
import sys

"""
Return the number of apparitions of a guid, on a specific table and database.

It takes three arguments:
    * database_name: The filename of the database
    * table_name: The name of the table within the database
    * check_guid: The guid of the entity that you want to look for in the
      stated table and database.

"""

if __name__ == '__main__':
    if len(sys.argv) != 4:
        sys.exit(255)
    database_name = str(sys.argv[1])
    table_name = str(sys.argv[2])
    check_guid = str(sys.argv[3])

    command = f"SELECT guid FROM {table_name} WHERE guid='{check_guid}'"
    try:
        connection = sqlite3.connect(database_name)
        c = connection.cursor()
        c.execute(command)
        num = len(c.fetchall())
        connection.close()
    except sqlite3.DatabaseError as e:
        sys.exit(255)

    sys.exit(num)
