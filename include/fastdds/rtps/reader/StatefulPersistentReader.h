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


#ifndef _FASTDDS_RTPS_STATEFULPERSISTENTREADER_H_
#define _FASTDDS_RTPS_STATEFULPERSISTENTREADER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/reader/StatefulReader.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class IPersistenceService;

/**
 * Class StatefulPersistentReader, specialization of StatefulReader that manages sequence number persistence.
 * @ingroup READER_MODULE
 */
class StatefulPersistentReader : public StatefulReader
{
    friend class RTPSParticipantImpl;

    StatefulPersistentReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            ReaderHistory* hist,
            ReaderListener* listen,
            IPersistenceService* persistence);

    StatefulPersistentReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            ReaderHistory* hist,
            ReaderListener* listen,
            IPersistenceService* persistence);

    StatefulPersistentReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            const std::shared_ptr<IChangePool>& change_pool,
            ReaderHistory* hist,
            ReaderListener* listen,
            IPersistenceService* persistence);

public:

    virtual ~StatefulPersistentReader();

protected:

    virtual void set_last_notified(
            const GUID_t& persistence_guid,
            const SequenceNumber_t& seq) override;

    bool may_remove_history_record(
            bool removed_by_lease) override;

private:

    void init(
            const GUID_t& guid,
            const ReaderAttributes& att);

    IPersistenceService* persistence_;
    std::string persistence_guid_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_STATEFULPERSISTENTREADER_H_ */
