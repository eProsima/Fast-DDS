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
 * @file CacheChange.h
 */

#ifndef _FASTDDS_RTPS_CACHECHANGE_H_
#define _FASTDDS_RTPS_CACHECHANGE_H_

#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/common/WriteParams.h>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastdds/rtps/common/FragmentNumber.h>

#include <vector>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * @enum ChangeKind_t, different types of CacheChange_t.
 * @ingroup COMMON_MODULE
 */
enum RTPS_DllAPI ChangeKind_t
{
    ALIVE,                            //!< ALIVE
    NOT_ALIVE_DISPOSED,               //!< NOT_ALIVE_DISPOSED
    NOT_ALIVE_UNREGISTERED,           //!< NOT_ALIVE_UNREGISTERED
    NOT_ALIVE_DISPOSED_UNREGISTERED   //!< NOT_ALIVE_DISPOSED_UNREGISTERED
};

/**
 * Structure CacheChange_t, contains information on a specific CacheChange.
 * @ingroup COMMON_MODULE
 */
struct RTPS_DllAPI CacheChange_t
{
    //!Kind of change, default value ALIVE.
    ChangeKind_t kind = ALIVE;
    //!GUID_t of the writer that generated this change.
    GUID_t writerGUID;
    //!Handle of the data associated wiht this change.
    InstanceHandle_t instanceHandle;
    //!SequenceNumber of the change
    SequenceNumber_t sequenceNumber;
    //!Serialized Payload associated with the change.
    SerializedPayload_t serializedPayload;
    //!Indicates if the cache has been read (only used in READERS)
    bool isRead = false;
    //!Source TimeStamp (only used in Readers)
    Time_t sourceTimestamp;
    //!Reception TimeStamp (only used in Readers)
    Time_t receptionTimestamp;

    WriteParams write_params;
    bool is_untyped_ = true;

    /*!
     * @brief Default constructor.
     * Creates an empty CacheChange_t.
     */
    CacheChange_t()
    {
    }

    CacheChange_t(
            const CacheChange_t&) = delete;
    const CacheChange_t& operator =(
            const CacheChange_t&) = delete;

    /**
     * Constructor with payload size
     * @param payload_size Serialized payload size
     * @param is_untyped Flag to mark the change as untyped.
     */
    CacheChange_t(
            uint32_t payload_size,
            bool is_untyped = false)
        : serializedPayload(payload_size)
        , is_untyped_(is_untyped)
    {
    }

    /*!
     * Copy a different change into this one. All the elements are copied, included the data, allocating new memory.
     * @param[in] ch_ptr Pointer to the change.
     * @return True if correct.
     */
    bool copy(
            const CacheChange_t* ch_ptr)
    {
        kind = ch_ptr->kind;
        writerGUID = ch_ptr->writerGUID;
        instanceHandle = ch_ptr->instanceHandle;
        sequenceNumber = ch_ptr->sequenceNumber;
        sourceTimestamp = ch_ptr->sourceTimestamp;
        write_params = ch_ptr->write_params;
        isRead = ch_ptr->isRead;
        fragment_size_ = ch_ptr->fragment_size_;
        fragment_count_ = ch_ptr->fragment_count_;
        first_missing_fragment_ = ch_ptr->first_missing_fragment_;

        return serializedPayload.copy(&ch_ptr->serializedPayload, !ch_ptr->is_untyped_);
    }

    /*!
     * Copy information form a different change into this one.
     * All the elements are copied except data.
     * @param[in] ch_ptr Pointer to the change.
     */
    void copy_not_memcpy(
            const CacheChange_t* ch_ptr)
    {
        kind = ch_ptr->kind;
        writerGUID = ch_ptr->writerGUID;
        instanceHandle = ch_ptr->instanceHandle;
        sequenceNumber = ch_ptr->sequenceNumber;
        sourceTimestamp = ch_ptr->sourceTimestamp;
        write_params = ch_ptr->write_params;
        isRead = ch_ptr->isRead;

        // Copy certain values from serializedPayload
        serializedPayload.encapsulation = ch_ptr->serializedPayload.encapsulation;

        // Copy fragment size and calculate fragment count
        setFragmentSize(ch_ptr->fragment_size_, false);
    }

