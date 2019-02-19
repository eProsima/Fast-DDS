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
 * @file WriterProxyLiveliness.h
 *
 */

#ifndef FASTRTPS_RTPS_READER_TIMEDEVENT_WRITERPROXYLIVELINESS_H_
#define FASTRTPS_RTPS_READER_TIMEDEVENT_WRITERPROXYLIVELINESS_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "../../resources/TimedEvent.h"
#include "../../common/Guid.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class StatefulReader;
/**
 * Class WriterProxyLiveliness, timed event to check the liveliness of a writer each leaseDuration.
 *  @ingroup READER_MODULE
 */
class WriterProxyLiveliness : public TimedEvent 
{
public:

	/**
     * Construct a WriterProxyLiveliness event object.
	 * @param reader StatefulReader creating this event.
	 */
    WriterProxyLiveliness(StatefulReader* reader);

	virtual ~WriterProxyLiveliness();

    /**
     * Starts this event for the specified writer.
     * @param writer_guid GUID of the writer proxy for which the liveliness should be checked.
     * @param interval Duration of the liveliness period.
     */
    void start(
            const GUID_t& writer_guid,
            const Duration_t& interval);

	/**
	 * Method invoked when the event occurs
	 *
	 * @param code Code representing the status of the event
	 * @param msg Message associated to the event
	 */
	void event(
            EventCode code, 
            const char* msg = nullptr);

private:

	//!Pointer to the StatefulReader associated with this specific event.
    StatefulReader* reader_;
    //! GUID of the writer proxy associated with this specific event.
    GUID_t writer_guid_;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif
#endif /* FASTRTPS_RTPS_READER_TIMEDEVENT_WRITERPROXYLIVELINESS_H_ */
