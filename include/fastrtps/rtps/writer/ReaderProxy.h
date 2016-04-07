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
#include "../attributes/WriterAttributes.h"

#include <set>

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
                    virtual ~ReaderProxy();

                    /**
                     * Constructor.
                     * @param rdata RemoteWriterAttributes to use in the creation.
                     * @param times WriterTimes to use in the ReaderProxy.
                     * @param SW Pointer to the StatefulWriter.
                     */
                ReaderProxy(RemoteReaderAttributes& rdata,const WriterTimes& times,StatefulWriter* SW);

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
                 * @return True if correct.
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

                void underway_changes_to_unacknowledged();

                void underway_changes_to_acknowledged();

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

                //!Timed Event to manage the Acknack response delay.
                NackResponseDelay* mp_nackResponse;
                //!Timed Event to manage the delay to mark a change as UNACKED after sending it.
                NackSupressionDuration* mp_nackSupression;
                //!Last ack/nack count
                uint32_t m_lastAcknackCount;


                /**
                 * Filter a CacheChange_t, in this version always returns true.
                 * @param change
                 * @return
                 */
                inline bool rtps_is_relevant(CacheChange_t* /*change*/){return true;};

                //!Mutex
                boost::recursive_mutex* mp_mutex;

                //!Set of the changes and its state.
                std::set<ChangeForReader_t, ChangeForReaderCmp> m_changesForReader;
            };
        }
    } /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* READERPROXY_H_ */
