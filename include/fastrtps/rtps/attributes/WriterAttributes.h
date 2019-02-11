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
 * @file WriterAttributes.h
 *
 */
#ifndef WRITERATTRIBUTES_H_
#define WRITERATTRIBUTES_H_

#include "../common/Time_t.h"
#include "../common/Guid.h"
#include "../flowcontrol/ThroughputControllerDescriptor.h"
#include "EndpointAttributes.h"
#include "../../utils/collections/ResourceLimitedContainerConfig.hpp"

#include <functional>

namespace eprosima{
namespace fastrtps{
namespace rtps{


typedef enum RTPSWriterPublishMode : octet
{
    SYNCHRONOUS_WRITER,
    ASYNCHRONOUS_WRITER
} RTPSWriterPublishMode;


/**
 * Class WriterTimes, defining the times associated with the Reliable Writers events.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class  WriterTimes
{
public:
    WriterTimes()
    {
        initialHeartbeatDelay.fraction = 200*1000*1000;
        heartbeatPeriod.seconds = 3;
        nackResponseDelay.fraction = 20*1000*1000;
    }

    virtual ~WriterTimes() {}

    bool operator==(const WriterTimes& b) const
    {
        return (this->initialHeartbeatDelay == b.initialHeartbeatDelay) &&
               (this->heartbeatPeriod == b.heartbeatPeriod) &&
               (this->nackResponseDelay == b.nackResponseDelay) &&
               (this->nackSupressionDuration == b.nackSupressionDuration);
    }

    //! Initial heartbeat delay. Default value ~45ms.
    Duration_t initialHeartbeatDelay;
    //! Periodic HB period, default value 3s.
    Duration_t heartbeatPeriod;
    //!Delay to apply to the response of a ACKNACK message, default value ~45ms.
    Duration_t nackResponseDelay;
    //!This time allows the RTPSWriter to ignore nack messages too soon after the data as sent, default value 0s.
    Duration_t nackSupressionDuration;
};

/**
 * Class WriterAttributes, defining the attributes of a RTPSWriter.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class  WriterAttributes
{
    public:

        WriterAttributes() : mode(SYNCHRONOUS_WRITER),
            disableHeartbeatPiggyback(false)
        {
            endpoint.endpointKind = WRITER;
            endpoint.durabilityKind = TRANSIENT_LOCAL;
            endpoint.reliabilityKind = RELIABLE;
        }

        virtual ~WriterAttributes(){}

        //!Attributes of the associated endpoint.
        EndpointAttributes endpoint;

        //!Writer Times (only used for RELIABLE).
        WriterTimes times;

        //!Indicates if the Writer is synchronous or asynchronous
        RTPSWriterPublishMode mode;

        // Throughput controller, always the last one to apply
        ThroughputControllerDescriptor throughputController;

        //! Disable the sending of heartbeat piggybacks.
        bool disableHeartbeatPiggyback;

        //! Define the allocation behaviour for matched-reader-dependent collections.
        ResourceLimitedContainerConfig matched_readers_allocation;
};

/**
 * Class RemoteReaderAttributes, to define the attributes of a Remote Reader.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class  RemoteReaderAttributes
{
    public:

        RemoteReaderAttributes() : expectsInlineQos(false),
        is_eprosima_endpoint(true)
        {
            endpoint.endpointKind = READER;
        }

        RemoteReaderAttributes(const VendorId_t& vendor_id) : expectsInlineQos(false),
        is_eprosima_endpoint(vendor_id == c_VendorId_eProsima)
        {
            endpoint.endpointKind = READER;
        }

        virtual ~RemoteReaderAttributes()
        {

        }

        std::function<bool(const RemoteReaderAttributes&)> compare_guid_function() const
        {
            return [this](const RemoteReaderAttributes& rhs)
            {
                return this->guid == rhs.guid;
            };
        }

        //!Attributes of the associated endpoint.
        EndpointAttributes endpoint;

        //!GUID_t of the reader.
        GUID_t guid;

        //!Expects inline QOS.
        bool expectsInlineQos;

        bool is_eprosima_endpoint;
};
}
}
}


#endif /* WRITERATTRIBUTES_H_ */
