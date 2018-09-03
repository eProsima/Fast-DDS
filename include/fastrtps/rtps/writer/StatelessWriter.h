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

#include "../common/Time_t.h"
#include "RTPSWriter.h"
#include "ReaderLocator.h"

#include <list>

namespace eprosima {
namespace fastrtps{
namespace rtps {


/**
 * Class StatelessWriter, specialization of RTPSWriter that manages writers that don't keep state of the matched readers.
 * @ingroup WRITER_MODULE
 */
class StatelessWriter : public RTPSWriter
{
    friend class RTPSParticipantImpl;

    protected:
    StatelessWriter(RTPSParticipantImpl*,GUID_t& guid,WriterAttributes& att,WriterHistory* hist,WriterListener* listen=nullptr);
    public:
    virtual ~StatelessWriter();
    /**
     * Add a specific change to all ReaderLocators.
     * @param p Pointer to the change.
     */
    void unsent_change_added_to_history(CacheChange_t* p) override;

    /**
     * Indicate the writer that a change has been removed by the history due to some HistoryQos requirement.
     * @param a_change Pointer to the change that is going to be removed.
     * @return True if removed correctly.
     */
    bool change_removed_by_history(CacheChange_t* a_change) override;
    /**
     * Add a matched reader.
     * @param ratt Attributes of the reader to add.
     * @return True if added.
     */
    bool matched_reader_add(const RemoteReaderAttributes& ratt) override;
    /**
     * Remove a matched reader.
     * @param ratt Attributes of the reader to remove.
     * @return True if removed.
     */
    bool matched_reader_remove(const RemoteReaderAttributes& ratt) override;
    /**
     * Tells us if a specific Reader is matched against this writer
     * @param ratt Attributes of the reader to check.
     * @return True if it was matched.
     */
    bool matched_reader_is_matched(const RemoteReaderAttributes& ratt) override;
    /**
     * Method to indicate that there are changes not sent in some of all ReaderProxy.
     */
    void send_any_unsent_changes() override;

    /**
     * Update the Attributes of the Writer.
     * @param att New attributes
     */
    void updateAttributes(WriterAttributes& att) override {
        (void)att;
        //FOR NOW THERE IS NOTHING TO UPDATE.
    }

    bool add_locator(Locator_t& loc);

    void update_unsent_changes(ReaderLocator& reader_locator,
            const SequenceNumber_t& seqNum, const FragmentNumber_t fragNum);

    //!Reset the unsent changes.
    void unsent_changes_reset();

    /**
     * Get the number of matched readers
     * @return Number of matched readers
     */
    inline size_t getMatchedReadersSize() const {return m_matched_readers.size();};

    bool is_acked_by_all(const CacheChange_t* a_change) const override;

    bool try_remove_change(std::chrono::microseconds&, std::unique_lock<std::recursive_mutex>&) override { 
        return remove_older_changes(1); 
    }

    void add_flow_controller(std::unique_ptr<FlowController> controller) override;

    private:

    std::vector<GUID_t> get_builtin_guid();

    void update_locators_nts_(const GUID_t& optionalGuid);

    std::vector<ReaderLocator> reader_locators, fixed_locators;
    std::vector<RemoteReaderAttributes> m_matched_readers;
    std::vector<std::unique_ptr<FlowController> > m_controllers;
};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif
#endif /* STATELESSWRITER_H_ */
