import shlex, subprocess, time, os, socket, sys

payload_demands = os.environ.get("CMAKE_CURRENT_SOURCE_DIR") + "/payloads_demands_16.csv"

command = os.environ.get("THROUGHPUT_TEST_BIN")
subscriber_proc = subprocess.Popen([command, "subscriber"])
publisher_proc = subprocess.Popen([command, "publisher", "--file", payload_demands, "--export_csv"])

subscriber_proc.communicate()
publisher_proc.communicate()

quit()