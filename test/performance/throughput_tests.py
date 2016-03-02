import shlex, subprocess, time, os, socket, sys

if len(sys.argv) != 2:
	print("ERROR: Provide a payload size")
	print("usage: python throughput_tests.py PAYLOAD_SIZE")
	quit(-1)

payload_demands = os.environ.get("CMAKE_CURRENT_SOURCE_DIR") + "/payloads_demands_" + sys.argv[1] + ".csv"

command = os.environ.get("THROUGHPUT_TEST_BIN")
subscriber_proc = subprocess.Popen([command, "subscriber"])
publisher_proc = subprocess.Popen([command, "publisher", "--file", payload_demands, "--export_csv"])

subscriber_proc.communicate()
publisher_proc.communicate()

quit()