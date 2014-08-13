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
	std::pair<std::string,std::string> property;
	std::stringstream ss;
	ss << "eProsimaEDPStatic_"<<type<<"_"<<status<<"_ID_"<<id;
	property.first = ss.str();
	ss.clear();
	ss.str(std::string());
	ss << ent.value[0]<<".";
	ss << ent.value[1]<<".";
	ss << ent.value[2]<<".";
	ss << ent.value[3];
	property.second = ss.str();
	return property;
}

bool EDPStaticProperty::fromProperty(std::pair<std::string,std::string> property)
{
	size_t pos1 = property.first.find("_");
	str1 = property.first.substr(0,pos1);

	size_t pos2 = property.first.find("_",pos1+1);
	type = property.first.substr(pos1+1,pos2);

	pos1 = property.first.find("_",pos2+1);
	status = property.first.substr(pos2+1,pos1);

	pos2 = property.first.find("_",pos1+1);
	idstr = property.first.substr(pos1+1,pos2);

	userIDstr = property.first.substr(pos2+1);

	if(str1 == "eProsimaEDPStatic" && idstr == "ID")
	{
		std::stringstream ss;
		ss << userIDstr;
		ss >> userId;
		ss.clear();
		ss.str(std::string());
		ss << property.second;
		int a,b,c,d;
		char ch;
		ss >> a >> ch >> b >> ch >> c >>ch >> d;
		entityId.value[0] = (octet)a;entityId.value[1] = (octet)b;
		entityId.value[2] = (octet)c;entityId.value[3] = (octet)d;
		return true;
	}
	return false;
}



bool EDPStatic::processLocalReaderProxyData(ReaderProxyData* rdata)
{
	//Add the property list entry to our local pdp
	ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
	localpdata->m_properties.properties.push_back(EDPStaticProperty::toProperty("Reader","ALIVE",rdata->m_userDefinedId,rdata->m_guid.entityId));
	this->mp_PDP->announceParticipantState(true);
	return true;
}

bool EDPStatic::processLocalWriterProxyData(WriterProxyData* wdata)
{
	//Add the property list entry to our local pdp
	ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
	localpdata->m_properties.properties.push_back(EDPStaticProperty::toProperty("Writer","ALIVE",wdata->m_userDefinedId,wdata->m_guid.entityId));
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
			if(staticproperty.entityId == R->getGuid().entityId)
			{
				*pit = EDPStaticProperty::toProperty("Reader","DEAD",R->getUserDefinedId(),R->getGuid().entityId);
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
			if(staticproperty.entityId == W->getGuid().entityId)
			{
				*pit = EDPStaticProperty::toProperty("Writer","DEAD",W->getUserDefinedId(),W->getGuid().entityId);
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
		EDPStaticProperty staticproperty;
		if(staticproperty.fromProperty(*pit))
		{
			if(staticproperty.type == "Reader" && staticproperty.status=="ALIVE")
			{
				ReaderProxyData* rdata=NULL;
				GUID_t guid(pdata->m_guid.guidPrefix,staticproperty.entityId);
				if(!this->mp_PDP->lookupReaderProxyData(guid,&rdata))//IF NOT FOUND, we CREATE AND PAIR IT
				{
					newRemoteReader(pdata,staticproperty.userId,staticproperty.entityId);
				}
			}
			else if(staticproperty.type == "Writer" && staticproperty.status == "ALIVE")
			{
				WriterProxyData* wdata=NULL;
				GUID_t guid(pdata->m_guid.guidPrefix,staticproperty.entityId);
				if(!this->mp_PDP->lookupWriterProxyData(guid,&wdata))//IF NOT FOUND, we CREATE AND PAIR IT
				{
					newRemoteWriter(pdata,staticproperty.userId,staticproperty.entityId);
				}
			}
			else if(staticproperty.type == "Reader" && staticproperty.status == "DEAD")
			{
				GUID_t guid(pdata->m_guid.guidPrefix,staticproperty.entityId);
				this->removeReaderProxy(guid);
			}
			else if(staticproperty.type == "Writer" && staticproperty.status == "DEAD")
			{
				GUID_t guid(pdata->m_guid.guidPrefix,staticproperty.entityId);
				this->removeWriterProxy(guid);
			}
			else
			{
				pWarning("eProsimaEDPStatic property with type: "<<staticproperty.type << " and status "<<staticproperty.status << " not recognized"<<endl);
			}
		}
	}
}

bool EDPStatic::newRemoteReader(ParticipantProxyData* pdata,uint16_t userId,EntityId_t& entId)
{
	ReaderProxyData* rpd = NULL;
	if(m_edpXML.lookforReader(pdata->m_participantName,userId,&rpd))
	{
		ReaderProxyData* newRPD = new ReaderProxyData();
		newRPD->copy(rpd);
		newRPD->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		newRPD->m_guid.entityId = entId;
		newRPD->m_key = newRPD->m_guid;
		newRPD->m_participantKey = pdata->m_guid;
		if(this->mp_PDP->addReaderProxyData(newRPD,false))
		{
			//CHECK the locators:
			if(newRPD->m_unicastLocatorList.empty())
				newRPD->m_unicastLocatorList = pdata->m_defaultUnicastLocatorList;
			if(newRPD->m_multicastLocatorList.empty())
				newRPD->m_multicastLocatorList = pdata->m_defaultMulticastLocatorList;
			newRPD->m_isAlive = true;
			this->pairingReaderProxy(newRPD);
			return true;
		}
	}
	return false;
}

bool EDPStatic::newRemoteWriter(ParticipantProxyData* pdata,uint16_t userId,EntityId_t& entId)
{
	WriterProxyData* wpd = NULL;
	if(m_edpXML.lookforWriter(pdata->m_participantName,userId,&wpd))
	{
		WriterProxyData* newWPD = new WriterProxyData();
		newWPD->copy(wpd);
		newWPD->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		newWPD->m_guid.entityId = entId;
		newWPD->m_key = newWPD->m_guid;
		newWPD->m_participantKey = pdata->m_guid;
		if(this->mp_PDP->addWriterProxyData(newWPD,false))
		{
			//CHECK the locators:
			if(newWPD->m_unicastLocatorList.empty())
				newWPD->m_unicastLocatorList = pdata->m_defaultUnicastLocatorList;
			if(newWPD->m_multicastLocatorList.empty())
				newWPD->m_multicastLocatorList = pdata->m_defaultMulticastLocatorList;
			newWPD->m_isAlive = true;
			this->pairingWriterProxy(newWPD);
			return true;
		}
	}
	return false;
}



} /* namespace rtps */
} /* namespace eprosima */
