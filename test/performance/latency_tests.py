import shlex, subprocess, time, os, socket, sys

command = os.environ.get("LATENCY_TEST_BIN")

# Best effort
subscriber_proc = subprocess.Popen([command, "subscriber", "--seed", str(os.getpid()), "--hostname"])
publisher_proc = subprocess.Popen([command, "publisher", "--seed", str(os.getpid()), "--hostname", "--export_csv"])

subscriber_proc.communicate()
publisher_proc.communicate()

# Reliable
subscriber_proc = subprocess.Popen([command, "subscriber", "-r", "reliable", "--seed", str(os.getpid()), "--hostname"])
publisher_proc = subprocess.Popen([command, "publisher", "-r", "reliable", "--seed", str(os.getpid()), "--hostname", "--export_csv"])

subscriber_proc.communicate()
publisher_proc.communicate()

quit()
