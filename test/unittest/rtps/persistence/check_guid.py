import sqlite3
import sys

if len(sys.argv) != 4:
    sys.exit("Not enough args")
database_name = str(sys.argv[1])
table_name = str(sys.argv[2])
check_guid = str(sys.argv[3])
command = "SELECT guid FROM "+table_name+" WHERE guid='"+check_guid+"'"

connection = sqlite3.connect(database_name)

c = connection.cursor()

c.execute(command)

num = len(c.fetchall())

connection.close()

sys.exit(num)