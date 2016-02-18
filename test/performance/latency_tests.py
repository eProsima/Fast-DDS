import shlex, subprocess, time, os, socket, sys

command = os.environ.get("LATENCY_TEST_BIN")
subscriber_proc = subprocess.Popen([command, "subscriber", "512"])
publisher_proc = subprocess.Popen([command, "publisher", "512"])

subscriber_proc.communicate()
publisher_proc.communicate()

quit()