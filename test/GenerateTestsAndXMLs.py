#!/usr/bin/python3

import sys, os, glob

tests_list = ["Latency", "Throughput"]
history_kind = ["KEEP_LAST", "KEEP_ALL"]
durability_kind = ["VOLATILE", "TRANSIENT_LOCAL", '''"TRANSIENT",''' "PERSISTENT"]
reliabilily_kind = ["BEST_EFFORT", "RELIABLE"]
publish_mode = ["SYNCHRONOUS", "ASYNCHRONOUS"]
item_type = ["Publisher", "Subscriber"]

# Create or clean the output folder
if not os.path.exists("./xml"):
    os.makedirs("./xml")
else:	
	files = glob.glob('./xml/*')
	for f in files:
		os.remove(f)

# Generate xml testing files
filename = ""
for test_id, test in enumerate(tests_list):
	for history_id, history in enumerate(history_kind):
		for durability_id, durability in enumerate(durability_kind):
			for reliabilily_id, reliabilily in enumerate(reliabilily_kind):
				for publish_id, publish in enumerate(publish_mode):
					for item in item_type:
						if item == "Publisher": 
							filename = "xml/" + item + "_" + test + "_" + str(history_id) + "_" + str(durability_id) + "_" + str(reliabilily_id) + "_" + str(publish_id) + ".xml"
						else:
							filename = "xml/" + item + "_" + test + "_" + str(history_id) + "_" + str(durability_id) + "_" + str(reliabilily_id) + ".xml"
							
						with open(filename, "w") as text_file:
							print("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>", file=text_file)
							print("<profiles>", file=text_file)
							print("\t<participant profile_name=\"participant_profile\">", file=text_file)
							print("\t\t<rtps>", file=text_file)
							print("\t\t\t<builtin>", file=text_file)
							print("\t\t\t\t<domainId>0</domainId>", file=text_file)
							print("\t\t\t\t<leaseDuration>", file=text_file)
							print("\t\t\t\t\t<durationbyname>INFINITE</durationbyname>", file=text_file)
							print("\t\t\t\t</leaseDuration>", file=text_file)
							print("\t\t\t</builtin>", file=text_file)
							print("\t\t</rtps>", file=text_file)
							print("\t</participant>\n", file=text_file)

							if item == "Publisher": 
								print("\t<publisher profile_name=\"publisher_profile\">", file=text_file)
								print("\t\t<topic>", file=text_file)
								print("\t\t\t<name>" + test + "_topic</name>", file=text_file)
								print("\t\t\t<dataType>XMLProfilesExample</dataType>", file=text_file)
								print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
								print("\t\t\t<historyQos>", file=text_file)
								print("\t\t\t\t<kind>" + history + "</kind>", file=text_file)
								print("\t\t\t\t<depth>20</depth>", file=text_file)
								print("\t\t\t</historyQos>", file=text_file)
								print("\t\t</topic>", file=text_file)
								print("\t\t<qos>", file=text_file)
								print("\t\t\t<durability>", file=text_file)
								print("\t\t\t\t<kind>" + durability + "</kind>", file=text_file)
								print("\t\t\t</durability>", file=text_file)
								print("\t\t\t<reliability>", file=text_file)
								print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
								print("\t\t\t</reliability>", file=text_file)
								print("\t\t\t<publishMode>", file=text_file)
								print("\t\t\t\t<kind>" + publish + "</kind>", file=text_file)
								print("\t\t\t</publishMode>", file=text_file)
								print("\t\t</qos>", file=text_file)
								print("\t\t<historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>", file=text_file)
								print("\t</publisher>\n", file=text_file)

							if item == "Subscriber":
								print("\t<subscriber profile_name=\"subscriber_profile\">", file=text_file)
								print("\t\t<topic>", file=text_file)
								print("\t\t\t<name>" + test + "_topic</name>", file=text_file)
								print("\t\t\t<dataType>XMLProfilesExample</dataType>", file=text_file)
								print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
								print("\t\t\t<historyQos>", file=text_file)
								print("\t\t\t\t<kind>" + history + "</kind>", file=text_file)
								print("\t\t\t\t<depth>20</depth>", file=text_file)
								print("\t\t\t</historyQos>", file=text_file)
								print("\t\t</topic>", file=text_file)
								print("\t\t<qos>", file=text_file)
								print("\t\t\t<durability>", file=text_file)
								print("\t\t\t\t<kind>" + durability + "</kind>", file=text_file)
								print("\t\t\t</durability>", file=text_file)
								print("\t\t\t<reliability>", file=text_file)
								print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
								print("\t\t\t</reliability>", file=text_file)
								print("\t\t</qos>", file=text_file)
								print("\t\t<historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>", file=text_file)
								print("\t</subscriber>\n", file=text_file)

							print("</profiles>", file=text_file)


with open("PublisherTestList.py", "w") as pub_test_file:
	with open("SubscriberTestList.py", "w") as sub_test_file:		
		print("#!/usr/bin/python3\n", file=pub_test_file)
		print("import sys, os\n", file=pub_test_file)
		print("if len(sys.argv) < 2:", file=pub_test_file)
		print("\tprint (\"Error. Invalid Input Parameters\")", file=pub_test_file)
		print("\tsys.exit(-1)\n", file=pub_test_file)

		print("#!/usr/bin/python3\n", file=sub_test_file)
		print("import sys, os\n", file=sub_test_file)
		print("if len(sys.argv) < 2:", file=sub_test_file)
		print("\tprint (\"Error. Invalid Input Parameters\")", file=sub_test_file)
		print("\tsys.exit(-1)\n", file=sub_test_file)

		for test_id, test in enumerate(tests_list):

			for pub_history_id, pub_history in enumerate(history_kind):
				for pub_durability_id, pub_durability in enumerate(durability_kind):
					for pub_reliabilily_id, pub_reliabilily in enumerate(reliabilily_kind):
						for pub_publish_id, pub_publish in enumerate(publish_mode):
						
							for sub_history_id, sub_history in enumerate(history_kind):
								for sub_durability_id, sub_durability in enumerate(durability_kind):
									for sub_reliabilily_id, sub_reliabilily in enumerate(reliabilily_kind):

										if pub_durability_id >= sub_durability_id  or pub_reliabilily_id >= sub_reliabilily_id:
											filename = "xml/Publisher_" + test + "_" + str(pub_history_id) + "_" + str(pub_durability_id) + "_" + str(pub_reliabilily_id) + "_" + str(pub_publish_id) + ".xml"
											print("os.system(sys.argv[1] + \"" + test + " publisher " + filename + " >> pub.log\")", file=pub_test_file)

											filename = "xml/Subscriber" + test + "_" + str(sub_history_id) + "_" + str(sub_durability_id) + "_" + str(sub_reliabilily_id) + ".xml"
											print("os.system(sys.argv[1] + \"" + test + " subscriber " + filename + " >> sub.log\")", file=sub_test_file)
