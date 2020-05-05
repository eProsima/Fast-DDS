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
        missing_fragments_ = new FragmentNumberSet_t();
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
        missing_fragments_ = new FragmentNumberSet_t();
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
        *missing_fragments_ = *(ch_ptr->missing_fragments_);

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
        delete missing_fragments_;
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
        return missing_fragments_->empty();
    }

    /*!
     * Fills a FragmentNumberSet_t with the list of missing fragments.
     * @param [out] frag_sns FragmentNumberSet_t where result is stored.
     */
    void get_missing_fragments(
            FragmentNumberSet_t& frag_sns)
    {
        frag_sns = *missing_fragments_;
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
        missing_fragments_->base(0);

        if (fragment_size > 0)
        {
            // This follows RTPS 8.3.7.3.5
            fragment_count_ = (serializedPayload.length + fragment_size - 1) / fragment_size;

            if (create_fragment_list)
            {
                // Fill range of missing fragments
                uint32_t first_missing = 1u;
                uint32_t first_not_missing = fragment_count_ + 1u;
                missing_fragments_->base(first_missing);
                missing_fragments_->add_range(first_missing, first_not_missing);
            }
            else
            {
                // List not created. This means we are going to send this change fragmented, so it is already
                // assembled, and the missing set is empty
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

        received_fragments(fragment_starting_num - 1, fragments_in_submessage);

        memcpy(
            &serializedPayload.data[original_offset],
            incoming_data.data, incoming_length);

        return is_fully_assembled();
    }

private:

    // Fragment size
    uint16_t fragment_size_ = 0;

    // Number of fragments
    uint32_t fragment_count_ = 0;

    // Missing fragments bitmap with sliding window
    BitmapRange<uint32_t>* missing_fragments_;

    /*!
     * Mark a set of consecutive fragments as received.
     * This will remove a set of consecutive fragments from the missing list.
     * Should be called BEFORE copying the received data into the serialized payload.
     *
     * @param initial_fragment Index (0-based) of first received fragment.
     * @param num_of_fragments Number of received fragments. Should be strictly positive.
     */
    void received_fragments(
            uint32_t initial_fragment,
            uint32_t num_of_fragments)
    {
        if ( (fragment_size_ > 0) && (initial_fragment < fragment_count_) )
        {
            uint32_t last_fragment = initial_fragment + num_of_fragments;
            if (last_fragment > fragment_count_)
            {
                last_fragment = fragment_count_;
            }

            // Update to 1-based fragment numbers
            initial_fragment++;
            last_fragment++;

            // We always keep the first missing fragment at the base of the bitmap
            uint32_t first_missing_fragment = missing_fragments_->base();
            assert(missing_fragments_->is_set(first_missing_fragment));
            if (initial_fragment <= first_missing_fragment)
            {
                // This will remove all from the beginning to last_fragment - 1.
                missing_fragments_->base_update(last_fragment);

                // There may be 0's at the beginning. Make sure that first missing is base()
                if (missing_fragments_->empty())
                {
                    // Items removed from bitmap were the only ones present.
                    // This means either that all fragments have alread been received, or that
                    // we are on the worst case of the sliding window (1 0 0 ...... 0).
                    // This means we are safe in assuming that the next missing fragment would be one
                    // complete window ahead.
                    first_missing_fragment += 256UL;
                    if (first_missing_fragment > fragment_count_)
                    {
                        // Getting here means there are no fragments pending
                        return;
                    }

                    // Try to add all possible fragments from the first one missing
                    missing_fragments_->base(first_missing_fragment);
                    missing_fragments_->add_range(first_missing_fragment, fragment_count_ + 1);
                }
                else
                {
                    // Remove possible leading 0's
                    first_missing_fragment = missing_fragments_->min();
                    missing_fragments_->base_update(first_missing_fragment);

                    // Try to add all possible fragments from the last one missing
                    first_missing_fragment = missing_fragments_->max();
                    missing_fragments_->add_range(first_missing_fragment, fragment_count_ + 1);
                }
            }
            else
            {
                for (uint32_t i = initial_fragment; i < last_fragment; ++i)
                {
                    missing_fragments_->remove(i);
                }
            }
        }
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
