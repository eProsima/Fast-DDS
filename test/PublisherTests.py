#!/usr/bin/python3

import sys, os

user = ""
password = ""

def writeTest(filename, text):
	with open(filename, "a") as myfile:
		myfile.write(text + "\n")

if len(sys.argv) <= 2:
	print("Error")
	sys.exit(-1)
else:
	user = sys.argv[1]
	password = sys.argv[2]


os.system("mkdir -p /mnt/jenkins")
os.system("mount -t cifs -o username=" + user + ",password=" + password + " //mainserver.intranet.eprosima.com/Public/JenkinsTests /mnt/jenkins")
writeTest("/mnt/jenkins/pub.log", "Start Tests")
os.system("cd '/workspace/Multi-Node Manual Linux/test' && python3 GenerateTestsAndXMLs.py")
writeTest("/mnt/jenkins/pub.log", "Generated XML files")
os.system("cd '/workspace/Multi-Node Manual Linux/test' && python3 PublisherTestList.py /mnt/jenkins/")
writeTest("/mnt/jenkins/pub.log", "Tests Completed")