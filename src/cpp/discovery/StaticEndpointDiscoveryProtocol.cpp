/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StaticEndpointDiscoveryProtocol.cpp
 *
 *  Created on: Apr 23, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/StaticEndpointDiscoveryProtocol.h"


#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/Participant.h"

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace eprosima {
namespace rtps {

StaticEndpointDiscoveryProtocol::StaticEndpointDiscoveryProtocol(Participant* p_par):
														mp_Participant(p_par)
{
	ParticipantStaticInfo_t pinfo;
	pinfo.m_name = p_par->m_participantName;
	this->m_StaticParticipantInfo.push_back(pinfo);
}

StaticEndpointDiscoveryProtocol::~StaticEndpointDiscoveryProtocol() {
	// TODO Auto-generated destructor stub
}

bool StaticEndpointDiscoveryProtocol::loadStaticEndpointFile()
{
	return loadStaticEndpointFile(this->m_staticEndpointFilename);
}



bool StaticEndpointDiscoveryProtocol::loadStaticEndpointFile(const std::string& filename)
{
	this->m_StaticParticipantInfo.clear();
	// Create an empty property tree object
	using boost::property_tree::ptree;
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
			ParticipantStaticInfo_t participantInfo;
			BOOST_FOREACH(ptree::value_type& xml_participant_child,xml_participant.second)
			{
				if(xml_participant_child.first == "name")
					participantInfo.m_name = xml_participant_child.second.data();
				else if(xml_participant_child.first == "endpoint")
				{
					EndpointStaticInfo_t endpointInfo;
					BOOST_FOREACH(ptree::value_type& xml_endpoint_child,xml_participant_child.second)
					{
						if(xml_endpoint_child.first == "id")
						{
							endpointInfo.m_id = boost::lexical_cast<int16_t>(xml_endpoint_child.second.data());
						}
						else if(xml_endpoint_child.first == "expectsInlineQos")
						{
							std::string auxString = (std::string)xml_endpoint_child.second.data();
							if(auxString == "true")
								endpointInfo.m_expectsInlineQos = true;
							else if (auxString == "false")
								endpointInfo.m_expectsInlineQos = false;
							else
							{
								pError("Bad XML file, endpoint of expectsInlineQos: " << auxString << " is not valid"<<endl);
								break;
							}
													}
						else if(xml_endpoint_child.first == "type")
						{
							std::string auxString = (std::string)xml_endpoint_child.second.data();
							if(auxString == "READER")
								endpointInfo.m_kind = READER;
							else if (auxString == "WRITER")
								endpointInfo.m_kind = WRITER;
							else
							{
								pError("Bad XML file, endpoint of type: " << auxString << " is not valid"<<endl);
								break;
							}

						}
						else if(xml_endpoint_child.first == "topicName")
						{
							endpointInfo.m_topicName = (std::string)xml_endpoint_child.second.data();
						}
						else if(xml_endpoint_child.first == "topicKind")
						{
							std::string auxString = (std::string)xml_endpoint_child.second.data();
							if(auxString == "NO_KEY")
								endpointInfo.m_topicKind = NO_KEY;
							else if (auxString == "WITH_KEY")
								endpointInfo.m_topicKind = WITH_KEY;
							else
							{
								pError("Bad XML file, topic of kind: " << auxString << " is not valid"<<endl);
								break;
							}
						}
						else if(xml_endpoint_child.first == "stateKind")
						{
							std::string auxString = (std::string)xml_endpoint_child.second.data();
							if(auxString == "STATELESS")
								endpointInfo.m_state = STATELESS;
							else if (auxString == "STATEFUL")
								endpointInfo.m_state = STATEFUL;
							else
							{
								pError("Bad XML file, endpoint of stateKind: " << auxString << " is not valid"<<endl);
								break;
							}
						}
						else if(xml_endpoint_child.first == "reliabilityKind")
						{
							std::string auxString = (std::string)xml_endpoint_child.second.data();
							if(auxString == "RELIABLE")
								endpointInfo.m_reliability = RELIABLE;
							else if (auxString == "BEST_EFFORT")
								endpointInfo.m_reliability = BEST_EFFORT;
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
							endpointInfo.m_unicastLocatorList.push_back(loc);
						}
						else if(xml_endpoint_child.first == "multicastLocator")
						{
							Locator_t loc;
							loc.kind = 1;
							std::string auxString = xml_endpoint_child.second.get("<xmlattr>.address","");
							loc.set_IP4_address(auxString);
							loc.port = xml_endpoint_child.second.get("<xmlattr>.port",0);
							endpointInfo.m_multicastLocatorList.push_back(loc);
						}
						else
						{
							pWarning("Unkown Endpoint-XML tag, ignoring..."<<endl)
						}
					}
					if((endpointInfo.m_reliability == RELIABLE && endpointInfo.m_state == STATELESS)
						|| (endpointInfo.m_reliability == BEST_EFFORT && endpointInfo.m_state == STATEFUL))
					{
						pWarning("Slected combination of reliability and statekind is not yet supported"<<endl);
					}
					else
					{
						participantInfo.m_endpoints.push_back(endpointInfo);
					}

				}

			}


			this->m_StaticParticipantInfo.push_back(participantInfo);
		}
	}
	printLoadedXMLInfo();
	return true;
}

