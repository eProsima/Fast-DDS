/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef EPROSIMA_DDS_CORE_REFERENCE_IMPL_HPP_
#define EPROSIMA_DDS_CORE_REFERENCE_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/Reference.hpp>
/*
namespace dds {
namespace core {

// Implementation
template<typename DELEGATE>
Reference<DELEGATE>::Reference(
        null_type&)
    : impl_()
{
}

template<typename DELEGATE>
Reference<DELEGATE>::Reference(
        const Reference& ref)
    : impl_(ref.impl_)
{
}

template<typename DELEGATE>
template<typename D>
Reference<DELEGATE>::Reference(
        const Reference<D>& ref)
{
    //To Implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    impl_ = OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<DELEGATE_T>(ref.impl_);
//    if (impl_ != ref.impl_) {
//        throw dds::core::IllegalOperationError(std::string("Attempted invalid cast: ") + typeid(ref).name() + " to " + typeid(*this).name());
//    }

}

template<typename DELEGATE>
Reference<DELEGATE>::Reference(
        DELEGATE_T* p)
    : impl_(p)
{
}

template<typename DELEGATE>
Reference<DELEGATE>::Reference(
        const DELEGATE_REF_T& p)
    : impl_(p)
{
    //OMG_DDS_LOG("MM", "Reference(DELEGATE_REF_T& p)");
}

template<typename DELEGATE>
Reference<DELEGATE>::~Reference()
{
}

template<typename DELEGATE>
Reference<DELEGATE>::operator DELEGATE_REF_T() const
{
    //To implement
//    ISOCPP_REPORT_STACK_NC_BEGIN();
//    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
//    return impl_;
}

template<typename DELEGATE>
template<typename R>
bool Reference<DELEGATE>::operator==(
        const R& ref) const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    bool equal = false;
//    if (this->is_nil() && ref.is_nil()) {
//        equal = true;
//    } else if (!this->is_nil() && !ref.is_nil()) {
//        equal = (this->delegate() == ref.delegate());
//    }
//    return equal;
}

template<typename DELEGATE>
template<typename R>
bool Reference<DELEGATE>::operator!=(
        const R& ref) const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return !(*this == ref);
}

template<typename DELEGATE>
template<typename D>
Reference<DELEGATE>& Reference<DELEGATE>::operator=(
        const Reference<D>& that)
{
    //To implement
//    OMG_DDS_STATIC_ASSERT((dds::core::is_base_of<DELEGATE_T, D>::value));
//    if(this != (Reference*)&that)
//    {
//        *this = Reference<DELEGATE_T>(that);
//    }
//    return *this;
}

template<typename DELEGATE>
template<typename R>
Reference<DELEGATE>& Reference<DELEGATE>::operator=(
        const R& rhs)
{
   //To implement
//    OMG_DDS_STATIC_ASSERT((dds::core::is_base_of< DELEGATE_T, typename R::DELEGATE_T>::value));
//    if(this != (Reference*)&rhs)
//    {
//        *this = Reference<DELEGATE_T>(rhs);
//    }
//    return *this;
}

template<typename DELEGATE>
Reference<DELEGATE>& Reference<DELEGATE>::operator=(
        const null_type)
{
    //To implement
//    DELEGATE_REF_T tmp;
//    impl_ = tmp;
//    return *this;
}

template<typename DELEGATE>
bool Reference<DELEGATE>::is_nil() const
{
    //To implement
//    return impl_.get() == 0;
}

template<typename DELEGATE>
bool Reference<DELEGATE>::operator==(
        const null_type) const
{
    //To implement
//    return this->is_nil();
}

template<typename DELEGATE>
bool Reference<DELEGATE>::operator!=(
        const null_type) const
{
    //To implement
//    return !(this->is_nil());
}

template<typename DELEGATE>
const typename Reference<DELEGATE>::DELEGATE_REF_T& Reference<DELEGATE>::delegate() const
{
    //To implement
//    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
//    return impl_;
}

template<typename DELEGATE>
typename Reference<DELEGATE>::DELEGATE_REF_T& Reference<DELEGATE>::delegate()
{
    //To implement
//    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
//    return impl_;
}

template<typename DELEGATE>
DELEGATE* Reference<DELEGATE>::operator->()
{
    //To implement
//    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
//    return impl_.get();
}

template<typename DELEGATE>
const DELEGATE* Reference<DELEGATE>::operator->() const
{
    //To implement
//    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
//    return impl_.get();
}

template<typename DELEGATE>
Reference<DELEGATE>::operator const typename Reference<DELEGATE>::DELEGATE_REF_T& () const
{
    //To implement
//    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
//    return impl_;
}

template<typename DELEGATE>
Reference<DELEGATE>::operator typename Reference<DELEGATE>::DELEGATE_REF_T& ()
{
    //To implement
//    ISOCPP_BOOL_CHECK_AND_THROW(impl_, ISOCPP_NULL_REFERENCE_ERROR, "Reference[%d] == dds::core::null", __LINE__);
//    return impl_;
}

template<typename DELEGATE>
void Reference<DELEGATE>::set_ref(
        DELEGATE_T* p)
{
    //To implement
//    impl_.reset(p);
}


template<class D>
bool operator ==(
        null_type, const Reference<D>& r)
{
    //To implement
//    return r.is_nil();
}

template<class D>
bool operator !=(
        null_type, const Reference<D>& r)
{
    //To implement
//    return !r.is_nil();
}

} //namespace core
} //namespace dds
*/
#endif //EPROSIMA_DDS_CORE_REFERENCE_IMPL_HPP_
