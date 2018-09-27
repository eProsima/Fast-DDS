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
for a in pub_list:
	print("Launch Publisher on: " + a)
	subprocess.Popen("docker -H "+ a + " run --cap-add=SYS_ADMIN --security-opt seccomp=unconfined --cap-add=DAC_READ_SEARCH --rm -d --network=host --name TestPub" + str(id) + " ubuntu-test python3 \"/workspace/Multi-Node Manual Linux/test/PublisherTests.py\"" + user + " " + password, shell=True)
	id = id + 1

p = None
id = 0
for a in sub_list:
	print("Launch Subscriber on: " + a)
	p = subprocess.Popen("docker -H "+ a + " run --cap-add=SYS_ADMIN --security-opt seccomp=unconfined --cap-add=DAC_READ_SEARCH --rm -d --network=host --name TestSub" + str(id) + " ubuntu-test python3 \"/workspace/Multi-Node Manual Linux/test/SubscriberTests.py\"" + user + " " + password, shell=True)
	id = id + 1

if p != None:
	p.communicate()
