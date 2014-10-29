/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPStatic.cpp
 *
 */

#include "eprosimartps/builtin/discovery/endpoint/EDPStatic.h"
#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"

#include "eprosimartps/reader/WriterProxyData.h"
#include "eprosimartps/writer/ReaderProxyData.h"

#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/writer/RTPSWriter.h"

#include "eprosimartps/utils/RTPSLog.h"

#include <sstream>

namespace eprosima {
namespace rtps {

EDPStatic::EDPStatic(PDPSimple* p,ParticipantImpl* part):
																																																		EDP(p,part)
{


}

EDPStatic::~EDPStatic()
{

}

bool EDPStatic::initEDP(BuiltinAttributes& attributes)
{
	pInfo(RTPS_B_CYAN<<"Initializing  STATIC EndpointDiscoveryProtocol"<<endl);
	m_attributes = attributes;
	return this->m_edpXML.loadXMLFile(m_attributes.m_staticEndpointXMLFilename);
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
	pDebugInfo("EDPStatic: processing local ReaderPD"<<endl;);
	//Add the property list entry to our local pdp
	ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
	localpdata->m_properties.properties.push_back(EDPStaticProperty::toProperty("Reader","ALIVE",rdata->m_userDefinedId,rdata->m_guid.entityId));
	localpdata->m_hasChanged = true;
	this->mp_PDP->announceParticipantState(true);
	return true;
}

bool EDPStatic::processLocalWriterProxyData(WriterProxyData* wdata)
{
	pDebugInfo("EDPStatic: processing local WriterPD"<<endl;);
	//Add the property list entry to our local pdp
	ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
	localpdata->m_properties.properties.push_back(EDPStaticProperty::toProperty("Writer","ALIVE",wdata->m_userDefinedId,wdata->m_guid.entityId));
	localpdata->m_hasChanged = true;
	this->mp_PDP->announceParticipantState(true);
	return true;
}

bool EDPStatic::removeLocalReader(RTPSReader* R)
{
	ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
	for(std::vector<std::pair<std::string,std::string>>::iterator pit = localpdata->m_properties.properties.begin();
			pit!=localpdata->m_properties.properties.end();++pit)
	{
		EDPStaticProperty staticproperty;
		if(staticproperty.fromProperty(*pit))
		{
			if(staticproperty.m_entityId == R->getGuid().entityId)
			{
				*pit = EDPStaticProperty::toProperty("Reader","ENDED",R->getUserDefinedId(),R->getGuid().entityId);
			}
		}
	}
	return false;
}

bool EDPStatic::removeLocalWriter(RTPSWriter*W)
{
	ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
	for(std::vector<std::pair<std::string,std::string>>::iterator pit = localpdata->m_properties.properties.begin();
			pit!=localpdata->m_properties.properties.end();++pit)
	{
		EDPStaticProperty staticproperty;
		if(staticproperty.fromProperty(*pit))
		{
			if(staticproperty.m_entityId == W->getGuid().entityId)
			{
				*pit = EDPStaticProperty::toProperty("Writer","ENDED",W->getUserDefinedId(),W->getGuid().entityId);
			}
		}
	}
	return false;
}

void EDPStatic::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
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
				pWarning("eProsimaEDPStatic property with type: "<<staticproperty.m_endpointType << " and status "<<staticproperty.m_status << " not recognized"<<endl);
			}
		}
		else
		{

		}
	}
}

bool EDPStatic::newRemoteReader(ParticipantProxyData* pdata,uint16_t userId,EntityId_t entId)
{
	
	ReaderProxyData* rpd = NULL;
	if(m_edpXML.lookforReader(pdata->m_participantName,userId,&rpd))
	{
		pDebugInfo("Activating new Remote Reader " << rpd->m_guid.entityId << " in topic " << rpd->m_topicName<< endl;)
		ReaderProxyData* newRPD = new ReaderProxyData();
		newRPD->copy(rpd);
		newRPD->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		if(entId != c_EntityId_Unknown)
			newRPD->m_guid.entityId = entId;
		if(!checkEntityId(newRPD))
		{
			pError("The provided entityId for Reader with ID: " << newRPD->m_userDefinedId << " does not match the topic type" << endl;)
				delete(newRPD);
			return false;
		}
		newRPD->m_key = newRPD->m_guid;
		newRPD->m_participantKey = pdata->m_guid;
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
	
	WriterProxyData* wpd = NULL;
	if(m_edpXML.lookforWriter(pdata->m_participantName,userId,&wpd))
	{
		pDebugInfo("Activating new Remote Writer " << wpd->m_guid.entityId << " in topic " << wpd->m_topicName<< endl;)
		WriterProxyData* newWPD = new WriterProxyData();
		newWPD->copy(wpd);
		newWPD->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		if(entId != c_EntityId_Unknown)
			newWPD->m_guid.entityId = entId;
		if(!checkEntityId(newWPD))
		{
			pError("The provided entityId for Writer with User ID: " << newWPD->m_userDefinedId << " does not match the topic type" << endl;)
			delete(newWPD);
			return false;
		}
		newWPD->m_key = newWPD->m_guid;
		newWPD->m_participantKey = pdata->m_guid;
		cout << "WRITER WITH LOCATOR LISTS OF SIZES: " << newWPD->m_unicastLocatorList.size() << "  " << newWPD->m_multicastLocatorList.size() << endl;
		if(this->mp_PDP->addWriterProxyData(newWPD,false))
		{
			//CHECK the locators:
			if(newWPD->m_unicastLocatorList.empty() && newWPD->m_multicastLocatorList.empty())
			{
				cout << "WRITER WITH EMPTY LISTSSSSSSSSSSSSSS" << endl;
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

} /* namespace rtps */
} /* namespace eprosima */
