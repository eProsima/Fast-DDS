import shlex, subprocess, time, os, socket, sys

subscriber_command = os.environ.get("LATENCY_TEST_BIN") + " subscriber"
args = shlex.split(subscriber_command)
subscriber_proc = subprocess.Popen(args)

publisher_command = os.environ.get("LATENCY_TEST_BIN") + " publisher"
args = shlex.split(publisher_command)
publisher_proc = subprocess.Popen(args)

subscriber_proc.communicate()
publisher_proc.communicate()

quit()