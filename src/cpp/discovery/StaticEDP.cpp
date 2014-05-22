/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StaticEDP.cpp
 *
 *  Created on: May 16, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/StaticEDP.h"
#include "eprosimartps/discovery/ParticipantDiscoveryProtocol.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/writer/RTPSWriter.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/writer/StatelessWriter.h"



namespace eprosima {
namespace rtps {
using boost::property_tree::ptree;

StaticEDP::StaticEDP(ParticipantDiscoveryProtocol*p):
	EndpointDiscoveryProtocol(p){
	// TODO Auto-generated constructor stub

}

StaticEDP::~StaticEDP() {
	// TODO Auto-generated destructor stub
}

bool StaticEDP::initEDP(DiscoveryAttributes& attributes)
{
	this->m_discovery = attributes;

	loadXMLFile(m_discovery.m_staticEndpointXMLFilename);

	return true;
}


bool StaticEDP::loadXMLFile(const std::string& filename)
{
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

	m_endpointIds.clear();
	BOOST_FOREACH(ptree::value_type& xml_participant ,pt.get_child("staticdiscovery"))
	{
		if(xml_participant.first == "participant")
		{
			DiscoveredParticipantData pdata;
			DiscoveredParticipantData* p_pdata = &pdata;
			bool different_participant = true;
			BOOST_FOREACH(ptree::value_type& xml_participant_child,xml_participant.second)
			{
				if(xml_participant_child.first == "name")
				{
					pdata.m_participantName = xml_participant_child.second.data();
					if(pdata.m_participantName == this->mp_participant->m_participantName)
					{
						p_pdata = this->mp_PDP->mp_localDPData;
						different_participant = false;
					}
				}
				else if(xml_participant_child.first == "endpoint")
				{
					std::string auxString = xml_participant_child.second.get("<xmlattr>.type","");
					if(auxString == "READER")
					{
						loadXMLReaderEndpoint(xml_participant_child,p_pdata);
					}
					else if(auxString == "WRITER")
					{
						loadXMLWriterEndpoint(xml_participant_child,p_pdata);
					}
					else
					{
						pError("Endpoint must be defined as READER or WRITER"<<endl);
					}
				}
			}
			if(different_participant)
				mp_PDP->m_discoveredParticipants.push_back(pdata);
		}
	}
	return true;
}

bool StaticEDP::loadXMLWriterEndpoint(ptree::value_type& xml_endpoint,DiscoveredParticipantData* pdata)
{
	DiscoveredWriterData wdata;
	BOOST_FOREACH(ptree::value_type& xml_endpoint_child,xml_endpoint.second)
	{
		if(xml_endpoint_child.first == "id")
		{
			int16_t id = boost::lexical_cast<int16_t>(xml_endpoint_child.second.data());
			if(!std::binary_search(m_endpointIds.begin(),m_endpointIds.end(),id))
			{
				wdata.userDefinedId= id;
				m_endpointIds.push_back(id);
			}
			else
			{
				pError("Repeated ID in XML File"<<endl);
				return false;
			}
		}
		else if(xml_endpoint_child.first == "expectsInlineQos")
		{
			pWarning("Writers don't use expectsInlineQos tag"<<endl);
		}
		else if(xml_endpoint_child.first == "topicName")
		{
			wdata.m_topicName = (std::string)xml_endpoint_child.second.data();
		}
		else if(xml_endpoint_child.first == "topicDataType")
		{
			wdata.m_typeName = (std::string)xml_endpoint_child.second.data();
		}
		else if(xml_endpoint_child.first == "topicKind")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "NO_KEY")
				wdata.topicKind = NO_KEY;
			else if (auxString == "WITH_KEY")
				wdata.topicKind = WITH_KEY;
			else
			{
				pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else if(xml_endpoint_child.first == "reliabilityKind")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "RELIABLE")
				wdata.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
			else if (auxString == "BEST_EFFORT")
				wdata.m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
			else
			{
				pError("Bad XML file, endpoint of stateKind: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else if(xml_endpoint_child.first == "unicastLocator")
		{
			Locator_t loc;
			loc.kind = 1;
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
			loc.set_IP4_address(auxString);
			loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
			wdata.m_writerProxy.unicastLocatorList.push_back(loc);
		}
		else if(xml_endpoint_child.first == "multicastLocator")
		{
			Locator_t loc;
			loc.kind = 1;
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
			loc.set_IP4_address(auxString);
			loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
			wdata.m_writerProxy.multicastLocatorList.push_back(loc);
		}
		else if(xml_endpoint_child.first == "topic")
		{
			wdata.m_topicName = xml_endpoint_child.second.get("<xmlattr>.name","");
			wdata.m_typeName = xml_endpoint_child.second.get("<xmlattr>.dataType","");
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.kind","");
			if(auxString == "NO_KEY")
				wdata.topicKind = NO_KEY;
			else if (auxString == "WITH_KEY")
				wdata.topicKind = WITH_KEY;
			else
			{
				pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else
		{
			pWarning("Unkown Endpoint-XML tag, ignoring..."<<endl)
		}
	}
	pdata->m_writers.push_back(wdata);
	return true;
}

bool StaticEDP::loadXMLReaderEndpoint(ptree::value_type& xml_endpoint,DiscoveredParticipantData* pdata)
{
	DiscoveredReaderData rdata;
	BOOST_FOREACH(ptree::value_type& xml_endpoint_child,xml_endpoint.second)
	{
		if(xml_endpoint_child.first == "id")
		{
			int16_t id = boost::lexical_cast<int16_t>(xml_endpoint_child.second.data());
			if(!std::binary_search(m_endpointIds.begin(),m_endpointIds.end(),id))
			{
				rdata.userDefinedId= id;
				m_endpointIds.push_back(id);
			}
			else
			{
				pError("Repeated ID in XML File"<<endl);
				return false;
			}
		}
		else if(xml_endpoint_child.first == "expectsInlineQos")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "true")
				rdata.m_readerProxy.expectsInlineQos = true;
			else if (auxString == "false")
				rdata.m_readerProxy.expectsInlineQos = false;
			else
			{
				pError("Bad XML file, endpoint of expectsInlineQos: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else if(xml_endpoint_child.first == "topicName")
		{
			rdata.m_topicName = (std::string)xml_endpoint_child.second.data();
		}
		else if(xml_endpoint_child.first == "topicDataType")
		{
			rdata.m_typeName = (std::string)xml_endpoint_child.second.data();
		}
		else if(xml_endpoint_child.first == "topicKind")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "NO_KEY")
				rdata.topicKind = NO_KEY;
			else if (auxString == "WITH_KEY")
				rdata.topicKind = WITH_KEY;
			else
			{
				pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else if(xml_endpoint_child.first == "reliabilityKind")
		{
			std::string auxString = (std::string)xml_endpoint_child.second.data();
			if(auxString == "RELIABLE")
				rdata.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
			else if (auxString == "BEST_EFFORT")
				rdata.m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
			else
			{
				pError("Bad XML file, endpoint of stateKind: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else if(xml_endpoint_child.first == "unicastLocator")
		{
			Locator_t loc;
			loc.kind = 1;
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
			loc.set_IP4_address(auxString);
			loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
			rdata.m_readerProxy.unicastLocatorList.push_back(loc);
		}
		else if(xml_endpoint_child.first == "multicastLocator")
		{
			Locator_t loc;
			loc.kind = 1;
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
			loc.set_IP4_address(auxString);
			loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
			rdata.m_readerProxy.multicastLocatorList.push_back(loc);
		}
		else if(xml_endpoint_child.first == "topic")
		{
			rdata.m_topicName = xml_endpoint_child.second.get("<xmlattr>.name","");
			rdata.m_typeName = xml_endpoint_child.second.get("<xmlattr>.dataType","");
			std::string auxString = xml_endpoint_child.second.get("<xmlattr>.kind","");
			if(auxString == "NO_KEY")
				rdata.topicKind = NO_KEY;
			else if (auxString == "WITH_KEY")
				rdata.topicKind = WITH_KEY;
			else
			{
				pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
				break;
			}
		}
		else
		{
			pWarning("Unkown Endpoint-XML tag, ignoring..."<<endl)
		}
	}
	pdata->m_readers.push_back(rdata);
	return true;
}

bool StaticEDP::localWriterMatching(RTPSWriter* writer,bool first_time)
{
	pInfo(MAGENTA "Matching local WRITER"<<endl);
	bool matched_global = false;
	for(std::vector<DiscoveredParticipantData>::iterator pit = this->mp_PDP->m_discoveredParticipants.begin();
			pit!=this->mp_PDP->m_discoveredParticipants.end();++pit)
	{
		for(std::vector<DiscoveredReaderData>::iterator it = pit->m_readers.begin();
				it!= pit->m_readers.end();++it)
		{
			if(writer->getTopicName() == it->m_topicName &&
					writer->getTopicKind() == it->topicKind &&
					writer->getTopicDataType() == it->m_typeName &&
					it->isAlive && it->userDefinedId>0) //Matching
			{
				bool matched = false;
				if(writer->getStateType() == STATELESS && it->m_qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS)
				{
					StatelessWriter* p_SLW = (StatelessWriter*)writer;
					ReaderLocator RL;
					RL.expectsInlineQos = it->m_readerProxy.expectsInlineQos;
					for(std::vector<Locator_t>::iterator lit = it->m_readerProxy.unicastLocatorList.begin();
							lit != it->m_readerProxy.unicastLocatorList.end();++lit)
					{
						//cout << "added unicast RL to my STATELESSWRITER"<<endl;
						RL.locator = *lit;
						if(p_SLW->reader_locator_add(RL))
							matched =true;
					}
					for(std::vector<Locator_t>::iterator lit = it->m_readerProxy.multicastLocatorList.begin();
							lit != it->m_readerProxy.multicastLocatorList.end();++lit)
					{
						RL.locator = *lit;
						if(p_SLW->reader_locator_add(RL))
							matched = true;
					}
				}
				else if(writer->getStateType() == STATEFUL)
				{
					StatefulWriter* p_SFW = (StatefulWriter*)writer;
					if(p_SFW->matched_reader_add(it->m_readerProxy))
						matched = true;
				}
				if(matched && writer->mp_listener!=NULL)
					writer->mp_listener->onPublicationMatched();
				matched_global = true;
			}
		}
	}

	return matched_global;
}


bool StaticEDP::localReaderMatching(RTPSReader* reader,bool first_time)
{
	pInfo(MAGENTA "Matching local WRITER"<<endl);
	bool matched_global = false;
	for(std::vector<DiscoveredParticipantData>::iterator pit = this->mp_PDP->m_discoveredParticipants.begin();
			pit!=this->mp_PDP->m_discoveredParticipants.end();++pit)
	{
		for(std::vector<DiscoveredWriterData>::iterator it = pit->m_writers.begin();
				it!=pit->m_writers.end();++it)
		{
			if(reader->getTopicName() == it->m_topicName &&
					reader->getTopicKind() == it->topicKind &&
					reader->getTopicDataType() == it->m_typeName &&
					it->isAlive && it->userDefinedId>0) //Matching
			{
				bool matched = false;
				if(reader->getStateType() == STATELESS)
				{

				}
				else if(reader->getStateType() == STATEFUL)
				{
					StatefulReader* p_SFR = (StatefulReader*)reader;
					if(p_SFR->matched_writer_add(&it->m_writerProxy))
						matched = true;
				}
				if(matched && reader->mp_listener!=NULL)
					reader->mp_listener->onSubscriptionMatched();
				matched_global = true;
			}
		}
	}
	return matched_global;
}


bool StaticEDP::checkLocalWriterCreation(RTPSWriter* writer)
{
	for(std::vector<DiscoveredWriterData>::iterator it = this->mp_PDP->mp_localDPData->m_writers.begin();
			it!=this->mp_PDP->mp_localDPData->m_writers.begin();++it)
	{
		if(writer->m_userDefinedId == it->userDefinedId)
		{
			bool equal = true;
			equal &= (writer->getTopicKind() == it->topicKind);
			if(!equal)
			{
				pWarning("Topic Kind different in XML" <<endl);
			}
			equal &= (writer->getTopicName() == it->m_topicName);
			if(!equal)
			{
				pWarning("Topic Name different in XML: " << writer->getTopicName() << " vs " <<it->m_topicName <<endl);
			}
			equal &= (writer->getTopicDataType() == it->m_typeName);
			if(!equal)
			{
				pWarning("Topic Data Type different in XML: " << writer->getTopicDataType() << " vs " <<it->m_typeName <<endl);
			}
			if(writer->getStateType() == STATELESS && it->m_qos.m_reliability.kind != BEST_EFFORT_RELIABILITY_QOS)
				equal &= false;
			if(writer->getStateType() == STATEFUL && it->m_qos.m_reliability.kind != RELIABLE_RELIABILITY_QOS)
				equal &= false;
			bool found;
			for(LocatorListIterator paramlit = writer->unicastLocatorList.begin();
					paramlit != writer->unicastLocatorList.end();++paramlit)
			{
				found = false;
				for(LocatorListIterator xmllit = it->m_writerProxy.unicastLocatorList.begin();
						xmllit!=it->m_writerProxy.unicastLocatorList.begin();++xmllit)
				{
					if(*paramlit == *xmllit)
					{
						found = true;
						break;
					}
				}
				if(!found)
				{
					pWarning("UnicastLocator: "<<paramlit->printIP4Port() << " not found in XML file"<<endl);
					equal &=false;
				}
			}
			for(LocatorListIterator paramlit = writer->multicastLocatorList.begin();
					paramlit != writer->multicastLocatorList.end();++paramlit)
			{
				found = false;
				for(LocatorListIterator xmllit = it->m_writerProxy.multicastLocatorList.begin();
						xmllit!=it->m_writerProxy.multicastLocatorList.begin();++xmllit)
				{
					if(*paramlit == *xmllit)
					{
						found = true;
						break;
					}
				}
				if(!found)
				{
					pWarning("UnicastLocator: "<<paramlit->printIP4Port() << " not found in XML file"<<endl);
					equal &=false;
				}
			}
			return equal;
		}
	}
	pWarning("Endpoint with ID: " << writer->m_userDefinedId << " not defined in XML file"<< endl);
	return false; //If the participant is found but the ID is not found.
}

bool StaticEDP::checkLocalReaderCreation(RTPSReader* reader)
{
	for(std::vector<DiscoveredReaderData>::iterator it = this->mp_PDP->mp_localDPData->m_readers.begin();
			it!=this->mp_PDP->mp_localDPData->m_readers.begin();++it)
	{
		if(reader->m_userDefinedId == it->userDefinedId)
		{
			bool equal = true;
			equal &= (reader->getTopicKind() == it->topicKind);
			if(!equal)
			{
				pWarning("Topic Kind different in XML" <<endl);
			}
			equal &= (reader->getTopicName() == it->m_topicName);
			if(!equal)
			{
				pWarning("Topic Name different in XML: " << reader->getTopicName() << " vs " <<it->m_topicName <<endl);
			}
			equal &= (reader->getTopicDataType() == it->m_typeName);
			if(!equal)
			{
				pWarning("Topic Data Type different in XML: " << reader->getTopicDataType() << " vs " <<it->m_typeName <<endl);
			}
			if(reader->getStateType() == STATELESS && it->m_qos.m_reliability.kind != BEST_EFFORT_RELIABILITY_QOS)
				equal &= false;
			if(reader->getStateType() == STATEFUL && it->m_qos.m_reliability.kind != RELIABLE_RELIABILITY_QOS)
				equal &= false;
			bool found;
			for(LocatorListIterator paramlit = reader->unicastLocatorList.begin();
					paramlit != reader->unicastLocatorList.end();++paramlit)
			{
				found = false;
				for(LocatorListIterator xmllit = it->m_readerProxy.unicastLocatorList.begin();
						xmllit!=it->m_readerProxy.unicastLocatorList.begin();++xmllit)
				{
					if(*paramlit == *xmllit)
					{
						found = true;
						break;
					}
				}
				if(!found)
				{
					pWarning("UnicastLocator: "<<paramlit->printIP4Port() << " not found in XML file"<<endl);
					equal &=false;
				}
			}
			for(LocatorListIterator paramlit = reader->multicastLocatorList.begin();
					paramlit != reader->multicastLocatorList.end();++paramlit)
			{
				found = false;
				for(LocatorListIterator xmllit = it->m_readerProxy.multicastLocatorList.begin();
						xmllit!=it->m_readerProxy.multicastLocatorList.begin();++xmllit)
				{
					if(*paramlit == *xmllit)
					{
						found = true;
						break;
					}
				}
				if(!found)
				{
					pWarning("UnicastLocator: "<<paramlit->printIP4Port() << " not found in XML file"<<endl);
					equal &=false;
				}
			}
			return equal;
		}
	}
	pWarning("Endpoint with ID: " << reader->m_userDefinedId << " not defined in XML file"<< endl);
	return false; //If the participant is found but the ID is not found.
}




} /* namespace rtps */
} /* namespace eprosima */