    ~CacheChange_t()
    {
    }

    /*!
     * Get the number of fragments this change is split into.
     * @return number of fragments.
     */
    uint32_t getFragmentCount() const
    {
        return fragment_count_;
    }

    /*!
     * Get the size of each fragment this change is split into.
     * @return size of fragment (0 means change is not fragmented).
     */
    uint16_t getFragmentSize() const
    { 
        return fragment_size_; 
    }

    /*!
     * Checks if all fragments have been received.
     * @return true when change is fully assembled (i.e. no missing fragments).
     */
    bool is_fully_assembled()
    {
        return first_missing_fragment_ >= fragment_count_;
    }

    /*!
     * Fills a FragmentNumberSet_t with the list of missing fragments.
     * @param [out] frag_sns FragmentNumberSet_t where result is stored.
     */
    void get_missing_fragments(
            FragmentNumberSet_t& frag_sns)
    {
        // Note: Fragment numbers are 1-based but we keep them 0 based.
        frag_sns.base(first_missing_fragment_ + 1);

        // Traverse list of missing fragments, adding them to frag_sns
        uint32_t current_frag = first_missing_fragment_;
        while (current_frag < fragment_count_)
        {
            frag_sns.add(current_frag + 1);
            current_frag = get_next_missing_fragment(current_frag);
        }
    }

    /*!
     * Set fragment size for this change.
     *
     * @param fragment_size Size of fragments.
     * @param create_fragment_list Whether to create missing fragments list or not.
     *
     * @remarks Parameter create_fragment_list should only be true when receiving the first
     *          fragment of a change.
     */
    void setFragmentSize(
            uint16_t fragment_size,
            bool create_fragment_list = false)
    {
        fragment_size_ = fragment_size;
        fragment_count_ = 0;
        first_missing_fragment_ = 0;

        if (fragment_size > 0)
        {
            // This follows RTPS 8.3.7.3.5
            fragment_count_ = (serializedPayload.length + fragment_size - 1) / fragment_size;

            if (create_fragment_list)
            {
                // Keep index of next fragment on the payload portion at the beginning of each fragment. Last
                // fragment will have fragment_count_ as 'next fragment index'
                size_t offset = 0;
                for (uint32_t i = 1; i <= fragment_count_; i++, offset += fragment_size_)
                {
                    set_next_missing_fragment(i - 1, i);  // index to next fragment in missing list
                }
            }
            else
            {
                // List not created. This means we are going to send this change fragmented, so it is already
                // assembled, and the missing list is empty (i.e. first missing points to fragment count)
                first_missing_fragment_ = fragment_count_;
            }
        }
    }

    bool add_fragments(
            const SerializedPayload_t& incoming_data,
            uint32_t fragment_starting_num,
            uint32_t fragments_in_submessage)
    {
        uint32_t original_offset = (fragment_starting_num - 1) * fragment_size_;
        uint32_t incoming_length = fragment_size_ * fragments_in_submessage;
        uint32_t last_fragment_index = fragment_starting_num + fragments_in_submessage - 1;

        // Validate fragment indexes
        if (last_fragment_index > fragment_count_)
        {
            return false;
        }

        // validate lengths
        if (last_fragment_index < fragment_count_)
        {
            if (incoming_data.length < incoming_length)
            {
                return false;
            }
        }
        else
        {
            incoming_length = serializedPayload.length - original_offset;
        }

        if (original_offset + incoming_length > serializedPayload.length)
        {
            return false;
        }

        if (received_fragments(fragment_starting_num - 1, fragments_in_submessage))
        {
            memcpy(
                &serializedPayload.data[original_offset],
                incoming_data.data, incoming_length);
        }

        return is_fully_assembled();
    }

private:

    // Fragment size
    uint16_t fragment_size_ = 0;

    // Number of fragments
    uint32_t fragment_count_ = 0;

    // First fragment in missing list
    uint32_t first_missing_fragment_ = 0;

