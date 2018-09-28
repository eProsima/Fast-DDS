#!/usr/bin/python3

import sys, os

user = ""
password = ""

print("LAUNCH PUBLISHER TESTS")
if len(sys.argv) <= 2:
	print("Error")
	sys.exit(-1)
else:
	user = sys.argv[1]
	password = sys.argv[2]


os.system("mkdir -p /mnt/jenkins")
os.system("mount -t cifs -o username=" + user + ",password=" + password + " //mainserver.intranet.eprosima.com/Public/JenkinsTests /mnt/jenkins")
os.system("touch /mnt/jenkins/pub.log")
os.system("GenerateTestsAndXMLs.py")
os.system("PublisherTestList.py /mnt/jenkins/")