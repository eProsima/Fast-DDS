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

    StatelessWriter(RTPSParticipantImpl*,GUID_t& guid,WriterAttributes& att,WriterHistory* hist,WriterListener* listen=nullptr);
    public:
    virtual ~StatelessWriter();
    /**
     * Add a specific change to all ReaderLocators.
     * @param p Pointer to the change.
     */
    void unsent_change_added_to_history(CacheChange_t* p);

    /**
     * Indicate the writer that a change has been removed by the history due to some HistoryQos requirement.
     * @param a_change Pointer to the change that is going to be removed.
     * @return True if removed correctly.
     */
    bool change_removed_by_history(CacheChange_t* a_change);
    /**
     * Add a matched reader.
     * @param ratt Attributes of the reader to add.
     * @return True if added.
     */
    bool matched_reader_add(RemoteReaderAttributes& ratt);
    /**
     * Remove a matched reader.
     * @param ratt Attributes of the reader to remove.
     * @return True if removed.
     */
    bool matched_reader_remove(RemoteReaderAttributes& ratt);
    /**
     * Tells us if a specific Reader is matched against this writer
     * @param ratt Attributes of the reader to check.
     * @return True if it was matched.
     */
    bool matched_reader_is_matched(RemoteReaderAttributes& ratt);
    /**
     * Method to indicate that there are changes not sent in some of all ReaderProxy.
     */
    void send_any_unsent_changes();

    /**
     * Update the Attributes of the Writer.
     * @param att New attributes
     */
    void updateAttributes(WriterAttributes& att){
        (void)att;
        //FOR NOW THERE IS NOTHING TO UPDATE.
    };

    /**
     * Add a remote locator.
     *
     * @param rdata RemoteReaderAttributes necessary to create a new locator.
     * @param loc Locator to add.
     * @return True on success.
     */
    bool add_locator(RemoteReaderAttributes& rdata,Locator_t& loc);

    void update_unsent_changes(ReaderLocator& reader_locator,
            const std::vector<CacheChange_t*>& changes);

    /**
     * Remove a remote locator from the writer.
     *
     * @param loc Locator to remove.
     * @return True on success.
     */
    bool remove_locator(Locator_t& loc);

    //!Reset the unsent changes.
    void unsent_changes_reset();

    /**
     * Get the number of matched readers
     * @return Number of matched readers
     */
    inline size_t getMatchedReadersSize() const {return m_matched_readers.size();};

    bool clean_history(unsigned int max = 0) { return remove_older_changes(max); }

    void add_flow_controller(std::unique_ptr<FlowController> controller);

    private:

    std::vector<GUID_t> get_remote_readers();

    //Duration_t resendDataPeriod; //FIXME: Not used yet.
    std::vector<ReaderLocator> reader_locators;
    std::vector<RemoteReaderAttributes> m_matched_readers;
    std::vector<std::unique_ptr<FlowController> > m_controllers;
};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif
#endif /* STATELESSWRITER_H_ */
