#!/usr/bin/python3

import sys, os

user = ""
password = ""

def writeTest(filename, text):
	with open(filename, "a") as myfile:
		myfile.write(text + "\n")

if len(sys.argv) <= 3:
	print("Error")
	sys.exit(-1)
else:
	user = sys.argv[1]
	password = sys.argv[2]
	folder = sys.argv[3]

writeTest("/mnt/jenkins/sub.log", "Start Publisher Tests")
writeTest("/mnt/jenkins/sub.log", "Arguments:")
for x in sys.argv:
	writeTest("/mnt/jenkins/sub.log", "\t" + x)

os.system("mkdir -p /mnt/jenkins")
os.system("mount -t cifs -o username=" + user + ",password=" + password + " //mainserver.intranet.eprosima.com/Public/JenkinsTests /mnt/jenkins")
writeTest("/mnt/jenkins/pub.log", "Start Tests")
os.chdir(folder)
os.system("python3 GenerateTestsAndXMLs.py '" + folder + "'")
writeTest("/mnt/jenkins/pub.log", "Generated XML files")
writeTest("/mnt/jenkins/pub.log", "Start Tests")
os.system("python3 PublisherTestList.py /mnt/jenkins/ '" + folder + "'")
writeTest("/mnt/jenkins/pub.log", "Tests Completed")