import shlex, subprocess, time, os, socket, sys

command = os.environ.get("LATENCY_TEST_BIN")
subscriber_proc = subprocess.Popen([command, "subscriber", "--seed", str(os.getpid()), "--hostname"])
publisher_proc = subprocess.Popen([command, "publisher", "--seed", str(os.getpid()), "--hostname", "--export_csv"])

subscriber_proc.communicate()
publisher_proc.communicate()

quit()