    uint32_t get_next_missing_fragment(
            uint32_t fragment_index)
    {
        uint32_t* ptr = next_fragment_pointer(fragment_index);
        return *ptr;
    }

    void set_next_missing_fragment(
            uint32_t fragment_index,
            uint32_t next_fragment_index)
    {
        uint32_t* ptr = next_fragment_pointer(fragment_index);
        *ptr = next_fragment_index;
    }

    uint32_t* next_fragment_pointer(
            uint32_t fragment_index)
    {
        size_t offset = fragment_size_;
        offset *= fragment_index;
        offset = (offset + 3) & ~3;
        return reinterpret_cast<uint32_t*>(&serializedPayload.data[offset]);
    }

    /*!
     * Mark a set of consecutive fragments as received.
     * This will remove a set of consecutive fragments from the missing list.
     * Should be called BEFORE copying the received data into the serialized payload.
     *
     * @param initial_fragment Index (0-based) of first received fragment.
     * @param num_of_fragments Number of received fragments. Should be strictly positive.
     * @return true if the list of missing fragments was modified, false otherwise.
     */
    bool received_fragments(
            uint32_t initial_fragment,
            uint32_t num_of_fragments)
    {
        bool at_least_one_changed = false;

        if ( (fragment_size_ > 0) && (initial_fragment < fragment_count_) )
        {
            uint32_t last_fragment = initial_fragment + num_of_fragments;
            if (last_fragment > fragment_count_)
            {
                last_fragment = fragment_count_;
            }

            if (initial_fragment <= first_missing_fragment_)
            {
                // Perform first = *first until first >= last_received
                while (first_missing_fragment_ < last_fragment)
                {
                    first_missing_fragment_ = get_next_missing_fragment(first_missing_fragment_);
                    at_least_one_changed = true;
                }
            }
            else
            {
                // Find prev in missing list
                uint32_t current_frag = first_missing_fragment_;
                while (current_frag < initial_fragment)
                {
                    uint32_t next_frag = get_next_missing_fragment(current_frag);
                    if (next_frag >= initial_fragment)
                    {
                        // This is the fragment previous to initial_fragment.
                        // Find future value for next by repeating next = *next until next >= last_fragment.
                        uint32_t next_missing_fragment = next_frag;
                        while (next_missing_fragment < last_fragment)
                        {
                            next_missing_fragment = get_next_missing_fragment(next_missing_fragment);
                            at_least_one_changed = true;
                        }

                        // Update next and finish loop
                        if (at_least_one_changed)
                        {
                            set_next_missing_fragment(current_frag, next_missing_fragment);
                        }
                        break;
                    }
                    current_frag = next_frag;
                }
            }
        }

        return at_least_one_changed;
    }

};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Enum ChangeForReaderStatus_t, possible states for a CacheChange_t in a ReaderProxy.
 *  @ingroup COMMON_MODULE
 */
enum ChangeForReaderStatus_t
{
    UNSENT = 0,                    //!< UNSENT
    REQUESTED = 1,                 //!< REQUESTED
    UNACKNOWLEDGED = 2,            //!< UNACKNOWLEDGED
    ACKNOWLEDGED = 3,              //!< ACKNOWLEDGED
    UNDERWAY = 4                   //!< UNDERWAY
};

/**
 * Struct ChangeForReader_t used to represent the state of a specific change with respect to a specific reader, as well as its relevance.
 *  @ingroup COMMON_MODULE
 */
class ChangeForReader_t
{
    friend struct ChangeForReaderCmp;

public:

    ChangeForReader_t()
        : status_(UNSENT)
        , is_relevant_(true)
        , change_(nullptr)
    {
    }

    ChangeForReader_t(
            const ChangeForReader_t& ch)
        : status_(ch.status_)
        , is_relevant_(ch.is_relevant_)
        , seq_num_(ch.seq_num_)
        , change_(ch.change_)
        , unsent_fragments_(ch.unsent_fragments_)
    {
    }

