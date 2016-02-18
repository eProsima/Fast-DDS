import shlex, subprocess, time, os, socket, sys

command = os.environ.get("LATENCY_TEST_BIN")
subscriber_proc = subprocess.Popen([command, "subscriber"])
publisher_proc = subprocess.Popen([command, "publisher"])

subscriber_proc.communicate()
publisher_proc.communicate()

quit()