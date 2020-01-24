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
 * @file StatelessWriter.h
 */


#ifndef STATELESSWRITER_H_
#define STATELESSWRITER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "RTPSWriter.h"
#include "ReaderLocator.h"
#include "../common/Time_t.h"
#include "../../utils/collections/ResourceLimitedVector.hpp"

#include <list>

namespace eprosima {
namespace fastrtps {
namespace rtps {


/**
 * Class StatelessWriter, specialization of RTPSWriter that manages writers that don't keep state of the matched readers.
 * @ingroup WRITER_MODULE
 */
class StatelessWriter : public RTPSWriter
{
    friend class RTPSParticipantImpl;

protected:

    StatelessWriter(
            RTPSParticipantImpl* participant,
            GUID_t& guid,
            WriterAttributes& attributes,
            WriterHistory* history,
            WriterListener* listener = nullptr);

public:

    virtual ~StatelessWriter();

    /**
     * Add a specific change to all ReaderLocators.
     * @param change Pointer to the change.
     * @param max_blocking_time
     */
    void unsent_change_added_to_history(
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) override;

    /**
     * Indicate the writer that a change has been removed by the history due to some HistoryQos requirement.
     * @param change Pointer to the change that is going to be removed.
     * @return True if removed correctly.
     */
    bool change_removed_by_history(
            CacheChange_t* change) override;

    /**
     * Add a matched reader.
     * @param data Pointer to the ReaderProxyData object added.
     * @return True if added.
     */
    bool matched_reader_add(
            const ReaderProxyData& data) override;

    /**
     * Remove a matched reader.
     * @param reader_guid GUID of the reader to remove.
     * @return True if removed.
     */
    bool matched_reader_remove(
            const GUID_t& reader_guid) override;

    /**
     * Tells us if a specific Reader is matched against this writer
     * @param reader_guid GUID of the reader to check.
     * @return True if it was matched.
     */
    bool matched_reader_is_matched(
            const GUID_t& reader_guid) override;

    /**
     * Method to indicate that there are changes not sent in some of all ReaderProxy.
     */
    void send_any_unsent_changes() override;

    /**
     * Update the Attributes of the Writer.
     * @param att New attributes
     */
    void updateAttributes(
            const WriterAttributes& att) override
    {
        (void)att;
        //FOR NOW THERE IS NOTHING TO UPDATE.
    }

    bool set_fixed_locators(
            const LocatorList_t& locator_list);

    void update_unsent_changes(
            const SequenceNumber_t& seq_num,
            const FragmentNumber_t& frag_num);

    //!Reset the unsent changes.
    void unsent_changes_reset();

    bool is_acked_by_all(
            const CacheChange_t* change) const override;

    bool try_remove_change(
            std::chrono::steady_clock::time_point&,
            std::unique_lock<RecursiveTimedMutex>&) override
    {
        return remove_older_changes(1);
    }

    void add_flow_controller(
            std::unique_ptr<FlowController> controller) override;

    /**
     * Send a message through this interface.
     *
     * @param message Pointer to the buffer with the message already serialized.
     * @param max_blocking_time_point Future timepoint where blocking send should end.
     */
    bool send(
            CDRMessage_t* message,
            std::chrono::steady_clock::time_point& max_blocking_time_point) const override;

private:

    void get_builtin_guid();

    bool has_builtin_guid();

    void update_reader_info(
            bool create_sender_resources);

    bool intraprocess_delivery(
            CacheChange_t* change,
            ReaderLocator& reader_locator);

    bool is_inline_qos_expected_ = false;
    LocatorList_t fixed_locators_;
    ResourceLimitedVector<ReaderLocator> matched_readers_;
    ResourceLimitedVector<ChangeForReader_t, std::true_type> unsent_changes_;
    std::vector<std::unique_ptr<FlowController> > flow_controllers_;
    uint64_t last_intraprocess_sequence_number_;
    bool there_are_remote_readers_ = false;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif
#endif /* STATELESSWRITER_H_ */
