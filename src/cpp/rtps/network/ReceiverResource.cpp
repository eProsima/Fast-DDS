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

#include <fastrtps/rtps/network/ReceiverResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>
#include <cassert>
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/writer/ReaderProxy.h>
#include <fastrtps/rtps/reader/WriterProxy.h>
#include <fastrtps/rtps/writer/timedevent/NackResponseDelay.h>

#define IDSTRING "(ID:" << std::this_thread::get_id() <<") "<<

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

ReceiverResource::ReceiverResource(RTPSParticipantImpl* participant, TransportInterface& transport,
    const Locator_t& locator, uint32_t maxMsgSize)
    : mValid(false)
    , m_participant(participant)
    , m_maxMsgSize(maxMsgSize)
{
    // Internal channel is opened and assigned to this resource.
    mValid = transport.OpenInputChannel(locator, this, m_maxMsgSize);
    if (!mValid)
    {
        return; // Invalid resource to be discarded by the factory.
    }

    // Implementation functions are bound to the right transport parameters
    Cleanup = [&transport, locator]() { transport.CloseInputChannel(locator); };
    LocatorMapsToManagedChannel = [&transport, locator](const Locator_t& locatorToCheck) -> bool
    { return transport.DoLocatorsMatch(locator, locatorToCheck); };
}

ReceiverResource::ReceiverResource(ReceiverResource&& rValueResource)
{
    Cleanup.swap(rValueResource.Cleanup);
    LocatorMapsToManagedChannel.swap(rValueResource.LocatorMapsToManagedChannel);
}

MessageReceiver* ReceiverResource::CreateMessageReceiver()
{
    MessageReceiver* newMsgReceiver = new MessageReceiver(m_participant, this);
    newMsgReceiver->init(m_maxMsgSize);
    return newMsgReceiver;
}

bool ReceiverResource::SupportsLocator(const Locator_t& localLocator)
{
    if (LocatorMapsToManagedChannel)
    {
        return LocatorMapsToManagedChannel(localLocator);
    }
    return false;
}

void ReceiverResource::Abort()
{
    if (Cleanup)
    {
        Cleanup();
    }
}

ReceiverResource::~ReceiverResource()
{
    if (Cleanup)
    {
        Cleanup();
    }
    assert(AssociatedWriters.size() == 0);
    assert(AssociatedReaders.size() == 0);
}

void ReceiverResource::associateEndpoint(Endpoint *to_add)
{
    bool found = false;

    std::lock_guard<std::mutex> guard(mtx);
    if (to_add->getAttributes()->endpointKind == WRITER)
    {
        for (auto it = AssociatedWriters.begin(); it != AssociatedWriters.end(); ++it)
        {
            if ((*it) == (RTPSWriter*)to_add)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            AssociatedWriters.push_back((RTPSWriter*)to_add);
        }
    }
    else
    {
        for (auto it = AssociatedReaders.begin(); it != AssociatedReaders.end(); ++it)
        {
            if ((*it) == (RTPSReader*)to_add)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            AssociatedReaders.push_back((RTPSReader*)to_add);
        }
    }
    return;
}

void ReceiverResource::removeEndpoint(Endpoint *to_remove)
{
    std::lock_guard<std::mutex> guard(mtx);
    if (to_remove->getAttributes()->endpointKind == WRITER)
    {
        RTPSWriter* var = (RTPSWriter *)to_remove;
        for (auto it = AssociatedWriters.begin(); it != AssociatedWriters.end(); ++it)
        {
            if ((*it) == var)
            {
                AssociatedWriters.erase(it);
                break;
            }
        }
    }
    else
    {
        RTPSReader *var = (RTPSReader *)to_remove;
        for (auto it = AssociatedReaders.begin(); it != AssociatedReaders.end(); ++it)
        {
            if ((*it) == var)
            {
                AssociatedReaders.erase(it);
                break;
            }
        }
    }
    return;
}

bool ReceiverResource::checkReaders(EntityId_t readerID)
{
    bool bFound(false);
    if (AssociatedReaders.empty())
    {
        logWarning(RTPS_MSG_IN, IDSTRING"Data received when NO readers are listening");
        return false;
    }

    std::lock_guard<std::mutex> guard(mtx);
    for (std::vector<RTPSReader*>::iterator it = AssociatedReaders.begin(); it != AssociatedReaders.end(); ++it)
    {
        if ((*it)->acceptMsgDirectedTo(readerID)) //add
        {
            bFound = true;
        }
    }

    if (!bFound)
    {
        //showCDRMessage(msg);
        logWarning(RTPS_MSG_IN, IDSTRING"No Reader accepts this message (directed to: " << readerID << ")");
        return false;
    }
    return bFound;
}

void ReceiverResource::processDataMsg(EntityId_t readerID, CacheChange_t* ch)
{
    logInfo(RTPS_MSG_IN, IDSTRING"from Writer " << ch->writerGUID << "; possible RTPSReaders: " << AssociatedReaders.size());

    std::lock_guard<std::mutex> guard(mtx);
    //Look for the correct reader to add the change
    for (std::vector<RTPSReader*>::iterator it = AssociatedReaders.begin(); it != AssociatedReaders.end(); ++it)
    {
        if ((*it)->acceptMsgDirectedTo(readerID))
        {
            (*it)->processDataMsg(ch);
        }
    }
}

