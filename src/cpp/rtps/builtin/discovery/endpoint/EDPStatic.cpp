/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPStatic.cpp
 *
 */

#include "fastrtps/rtps/builtin/discovery/endpoint/EDPStatic.h"
#include "fastrtps/rtps/builtin/discovery/endpoint/EDPStaticXML.h"
#include "fastrtps/rtps/builtin/discovery/participant/PDPSimple.h"

#include "fastrtps/rtps/builtin/data/WriterProxyData.h"
#include "fastrtps/rtps/builtin/data/ReaderProxyData.h"
#include "fastrtps/rtps/builtin/data/ParticipantProxyData.h"


#include "fastrtps/rtps/reader/RTPSReader.h"
#include "fastrtps/rtps/writer/RTPSWriter.h"

#include "fastrtps/utils/RTPSLog.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <sstream>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "EDPStatic";

EDPStatic::EDPStatic(PDPSimple* p,RTPSParticipantImpl* part):
						EDP(p,part),
						mp_edpXML(nullptr)
{


}

EDPStatic::~EDPStatic()
{
	if(mp_edpXML != nullptr)
		delete(mp_edpXML);
}

bool EDPStatic::initEDP(BuiltinAttributes& attributes)
{
	const char* const METHOD_NAME = "initEDP";
	logInfo(RTPS_EDP,"Beginning STATIC EndpointDiscoveryProtocol",C_B_CYAN);
	m_attributes = attributes;
	mp_edpXML = new EDPStaticXML();
	std::string filename = std::string(m_attributes.getStaticEndpointXMLFilename());
	return this->mp_edpXML->loadXMLFile(filename);
}

std::pair<std::string,std::string> EDPStaticProperty::toProperty(std::string type,std::string status,uint16_t id,const EntityId_t& ent)
{
	std::pair<std::string,std::string> prop;
	std::stringstream ss;
	ss << "eProsimaEDPStatic_"<<type<<"_"<<status<<"_ID_"<<id;
	prop.first = ss.str();
	ss.clear();
	ss.str(std::string());
	ss << (int)ent.value[0]<<".";
	ss << (int)ent.value[1]<<".";
	ss << (int)ent.value[2]<<".";
	ss << (int)ent.value[3];
	prop.second = ss.str();
	return prop;
}

bool EDPStaticProperty::fromProperty(std::pair<std::string,std::string> prop)
{
	if(prop.first.substr(0,17) == "eProsimaEDPStatic" && prop.first.substr(31,2) == "ID")
	{
		this->m_endpointType = prop.first.substr(18,6);
		this->m_status = prop.first.substr(25,5);
		this->m_userIdStr = prop.first.substr(34,100);
		std::stringstream ss;
		ss << m_userIdStr;
		ss >> m_userId;
		ss.clear();
		ss.str(std::string());
		ss << prop.second;
		int a,b,c,d;
		char ch;
		ss >> a >> ch >> b >> ch >> c >>ch >> d;
		m_entityId.value[0] = (octet)a;m_entityId.value[1] = (octet)b;
		m_entityId.value[2] = (octet)c;m_entityId.value[3] = (octet)d;
		return true;
	}
	return false;
}



bool EDPStatic::processLocalReaderProxyData(ReaderProxyData* rdata)
{
	const char* const METHOD_NAME = "processLocalReaderProxyData";
	logInfo(RTPS_EDP,rdata->m_guid.entityId<< " in topic: " <<rdata->m_topicName,C_CYAN);
	//Add the property list entry to our local pdp
	ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
	boost::lock_guard<boost::recursive_mutex> guard(*localpdata->mp_mutex);
	localpdata->m_properties.properties.push_back(EDPStaticProperty::toProperty("Reader","ALIVE",rdata->m_userDefinedId,rdata->m_guid.entityId));
	localpdata->m_hasChanged = true;
	this->mp_PDP->announceParticipantState(true);
	return true;
}

