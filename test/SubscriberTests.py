#!/usr/bin/python3

import sys, os

user = ""
password = ""
folder = ""

def writeTest(filename, text):
        with open(filename, "a") as myfile:
                myfile.write(text + "\n")

if len(sys.argv) == 1:
    folder = os.getcwd()

if len(sys.argv) == 2:
    folder = os.path.abspath(sys.argv[1])

if len(sys.argv) == 4:
    user = sys.argv[1]
    password = sys.argv[2]
    folder = os.path.abspath(sys.argv[3])
    if not os.path.ismount("/mnt/jenkins"):
        os.system("mkdir -p /mnt/jenkins")
        os.system("mount -t cifs -o username=" + user + ",password=" + password + " //mainserver.intranet.eprosima.com/Public/JenkinsTests /mnt/jenkins")
        print("/mnt/jenkins mounted")
    else:
        print("/mnt/jenkins is already mounted")

if not folder:
    print("Error")
    sys.exit(-1)


writeTest("/mnt/jenkins/sub.log", "Start Subscriber Tests")
writeTest("/mnt/jenkins/sub.log", "Arguments:")
for x in sys.argv:
	writeTest("/mnt/jenkins/sub.log", "\t" + x)

writeTest("/mnt/jenkins/sub.log", "Start Tests")
os.chdir(folder)
os.system("python3 GenerateTestsAndXMLs.py '" + folder + "'")
writeTest("/mnt/jenkins/sub.log", "Generated XML files")
writeTest("/mnt/jenkins/sub.log", "Start Tests")
os.system("python3 SubscriberTestList.py /mnt/jenkins/ '" + folder + "'")
writeTest("/mnt/jenkins/sub.log", "Tests Completed")