bool StaticEndpointDiscoveryProtocol::printLoadedXMLInfo()
{
	pInfo("Printing Loaded XML Info"<<endl);
	pInfo("Number of participants: " <<this->m_StaticParticipantInfo.size()<<endl);
	std::string auxString;
	for(std::vector<ParticipantStaticInfo_t>::iterator pit =this->m_StaticParticipantInfo.begin();
			pit!=m_StaticParticipantInfo.end();++pit)
	{
		pInfo("Participant with name: " << pit->m_name <<" has " << pit->m_endpoints.size() << " endpoints"<<endl);
		for(std::vector<EndpointStaticInfo_t>::iterator eit =pit->m_endpoints.begin();
				eit!=pit->m_endpoints.end();++eit)
		{
			auxString = eit->m_state == STATELESS ? "STATELESS" : "STATEFUL";
			pLongInfo("Endoint " << auxString << " ");
			auxString = eit->m_reliability == RELIABLE ? "RELIABLE" : "BEST_EFFORT";
			pLongInfo(auxString << " ");
			auxString = eit->m_kind == READER ? "READER" : "WRITER";
			pLongInfo(auxString << " with id: " << eit->m_id << " expectsInlineQos: " << eit->m_expectsInlineQos <<endl);
			for(std::vector<Locator_t>::iterator lit = eit->m_unicastLocatorList.begin();
					lit!=eit->m_unicastLocatorList.end();++lit)
				pLongInfo("ULoc: " << lit->to_IP4_string() << ":" << lit->port << endl);

			for(std::vector<Locator_t>::iterator lit = eit->m_multicastLocatorList.begin();
					lit!=eit->m_multicastLocatorList.end();++lit)
				pLongInfo("MLoc: " << lit->to_IP4_string() << ":" << lit->port << endl);
			pLongInfoPrint;

		}
	}
	return true;
}



bool StaticEndpointDiscoveryProtocol::localEndpointMatching(Endpoint* endpoint, char type)
{
	//First we match agains our own participant:
	localEndpointMatching(endpoint,&mp_Participant->m_SPDP.m_DPD,type);
	for(std::vector<DiscoveredParticipantData*>::iterator it = mp_Participant->m_SPDP.m_matched_participants.begin();
			it!=mp_Participant->m_SPDP.m_matched_participants.end();++it)
	{
		localEndpointMatching(endpoint,*it,type);
	}
	return true;
}

bool StaticEndpointDiscoveryProtocol::localEndpointMatching(Endpoint* endpoint,DiscoveredParticipantData* dpd, char type)
{
	if(type == 'W')
	{
		localWriterMatching((RTPSWriter*)endpoint,dpd);
	}
	else if(type == 'R')
	{
		localReaderMatching((RTPSReader*)endpoint,dpd);
	}
	return false;
}

