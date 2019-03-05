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
 * @file ReaderAttributes.h
 *
 */

#ifndef FASTRTPS_RTPS_ATTRIBUTES_READERATTRIBUTES_H_
#define FASTRTPS_RTPS_ATTRIBUTES_READERATTRIBUTES_H_

#include "../common/Time_t.h"
#include "../common/Guid.h"
#include "EndpointAttributes.h"
#include "../../utils/collections/ResourceLimitedContainerConfig.hpp"

#include <functional>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class WriterProxyData;

/**
 * Class ReaderTimes, defining the times associated with the Reliable Readers events.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class ReaderTimes
{
public:
    ReaderTimes()
    {
        initialAcknackDelay.fraction = 300*1000*1000;
        heartbeatResponseDelay.fraction = 20*1000*1000;
    }

    virtual ~ReaderTimes() {}

    bool operator==(const ReaderTimes& b) const
    {
        return (this->initialAcknackDelay == b.initialAcknackDelay)  &&
               (this->heartbeatResponseDelay == b.heartbeatResponseDelay);
    }

    //!Initial AckNack delay. Default value ~70ms.
    Duration_t initialAcknackDelay;
    //!Delay to be applied when a hearbeat message is received, default value ~5ms.
    Duration_t heartbeatResponseDelay;
};

/**
 * Class ReaderAttributes, to define the attributes of a RTPSReader.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class ReaderAttributes
{
    public:

        ReaderAttributes() 
            : expectsInlineQos(false)
        {
            endpoint.endpointKind = READER;
            endpoint.durabilityKind = VOLATILE;
            endpoint.reliabilityKind = BEST_EFFORT;
        };

        virtual ~ReaderAttributes(){};

        //!Attributes of the associated endpoint.
        EndpointAttributes endpoint;

        //!Times associated with this reader.
        ReaderTimes times;

        //!Indicates if the reader expects Inline qos, default value 0.
        bool expectsInlineQos;

        //! Define the allocation behaviour for matched-writer-dependent collections.
        ResourceLimitedContainerConfig matched_writers_allocation;
};

/**
 * Class RemoteWriterAttributes, to define the attributes of a Remote Writer.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class  RemoteWriterAttributes
{
    public:
        RemoteWriterAttributes() 
            : livelinessLeaseDuration(c_TimeInfinite)
            , ownershipStrength(0)
        {
            endpoint.endpointKind = WRITER;
        }

        RemoteWriterAttributes(const WriterProxyData& data);

        virtual ~RemoteWriterAttributes()
        {
        }

        std::function<bool(const RemoteWriterAttributes&)> compare_guid_function() const
        {
            return [this](const RemoteWriterAttributes& rhs)
            {
                return this->guid == rhs.guid;
            };
        }

        //!Attributes of the associated endpoint.
        EndpointAttributes endpoint;

        //!GUID_t of the writer, can be unknown if the reader is best effort.
        GUID_t guid;

        //!Liveliness lease duration, default value c_TimeInfinite.
        Duration_t livelinessLeaseDuration;

        //!Ownership Strength of the associated writer.
        uint16_t ownershipStrength;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* FASTRTPS_RTPS_ATTRIBUTES_READERATTRIBUTES_H_ */
