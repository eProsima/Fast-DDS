/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPStaticXML.cpp
 *
 */

#include "eprosimartps/builtin/discovery/endpoint/EDPStaticXML.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/TimeConversion.h"
#include "eprosimartps/reader/WriterProxyData.h"
#include "eprosimartps/writer/ReaderProxyData.h"

#include "boost/lexical_cast.hpp"

namespace eprosima {
namespace rtps {

EDPStaticXML::EDPStaticXML() {
	// TODO Auto-generated constructor stub

}

EDPStaticXML::~EDPStaticXML()
{
	// TODO Auto-generated destructor stub
	for(std::vector<StaticParticipantInfo*>::iterator pit = m_participants.begin();
			pit!=m_participants.end();++pit)
	{
		for(std::vector<ReaderProxyData*>::iterator rit = (*pit)->m_readers.begin();
				rit!=(*pit)->m_readers.end();++rit)
		{
			delete(*rit);
		}
		for(std::vector<WriterProxyData*>::iterator wit = (*pit)->m_writers.begin();
				wit!=(*pit)->m_writers.end();++wit)
		{
			delete(*wit);
		}
	}
}

bool EDPStaticXML::loadXMLFile(std::string& filename)
{
	pInfo(RTPS_B_CYAN<<"Loading XML file: "<<filename<<endl);
	// Create an empty property tree object
	ptree pt;
	// Load the XML file into the property tree. If reading fails
	// (cannot open file, parse error), an exception is thrown.
	try{
		read_xml(filename, pt);
	}
	catch (std::exception &e)
	{
		pError("Error reading xml file: " << e.what() << endl);
		return false;
	}
	BOOST_FOREACH(ptree::value_type& xml_participant ,pt.get_child("staticdiscovery"))
	{
		if(xml_participant.first == "participant")
		{
			StaticParticipantInfo* pdata= new StaticParticipantInfo();
			BOOST_FOREACH(ptree::value_type& xml_participant_child,xml_participant.second)
			{
				if(xml_participant_child.first == "name")
				{
					pdata->m_participantName = xml_participant_child.second.data();
				}
				else if(xml_participant_child.first == "reader")
				{
					
						if(!loadXMLReaderEndpoint(xml_participant_child,pdata))
						{
							pError("Reader Endpoint has error, ignoring"<<endl);
						}
				}
				else if(xml_participant_child.first == "writer")
				{
					
						if(!loadXMLWriterEndpoint(xml_participant_child,pdata))
						{
							pError("Writer Endpoint has error, ignoring"<<endl);
						}
				}
				else
				{
					pError("Unknown XMK tag: " << xml_participant_child.first);
				}
			}
			m_participants.push_back(pdata);
		}
	}
	return true;
}

bool EDPStaticXML::loadXMLReaderEndpoint(ptree::value_type& xml_endpoint,StaticParticipantInfo* pdata)
{
	ReaderProxyData* rdata = new ReaderProxyData();
	BOOST_FOREACH(ptree::value_type& xml_endpoint_child,xml_endpoint.second)
	{
		//cout << "READER ENDPOINT: " << xml_endpoint_child.first << endl;
		if(xml_endpoint_child.first == "userId")
		{
			//cout << "USER ID FOUND"<<endl;
			int16_t id = boost::lexical_cast<int16_t>(xml_endpoint_child.second.data());
			if(id<=0 || m_endpointIds.insert(id).second == false)
			{
				pError("Repeated or negative ID in XML file"<<endl);
				delete(rdata);
				return false;
			}
			rdata->m_userDefinedId = id;
		}
		else if(xml_endpoint_child.first == "entityId")
		{
			int32_t id = boost::lexical_cast<int32_t>(xml_endpoint_child.second.data());
			if(id<=0 || m_entityIds.insert(id).second == false)
			{
				pError("Repeated or negative entityId in XML file"<<endl);
				delete(rdata);
				return false;
			}
			octet* c = (octet*)&id;
		    rdata->m_guid.entityId.value[2] = c[0];
		    rdata->m_guid.entityId.value[1] = c[1];
            rdata->m_guid.entityId.value[0] = c[2]; 
		}
		else if(xml_endpoint_child.first == "expectsInlineQos")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "true")
			{
				rdata->m_expectsInlineQos = true;
				//cout << "READER WITH EXPECTS INLINE QOS " << endl;
			}
			else if (auxString == "false")
				rdata->m_expectsInlineQos = false;
			else
			{
				pError("Bad XML file, endpoint of expectsInlineQos: " << auxString << " is not valid"<<endl);
				delete(rdata);return false;
			}
		}
		else if(xml_endpoint_child.first == "topicName")
		{
			rdata->m_topicName = (std::string)xml_endpoint_child.second.data();
		}
		else if(xml_endpoint_child.first == "topicDataType")
		{
			rdata->m_typeName = (std::string)xml_endpoint_child.second.data();
		}
		else if(xml_endpoint_child.first == "topicKind")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "NO_KEY")
			{
				rdata->m_topicKind = NO_KEY;
				rdata->m_guid.entityId.value[3] = 0x04;
			}
			else if (auxString == "WITH_KEY")
			{
				rdata->m_topicKind = WITH_KEY;
				rdata->m_guid.entityId.value[3] = 0x07;
			}
			else
			{
				pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
				delete(rdata);return false;
			}
		}
		else if(xml_endpoint_child.first == "reliabilityQos")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "RELIABLE_RELIABILITY_QOS")
				rdata->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
			else if (auxString == "BEST_EFFORT_RELIABILITY_QOS")
				rdata->m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
			else
			{
				pError("Bad XML file, endpoint of stateKind: " << auxString << " is not valid"<<endl);
				delete(rdata);return false;
			}
		}
		else if(xml_endpoint_child.first == "unicastLocator")
		{
			Locator_t loc;
			loc.kind = 1;
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
			loc.set_IP4_address(auxString);
			loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
			rdata->m_unicastLocatorList.push_back(loc);
		}
		else if(xml_endpoint_child.first == "multicastLocator")
		{
			Locator_t loc;
			loc.kind = 1;
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
			loc.set_IP4_address(auxString);
			loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
			rdata->m_multicastLocatorList.push_back(loc);
		}
		else if(xml_endpoint_child.first == "topic")
		{
			rdata->m_topicName = xml_endpoint_child.second.get("<xmlattr>.name","");
			rdata->m_typeName = xml_endpoint_child.second.get("<xmlattr>.dataType","");
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.kind","");
			if(auxString == "NO_KEY")
			{
				rdata->m_topicKind = NO_KEY;
				rdata->m_guid.entityId.value[3] = 0x04;
			}
			else if (auxString == "WITH_KEY")
			{
				rdata->m_topicKind = WITH_KEY;
				rdata->m_guid.entityId.value[3] = 0x07;
			}
			else
			{
				pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
				delete(rdata);return false;
			}
			if(rdata->m_topicName == "EPROSIMA_UNKNOWN_STRING" || rdata->m_typeName == "EPROSIMA_UNKNOWN_STRING")
			{
				pError("Bad XML file, topic: "<<rdata->m_topicName << " or typeName: "<<rdata->m_typeName << " undefined"<<endl);
				delete(rdata);
				return false;
			}
		}
		else if(xml_endpoint_child.first == "durabilityQos")
		{
			std::string auxstring = (std::string)xml_endpoint_child.second.data();
			if(auxstring == "TRANSIENT_LOCAL_DURABILITY_QOS")
				rdata->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
			else if(auxstring == "VOLATILE_DURABILITY_QOS")
				rdata->m_qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
			else
			{
				pError("Bad XML file, durability of kind: " << auxstring << " is not valid"<<endl);
				delete(rdata);return false;
			}
		}
		else if(xml_endpoint_child.first == "ownershipQos")
		{
			std::string auxstring= xml_endpoint_child.second.get("<xmlattr>.kind","OWNERHSIP kind NOT PRESENT");
			if(auxstring == "SHARED_OWNERSHIP_QOS")
				rdata->m_qos.m_ownership.kind = SHARED_OWNERSHIP_QOS;
			else if(auxstring == "EXCLUSIVE_OWNERSHIP_QOS")
				rdata->m_qos.m_ownership.kind = EXCLUSIVE_OWNERSHIP_QOS;
			else
			{
				pError("Bad XML file, ownership of kind: " << auxstring << " is not valid"<<endl);
				delete(rdata);return false;
			}
		}
		else if(xml_endpoint_child.first == "partitionQos")
		{
			rdata->m_qos.m_partition.names.push_back((std::string)xml_endpoint_child.second.data());
		}
		else if(xml_endpoint_child.first == "livelinessQos")
		{
			std::string auxstring= xml_endpoint_child.second.get("<xmlattr>.kind","LIVELINESS kind NOT PRESENT");
			if(auxstring == "AUTOMATIC_LIVELINESS_QOS")
				rdata->m_qos.m_liveliness.kind = AUTOMATIC_LIVELINESS_QOS;
			else if(auxstring == "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS")
				rdata->m_qos.m_liveliness.kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
			else if(auxstring == "MANUAL_BY_TOPIC_LIVELINESS_QOS")
				rdata->m_qos.m_liveliness.kind = MANUAL_BY_TOPIC_LIVELINESS_QOS;
			else
			{
				pError("Bad XML file, liveliness of kind: " << auxstring << " is not valid"<<endl);
				delete(rdata);return false;
			}
			auxstring = xml_endpoint_child.second.get("<xmlattr>.leaseDuration_ms","INF");
			if(auxstring == "INF")
				rdata->m_qos.m_liveliness.lease_duration = c_TimeInfinite;
			else
			{
			try
			{
				uint32_t milliseclease = boost::lexical_cast<uint32_t>(auxstring);
				rdata->m_qos.m_liveliness.lease_duration = TimeConv::MilliSeconds2Time_t((double)milliseclease);
			}
			#pragma warning(disable: 4101)
			catch(std::exception &e)
			{
				pWarning("BAD XML:livelinessQos leaseDuration is a bad number: "<<auxstring<<" setting to INF"<<endl);
				rdata->m_qos.m_liveliness.lease_duration = c_TimeInfinite;
			}
			}
		}
		else
		{
			pWarning("Unkown Endpoint-XML tag, ignoring "<< xml_endpoint_child.first<<endl)
		}
	}
	if(rdata->m_userDefinedId == 0)
	{
		pError("Reader XML endpoint with NO ID defined"<<endl);
		delete(rdata);
		return false;
	}
	pdata->m_readers.push_back(rdata);
	return true;
}