bool EDPStatic::processLocalWriterProxyData(WriterProxyData* wdata)
{
	const char* const METHOD_NAME = "processLocalWriterProxyData";
		logInfo(RTPS_EDP,wdata->m_guid.entityId<< " in topic: " <<wdata->m_topicName,C_CYAN);
	//Add the property list entry to our local pdp
	ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
	boost::lock_guard<boost::recursive_mutex> guard(*localpdata->mp_mutex);
	localpdata->m_properties.properties.push_back(EDPStaticProperty::toProperty("Writer","ALIVE",
			wdata->m_userDefinedId,wdata->m_guid.entityId));
	localpdata->m_hasChanged = true;
	this->mp_PDP->announceParticipantState(true);
	return true;
}

bool EDPStatic::removeLocalReader(RTPSReader* R)
{
	ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
	boost::lock_guard<boost::recursive_mutex> guard(*localpdata->mp_mutex);
	for(std::vector<std::pair<std::string,std::string>>::iterator pit = localpdata->m_properties.properties.begin();
			pit!=localpdata->m_properties.properties.end();++pit)
	{
		EDPStaticProperty staticproperty;
		if(staticproperty.fromProperty(*pit))
		{
			if(staticproperty.m_entityId == R->getGuid().entityId)
			{
				*pit = EDPStaticProperty::toProperty("Reader","ENDED",R->getAttributes()->getUserDefinedID(),
						R->getGuid().entityId);
			}
		}
	}
	return false;
}

bool EDPStatic::removeLocalWriter(RTPSWriter*W)
{
	ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
	boost::lock_guard<boost::recursive_mutex> guard(*localpdata->mp_mutex);
	for(std::vector<std::pair<std::string,std::string>>::iterator pit = localpdata->m_properties.properties.begin();
			pit!=localpdata->m_properties.properties.end();++pit)
	{
		EDPStaticProperty staticproperty;
		if(staticproperty.fromProperty(*pit))
		{
			if(staticproperty.m_entityId == W->getGuid().entityId)
			{
				*pit = EDPStaticProperty::toProperty("Writer","ENDED",W->getAttributes()->getUserDefinedID(),
						W->getGuid().entityId);
			}
		}
	}
	return false;
}

void EDPStatic::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
	const char* const METHOD_NAME = "assignRemoteEndpoints";
	boost::lock_guard<boost::recursive_mutex> guard(*pdata->mp_mutex);
	for(std::vector<std::pair<std::string,std::string>>::iterator pit = pdata->m_properties.properties.begin();
			pit!=pdata->m_properties.properties.end();++pit)
	{
		//cout << "STATIC EDP READING PROPERTY " << pit->first << "// " << pit->second << endl;
		EDPStaticProperty staticproperty;
		if(staticproperty.fromProperty(*pit))
		{
			if(staticproperty.m_endpointType == "Reader" && staticproperty.m_status=="ALIVE")
			{
				ReaderProxyData* rdata=NULL;
				GUID_t guid(pdata->m_guid.guidPrefix,staticproperty.m_entityId);
				if(!this->mp_PDP->lookupReaderProxyData(guid,&rdata))//IF NOT FOUND, we CREATE AND PAIR IT
				{
					newRemoteReader(pdata,staticproperty.m_userId,staticproperty.m_entityId);
				}
			}
			else if(staticproperty.m_endpointType == "Writer" && staticproperty.m_status == "ALIVE")
			{
				WriterProxyData* wdata=NULL;
				GUID_t guid(pdata->m_guid.guidPrefix,staticproperty.m_entityId);
				if(!this->mp_PDP->lookupWriterProxyData(guid,&wdata))//IF NOT FOUND, we CREATE AND PAIR IT
				{
					newRemoteWriter(pdata,staticproperty.m_userId,staticproperty.m_entityId);
				}
			}
			else if(staticproperty.m_endpointType == "Reader" && staticproperty.m_status == "ENDED")
			{
				GUID_t guid(pdata->m_guid.guidPrefix,staticproperty.m_entityId);
				this->removeReaderProxy(guid);
			}
			else if(staticproperty.m_endpointType == "Writer" && staticproperty.m_status == "ENDED")
			{
				GUID_t guid(pdata->m_guid.guidPrefix,staticproperty.m_entityId);
				this->removeWriterProxy(guid);
			}
			else
			{
				logWarning(RTPS_EDP,"Property with type: "<<staticproperty.m_endpointType
						<< " and status "<<staticproperty.m_status << " not recognized",C_CYAN);
			}
		}
		else
		{

		}
	}
}

