/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderProxy.h
 *
 */
#ifndef READERPROXY_H_
#define READERPROXY_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "../common/Types.h"
#include "../common/Locator.h"
#include "../common/SequenceNumber.h"
#include "../common/CacheChange.h"
#include "../common/FragmentNumber.h"
#include "../attributes/WriterAttributes.h"

#include <set>
#include <unordered_map>

namespace boost
{
    class recursive_mutex;
}

namespace eprosima
{
    namespace fastrtps
    {
        namespace rtps
        {

            class StatefulWriter;
            class NackResponseDelay;
            class NackSupressionDuration;


            /**
             * ReaderProxy class that helps to keep the state of a specific Reader with respect to the RTPSWriter.
             * @ingroup WRITER_MODULE
             */
            class ReaderProxy
            {
                public:
                    ~ReaderProxy();

                    /**
                     * Constructor.
                     * @param rdata RemoteWriterAttributes to use in the creation.
                     * @param times WriterTimes to use in the ReaderProxy.
                     * @param SW Pointer to the StatefulWriter.
                     */
                ReaderProxy(RemoteReaderAttributes& rdata,const WriterTimes& times,StatefulWriter* SW);

                void destroy_timers();

                /**
                 * Get the ChangeForReader struct associated with a determined change
                 * @param[in] change Pointer to the change.
                 * @param[out] changeForReader Pointer to a changeforreader structure.
                 * @return True if found.
                 */
                bool getChangeForReader(const CacheChange_t* change, ChangeForReader_t* changeForReader);

                /**
                 * Get the ChangeForReader struct associated with a determined sequenceNumber
                 * @param[in] seq SequenceNumber
                 * @param[out] changeForReader Pointer to a changeforreader structure.
                 * @return True if found.
                 */
                bool getChangeForReader(const SequenceNumber_t& seq, ChangeForReader_t* changeForReader);

                /**
                 * Mark all changes up to the one indicated by the seqNum as Acknowledged.
                 * If seqNum == 30, changes 1-29 are marked as ack.
                 * @param seqNum Pointer to the seqNum
                 * @return True if all changes are acknowledge and anyone with other state.
                 */
                bool acked_changes_set(const SequenceNumber_t& seqNum);

                /**
                 * Mark all changes in the vector as requested.
                 * @param seqNumSet Vector of sequenceNumbers
                 * @return True if correct.
                 */
                bool requested_changes_set(std::vector<SequenceNumber_t>& seqNumSet);

                /*!
                 * @brief Sets the REQUESTED changes to UNDERWAY and returns a STL vector with the modified ChangeForReader_t.
                 * The content of the STL vector has to be used in the same synchronized code than the call of this function.
                 * @return STL vector with the modified ChangeForReader_t.
                 */
                std::vector<const ChangeForReader_t*> requested_changes_to_underway();
                

                /*!
                 * @brief Sets the UNSENT changes to UNDERWAY and returns a STL vector with the modified ChangeForReader_t.
                 * The content of the STL vector has to be used in the same synchronized code than the call of this function.
                 * @return STL vector with the modified ChangeForReader_t.
                 */
                std::vector<const ChangeForReader_t*> unsent_changes_to_underway();
               
                /*!
                 * @brief Lists all unsent changes.
                 * @return STL vector with the unsent change list.
                 */
                std::vector<const ChangeForReader_t*> get_unsent_changes() const;
                /*!
                 * @brief Lists all requested changes.
                 * @return STL vector with the requested change list.
                 */
                std::vector<const ChangeForReader_t*> get_requested_changes() const;

                /*!
                 * @brief Sets a change to a particular status (if present in the ReaderProxy)
                 * @param change change to search and set.
                 * @param status Status to apply.
                 */
                void set_change_to_status(const CacheChange_t* change, ChangeForReaderStatus_t status);

                void underway_changes_to_unacknowledged();

                void underway_changes_to_acknowledged();
                
                void requested_changes_to_unsent();

                void setNotValid(const CacheChange_t* change);

                // Not thread-safe
                void cleanup();

                /*!
                 * @brief Returns there is some UNACKNOWLEDGED change.
                 * @return There is some UNACKNOWLEDGED change.
                 */
                bool thereIsUnacknowledged() const;

                /**
                 * Get a vector of all unacked changes by this Reader.
                 * @param reqChanges Pointer to a vector of pointers.
                 * @return True if correct.
                 */
                bool unacked_changes(std::vector<const ChangeForReader_t*>* reqChanges);

                //!Attributes of the Remote Reader
                RemoteReaderAttributes m_att;

                //!Pointer to the associated StatefulWriter.
                StatefulWriter* mp_SFW;

                //!Tells whether the requested changes list is empty
                bool m_isRequestedChangesEmpty;

                /**
                 * Returns a list of CacheChange_t that have the passes status.
                 * @param Changes Pointer to a vector of CacheChange_t pointers.
                 * @param status Status to be used.
                 * @return True if correctly obtained.
                 */
                bool changesList(std::vector<const ChangeForReader_t*>* Changes, const ChangeForReaderStatus_t status);

                /**
                 * Return the minimum change in a vector of CacheChange_t.
                 * @param Changes Pointer to a vector of CacheChange_t.
                 * @param changeForReader Pointer to the CacheChange_t.
                 * @return True if correct.
                 */
                bool minChange(std::vector<ChangeForReader_t*>* Changes, ChangeForReader_t* changeForReader);

                /*!
                 * @brief Adds requested fragments. These fragments will be sent in next NackResponseDelay.
                 * @param[in] frag_set set containing the requested fragments to be sent.
                 * @param[in] sequence_number Sequence number to be paired with the requested fragments.
                 * @return True if there is at least one requested fragment. False in other case.
                 * @remarks It is not thread-safe.
                 */
                bool requested_fragment_set(const SequenceNumber_t& sequence_number, const FragmentNumberSet_t& frag_set);

                const std::unordered_map<SequenceNumber_t, std::set<FragmentNumber_t>, SequenceNumberHash>&
                    getRequestedFragments() const { return requested_fragments_; }

                void clearRequestedFragments() { requested_fragments_.clear(); }

                /*!
                 * @brief Returns the last NACKFRAG count.
                 * @return Last NACKFRAG count.
                 */
                uint32_t getLastNackfragCount() const { return lastNackfragCount_; }

                /*!
                 * @brief Sets the last NACKFRAG count.
                 * @param lastNackfragCount New value for last NACKFRAG count.
                 */
                void setLastNackfragCount(uint32_t lastNackfragCount) { lastNackfragCount_ = lastNackfragCount; }

                //! Timed Event to manage the Acknack response delay.
                NackResponseDelay* mp_nackResponse;
                //! Timed Event to manage the delay to mark a change as UNACKED after sending it.
                NackSupressionDuration* mp_nackSupression;
                //! Last ack/nack count
                uint32_t m_lastAcknackCount;

                /**
                 * Filter a CacheChange_t, in this version always returns true.
                 * @param change
                 * @return
                 */
                inline bool rtps_is_relevant(CacheChange_t* change){(void)change; return true;};

                //!Mutex
                boost::recursive_mutex* mp_mutex;

                //!Set of the changes and its state.
                std::set<ChangeForReader_t, ChangeForReaderCmp> m_changesForReader;

                private:
                //! Last  NACKFRAG count.
                uint32_t lastNackfragCount_;
                //TODO Temporal
                //! Contains requested fragments per CacheChange_t.
                std::unordered_map<SequenceNumber_t, std::set<FragmentNumber_t>, SequenceNumberHash> requested_fragments_;
            };
        }
    } /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* READERPROXY_H_ */
