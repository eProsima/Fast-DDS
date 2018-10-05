#!/usr/bin/python3

import sys, os, subprocess

pub_str = ""
sub_str = ""
user = ""
password = ""

if len(sys.argv) <= 4:
	pub_str = "192.168.1.59:2376"
	sub_str = "192.168.1.59:2376"
	user = ""
	password = ""
#     print ("Error.")
#   sys.exit(-1)
else:
	user = sys.argv[1] 
	password = sys.argv[2]
	pub_str = sys.argv[3]
	sub_str = sys.argv[4]

pub_list = pub_str.split(',')
sub_list = sub_str.split(',')

id = 0
last_docker = None
last_ip = None
for a in pub_list:
	print("Launch Publisher on: " + a)
	command = "docker -H "+ a + " run --cap-add=SYS_ADMIN --network=host --cap-add=DAC_READ_SEARCH --rm -d --name TestPub" + str(id) + " ubuntu-test-image bash -c \"mkdir -p /mnt/jenkins; mount -t cifs -o username=" + user + ",password=" + password + " //mainserver.intranet.eprosima.com/Public/JenkinsTests /mnt/jenkins; python3 /mnt/jenkins/PublisherTests.py " + user + " " + password + " /mnt/jenkins/\""
	print("Command: " + command)
	last_ip = a
	last_docker = "TestPub" + str(id)
	subprocess.Popen(command, shell=True)
	id = id + 1

p = None
id = 0
for a in sub_list:
	print("Launch Subscriber on: " + a)
	command = "docker -H "+ a + " run --cap-add=SYS_ADMIN --network=host --cap-add=DAC_READ_SEARCH --rm -d --name TestSub" + str(id) + " ubuntu-test-image bash -c \"mkdir -p /mnt/jenkins; mount -t cifs -o username=" + user + ",password=" + password + " //mainserver.intranet.eprosima.com/Public/JenkinsTests /mnt/jenkins; python3 /mnt/jenkins/SubscriberTests.py " + user + " " + password + " /mnt/jenkins/\""
	print("Command: " + command)
	last_ip = a
	last_docker = "TestSub" + str(id)
	p = subprocess.Popen(command, shell=True)
	id = id + 1

if p != None:
	print("Wait for communicate")
	p.communicate()
	print("Communicate Finished")
if last_docker != None:
	print("Docker Wait")
	os.system("docker -H " + last_ip + " wait " + last_docker)
	print("Docker Wait Finished")
