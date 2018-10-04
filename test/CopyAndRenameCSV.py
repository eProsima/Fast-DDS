#!/usr/bin/python3

import sys, os, datetime, shutil

tests_list = ["LatencyTest", "ThroughputTest", "VideoTest", "MemoryTest"]
filter_list = [["_average", "_minimum"], ["_all_"], [], []]

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

global_files_list = []
# Loops for test and combination
for test_id, test in enumerate(tests_list):
	for file in os.listdir(output_folder):
		# Take all the tests by name
		if file.startswith(test) and file.endswith(".csv"):
			
			# Filter files for the test
			if len(filter_list[test_id]) > 0:
				for filter_name in filter_list[test_id]:
					if file.find(filter_name) != -1:
						global_files_list.append(file)
						break		
			else:
				global_files_list.append(file)


for file in global_files_list:
	shutil.copy(output_folder + "/" + file, csvs_folder + "/" + file[:-4] + now.strftime("%Y-%m-%d_%H-%M-%S") + ".csv")

			