bool EDPStatic::newRemoteReader(ParticipantProxyData* pdata,uint16_t userId,EntityId_t entId)
{
	const char* const METHOD_NAME = "newRemoteReader";
	ReaderProxyData* rpd = NULL;
	if(mp_edpXML->lookforReader(pdata->m_participantName,userId,&rpd))
	{
		logInfo(RTPS_EDP,"Activating: " << rpd->m_guid.entityId << " in topic " << rpd->m_topicName,C_CYAN);
		ReaderProxyData* newRPD = new ReaderProxyData();
		newRPD->copy(rpd);
		newRPD->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		if(entId != c_EntityId_Unknown)
			newRPD->m_guid.entityId = entId;
		if(!checkEntityId(newRPD))
		{
			logError(RTPS_EDP,"The provided entityId for Reader with ID: "
					<< newRPD->m_userDefinedId << " does not match the topic Kind",C_CYAN);
			delete(newRPD);
			return false;
		}
		newRPD->m_key = newRPD->m_guid;
		newRPD->m_RTPSParticipantKey = pdata->m_guid;
		if(this->mp_PDP->addReaderProxyData(newRPD,false))
		{
			//CHECK the locators:
			if(newRPD->m_unicastLocatorList.empty() && newRPD->m_multicastLocatorList.empty())
			{
				newRPD->m_unicastLocatorList = pdata->m_defaultUnicastLocatorList;
				newRPD->m_multicastLocatorList = pdata->m_defaultMulticastLocatorList;
			}
			newRPD->m_isAlive = true;
			this->pairingReaderProxy(newRPD);
			return true;
		}
	}
	return false;
}

bool EDPStatic::newRemoteWriter(ParticipantProxyData* pdata,uint16_t userId,EntityId_t entId)
{
	const char* const METHOD_NAME = "newRemoteWriter";
	WriterProxyData* wpd = NULL;
	if(mp_edpXML->lookforWriter(pdata->m_participantName,userId,&wpd))
	{
		logInfo(RTPS_EDP,"Activating: " << wpd->m_guid.entityId << " in topic " << wpd->m_topicName,C_CYAN);
		WriterProxyData* newWPD = new WriterProxyData();
		newWPD->copy(wpd);
		newWPD->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		if(entId != c_EntityId_Unknown)
			newWPD->m_guid.entityId = entId;
		if(!checkEntityId(newWPD))
		{
			logError(RTPS_EDP,"The provided entityId for Writer with User ID: "
					<< newWPD->m_userDefinedId << " does not match the topic Kind");
			delete(newWPD);
			return false;
		}
		newWPD->m_key = newWPD->m_guid;
		newWPD->m_RTPSParticipantKey = pdata->m_guid;
		if(this->mp_PDP->addWriterProxyData(newWPD,false))
		{
			//CHECK the locators:
			if(newWPD->m_unicastLocatorList.empty() && newWPD->m_multicastLocatorList.empty())
			{
				newWPD->m_unicastLocatorList = pdata->m_defaultUnicastLocatorList;
				newWPD->m_multicastLocatorList = pdata->m_defaultMulticastLocatorList;
			}
			newWPD->m_isAlive = true;
			this->pairingWriterProxy(newWPD);
			return true;
		}
	}
	return false;
}

bool EDPStatic::checkEntityId(ReaderProxyData* rdata)
{
	if(rdata->m_topicKind == WITH_KEY && rdata->m_guid.entityId.value[3] == 0x07)
		return true;
	if(rdata->m_topicKind == NO_KEY && rdata->m_guid.entityId.value[3] == 0x04)
		return true;
	return false;
}

bool EDPStatic::checkEntityId(WriterProxyData* wdata)
{
	if(wdata->m_topicKind == WITH_KEY && wdata->m_guid.entityId.value[3] == 0x02)
		return true;
	if(wdata->m_topicKind == NO_KEY && wdata->m_guid.entityId.value[3] == 0x03)
		return true;
	return false;
}


}
} /* namespace rtps */
} /* namespace eprosima */