bool EDPStaticXML::loadXMLWriterEndpoint(ptree::value_type& xml_endpoint,StaticParticipantInfo* pdata)
{
	WriterProxyData* wdata = new WriterProxyData();
	BOOST_FOREACH(ptree::value_type& xml_endpoint_child,xml_endpoint.second)
	{
		if(xml_endpoint_child.first == "userId")
		{
			int16_t id = boost::lexical_cast<int16_t>(xml_endpoint_child.second.data());
			if(id<=0 || m_endpointIds.insert(id).second == false)
			{
				pError("Repeated or negative ID in XML file"<<endl);
				delete(wdata);
				return false;
			}
			wdata->m_userDefinedId = id;
		}
		else if(xml_endpoint_child.first == "entityId")
		{
			int32_t id = boost::lexical_cast<int32_t>(xml_endpoint_child.second.data());
			if(id<=0 || m_entityIds.insert(id).second == false)
			{
				pError("Repeated or negative entityId in XML file"<<endl);
				delete(wdata);
				return false;
			}
			octet* c = (octet*)&id;
		    wdata->m_guid.entityId.value[2] = c[0];
		    wdata->m_guid.entityId.value[1] = c[1];
            wdata->m_guid.entityId.value[0] = c[2]; 
		}
		else if(xml_endpoint_child.first == "expectsInlineQos")
		{
			pWarning("BAD XML tag: Writers don't use expectInlineQos tag"<<endl);
		}
		else if(xml_endpoint_child.first == "topicName")
		{
			wdata->m_topicName = (std::string)xml_endpoint_child.second.data();
		}
		else if(xml_endpoint_child.first == "topicDataType")
		{
			wdata->m_typeName = (std::string)xml_endpoint_child.second.data();
		}
		else if(xml_endpoint_child.first == "topicKind")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "NO_KEY")
			{
				wdata->m_topicKind = NO_KEY;
				wdata->m_guid.entityId.value[3] = 0x03;
			}
			else if (auxString == "WITH_KEY")
			{
				wdata->m_topicKind = WITH_KEY;
				wdata->m_guid.entityId.value[3] = 0x02;
			}
			else
			{
				pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
				delete(wdata);return false;
			}
		}
		else if(xml_endpoint_child.first == "reliabilityQos")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "RELIABLE_RELIABILITY_QOS")
				wdata->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
			else if (auxString == "BEST_EFFORT_RELIABILITY_QOS")
				wdata->m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
			else
			{
				pError("Bad XML file, endpoint of stateKind: " << auxString << " is not valid"<<endl);
				delete(wdata);return false;
			}
		}
		else if(xml_endpoint_child.first == "unicastLocator")
		{
			Locator_t loc;
			loc.kind = 1;
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
			loc.set_IP4_address(auxString);
			loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
			wdata->m_unicastLocatorList.push_back(loc);
		}
		else if(xml_endpoint_child.first == "multicastLocator")
		{
			Locator_t loc;
			loc.kind = 1;
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
			loc.set_IP4_address(auxString);
			loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
			wdata->m_multicastLocatorList.push_back(loc);
		}
		else if(xml_endpoint_child.first == "topic")
		{
			wdata->m_topicName = xml_endpoint_child.second.get("<xmlattr>.name","EPROSIMA_UNKNOWN_STRING");
			wdata->m_typeName = xml_endpoint_child.second.get("<xmlattr>.dataType","EPROSIMA_UNKNOWN_STRING");
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.kind","EPROSIMA_UNKNOWN_STRING");
			if(auxString == "NO_KEY")
			{
				wdata->m_topicKind = NO_KEY;
				wdata->m_guid.entityId.value[3] = 0x03;
			}
			else if (auxString == "WITH_KEY")
			{
				wdata->m_topicKind = WITH_KEY;
				wdata->m_guid.entityId.value[3] = 0x02;
			}
			else
			{
				pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
				delete(wdata);return false;
			}
			if(wdata->m_topicName == "EPROSIMA_UNKNOWN_STRING" || wdata->m_typeName == "EPROSIMA_UNKNOWN_STRING")
			{
				pError("Bad XML file, topic: "<<wdata->m_topicName << " or typeName: "<<wdata->m_typeName << " undefined"<<endl);
				delete(wdata);
				return false;
			}
		}
		else if(xml_endpoint_child.first == "durabilityQos")
		{
			std::string auxstring = (std::string)xml_endpoint_child.second.data();
			if(auxstring == "TRANSIENT_LOCAL_DURABILITY_QOS")
				wdata->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
			else if(auxstring == "VOLATILE_DURABILITY_QOS")
				wdata->m_qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
			else
			{
				pError("Bad XML file, durability of kind: " << auxstring << " is not valid"<<endl);
				delete(wdata);return false;
			}
		}
		else if(xml_endpoint_child.first == "ownershipQos")
		{
			std::string auxstring= xml_endpoint_child.second.get("<xmlattr>.kind","OWNERHSIP kind NOT PRESENT");
			if(auxstring == "SHARED_OWNERSHIP_QOS")
				wdata->m_qos.m_ownership.kind = SHARED_OWNERSHIP_QOS;
			else if(auxstring == "EXCLUSIVE_OWNERSHIP_QOS")
				wdata->m_qos.m_ownership.kind = EXCLUSIVE_OWNERSHIP_QOS;
			else
			{
				pError("Bad XML file, ownership of kind: " << auxstring << " is not valid"<<endl);
				delete(wdata);return false;
			}
			wdata->m_qos.m_ownershipStrength.value = xml_endpoint_child.second.get("<xmlattr>.strength",0);
		}
		else if(xml_endpoint_child.first == "partitionQos")
		{
			wdata->m_qos.m_partition.names.push_back((std::string)xml_endpoint_child.second.data());
		}
		else if(xml_endpoint_child.first == "livelinessQos")
		{
			std::string auxstring= xml_endpoint_child.second.get("<xmlattr>.kind","LIVELINESS kind NOT PRESENT");
			if(auxstring == "AUTOMATIC_LIVELINESS_QOS")
				wdata->m_qos.m_liveliness.kind = AUTOMATIC_LIVELINESS_QOS;
			else if(auxstring == "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS")
				wdata->m_qos.m_liveliness.kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
			else if(auxstring == "MANUAL_BY_TOPIC_LIVELINESS_QOS")
				wdata->m_qos.m_liveliness.kind = MANUAL_BY_TOPIC_LIVELINESS_QOS;
			else
			{
				pError("Bad XML file, liveliness of kind: " << auxstring << " is not valid"<<endl);
				delete(wdata);return false;
			}
			auxstring = xml_endpoint_child.second.get("<xmlattr>.leaseDuration_ms","INF");
			if(auxstring == "INF")
				wdata->m_qos.m_liveliness.lease_duration = c_TimeInfinite;
			else
			{
			try
			{
				uint32_t milliseclease = boost::lexical_cast<uint32_t>(auxstring);
				wdata->m_qos.m_liveliness.lease_duration = TimeConv::MilliSeconds2Time_t((double)milliseclease);
			}
			#pragma warning(disable: 4101)
			catch(std::exception &e)
			{
				pWarning("BAD XML:livelinessQos leaseDuration is a bad number: "<<auxstring<<" setting to INF"<<endl);
				wdata->m_qos.m_liveliness.lease_duration = c_TimeInfinite;
			}
			}
		}
		else
		{
			pWarning("Unkown Endpoint-XML tag, ignoring "<< xml_endpoint_child.first<<endl)
		}
	}
	if(wdata->m_userDefinedId == 0)
	{
		pError("Writer XML endpoint with NO ID defined"<<endl);
		delete(wdata);
		return false;
	}
	pdata->m_writers.push_back(wdata);
	return true;
}

