import shlex, subprocess, time, os, socket, sys

payload_demands = os.environ.get("CMAKE_CURRENT_SOURCE_DIR") + "/payloads_demands.csv"

command = os.environ.get("THROUGHPUT_TEST_BIN")
subscriber_proc = subprocess.Popen([command, "subscriber", "--seed", str(os.getpid()), "--hostname"])
publisher_proc = subprocess.Popen([command, "publisher", "--seed", str(os.getpid()), "--hostname", "--file", payload_demands])

subscriber_proc.communicate()
publisher_proc.communicate()

quit()
