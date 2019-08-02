/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef OSPL_DDS_SUB_DETAIL_LOANED_SAMPLES_IMPL_HPP_
#define OSPL_DDS_SUB_DETAIL_LOANED_SAMPLES_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */

// Implementation

namespace dds
{
namespace sub
{

template <typename T, template <typename Q> class DELEGATE>
LoanedSamples<T, DELEGATE>::LoanedSamples() : delegate_(new DELEGATE<T>()) { }

template <typename T, template <typename Q> class DELEGATE>
LoanedSamples<T, DELEGATE>::~LoanedSamples() {  }

template <typename T, template <typename Q> class DELEGATE>
LoanedSamples<T, DELEGATE>::LoanedSamples(const LoanedSamples& other)
{
    delegate_ = other.delegate_;
}

template <typename T, template <typename Q> class DELEGATE>
typename LoanedSamples<T, DELEGATE>::const_iterator LoanedSamples<T, DELEGATE>::begin() const
{
    return delegate()->begin();
}

template <typename T, template <typename Q> class DELEGATE>
typename LoanedSamples<T, DELEGATE>::const_iterator LoanedSamples<T, DELEGATE>::end() const
{
    return delegate()->end();
}

template <typename T, template <typename Q> class DELEGATE>
const typename LoanedSamples<T, DELEGATE>::DELEGATE_REF_T& LoanedSamples<T, DELEGATE>::delegate() const
{
    return delegate_;
}

template <typename T, template <typename Q> class DELEGATE>
typename LoanedSamples<T, DELEGATE>::DELEGATE_REF_T& LoanedSamples<T, DELEGATE>::delegate()
{
    return delegate_;
}

template <typename T, template <typename Q> class DELEGATE>
uint32_t LoanedSamples<T, DELEGATE>::length() const
{
    return delegate_->length();
}

template <typename T, template <typename Q> class D>
LoanedSamples<T, D> move(LoanedSamples<T, D>& a)
{
    /* Copy reference (not the data) into new LoanedSamples. */
    LoanedSamples<T, D> samples(a);
    /* Reset reference of the old LoanedSamples. */
    a.delegate() = NULL;
    /* Move is completed. */
    return samples;
}

}
}

// End of implementation

#endif /* OSPL_DDS_SUB_DETAIL_LOANED_SAMPLES_IMPL_HPP_ */
