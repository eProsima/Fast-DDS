// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _TYPES_DYNAMIC_LOANABLE_SEQUENCE_HPP_
#define _TYPES_DYNAMIC_LOANABLE_SEQUENCE_HPP_

#include <cassert>
#include <memory>

#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicTypePtr.h>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * @brief LoanableSequence specialization for DynamicData.
 *
 * This class provides a sequence container for handling loanable collections
 * of DynamicData.
 *
 * @tparam _NonConstEnabler to enable data access through [] operator on non-const sequences.
 */
template<typename _NonConstEnabler>
class LoanableSequence<fastrtps::types::DynamicData, _NonConstEnabler>
    : public LoanableTypedCollection<fastrtps::types::DynamicData, _NonConstEnabler>
{
public:

    /// Type for the size of the sequence.
    using size_type = LoanableCollection::size_type;

    /// Type for the elements in the sequence.
    using element_type = LoanableCollection::element_type;

    /**
     * @brief Construct a LoanableSequence with a specified dynamic type.
     *
     * @param[in] dyn_type Pointer to the DynamicType.
     */
    LoanableSequence(
            fastrtps::types::DynamicType_ptr dyn_type)
        : dynamic_type_support_(new fastrtps::types::DynamicPubSubType(dyn_type))
    {
    }

    /**
     * @brief Construct a LoanableSequence with a specified maximum size.
     *
     * @param[in] max Maximum size of the sequence.
     */
    LoanableSequence(
            size_type max)
    {
        if (max <= 0)
        {
            return;
        }

        resize(max);
    }

    /**
     * @brief Destructor for LoanableSequence.
     */
    ~LoanableSequence()
    {
        if (elements_ && !has_ownership_)
        {
            EPROSIMA_LOG_WARNING(SUBSCRIBER, "Sequence destroyed with active loan");
            return;
        }

        release();
    }

    /// Deleted copy constructor for LoanableSequence.
    LoanableSequence(
            const LoanableSequence&) = delete;

    /// Deleted copy assignment operator for LoanableSequence.
    LoanableSequence& operator =(
            const LoanableSequence&) = delete;

    /// Move constructor for LoanableSequence.
    LoanableSequence(
            LoanableSequence&&) = default;

    /// Move assignment operator for LoanableSequence.
    LoanableSequence& operator =(
            LoanableSequence&&) = default;

protected:

    using LoanableCollection::maximum_;
    using LoanableCollection::length_;
    using LoanableCollection::elements_;
    using LoanableCollection::has_ownership_;

private:

    /**
     * @brief Resize the sequence to a new maximum size.
     *
     * @param[in] maximum The new maximum size.
     */
    void resize(
            size_type maximum) override
    {
        assert(has_ownership_);

        // Resize collection and get new pointer
        data_.reserve(maximum);
        data_.resize(maximum);
        elements_ = reinterpret_cast<element_type*>(data_.data());

        // Allocate individual elements
        while (maximum_ < maximum)
        {
            data_[maximum_++] = static_cast<fastrtps::types::DynamicData*>(dynamic_type_support_->createData());
        }
    }

    /**
     * @brief Release all elements and clear the sequence.
     */
    void release()
    {
        if (has_ownership_ && elements_)
        {
            for (size_type n = 0; n < maximum_; ++n)
            {
                fastrtps::types::DynamicData* elem = data_[n];
                dynamic_type_support_->deleteData(elem);
            }
            std::vector<fastrtps::types::DynamicData*>().swap(data_);
        }

        maximum_ = 0u;
        length_ = 0u;
        elements_ = nullptr;
        has_ownership_ = true;
    }

    /// Container for holding the DynamicData elements.
    std::vector<fastrtps::types::DynamicData*> data_;

    /// Pointer to the DynamicPubSubType type support.
    std::unique_ptr<fastrtps::types::DynamicPubSubType> dynamic_type_support_;
};

/// Alias for LoanableSequence with DynamicData and true_type.
using DynamicLoanableSequence = LoanableSequence<fastrtps::types::DynamicData, std::true_type>;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _TYPES_DYNAMIC_LOANABLE_SEQUENCE_HPP_
