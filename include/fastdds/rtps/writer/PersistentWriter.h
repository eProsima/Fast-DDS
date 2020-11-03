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
 * @file PersistentWriter.h
 */
#ifndef _FASTDDS_RTPS_PERSISTENTWRITER_H_
#define _FASTDDS_RTPS_PERSISTENTWRITER_H_

#include <fastdds/rtps/writer/RTPSWriter.h>
#include <string>

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

namespace eprosima {
namespace fastrtps {
namespace rtps {

class IPersistenceService;
class WriterHistory;

/**
 * Class PersistentWriter, that manages history persistence.
 * @ingroup WRITER_MODULE
 */
class PersistentWriter
{
protected:

    PersistentWriter(
            const GUID_t& guid,
            const WriterAttributes& att,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            const std::shared_ptr<IChangePool>& change_pool,
            WriterHistory* hist,
            IPersistenceService* persistence);

public:

    virtual ~PersistentWriter();

    /**
     * Add a specific change to storage.
     * @param change Pointer to the change to be stored.
     */
    void add_persistent_change(
            CacheChange_t* change);

    /**
     * Remove a change from storage.
     * @param change Pointer to the change to be removed.
     */
    void remove_persistent_change(
            CacheChange_t* change);

private:

    //!Persistence service
    IPersistenceService* persistence_;
    //!Persistence GUID
    std::string persistence_guid_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_PERSISTENTWRITER_H_ */
