#!/usr/bin/python3

import sys, os, signal, datetime

def signal_handler(sig, frame):
	print('Execution aborted by user')
	sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)

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

now = datetime.datetime.now()

writeTest("/mnt/jenkins/output/sub.log", "---------------------------------")
writeTest("/mnt/jenkins/output/sub.log", "Start Publisher Test - " + now.strftime("%Y-%m-%d_%H-%M-%S"))
os.system("mkdir -p /mnt/jenkins/output")
writeTest("/mnt/jenkins/output/sub.log", "Start Tests")
os.chdir(folder)
os.system("python3 GenerateTestsAndXMLs.py '" + folder + "'")
writeTest("/mnt/jenkins/output/sub.log", "Generated XML files")
writeTest("/mnt/jenkins/output/sub.log", "Start Tests")
os.system("python3 SubscriberTestList.py /mnt/jenkins/ '" + folder + "'")
writeTest("/mnt/jenkins/output/sub.log", "Tests Completed")