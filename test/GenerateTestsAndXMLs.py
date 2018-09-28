#!/usr/bin/python3

import sys, os, glob, datetime

tests_list = ["LatencyTest", "ThroughputTest"]
configs_list = [[["KEEP_LAST"], ["VOLATILE"], ["BEST_EFFORT", "RELIABLE"], ["SYNCHRONOUS", "ASYNCHRONOUS"]],
 [["KEEP_LAST"], ["VOLATILE"], ["BEST_EFFORT", "RELIABLE"], ["SYNCHRONOUS", "ASYNCHRONOUS"]]]

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

#### GENERATE LATENCY TESTS
filename = ""
test = tests_list[0]
test_config = configs_list[0];
for history_id, history in enumerate(test_config[0]):
	for durability_id, durability in enumerate(test_config[1]):
		for reliabilily_id, reliabilily in enumerate(test_config[2]):
			for publish_id, publish in enumerate(test_config[3]):
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
						print("\t\t\t<defaultSendPort>10042</defaultSendPort>", file=text_file)
						print("\t\t\t<sendSocketBufferSize>65536</sendSocketBufferSize>", file=text_file)
						print("\t\t\t<listenSocketBufferSize>131072</listenSocketBufferSize>", file=text_file)
						print("\t\t\t<builtin>", file=text_file)
						print("\t\t\t\t<domainId>0</domainId>", file=text_file)
						print("\t\t\t\t<use_SIMPLE_RTPS_PDP>true</use_SIMPLE_RTPS_PDP>", file=text_file)
						print("\t\t\t\t<EDP>SIMPLE</EDP>", file=text_file)
						print("\t\t\t\t<simpleEDP>", file=text_file)
						print("\t\t\t\t\t<PUBWRITER_SUBREADER>true</PUBWRITER_SUBREADER>", file=text_file)
						print("\t\t\t\t\t<PUBREADER_SUBWRITER>true</PUBREADER_SUBWRITER>", file=text_file)
						print("\t\t\t\t</simpleEDP>", file=text_file)
						print("\t\t\t\t<leaseDuration>", file=text_file)
						print("\t\t\t\t\t<durationbyname>INFINITE</durationbyname>", file=text_file)
						print("\t\t\t\t</leaseDuration>", file=text_file)
						print("\t\t\t</builtin>", file=text_file)

						if item == "Publisher": 
							print("\t\t\t<name>Participant_pub</name>", file=text_file)
						else:
							print("\t\t\t<name>Participant_sub</name>", file=text_file)
						
						print("\t\t</rtps>", file=text_file)
						print("\t</participant>\n", file=text_file)

						if item == "Publisher": 
							print("\t<publisher profile_name=\"publisher_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "_PUB2SUB</name>", file=text_file)
							print("\t\t\t<dataType>LatencyType</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_ALL</kind>", file=text_file)
							print("\t\t\t\t<depth>20</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>TRANSIENT_LOCAL</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t\t<publishMode>", file=text_file)
							print("\t\t\t\t<kind>" + publish + "</kind>", file=text_file)
							print("\t\t\t</publishMode>", file=text_file)
							print("\t\t</qos>", file=text_file)
							print("\t\t<times>", file=text_file)
							print("\t\t\t<heartbeatPeriod>", file=text_file)
							print("\t\t\t\t<durationbyval>", file=text_file)
							print("\t\t\t\t\t<seconds>0</seconds>", file=text_file)
							print("\t\t\t\t\t<fraction>429496700</fraction>", file=text_file)
							print("\t\t\t\t</durationbyval>", file=text_file)
							print("\t\t\t</heartbeatPeriod>", file=text_file)
							print("\t\t</times>", file=text_file)
							print("\t\t<historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>", file=text_file)
							print("\t</publisher>\n", file=text_file)

							print("\t<subscriber profile_name=\"subscriber_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "_SUB2PUB</name>", file=text_file)
							print("\t\t\t<dataType>LatencyType</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_LAST</kind>", file=text_file)
							print("\t\t\t\t<depth>1</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>VOLATILE</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t</qos>", file=text_file)
							print("\t\t<historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>", file=text_file)
							print("\t</subscriber>\n", file=text_file)

							print("\t<publisher profile_name=\"publisher_cmd_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "cmd_PUB2SUB</name>", file=text_file)
							print("\t\t\t<dataType>TestCommandType</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_ALL</kind>", file=text_file)
							print("\t\t\t\t<depth>100</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>TRANSIENT_LOCAL</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t\t<publishMode>", file=text_file)
							print("\t\t\t\t<kind>SYNCHRONOUS</kind>", file=text_file)
							print("\t\t\t</publishMode>", file=text_file)
							print("\t\t</qos>", file=text_file)
							print("\t</publisher>\n", file=text_file)

							print("\t<subscriber profile_name=\"subscriber_cmd_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "cmd_SUB2PUB</name>", file=text_file)
							print("\t\t\t<dataType>TestCommandType</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_ALL</kind>", file=text_file)
							print("\t\t\t\t<depth>100</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>TRANSIENT_LOCAL</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t</qos>", file=text_file)
							print("\t\t<historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>", file=text_file)
							print("\t</subscriber>\n", file=text_file)

						if item == "Subscriber":
							print("\t<publisher profile_name=\"publisher_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "_SUB2PUB</name>", file=text_file)
							print("\t\t\t<dataType>LatencyType</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_ALL</kind>", file=text_file)
							print("\t\t\t\t<depth>20</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>TRANSIENT_LOCAL</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t\t<publishMode>", file=text_file)
							print("\t\t\t\t<kind>" + publish + "</kind>", file=text_file)
							print("\t\t\t</publishMode>", file=text_file)
							print("\t\t</qos>", file=text_file)
							print("\t\t<times>", file=text_file)
							print("\t\t\t<heartbeatPeriod>", file=text_file)
							print("\t\t\t\t<durationbyval>", file=text_file)
							print("\t\t\t\t\t<seconds>0</seconds>", file=text_file)
							print("\t\t\t\t\t<fraction>429496700</fraction>", file=text_file)
							print("\t\t\t\t</durationbyval>", file=text_file)
							print("\t\t\t</heartbeatPeriod>", file=text_file)
							print("\t\t</times>", file=text_file)
							print("\t\t<historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>", file=text_file)
							print("\t</publisher>\n", file=text_file)

							print("\t<subscriber profile_name=\"subscriber_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "_PUB2SUB</name>", file=text_file)
							print("\t\t\t<dataType>LatencyType</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_LAST</kind>", file=text_file)
							print("\t\t\t\t<depth>1</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>VOLATILE</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t</qos>", file=text_file)
							print("\t\t<historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>", file=text_file)
							print("\t</subscriber>\n", file=text_file)

							print("\t<publisher profile_name=\"publisher_cmd_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "cmd_SUB2PUB</name>", file=text_file)
							print("\t\t\t<dataType>TestCommandType</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_ALL</kind>", file=text_file)
							print("\t\t\t\t<depth>100</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>TRANSIENT_LOCAL</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t\t<publishMode>", file=text_file)
							print("\t\t\t\t<kind>SYNCHRONOUS</kind>", file=text_file)
							print("\t\t\t</publishMode>", file=text_file)
							print("\t\t</qos>", file=text_file)
							print("\t</publisher>\n", file=text_file)

							print("\t<subscriber profile_name=\"subscriber_cmd_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "cmd_PUB2SUB</name>", file=text_file)
							print("\t\t\t<dataType>TestCommandType</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_ALL</kind>", file=text_file)
							print("\t\t\t\t<depth>100</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>TRANSIENT_LOCAL</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t</qos>", file=text_file)
							print("\t\t<historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>", file=text_file)
							print("\t</subscriber>\n", file=text_file)

						print("</profiles>", file=text_file)

#### GENERATE THROUGHPUT TESTS
filename = ""
test = tests_list[1];
test_config = configs_list[1];
for history_id, history in enumerate(test_config[0]):
	for durability_id, durability in enumerate(test_config[1]):
		for reliabilily_id, reliabilily in enumerate(test_config[2]):
			for publish_id, publish in enumerate(test_config[3]):
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
						if item == "Publisher": 
							print("\t\t\t<name>Participant_publisher</name>", file=text_file)
						else:
							print("\t\t\t<name>Participant_subscriber</name>", file=text_file)
						print("\t\t</rtps>", file=text_file)
						print("\t</participant>\n", file=text_file)

						if item == "Publisher": 
							print("\t<publisher profile_name=\"publisher_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "_topic</name>", file=text_file)
							print("\t\t\t<dataType>ThroughputType</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_ALL</kind>", file=text_file)
							print("\t\t\t\t<depth>100</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>TRANSIENT_LOCAL</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)							
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t\t<publishMode>", file=text_file)
							print("\t\t\t\t<kind>" + publish + "</kind>", file=text_file)
							print("\t\t\t</publishMode>", file=text_file)
							print("\t\t</qos>", file=text_file)

							if reliabilily == "RELIABLE":
								print("\t\t<times>", file=text_file)
								print("\t\t\t<heartbeatPeriod>", file=text_file)
								print("\t\t\t\t<durationbyval>", file=text_file)
								print("\t\t\t\t\t<seconds>0</seconds>", file=text_file)
								print("\t\t\t\t\t<fraction>429496700</fraction>", file=text_file)
								print("\t\t\t\t</durationbyval>", file=text_file)
								print("\t\t\t</heartbeatPeriod>", file=text_file)
								print("\t\t</times>", file=text_file)

							print("\t\t<historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>", file=text_file)
							print("\t</publisher>\n", file=text_file)

							print("\t<publisher profile_name=\"publisher_cmd_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "cmd_PUB2SUB</name>", file=text_file)
							print("\t\t\t<dataType>ThroughputCommand</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_LAST</kind>", file=text_file)
							print("\t\t\t\t<depth>50</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>TRANSIENT_LOCAL</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t\t<publishMode>", file=text_file)
							print("\t\t\t\t<kind>SYNCHRONOUS</kind>", file=text_file)
							print("\t\t\t</publishMode>", file=text_file)
							print("\t\t</qos>", file=text_file)
							print("\t</publisher>\n", file=text_file)

							print("\t<subscriber profile_name=\"subscriber_cmd_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "cmd_SUB2PUB</name>", file=text_file)
							print("\t\t\t<dataType>ThroughputCommand</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_ALL</kind>", file=text_file)
							print("\t\t\t\t<depth>20</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>TRANSIENT_LOCAL</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t</qos>", file=text_file)
							print("\t\t<historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>", file=text_file)
							print("\t</subscriber>\n", file=text_file)

						if item == "Subscriber":
							print("\t<subscriber profile_name=\"subscriber_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "_topic</name>", file=text_file)
							print("\t\t\t<dataType>ThroughputType</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_ALL</kind>", file=text_file)
							print("\t\t\t\t<depth>1</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>VOLATILE</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t</qos>", file=text_file)
							print("\t\t<historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>", file=text_file)
							print("\t</subscriber>\n", file=text_file)

							print("\t<publisher profile_name=\"publisher_cmd_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "cmd_SUB2PUB</name>", file=text_file)
							print("\t\t\t<dataType>ThroughputCommand</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_LAST</kind>", file=text_file)
							print("\t\t\t\t<depth>50</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>TRANSIENT_LOCAL</kind>", file=text_file)
							print("\t\t\t</durability>", file=text_file)
							print("\t\t\t<reliability>", file=text_file)
							print("\t\t\t\t<kind>" + reliabilily + "</kind>", file=text_file)
							print("\t\t\t</reliability>", file=text_file)
							print("\t\t\t<publishMode>", file=text_file)
							print("\t\t\t\t<kind>SYNCHRONOUS</kind>", file=text_file)
							print("\t\t\t</publishMode>", file=text_file)
							print("\t\t</qos>", file=text_file)
							print("\t</publisher>\n", file=text_file)

							print("\t<subscriber profile_name=\"subscriber_cmd_profile\">", file=text_file)
							print("\t\t<topic>", file=text_file)
							print("\t\t\t<name>" + test + "cmd_PUB2SUB</name>", file=text_file)
							print("\t\t\t<dataType>ThroughputCommand</dataType>", file=text_file)
							print("\t\t\t<kind>NO_KEY</kind>", file=text_file)
							print("\t\t\t<historyQos>", file=text_file)
							print("\t\t\t\t<kind>KEEP_LAST</kind>", file=text_file)
							print("\t\t\t\t<depth>20</depth>", file=text_file)
							print("\t\t\t</historyQos>", file=text_file)
							print("\t\t</topic>", file=text_file)
							print("\t\t<qos>", file=text_file)
							print("\t\t\t<durability>", file=text_file)
							print("\t\t\t\t<kind>TRANSIENT_LOCAL</kind>", file=text_file)
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
		print("def writeTestTitle(filename, testname):", file=pub_test_file)
		print("\twith open(filename, \"a\") as myfile:", file=pub_test_file)
		print("\t\tmyfile.write(\"\\n\\n*****************************************************************************\\n\")", file=pub_test_file)
		print("\t\tmyfile.write(testname + \"\\n\")", file=pub_test_file)
		print("\t\tmyfile.write(\"*****************************************************************************\\n\")\n\n", file=pub_test_file)
		
		print("#!/usr/bin/python3\n", file=sub_test_file)
		print("import sys, os\n", file=sub_test_file)
		print("if len(sys.argv) < 2:", file=sub_test_file)
		print("\tprint (\"Error. Invalid Input Parameters\")", file=sub_test_file)
		print("\tsys.exit(-1)\n", file=sub_test_file)
		print("def writeTestTitle(filename, testname):", file=sub_test_file)
		print("\twith open(filename, \"a\") as myfile:", file=sub_test_file)
		print("\t\tmyfile.write(\"\\n\\n*****************************************************************************\\n\")", file=sub_test_file)
		print("\t\tmyfile.write(testname + \"\\n\")", file=sub_test_file)
		print("\t\tmyfile.write(\"*****************************************************************************\\n\")\n\n", file=sub_test_file)

		pub_log_filename = "pub_" + str(datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")) + ".log"
		sub_log_filename = "sub_" + str(datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")) + ".log"
		pub_filename = ""
		sub_filename = ""
		for test_id, test in enumerate(tests_list):
			test_config = configs_list[test_id];
			for pub_history_id, pub_history in enumerate(test_config[0]):
				for pub_durability_id, pub_durability in enumerate(test_config[1]):
					for pub_reliabilily_id, pub_reliabilily in enumerate(test_config[2]):
						for pub_publish_id, pub_publish in enumerate(test_config[3]):

							for sub_history_id, sub_history in enumerate(test_config[0]):
								for sub_durability_id, sub_durability in enumerate(test_config[1]):
									for sub_reliabilily_id, sub_reliabilily in enumerate(test_config[2]):
										'''
			for pub_history_id, pub_history in enumerate(history_kind):
				for pub_durability_id, pub_durability in enumerate(durability_kind):
					for pub_reliabilily_id, pub_reliabilily in enumerate(reliabilily_kind):
						for pub_publish_id, pub_publish in enumerate(publish_mode):
						
							for sub_history_id, sub_history in enumerate(history_kind):
								for sub_durability_id, sub_durability in enumerate(durability_kind):
									for sub_reliabilily_id, sub_reliabilily in enumerate(reliabilily_kind):
										'''
										if pub_durability_id >= sub_durability_id and pub_reliabilily_id >= sub_reliabilily_id:
											pub_filename = "'" + sys.argv[1] + "xml/Publisher_" + test + "_" + str(pub_history_id) + "_" + str(pub_durability_id) + "_" + str(pub_reliabilily_id) + "_" + str(pub_publish_id) + ".xml'"
											sub_filename = "'" + sys.argv[1] + "xml/Subscriber_" + test + "_" + str(sub_history_id) + "_" + str(sub_durability_id) + "_" + str(sub_reliabilily_id) + ".xml'"

											print("writeTestTitle(sys.argv[1] + \"" + pub_log_filename + "\", \"" + pub_filename + " -> " + sub_filename + "\")", file=pub_test_file)
											print("os.system(sys.argv[1] + \"" + test + " publisher --xml " + pub_filename + " >> \" + sys.argv[1] + \"" + pub_log_filename + "\")", file=pub_test_file)

											print("writeTestTitle(sys.argv[1] + \"" + sub_log_filename + "\", \"" + pub_filename + " -> " + sub_filename +  "\")", file=sub_test_file)
											print("os.system(sys.argv[1] + \"" + test + " subscriber --xml " + sub_filename + " >> \" + sys.argv[1] + \"" + sub_log_filename + "\")", file=sub_test_file)
