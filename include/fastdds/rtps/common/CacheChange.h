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

enum ChangeFragmentStatus_t
{
    NOT_PRESENT = 0,
    PRESENT = 1
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

    WriteParams write_params;
    bool is_untyped_ = true;;

    /*!
     * @brief Default constructor.
     * Creates an empty CacheChange_t.
     */
    CacheChange_t()
    {
    }

    CacheChange_t(
            const CacheChange_t&) = delete;
    const CacheChange_t& operator=(
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

        return serializedPayload.copy(&ch_ptr->serializedPayload, (ch_ptr->is_untyped_ ? false : true));
    }

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

        setFragmentSize(ch_ptr->fragment_size_, true);
    }

    ~CacheChange_t()
    {
    }

    uint32_t getFragmentCount() const
    {
        return fragment_count_;
    }

    std::vector<uint32_t>* getDataFragments() { return dataFragments_; }

    uint16_t getFragmentSize() const { return fragment_size_; }

    void setFragmentSize(
            uint16_t fragment_size,
            bool set_fragment_state = false)
    {
        fragment_size_ = fragment_size;
        fragment_count_ = 0;
        first_missing_fragment_ = 0;

        if (fragment_size > 0)
        {
            // This follows RTPS 8.3.7.3.5
            fragment_count_ = (serializedPayload.length + fragment_size - 1) / fragment_size;
            if (set_fragment_state)
            {
                octet* fragment_ptr = serializedPayload.data;
                for (uint32_t i = 1; i <= fragment_count_; i++, fragment_ptr += fragment_size_)
                {
                    *((uint32_t*)fragment_ptr) = i;  // index to next fragment in missing list
                }
            }
        }
    }

    void received_fragments(
            uint32_t initial_fragment,
            uint32_t num_of_fragments)
    {
        if (fragment_size_ > 0)
        {
            if (initial_fragment == first_missing_fragment_)
            {
                first_missing_fragment_ += num_of_fragments;
            }
            else
            {
                // Find prev in missing list
                uint32_t current_frag = first_missing_fragment_;
                while (current_frag < initial_fragment)
                {
                    size_t offset = fragment_size_;
                    offset *= current_frag;
                    uint32_t* fragment = (uint32_t*) &serializedPayload.data[offset];
                    if (*fragment >= initial_fragment)
                    {
                        if (*fragment < initial_fragment + num_of_fragments)
                        {
                            *fragment = initial_fragment + num_of_fragments;
                        }
                    }
                    current_frag = *fragment;
                }
            }
        }
    }

private:

    // Data fragments
    std::vector<uint32_t>* dataFragments_ = nullptr;

    // Fragment size
    uint16_t fragment_size_ = 0;

    // Number of fragments
    uint32_t fragment_count_ = 0;

    // First fragment in missing list
    uint32_t first_missing_fragment_ = 0;
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
 * Enum ChangeFromWriterStatus_t, possible states for a CacheChange_t in a WriterProxy.
 *  @ingroup COMMON_MODULE
 */
enum ChangeFromWriterStatus_t
{
    UNKNOWN = 0,
    MISSING = 1,
    //REQUESTED_WITH_NACK,
    RECEIVED = 2,
    LOST = 3
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
            for (uint32_t i = 1; i != change->getFragmentCount() + 1; i++)
            {
                unsent_fragments_.insert(i);             // Indexed on 1
            }
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

    ~ChangeForReader_t(){}

    ChangeForReader_t& operator=(
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
        FragmentNumberSet_t rv;
        auto min = unsent_fragments_.begin();
        if (min != unsent_fragments_.end())
        {
            rv.base(*min);
            for (FragmentNumber_t fn : unsent_fragments_)
            {
                rv.add(fn);
            }
        }

        return rv;
    }

    void markAllFragmentsAsUnsent()
    {
        if (change_ != nullptr && change_->getFragmentSize() != 0)
        {
            for (uint32_t i = 1; i != change_->getFragmentCount() + 1; i++)
            {
                unsent_fragments_.insert(i);            // Indexed on 1
            }
        }
    }

    void markFragmentsAsSent(
            const FragmentNumber_t& sentFragment)
    {
        unsent_fragments_.erase(sentFragment);
    }

    void markFragmentsAsUnsent(
            const FragmentNumberSet_t& unsentFragments)
    {
        unsentFragments.for_each([this](FragmentNumber_t element)
                    {
                        unsent_fragments_.insert(element);
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

    std::set<FragmentNumber_t> unsent_fragments_;
};

struct ChangeForReaderCmp
{
    bool operator()(
            const ChangeForReader_t& a,
            const ChangeForReader_t& b) const
    {
        return a.seq_num_ < b.seq_num_;
    }
};

/**
 * Struct ChangeFromWriter_t used to indicate the state of a specific change with respect to a specific writer, as well as its relevance.
 *  @ingroup COMMON_MODULE
 */
class ChangeFromWriter_t
{
    friend struct ChangeFromWriterCmp;

public:

    ChangeFromWriter_t()
        : status_(UNKNOWN)
        , is_relevant_(true)
    {

    }

    ChangeFromWriter_t(
            const ChangeFromWriter_t& ch)
        : status_(ch.status_)
        , is_relevant_(ch.is_relevant_)
        , seq_num_(ch.seq_num_)
    {
    }

    ChangeFromWriter_t(
            const SequenceNumber_t& seq_num)
        : status_(UNKNOWN)
        , is_relevant_(true)
        , seq_num_(seq_num)
    {
    }

    ~ChangeFromWriter_t(){};

    ChangeFromWriter_t& operator=(
            const ChangeFromWriter_t& ch)
    {
        status_ = ch.status_;
        is_relevant_ = ch.is_relevant_;
        seq_num_ = ch.seq_num_;
        return *this;
    }

    void setStatus(
            const ChangeFromWriterStatus_t status)
    {
        status_ = status;
    }

    ChangeFromWriterStatus_t getStatus() const
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
    }

    bool operator < (
            const ChangeFromWriter_t& rhs) const
    {
        return seq_num_ < rhs.seq_num_;
    }

private:

    //! Status
    ChangeFromWriterStatus_t status_;

    //! Boolean specifying if this change is relevant
    bool is_relevant_;

    //! Sequence number
    SequenceNumber_t seq_num_;
};
#endif
}
}
}
#endif /* _FASTDDS_RTPS_CACHECHANGE_H_ */
