#!/usr/bin/python3

import sys, os, datetime, shutil

output_folder = ""
csvs_folder = ""
if len(sys.argv) >= 3:
	output_folder = os.path.join(os.path.abspath(sys.argv[1]), "")
	csvs_folder = os.path.join(os.path.abspath(sys.argv[2]), "")
else:
	print("Invalid input parameters")
	exit(-2)

now = datetime.datetime.now()
os.system("mkdir -p " + csvs_folder)
for file in os.listdir(output_folder):
	if file.endswith(".csv"):
		shutil.copy(output_folder + "/" + file, csvs_folder + "/" + file[:-4] + now.strftime("%Y-%m-%d_%H-%M-%S") + ".csv")

			
