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
 */
template<typename _NonConstEnabler>
class LoanableSequence<fastrtps::types::DynamicData, _NonConstEnabler>
    : public LoanableTypedCollection<fastrtps::types::DynamicData, _NonConstEnabler>
{
public:

    using size_type = LoanableCollection::size_type;
    using element_type = LoanableCollection::element_type;

    LoanableSequence(
            fastrtps::types::DynamicType_ptr dyn_type)
        : dynamic_type_support_(new fastrtps::types::DynamicPubSubType(dyn_type))
    {
    }

    LoanableSequence(
            size_type max)
    {
        if (max <= 0)
        {
            return;
        }

        resize(max);
    }

    ~LoanableSequence()
    {
        if (elements_ && !has_ownership_)
        {
            EPROSIMA_LOG_WARNING(SUBSCRIBER, "Sequence destroyed with active loan");
            return;
        }

        release();
    }

    LoanableSequence(
            const LoanableSequence& other)
    {
        *this = other;
    }

    LoanableSequence& operator =(
            const LoanableSequence& other)
    {
        if (!has_ownership_)
        {
            release();
        }

        LoanableCollection::length(other.length());
        const element_type* other_buf = other.buffer();
        for (size_type n = 0; n < length_; ++n)
        {
            *static_cast<fastrtps::types::DynamicData*>(elements_[n]) =
                    *static_cast<const fastrtps::types::DynamicData*>(other_buf[n]);
        }

        return *this;
    }

    LoanableSequence(
            LoanableSequence&&) = default;

    LoanableSequence& operator =(
            LoanableSequence&&) = default;

protected:

    using LoanableCollection::maximum_;
    using LoanableCollection::length_;
    using LoanableCollection::elements_;
    using LoanableCollection::has_ownership_;

private:

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

    std::vector<fastrtps::types::DynamicData*> data_;

    std::unique_ptr<fastrtps::types::DynamicPubSubType> dynamic_type_support_;
};

using DynamicLoanableSequence = LoanableSequence<fastrtps::types::DynamicData, std::true_type>;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _TYPES_DYNAMIC_LOANABLE_SEQUENCE_HPP_
