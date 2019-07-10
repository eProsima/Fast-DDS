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
 * @file StatefulPersistentWriter.cpp
 *
 */

#include <fastrtps/rtps/writer/StatefulPersistentWriter.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include "../persistence/PersistenceService.h"
#include "../participant/RTPSParticipantImpl.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {


 StatefulPersistentWriter::StatefulPersistentWriter(RTPSParticipantImpl* pimpl,GUID_t& guid,
        WriterAttributes& att,WriterHistory* hist,WriterListener* listen, 
     IPersistenceService* persistence):
     StatefulWriter(pimpl,guid,att,hist,listen),
     PersistentWriter(guid,att,hist,persistence)
{
}

 StatefulPersistentWriter::~StatefulPersistentWriter()
{
}

/*
 *	CHANGE-RELATED METHODS
 */

void StatefulPersistentWriter::unsent_change_added_to_history(
        CacheChange_t* cptr,
        std::chrono::time_point<std::chrono::steady_clock> max_blocking_time)
{
    add_persistent_change(cptr);
    StatefulWriter::unsent_change_added_to_history(cptr, max_blocking_time);
}

bool StatefulPersistentWriter::change_removed_by_history(CacheChange_t* change)
{
    remove_persistent_change(change);
    return StatefulWriter::change_removed_by_history(change);
}

} /* namespace rtps */
} /* namespace eprosima */
}
