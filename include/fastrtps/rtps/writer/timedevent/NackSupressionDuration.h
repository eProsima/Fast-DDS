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

/**
 * @file NackSupressionDuration.h
 *
 */

#ifndef FASTRTPS_RTPS_WRITER_TIMEDEVENT_NACKSUPRESSIONDURATION_H_
#define FASTRTPS_RTPS_WRITER_TIMEDEVENT_NACKSUPRESSIONDURATION_H_

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "../../resources/TimedEvent.h"
#include "../../common/Guid.h"

namespace eprosima {
namespace fastrtps {
namespace rtps {

class StatefulWriter;

/**
 * NackSupressionDuration class, used to avoid too "recent" NACK messages.
 * @ingroup WRITER_MODULE
 */
class NackSupressionDuration : public TimedEvent
{
public:
    /**
     * Construct a NackSupressionDuration event.
     *
     * @param writer           Pointer to the StatefulWriter creating this event
     * @param interval_in_ms   Event interval in miliseconds
     */
    NackSupressionDuration(
            StatefulWriter* writer,
            double interval_in_ms);

    virtual ~NackSupressionDuration();

    /**
     * Method invoked when the event occurs
     *
     * @param code Code representing the status of the event
     * @param msg Message associated to the event
     */
    void event(
            EventCode code,
            const char* msg = nullptr) override;

    /**
     * Set a new reader guid value
     */
    inline void reader_guid(const GUID_t guid)
    {
        reader_guid_ = guid;
    }

private:

    //! Associated stateful writer
    StatefulWriter* writer_;

    //! GUID of the reader proxy
    GUID_t reader_guid_;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif
#endif /* FASTRTPS_RTPS_WRITER_TIMEDEVENT_NACKSUPRESSIONDURATION_H_ */