bool EDPStaticXML::lookforReader(std::string partname, uint16_t id,
		ReaderProxyData** rdataptr)
{
	for(std::vector<StaticParticipantInfo*>::iterator pit = m_participants.begin();
			pit!=m_participants.end();++pit)
	{
		if((*pit)->m_participantName == partname || true) //it doenst matter the name fo the participant, only for organizational purposes
		{
			for(std::vector<ReaderProxyData*>::iterator rit = (*pit)->m_readers.begin();
					rit!=(*pit)->m_readers.end();++rit)
			{
				if((*rit)->m_userDefinedId == id)
				{
					*rdataptr = *rit;
					return true;
				}
			}
		}
	}
	return false;
}

bool EDPStaticXML::lookforWriter(std::string partname, uint16_t id,
		WriterProxyData** wdataptr)
{
	for(std::vector<StaticParticipantInfo*>::iterator pit = m_participants.begin();
			pit!=m_participants.end();++pit)
	{
		if((*pit)->m_participantName == partname || true) //it doenst matter the name fo the participant, only for organizational purposes
		{
			for(std::vector<WriterProxyData*>::iterator wit = (*pit)->m_writers.begin();
					wit!=(*pit)->m_writers.end();++wit)
			{
				if((*wit)->m_userDefinedId == id)
				{
					*wdataptr = *wit;
					return true;
				}
			}
		}
	}
	return false;
}






} /* namespace rtps */
} /* namespace eprosima */