void ReceiverResource::processDataFragMsg(EntityId_t readerID, CacheChange_t *incomingChange, uint32_t sampleSize,
    uint32_t fragmentStartingNum)
{
    logInfo(RTPS_MSG_IN, IDSTRING"from Writer " << incomingChange->writerGUID << "; possible RTPSReaders: " << AssociatedReaders.size());

    std::lock_guard<std::mutex> guard(mtx);
    //Look for the correct reader to add the change
    for (std::vector<RTPSReader*>::iterator it = AssociatedReaders.begin(); it != AssociatedReaders.end(); ++it)
    {
        if ((*it)->acceptMsgDirectedTo(readerID))
        {
            (*it)->processDataFragMsg(incomingChange, sampleSize, fragmentStartingNum);
        }
    }
}

void ReceiverResource::processHeartbeatMsg(EntityId_t readerID, GUID_t &writerGUID, uint32_t hbCount,
    SequenceNumber_t &firstSN, SequenceNumber_t &lastSN, bool finalFlag, bool livelinessFlag)
{
    std::lock_guard<std::mutex> guard(mtx);
    //Look for the correct reader and writers:
    for (std::vector<RTPSReader*>::iterator it = AssociatedReaders.begin(); it != AssociatedReaders.end(); ++it)
    {
        if ((*it)->acceptMsgDirectedTo(readerID))
        {
            (*it)->processHeartbeatMsg(writerGUID, hbCount, firstSN, lastSN, finalFlag, livelinessFlag);
        }
    }
}

void ReceiverResource::processGapMsg(EntityId_t readerID, GUID_t &writerGUID, SequenceNumber_t &gapStart,
    SequenceNumberSet_t &gapList)
{
    std::lock_guard<std::mutex> guard(mtx);
    for (std::vector<RTPSReader*>::iterator it = AssociatedReaders.begin(); it != AssociatedReaders.end(); ++it)
    {
        if ((*it)->acceptMsgDirectedTo(readerID))
        {
            (*it)->processGapMsg(writerGUID, gapStart, gapList);
        }
    }
}

bool ReceiverResource::processSubMsgNackFrag(const GUID_t& readerGUID, const GUID_t& writerGUID,
    SequenceNumber_t& writerSN, FragmentNumberSet_t& fnState, uint32_t Ackcount)
{
    std::lock_guard<std::mutex> guard(mtx);
    //Look for the correct writer to use the acknack
    for (std::vector<RTPSWriter*>::iterator it = AssociatedWriters.begin(); it != AssociatedWriters.end(); ++it)
    {
        //Look for the readerProxy the acknack is from
        std::lock_guard<std::recursive_mutex> guardW(*(*it)->getMutex());
        if ((*it)->getGuid() == writerGUID)
        {
            if ((*it)->getAttributes()->reliabilityKind == RELIABLE)
            {
                StatefulWriter* SF = (StatefulWriter*)(*it);
                for (auto rit = SF->matchedReadersBegin(); rit != SF->matchedReadersEnd(); ++rit)
                {
                    std::lock_guard<std::recursive_mutex> guardReaderProxy(*(*rit)->mp_mutex);
                    if ((*rit)->m_att.guid == readerGUID)
                    {
                        if ((*rit)->getLastNackfragCount() < Ackcount)
                        {
                            (*rit)->setLastNackfragCount(Ackcount);
                            // TODO Not doing Acknowledged.
                            if ((*rit)->requested_fragment_set(writerSN, fnState))
                            {
                                (*rit)->mp_nackResponse->restart_timer();
                            }
                        }
                        break;
                    }
                }
                return true;
            }
            else
            {
                logInfo(RTPS_MSG_IN, IDSTRING"Acknack msg to NOT stateful writer ");
                return false;
            }
        }
    }
    logInfo(RTPS_MSG_IN, IDSTRING"Acknack msg to UNKNOWN writer (I looked through "
        << AssociatedWriters.size() << " writers in this ListenResource)");
    return false;
}

bool ReceiverResource::processAckNack(const GUID_t& readerGUID, const GUID_t& writerGUID, uint32_t ackCount,
    const SequenceNumberSet_t& snSet, bool finalFlag)
{
    std::lock_guard<std::mutex> guard(mtx);
    //Look for the correct writer to use the acknack
    for (std::vector<RTPSWriter*>::iterator it = AssociatedWriters.begin();
        it != AssociatedWriters.end(); ++it)
    {
        if ((*it)->getGuid() == writerGUID)
        {
            if ((*it)->getAttributes()->reliabilityKind == RELIABLE)
            {
                StatefulWriter* SF = (StatefulWriter*)(*it);
                SF->process_acknack(readerGUID, ackCount, snSet, finalFlag);
                return true;
            }
            else
            {
                logInfo(RTPS_MSG_IN, IDSTRING"Acknack msg to NOT stateful writer ");
                return false;
            }
        }
    }
    logInfo(RTPS_MSG_IN, IDSTRING"Acknack msg to UNKNOWN writer (I loooked through "
        << AssociatedWriters.size() << " writers in this ListenResource)");
    return false;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
