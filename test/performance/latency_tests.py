import shlex, subprocess, time, os, socket, sys

command = os.environ.get("LATENCY_TEST_BIN")
subscriber_proc = subprocess.Popen([command, "subscriber", "--pid", str(os.getpid())])
publisher_proc = subprocess.Popen([command, "publisher", "--pid", str(os.getpid())])

subscriber_proc.communicate()
publisher_proc.communicate()

quit()