bool StaticEndpointDiscoveryProtocol::localWriterMatching(RTPSWriter* writer,DiscoveredParticipantData* dpd)
{
	pInfo(MAGENTA "Matching local WRITER"<<endl);
	std::string topic_name = writer->getTopicName();
	std::string remote_part_name = dpd->m_proxy.m_participantName;
	//Look in the participants defined by the StaticEndpointDiscovery.
	for(std::vector<ParticipantStaticInfo_t>::iterator remotepit = m_StaticParticipantInfo.begin();
			remotepit != m_StaticParticipantInfo.end();++remotepit)
	{
		if(remote_part_name == remotepit->m_name) // Found a match for the participant, begin pairing
		{
			for(std::vector<EndpointStaticInfo_t>::iterator eit = remotepit->m_endpoints.begin();
					eit!=remotepit->m_endpoints.end();++eit)
			{
				if(eit->m_kind == READER)
				{
					//look for real entityId in dpd
					bool found = false;
					EntityId_t readerId;
					for(std::vector<std::pair<uint16_t,EntityId_t>>::iterator entityit = dpd->m_staticedpEntityId.begin();
							entityit != dpd->m_staticedpEntityId.end();++entityit )
					{
						if(eit->m_id == entityit->first)
						{
							found = true;
							readerId = entityit->second;
							break;
						}
					}
					if(found)
					{
						if(writer->getStateType() == STATELESS)
						{
							StatelessWriter* p_SLW = (StatelessWriter*)writer;
							ReaderLocator RL;
							RL.expectsInlineQos = eit->m_expectsInlineQos;
							for(std::vector<Locator_t>::iterator lit = eit->m_unicastLocatorList.begin();
									lit != eit->m_unicastLocatorList.end();++lit)
							{
								cout << "added unicast RL to my STATELESSWRITER"<<endl;
								RL.locator = *lit;
								p_SLW->reader_locator_add(RL);
							}
							for(std::vector<Locator_t>::iterator lit = eit->m_multicastLocatorList.begin();
									lit != eit->m_multicastLocatorList.end();++lit)
							{
								RL.locator = *lit;
								p_SLW->reader_locator_add(RL);
							}
							if(writer->mp_listener!=NULL)
								writer->mp_listener->onPublicationMatched();
						}
						else if(writer->getStateType() == STATEFUL)
						{
							StatefulWriter* p_SFW = (StatefulWriter*)writer;
							ReaderProxy_t RP;
							RP.expectsInlineQos = eit->m_expectsInlineQos;
							RP.unicastLocatorList = eit->m_unicastLocatorList;
							RP.multicastLocatorList = eit->m_multicastLocatorList;
							RP.remoteReaderGuid.guidPrefix = dpd->m_proxy.m_guidPrefix;
							RP.remoteReaderGuid.entityId = readerId;
							p_SFW->matched_reader_add(RP);
							if(writer->mp_listener!=NULL)
								writer->mp_listener->onPublicationMatched();
						}
					}

				}
			}
			break;
		}
	}
	return true;
}

bool StaticEndpointDiscoveryProtocol::localReaderMatching(RTPSReader* reader,DiscoveredParticipantData* dpd)
{
	pInfo(MAGENTA "Matching local  READER"<<endl);
	std::string topic_name = reader->getTopicName();
	std::string remote_part_name = dpd->m_proxy.m_participantName;

	//Look in the participants defined by the StaticEndpointDiscovery.
	for(std::vector<ParticipantStaticInfo_t>::iterator remotepit = m_StaticParticipantInfo.begin();
			remotepit != m_StaticParticipantInfo.end();++remotepit)
	{
		if(remote_part_name == remotepit->m_name) // Found a match, begin pairing
		{
			for(std::vector<EndpointStaticInfo_t>::iterator eit = remotepit->m_endpoints.begin();
					eit!=remotepit->m_endpoints.end();++eit)
			{
				if(eit->m_kind == WRITER)
				{
					//look for real entityId in dpd
					bool found = false;
					EntityId_t writerId;
					for(std::vector<std::pair<uint16_t,EntityId_t>>::iterator entityit = dpd->m_staticedpEntityId.begin();
							entityit != dpd->m_staticedpEntityId.end();++entityit )
					{
						if(eit->m_id == entityit->first)
						{
							found = true;
							writerId = entityit->second;
							break;
						}
					}
					if(found)
					{
						if(reader->getStateType() == STATELESS)
						{
							if(reader->mp_listener!=NULL)
								reader->mp_listener->onSubscriptionMatched();
						}
						else if(reader->getStateType() == STATEFUL)
						{
							StatefulReader* p_SFR = (StatefulReader*)reader;
							WriterProxy_t WP;
							WP.unicastLocatorList = eit->m_unicastLocatorList;
							WP.multicastLocatorList = eit->m_multicastLocatorList;
							WP.remoteWriterGuid.guidPrefix = dpd->m_proxy.m_guidPrefix;
							WP.remoteWriterGuid.entityId = writerId;
							p_SFR->matched_writer_add(&WP);
							if(reader->mp_listener!=NULL)
								reader->mp_listener->onSubscriptionMatched();
						}
					}

				}
			}
		}
		break;
	}
	return true;
}




} /* namespace rtps */
} /* namespace eprosima */
