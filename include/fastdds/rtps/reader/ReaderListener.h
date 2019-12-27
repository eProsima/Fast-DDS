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
 * @file ReaderListener.h
 *
 */

#ifndef _FASTDDS_RTPS_READERLISTENER_H_
#define _FASTDDS_RTPS_READERLISTENER_H_

#include <fastdds/rtps/common/MatchingInfo.h>
#include <fastrtps/qos/LivelinessChangedStatus.h>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/SampleRejectedStatus.hpp>

#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSReader;
struct CacheChange_t;

/**
 * Class ReaderListener, to be used by the user to override some of is virtual method to program actions to
 * certain events.
 *  @ingroup READER_MODULE
 */
class RTPS_DllAPI ReaderListener
{
public:

    ReaderListener() = default;

    virtual ~ReaderListener() = default;

    /**
     * This method is invoked when a new reader matches
     * @param reader Matching reader
     * @param info Matching information of the reader
     */
    virtual void onReaderMatched(
            RTPSReader* reader,
            MatchingInfo& info)
    {
        (void)reader;
        (void)info;
    }

    /**
     * This method is invoked when a new reader matches
     * @param reader Matching reader
     * @param info Subscription matching information
     */
    virtual void onReaderMatched(
            RTPSReader* reader,
            const fastdds::dds::SubscriptionMatchedStatus& info)
    {
        (void)reader;
        (void)info;
    }

    /**
     * This method is called when a new CacheChange_t is added to the ReaderHistory.
     * @param reader Pointer to the reader.
     * @param change Pointer to the CacheChange_t. This is a const pointer to const data
     * to indicate that the user should not dispose of this data himself.
     * To remove the data call the remove_change method of the ReaderHistory.
     * reader->getHistory()->remove_change((CacheChange_t*)change).
     */
    virtual void onNewCacheChangeAdded(
            RTPSReader* reader,
            const CacheChange_t* const change)
    {
        (void)reader;
        (void)change;
    }

    /**
     * @brief Method called when the livelivess of a reader changes
     * @param reader The reader
     * @param status The liveliness changed status
     */
    virtual void on_liveliness_changed(
            RTPSReader* reader,
            const LivelinessChangedStatus& status)
    {
        (void)reader;
        (void)status;
    }

    /**
     * Virtual method to be called when the requested qos is incompatible with the one offered
     * @param sub Reader
     * @param status The requested incompatible qos status
     */
    virtual void on_requested_incompatible_qos(
            RTPSReader* reader,
            const fastdds::dds::RequestedIncompatibleQosStatus& status)
    {
        (void)reader;
        (void)status;
    }

    /**
     * Virtual method to be called when a sample is rejected by the Reader
     * @param sub Reader
     * @param status The sample rejected status
     */
    virtual void on_sample_rejected(
            RTPSReader* reader,
            const fastdds::dds::SampleRejectedStatus& status)
    {
        (void)reader;
        (void)status;
    }

    /**
     * Virtual method to be called when a sample is lost by the Reader
     * @param sub Reader
     * @param status The sample lost status
     */
    virtual void on_sample_lost(
            RTPSReader* reader,
            const fastdds::dds::SampleLostStatus& status)
    {
        (void)reader;
        (void)status;
    }

};

//Namespace enders
}
}
}

#endif /* _FASTDDS_RTPS_READERLISTENER_H_ */