    //TODO(Ricardo) Temporal
    //ChangeForReader_t(const CacheChange_t* change) : status_(UNSENT),
    ChangeForReader_t(
            CacheChange_t* change)
        : status_(UNSENT)
        , is_relevant_(true)
        , seq_num_(change->sequenceNumber)
        , change_(change)
    {
        if (change->getFragmentSize() != 0)
        {
            unsent_fragments_.base(1u);
            unsent_fragments_.add_range(1u, change->getFragmentCount() + 1u);
        }
    }

    ChangeForReader_t(
            const SequenceNumber_t& seq_num)
        : status_(UNSENT)
        , is_relevant_(true)
        , seq_num_(seq_num)
        , change_(nullptr)
    {
    }

    ~ChangeForReader_t()
    {
    }

    ChangeForReader_t& operator =(
            const ChangeForReader_t& ch)
    {
        status_ = ch.status_;
        is_relevant_ = ch.is_relevant_;
        seq_num_ = ch.seq_num_;
        change_ = ch.change_;
        unsent_fragments_ = ch.unsent_fragments_;
        return *this;
    }

    /**
     * Get the cache change
     * @return Cache change
     */
    // TODO(Ricardo) Temporal
    //const CacheChange_t* getChange() const
    CacheChange_t* getChange() const
    {
        return change_;
    }

    void setStatus(
            const ChangeForReaderStatus_t status)
    {
        status_ = status;
    }

    ChangeForReaderStatus_t getStatus() const
    {
        return status_;
    }

    void setRelevance(
            const bool relevance)
    {
        is_relevant_ = relevance;
    }

    bool isRelevant() const
    {
        return is_relevant_;
    }

    const SequenceNumber_t getSequenceNumber() const
    {
        return seq_num_;
    }

    //! Set change as not valid
    void notValid()
    {
        is_relevant_ = false;
        change_ = nullptr;
    }

    //! Set change as valid
    bool isValid() const
    {
        return change_ != nullptr;
    }

    FragmentNumberSet_t getUnsentFragments() const
    {
        return unsent_fragments_;
    }

    void markAllFragmentsAsUnsent()
    {
        if (change_ != nullptr && change_->getFragmentSize() != 0)
        {
            unsent_fragments_.base(1u);
            unsent_fragments_.add_range(1u, change_->getFragmentCount() + 1u);
        }
    }

    void markFragmentsAsSent(
            const FragmentNumber_t& sentFragment)
    {
        unsent_fragments_.remove(sentFragment);

        if (!unsent_fragments_.empty() && unsent_fragments_.max() < change_->getFragmentCount())
        {
            FragmentNumber_t base = unsent_fragments_.base();
            FragmentNumber_t max = unsent_fragments_.max();
            assert(!unsent_fragments_.is_set(base));

            // Update base to first bit set
            base = unsent_fragments_.min();
            unsent_fragments_.base_update(base);

            // Add all possible fragments
            unsent_fragments_.add_range(max + 1u, change_->getFragmentCount() + 1u);
        }
    }

    void markFragmentsAsUnsent(
            const FragmentNumberSet_t& unsentFragments)
    {
        FragmentNumber_t other_base = unsentFragments.base();
        if (other_base < unsent_fragments_.base())
        {
            unsent_fragments_.base_update(other_base);
        }
        unsentFragments.for_each(
            [this](
                    FragmentNumber_t element)
            {
                unsent_fragments_.add(element);
            });
    }

private:

    //!Status
    ChangeForReaderStatus_t status_;

    //!Boolean specifying if this change is relevant
    bool is_relevant_;

    //!Sequence number
    SequenceNumber_t seq_num_;

    // TODO(Ricardo) Temporal
    //const CacheChange_t* change_;
    CacheChange_t* change_;

    FragmentNumberSet_t unsent_fragments_;
};

struct ChangeForReaderCmp
{
    bool operator ()(
            const ChangeForReader_t& a,
            const ChangeForReader_t& b) const
    {
        return a.seq_num_ < b.seq_num_;
    }

};

#endif
}
}
}
#endif /* _FASTDDS_RTPS_CACHECHANGE_H_ */
