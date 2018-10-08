#!/usr/bin/python3

import sys, os, glob, datetime, csv

tests_list = ["LatencyTest", "ThroughputTest", "VideoTest", "MemoryTest"]
filter_list = [["_average", "_minimum"], ["_all_"], [], []]

def AnalyzeTestFiles(test_name, file_list):
	print("Start " + test_name)
	stats = {}
	columns = []
	for filename in file_list:
		with open(filename, 'r') as csvfile:
			reader = csv.reader(csvfile)
			for row_id, row in enumerate(reader):
				for column_id, column in enumerate(row):
					if column:
						if row_id == 0 and column not in stats:
							stats[column] = []
							columns.append(column)
						elif row_id == 1:
							stats[columns[column_id]].append(float(column))

	# Get the maximum and the minimum of the list without the last value
	for column_id in columns:
		min_value = min(stats[column_id][:-1])
		max_value = max(stats[column_id][:-1])
		if stats[column_id][-1] > max_value and abs(max_value - min_value) < abs(stats[column_id][-1] - max_value):
			print("ALARM!! " + test_name + ". Unexpectly high value for column: " + column_id)
			return -1
	return 0

def AnalyzeFiles(test_id, file_list):
	if len(file_list) > minimum_test_files:
		if test_id == 0:
			if "_average" in file_list[0]:
				return AnalyzeTestFiles("Latency Average Test", file_list)
			elif "_minimum" in file_list[0]:
				return AnalyzeTestFiles("Latency Minimum Test", file_list)
		elif test_id == 1:
			return AnalyzeTestFiles("Throughput Test", file_list)
		elif test_id == 2:
			return AnalyzeTestFiles("Video Test", file_list)
		elif test_id == 3:
			return AnalyzeTestFiles("Memory Test", file_list)			
	else:
		print("There aren't enough files to analyze the test: " + tests_list[test_id])

output_folder = ""
if len(sys.argv) >= 2:
	output_folder = os.path.join(os.path.abspath(sys.argv[1]), "")
else:
	print("Invalid input parameters")
	exit(-2)

minimum_test_files = 5
maximum_historic_files = 10

global_result = 0
global_files_list = [];
files_by_test = {}

# Loops for test and combination
for test_id, test in enumerate(tests_list):
	global_files_list = []
	files_by_test = {}
	for file in os.listdir(output_folder):
		# Take all the tests by name
		if file.startswith(test) and file.endswith(".csv"):
			filename = os.path.join(output_folder, file)
			
			# Filter files for the test
			if len(filter_list[test_id]) > 0:
				for filter_name in filter_list[test_id]:
					if filename.find(filter_name) != -1:
						global_files_list.append(filename)
						break		
			else:
				global_files_list.append(filename)
	
	print("--------------------------------")
	if len(global_files_list) == 0:
		print("There is no csv files to check for the test: " + test)
	else:
		global_files_list.sort()

		# Filter test by prefix
		prev_prefix = global_files_list[0][:-23]
		cur_prefix = prev_prefix
		test_files_list = []
		for file_id, file in enumerate(global_files_list):
			cur_prefix = file[:-23]
			if prev_prefix != cur_prefix:
				files_by_test[prev_prefix] = test_files_list
				test_files_list = [file]
				prev_prefix = cur_prefix
			else:
				test_files_list.append(file)

		files_by_test[prev_prefix] = test_files_list

		# Order tests by date
		for name in files_by_test:
			files_by_test[name].sort()

		# Delete old files
		for name in files_by_test:			
			if len(files_by_test[name]) > maximum_historic_files:
				for i in range(0, len(files_by_test[name]) - maximum_historic_files):
					print("delete old file: " + files_by_test[name][i])
					os.remove(files_by_test[name][i])
				del files_by_test[name][:len(files_by_test[name]) - maximum_historic_files]

		for name in files_by_test:
			result = AnalyzeFiles(test_id, files_by_test[name])
			if result == -1:
				global_result = -1
				file_name = files_by_test[name][-1]
				file_path = file_name.split("/")
				file_name = ""
				for field in file_path[:-1]:
					file_name += field + "/"
				file_name += "w_" + file_path[-1]
				os.rename(files_by_test[name][-1], file_name)

exit(global_result)
