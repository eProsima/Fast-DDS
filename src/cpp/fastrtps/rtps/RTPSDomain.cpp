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

#include <fastrtps/log/Log.h>

#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/transport/UDPv6Transport.h>
#include <fastrtps/transport/test_UDPv4Transport.h>

#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/utils/System.h>
#include <fastrtps/utils/md5.h>

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/reader/RTPSReader.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

std::mutex RTPSDomain::m_mutex;
std::atomic<uint32_t> RTPSDomain::m_maxRTPSParticipantID(1);
std::vector<RTPSDomain::t_p_RTPSParticipant> RTPSDomain::m_RTPSParticipants;
std::set<uint32_t> RTPSDomain::m_RTPSParticipantIDs;

void RTPSDomain::stopAll()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    logInfo(RTPS_PARTICIPANT,"DELETING ALL ENDPOINTS IN THIS DOMAIN");

    while(m_RTPSParticipants.size()>0)
    {
        RTPSDomain::removeRTPSParticipant_nts(m_RTPSParticipants.begin());
    }
    logInfo(RTPS_PARTICIPANT,"RTPSParticipants deleted correctly ");
    eClock::my_sleep(100);
}

RTPSParticipant* RTPSDomain::createParticipant(
        const RTPSParticipantAttributes& attrs,
        RTPSParticipantListener* listen)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    logInfo(RTPS_PARTICIPANT,"");

    RTPSParticipantAttributes PParam = attrs;

    if(PParam.builtin.discovery_config.leaseDuration < c_TimeInfinite && PParam.builtin.discovery_config.leaseDuration <= PParam.builtin.discovery_config.leaseDuration_announcementperiod) //TODO CHeckear si puedo ser infinito
    {
        logError(RTPS_PARTICIPANT,"RTPSParticipant Attributes: LeaseDuration should be >= leaseDuration announcement period");
        return nullptr;
    }
    uint32_t ID;
    if(PParam.participantID < 0)
    {
        ID = getNewId();
        while(m_RTPSParticipantIDs.insert(ID).second == false)
        {
            ID = getNewId();
        }
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
    LocatorList_t loc;
    IPFinder::getIP4Address(&loc);

    if (loc.empty() && PParam.builtin.initialPeersList.empty())
    {
        Locator_t local;
        IPLocator::setIPv4(local, 127, 0, 0, 1);
        PParam.builtin.initialPeersList.push_back(local);
    }

    GuidPrefix_t guidP(PParam.prefix);

    if (guidP == c_GuidPrefix_Unknown)
    {
        // Make a new participant GuidPrefix_t up
        int pid = System::GetPID();

        guidP.value[0] = c_VendorId_eProsima[0];
        guidP.value[1] = c_VendorId_eProsima[1];

        if (loc.size() > 0)
        {
            MD5 md5;
            for (auto& l : loc)
            {
                md5.update(l.address, sizeof(l.address));
            }
            md5.finalize();
            uint16_t hostid = 0;
            for (size_t i = 0; i < sizeof(md5.digest); i += 2)
            {
                hostid ^= ((md5.digest[i] << 8) | md5.digest[i + 1]);
            }
            guidP.value[2] = octet(hostid);
            guidP.value[3] = octet(hostid >> 8);
        }
        else
        {
            guidP.value[2] = 127;
            guidP.value[3] = 1;
        }
        guidP.value[4] = octet(pid);
        guidP.value[5] = octet(pid >> 8);
        guidP.value[6] = octet(pid >> 16);
        guidP.value[7] = octet(pid >> 24);
        guidP.value[8] = octet(ID);
        guidP.value[9] = octet(ID >> 8);
        guidP.value[10] = octet(ID >> 16);
        guidP.value[11] = octet(ID >> 24);
    }
    
    RTPSParticipant* p = new RTPSParticipant(nullptr);
    RTPSParticipantImpl* pimpl = new RTPSParticipantImpl(PParam,guidP,p,listen);

#if HAVE_SECURITY
    // Check security was correctly initialized
    if (!pimpl->is_security_initialized())
    {
        logError(RTPS_PARTICIPANT, "Cannot create participant due to security initialization error");
        delete pimpl;
        return nullptr;
    }
#endif

    // Check there is at least one transport registered.
    if(!pimpl->networkFactoryHasRegisteredTransports())
    {
        logError(RTPS_PARTICIPANT,"Cannot create participant, because there is any transport");
        delete pimpl;
        return nullptr;
    }

    m_RTPSParticipants.push_back(t_p_RTPSParticipant(p,pimpl));
    return p;
}

bool RTPSDomain::removeRTPSParticipant(RTPSParticipant* p)
{
    if(p!=nullptr)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        for(auto it = m_RTPSParticipants.begin();it!= m_RTPSParticipants.end();++it)
        {
            if(it->second->getGuid().guidPrefix == p->getGuid().guidPrefix)
            {
                removeRTPSParticipant_nts(it);
                return true;
            }
        }
    }
    logError(RTPS_PARTICIPANT,"RTPSParticipant not valid or not recognized");
    return false;
}

void RTPSDomain::removeRTPSParticipant_nts(std::vector<RTPSDomain::t_p_RTPSParticipant>::iterator it)
{
    m_RTPSParticipantIDs.erase(m_RTPSParticipantIDs.find(it->second->getRTPSParticipantID()));
    delete(it->second);
    m_RTPSParticipants.erase(it);
}

RTPSWriter* RTPSDomain::createRTPSWriter(
        RTPSParticipant* p,
        WriterAttributes& watt,
        WriterHistory* hist,
        WriterListener* listen)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    for(auto it= m_RTPSParticipants.begin();it!=m_RTPSParticipants.end();++it)
    {
        if(it->first->getGuid().guidPrefix == p->getGuid().guidPrefix)
        {
            RTPSWriter* writ;
            if(it->second->createWriter(&writ,watt,hist,listen))
            {
                return writ;
            }
            return nullptr;
        }
    }
    return nullptr;
}

bool RTPSDomain::removeRTPSWriter(RTPSWriter* writer)
{
    if(writer!=nullptr)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
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

RTPSReader* RTPSDomain::createRTPSReader(
        RTPSParticipant* p,
        ReaderAttributes& ratt,
        ReaderHistory* rhist,
        ReaderListener* rlisten)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    for(auto it= m_RTPSParticipants.begin();it!=m_RTPSParticipants.end();++it)
    {
        if(it->first->getGuid().guidPrefix == p->getGuid().guidPrefix)
        {
            RTPSReader* reader;
            if(it->second->createReader(&reader,ratt,rhist,rlisten))
            {
                return reader;
            }

            return nullptr;
        }
    }
    return nullptr;
}

bool RTPSDomain::removeRTPSReader(RTPSReader* reader)
{
    if(reader !=  nullptr)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
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

} /* namespace  rtps */
} /* namespace  fastrtps */
} /* namespace eprosima */
