#!/usr/bin/python3

import sys, os

user = ""
password = ""

print("LAUNCH SUBSCRIBER TESTS")
if len(sys.argv) <= 2:
	print("Error")
	sys.exit(-1)
else:
	user = sys.argv[1]
	password = sys.argv[2]


os.system("mkdir -p /mnt/jenkins")
os.system("mount -t cifs -o username=" + user + ",password=" + password + " //mainserver.intranet.eprosima.com/Public/JenkinsTests /mnt/jenkins")
os.system("touch /mnt/jenkins/sub.log")
os.system("python3 '/workspace/Multi-Node Manual Linux/test/GenerateTestsAndXMLs.py'")
os.system("python3 '/workspace/Multi-Node Manual Linux/test/SubscriberTestList.py' /mnt/jenkins/")