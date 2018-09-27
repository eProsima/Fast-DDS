#!/usr/bin/python3

import sys, os

str = ""
user = ""
password = ""

if len(sys.argv) <= 3:
    str = "192.168.1.59:2376"
    user = ""
    password = ""
#     print ("Error.")
#   sys.exit(-1)
else:
	user = sys.argv[1] 
	password = sys.argv[2]
	str = sys.argv[3]

list = str.split(',')
for a in list:
	print("Launch Subscriber on: " + a)
    os.system("docker -H" + a + " run --cap-add SYS_ADMIN --security-opt seccomp=unconfined " +
	"--cap-add DAC_READ_SEARCH -ti ubuntu-test python3 \"/workspace/Multi-Node Manual Linux/test/SubscriberTests.py\" " +
	user + " " + password)
