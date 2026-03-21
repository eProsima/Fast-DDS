// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataReaderLoanManager.hpp
 */

#ifndef _FASTDDS_SUBSCRIBER_DATAREADERIMPL_DATAREADERLOANMANAGER_HPP_
#define _FASTDDS_SUBSCRIBER_DATAREADERIMPL_DATAREADERLOANMANAGER_HPP_

#include <algorithm>
#include <cassert>

#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableTypedCollection.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct DataReaderLoanManager
{
    using SampleInfoSeq = LoanableTypedCollection<SampleInfo>;

    explicit DataReaderLoanManager(
            const DataReaderQos& qos)
        : max_samples_(qos.reader_resource_limits().max_samples_per_read)
        , free_loans_(qos.reader_resource_limits().outstanding_reads_allocation)
        , used_loans_(qos.reader_resource_limits().outstanding_reads_allocation)
    {
        for (size_t n = 0; n < qos.reader_resource_limits().outstanding_reads_allocation.initial; ++n)
        {
            OutstandingLoanItem tmp;
            tmp.data_values = new LoanableCollection::element_type[max_samples_];
            tmp.sample_infos = new LoanableCollection::element_type[max_samples_];
            OutstandingLoanItem* result = free_loans_.push_back(tmp);
            static_cast<void>(result);
            assert(result != nullptr);
        }
    }

    ~DataReaderLoanManager()
    {
        for (OutstandingLoanItem& it : free_loans_)
        {
            delete[] it.data_values;
            delete[] it.sample_infos;
        }
    }

    bool has_outstanding_loans() const
    {
        return !used_loans_.empty();
    }

    ReturnCode_t get_loan(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos)
    {
        OutstandingLoanItem* result = nullptr;

        if (free_loans_.empty())
        {
            OutstandingLoanItem tmp;
            result = used_loans_.push_back(tmp);
            if (nullptr == result)
            {
                return RETCODE_OUT_OF_RESOURCES;
            }

            result->data_values = new LoanableCollection::element_type[max_samples_];
            result->sample_infos = new LoanableCollection::element_type[max_samples_];
        }
        else
        {
            result = used_loans_.push_back(free_loans_.back());
            static_cast<void>(result);
            assert(result != nullptr);
            free_loans_.pop_back();
        }

        data_values.loan(result->data_values, max_samples_, 0);
        sample_infos.loan(result->sample_infos, max_samples_, 0);
        return RETCODE_OK;
    }

    ReturnCode_t return_loan(
            LoanableCollection& data_values,
            SampleInfoSeq& sample_infos)
    {
        OutstandingLoanItem tmp;
        tmp.data_values = const_cast<LoanableCollection::element_type*>(data_values.buffer());
        tmp.sample_infos = const_cast<LoanableCollection::element_type*>(sample_infos.buffer());

        if (!used_loans_.remove(tmp))
        {
            return RETCODE_PRECONDITION_NOT_MET;
        }

        OutstandingLoanItem* result = free_loans_.push_back(tmp);
        static_cast<void>(result);
        assert(result != nullptr);
        return RETCODE_OK;
    }

private:

    struct OutstandingLoanItem
    {
        LoanableCollection::element_type* data_values = nullptr;
        LoanableCollection::element_type* sample_infos = nullptr;

        bool operator == (
                const OutstandingLoanItem& other) const
        {
            return other.data_values == data_values && other.sample_infos == sample_infos;
        }

    };

    using collection_type = eprosima::fastdds::ResourceLimitedVector<OutstandingLoanItem>;

    int32_t max_samples_ = 0;
    collection_type free_loans_;
    collection_type used_loans_;
};

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_SUBSCRIBER_DATAREADERIMPL_DATAREADERLOANMANAGER_HPP_
