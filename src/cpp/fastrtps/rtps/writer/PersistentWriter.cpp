// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PersistentWriter.cpp
 *
 */

#include <fastrtps/rtps/writer/PersistentWriter.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include "../persistence/PersistenceService.h"
#include "../participant/RTPSParticipantImpl.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {


PersistentWriter::PersistentWriter(GUID_t& guid,WriterAttributes& att,WriterHistory* hist,IPersistenceService* persistence):
    persistence_(persistence),
    persistence_guid_()
{
     // When persistence GUID is unknown, create from rtps GUID
     GUID_t p_guid = att.endpoint.persistence_guid == c_Guid_Unknown ? guid : att.endpoint.persistence_guid;
     std::ostringstream ss;
     ss << p_guid;
     persistence_guid_ = ss.str();

     if (persistence_->load_writer_from_storage(persistence_guid_, guid, hist->m_changes, &(hist->m_changePool)))
     {
         hist->updateMaxMinSeqNum();
         CacheChange_t* max_change;
         if (hist->get_max_change(&max_change))
         {
             hist->m_lastCacheChangeSeqNum = max_change->sequenceNumber;
         }
     }
 }

PersistentWriter::~PersistentWriter()
{
    delete persistence_;
}

/*
 * CHANGE-RELATED METHODS
 */

void PersistentWriter::add_persistent_change(CacheChange_t* cptr)
{
    persistence_->add_writer_change_to_storage(persistence_guid_, *cptr);
}

void PersistentWriter::remove_persistent_change(CacheChange_t* change)
{
    persistence_->remove_writer_change_from_storage(persistence_guid_, *change);
}

} /* namespace rtps */
} /* namespace eprosima */
}
