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
#ifndef OSPL_DDS_CORE_REFERENCE_IMPL_HPP_
#define OSPL_DDS_CORE_REFERENCE_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/Reference.hpp>
#include <org/opensplice/core/ReportUtils.hpp>

// Implementation
template <typename DELEGATE>
dds::core::Reference<DELEGATE>::Reference(dds::core::null_type&) : impl_()
{
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::Reference(const Reference& ref) : impl_(ref.impl_)
{
}

template <typename DELEGATE>
template <typename D>
dds::core::Reference<DELEGATE>::Reference(const Reference<D>& ref)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    impl_ = OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<DELEGATE_T>(ref.impl_);
    if (impl_ != ref.impl_) {
        throw dds::core::IllegalOperationError(std::string("Attempted invalid cast: ") + typeid(ref).name() + " to " + typeid(*this).name());
    }
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::Reference(DELEGATE_T* p) : impl_(p)
{
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::Reference(const DELEGATE_REF_T& p) : impl_(p)
{
    //OMG_DDS_LOG("MM", "Reference(DELEGATE_REF_T& p)");
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::~Reference()
{
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::operator DELEGATE_REF_T() const
{
    ISOCPP_REPORT_STACK_NC_BEGIN();
    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
    return impl_;
}

template <typename DELEGATE>
template <typename R>
bool
dds::core::Reference<DELEGATE>::operator==(const R& ref) const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    bool equal = false;
    if (this->is_nil() && ref.is_nil()) {
        /* Both delegates are null. */
        equal = true;
    } else if (!this->is_nil() && !ref.is_nil()) {
        /* Check delegates. */
        equal = (this->delegate() == ref.delegate());
    }
    return equal;
}

template <typename DELEGATE>
template <typename R>
bool
dds::core::Reference<DELEGATE>::operator!=(const R& ref) const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return !(*this == ref);
}

template <typename DELEGATE>
template <typename D>
dds::core::Reference<DELEGATE>&
dds::core::Reference<DELEGATE>::operator=(const Reference<D>& that)
{
    OMG_DDS_STATIC_ASSERT((dds::core::is_base_of<DELEGATE_T, D>::value));
    if(this != (Reference*)&that)
    {
        *this = Reference<DELEGATE_T>(that);
    }
    return *this;
}

template <typename DELEGATE>
template <typename R>
dds::core::Reference<DELEGATE>&
dds::core::Reference<DELEGATE>::operator=(const R& rhs)
{
    OMG_DDS_STATIC_ASSERT((dds::core::is_base_of< DELEGATE_T, typename R::DELEGATE_T>::value));
    if(this != (Reference*)&rhs)
    {
        *this = Reference<DELEGATE_T>(rhs);
    }
    return *this;
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>&
dds::core::Reference<DELEGATE>::operator=(const null_type)
{
    DELEGATE_REF_T tmp;
    impl_ = tmp;
    return *this;
}

template <typename DELEGATE>
bool
dds::core::Reference<DELEGATE>::is_nil() const
{
    return impl_.get() == 0;
}

template <typename DELEGATE>
bool
dds::core::Reference<DELEGATE>::operator==(const null_type) const
{
    return this->is_nil();
}

template <typename DELEGATE>
bool
dds::core::Reference<DELEGATE>::operator!=(const null_type) const
{
    return !(this->is_nil());
}

template <typename DELEGATE>
const typename dds::core::Reference<DELEGATE>::DELEGATE_REF_T&
dds::core::Reference<DELEGATE>::delegate() const
{
    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
    return impl_;
}

template <typename DELEGATE>
typename dds::core::Reference<DELEGATE>::DELEGATE_REF_T&
dds::core::Reference<DELEGATE>::delegate()
{
    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
    return impl_;
}

template <typename DELEGATE>
DELEGATE*
dds::core::Reference<DELEGATE>::operator->()
{
    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
    return impl_.get();
}

template <typename DELEGATE>
const DELEGATE*
dds::core::Reference<DELEGATE>::operator->() const
{
    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
    return impl_.get();
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::operator
const typename dds::core::Reference<DELEGATE>::DELEGATE_REF_T& () const
{
    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
    return impl_;
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::operator
typename dds::core::Reference<DELEGATE>::DELEGATE_REF_T& ()
{
    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
    return impl_;
}

template <typename DELEGATE>
void dds::core::Reference<DELEGATE>::set_ref(DELEGATE_T* p)
{
    impl_.reset(p);
}


template <class D> bool operator == (dds::core::null_type, const dds::core::Reference<D>& r)
{
    return r.is_nil();
}

template <class D> bool operator != (dds::core::null_type, const dds::core::Reference<D>& r)
{
    return !r.is_nil();
}

// End of implementation

#endif /* OSPL_DDS_CORE_REFERENCE_IMPL_HPP_ */
