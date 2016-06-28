// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
 * RTPSDomain.cpp
 *
 */

#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include "participant/RTPSParticipantImpl.h"

#include <fastrtps/utils/RTPSLog.h>

#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/utils/md5.h>

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/reader/RTPSReader.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "RTPSDomain";


uint32_t RTPSDomain::m_maxRTPSParticipantID = 0;
std::vector<RTPSDomain::t_p_RTPSParticipant> RTPSDomain::m_RTPSParticipants;
std::set<uint32_t> RTPSDomain::m_RTPSParticipantIDs;



RTPSDomain::RTPSDomain()
{
	srand (static_cast <unsigned> (time(0)));
}

RTPSDomain::~RTPSDomain()
{

}

void RTPSDomain::stopAll()
{
	const char* const METHOD_NAME = "~RTPSDomain";
	logInfo(RTPS_PARTICIPANT,"DELETING ALL ENDPOINTS IN THIS DOMAIN",C_WHITE);

	while(m_RTPSParticipants.size()>0)
	{
		RTPSDomain::removeRTPSParticipant(m_RTPSParticipants.begin()->first);
	}
	logInfo(RTPS_PARTICIPANT,"RTPSParticipants deleted correctly ");
	eClock::my_sleep(100);
}

RTPSParticipant* RTPSDomain::createParticipant(RTPSParticipantAttributes& PParam,
		RTPSParticipantListener* listen)
{
	const char* const METHOD_NAME = "createParticipant";
	logInfo(RTPS_PARTICIPANT,"");

	if(PParam.builtin.leaseDuration < c_TimeInfinite && PParam.builtin.leaseDuration <= PParam.builtin.leaseDuration_announcementperiod) //TODO CHeckear si puedo ser infinito
	{
		logError(RTPS_PARTICIPANT,"RTPSParticipant Attributes: LeaseDuration should be >= leaseDuration announcement period");
		return nullptr;
	}
	if(PParam.use_IP4_to_send == false && PParam.use_IP6_to_send == false)
	{
		logError(RTPS_PARTICIPANT,"Use IP4 OR User IP6 to send must be set to true");
		return nullptr;
	}
	uint32_t ID;
	if(PParam.participantID < 0)
	{
		ID = getNewId();
		while(m_RTPSParticipantIDs.insert(ID).second == false)
			ID = getNewId();
	}
	else
	{
		ID = PParam.participantID;
		if(m_RTPSParticipantIDs.insert(ID).second == false)
		{
			logError(RTPS_PARTICIPANT,"RTPSParticipant with the same ID already exists");
			return nullptr;
		}
	}
	if(!PParam.defaultUnicastLocatorList.isValid())
	{
		logError(RTPS_PARTICIPANT,"Default Unicast Locator List contains invalid Locator");
		return nullptr;
	}
	if(!PParam.defaultMulticastLocatorList.isValid())
	{
		logError(RTPS_PARTICIPANT,"Default Multicast Locator List contains invalid Locator");
		return nullptr;
	}
	PParam.participantID = ID;
	int pid;
#if defined(_WIN32)
	pid = (int)_getpid();
#else
	pid = (int)getpid();
#endif
	GuidPrefix_t guidP;
	LocatorList_t loc;
	IPFinder::getIP4Address(&loc);
	if(loc.size()>0)
	{
		MD5 md5;
		for(auto& l : loc)
		{
			md5.update(l.address, sizeof(l.address));
		}
		md5.finalize();
		guidP.value[0] = c_VendorId_eProsima[0];
		guidP.value[1] = c_VendorId_eProsima[1];
		guidP.value[2] = md5.digest[0];
		guidP.value[3] = md5.digest[1];
	}
	else
	{
		guidP.value[0] = c_VendorId_eProsima[0];
		guidP.value[1] = c_VendorId_eProsima[1];
		guidP.value[2] = 127;
		guidP.value[3] = 1;
	}
	guidP.value[4] = ((octet*)&pid)[0];
	guidP.value[5] = ((octet*)&pid)[1];
	guidP.value[6] = ((octet*)&pid)[2];
	guidP.value[7] = ((octet*)&pid)[3];
	guidP.value[8] = ((octet*)&ID)[0];
	guidP.value[9] = ((octet*)&ID)[1];
	guidP.value[10] = ((octet*)&ID)[2];
	guidP.value[11] = ((octet*)&ID)[3];

	RTPSParticipant* p = new RTPSParticipant(nullptr);
	RTPSParticipantImpl* pimpl = new RTPSParticipantImpl(PParam,guidP,p,listen);

	m_RTPSParticipants.push_back(t_p_RTPSParticipant(p,pimpl));
	return p;
}



bool RTPSDomain::removeRTPSParticipant(RTPSParticipant* p)
{
	const char* const METHOD_NAME = "removeRTPSParticipant";
	if(p!=nullptr)
	{
		for(auto it = m_RTPSParticipants.begin();it!= m_RTPSParticipants.end();++it)
		{
			if(it->second->getGuid().guidPrefix == p->getGuid().guidPrefix)
			{
				m_RTPSParticipantIDs.erase(m_RTPSParticipantIDs.find(it->second->getRTPSParticipantID()));
				delete(it->second);
				m_RTPSParticipants.erase(it);
				return true;
			}
		}
	}
	logError(RTPS_PARTICIPANT,"RTPSParticipant not valid or not recognized");
	return false;
}

RTPSWriter* RTPSDomain::createRTPSWriter(RTPSParticipant* p, WriterAttributes& watt, WriterHistory* hist, WriterListener* listen)
{
	for(auto it= m_RTPSParticipants.begin();it!=m_RTPSParticipants.end();++it)
	{
		if(it->first->getGuid().guidPrefix == p->getGuid().guidPrefix)
		{
			RTPSWriter* writ;
			if(it->second->createWriter(&writ,watt,hist,listen))
				return writ;
			return nullptr;
		}
	}
	return nullptr;
}

bool RTPSDomain::removeRTPSWriter(RTPSWriter* writer)
{
	if(writer!=nullptr)
	{
		for(auto it= m_RTPSParticipants.begin();it!=m_RTPSParticipants.end();++it)
		{
			if(it->first->getGuid().guidPrefix == writer->getGuid().guidPrefix)
			{
				return it->second->deleteUserEndpoint((Endpoint*)writer);
			}
		}
	}
	return false;
}

RTPSReader* RTPSDomain::createRTPSReader(RTPSParticipant* p, ReaderAttributes& ratt,
		ReaderHistory* rhist, ReaderListener* rlisten)
{
	for(auto it= m_RTPSParticipants.begin();it!=m_RTPSParticipants.end();++it)
	{
		if(it->first->getGuid().guidPrefix == p->getGuid().guidPrefix)
		{
			RTPSReader* reader;
			if(it->second->createReader(&reader,ratt,rhist,rlisten))
				return reader;
			return nullptr;
		}
	}
	return nullptr;
}

bool RTPSDomain::removeRTPSReader(RTPSReader* reader)
{
	if(reader !=  nullptr)
	{
		for(auto it= m_RTPSParticipants.begin();it!=m_RTPSParticipants.end();++it)
		{
			if(it->first->getGuid().guidPrefix == reader->getGuid().guidPrefix)
			{
				return it->second->deleteUserEndpoint((Endpoint*)reader);
			}
		}
	}
	return false;
}


}
} /* namespace  */
} /* namespace eprosima */



