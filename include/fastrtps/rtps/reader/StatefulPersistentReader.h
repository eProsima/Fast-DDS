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

/**
 * @file StatefulPersistentReader.h
 */


#ifndef STATEFULPERSISTENTREADER_H_
#define STATEFULPERSISTENTREADER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "StatefulReader.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class IPersistenceService;

/**
 * Class StatefulPersistentReader, specialization of StatefulReader that manages sequence number persistence.
 * @ingroup READER_MODULE
 */
class StatefulPersistentReader : public StatefulReader
{
    friend class RTPSParticipantImpl;

    StatefulPersistentReader(RTPSParticipantImpl*, GUID_t& guid,
        ReaderAttributes& att, ReaderHistory* hist, ReaderListener* listen,
        IPersistenceService* persistence);
    public:
    virtual ~StatefulPersistentReader();

    protected:
    virtual void set_last_notified(const GUID_t& persistence_guid, const SequenceNumber_t& seq) override;

    private:
    IPersistenceService* persistence_;
    std::string persistence_guid_;
};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif
#endif /* STATEFULPERSISTENTREADER_H_ */
