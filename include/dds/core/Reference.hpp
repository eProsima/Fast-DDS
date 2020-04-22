/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
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
 */

#ifndef OMG_DDS_CORE_REFERENCE_HPP_
#define OMG_DDS_CORE_REFERENCE_HPP_

#include <dds/core/types.hpp>
#include <dds/core/refmacros.hpp>
#include <dds/core/ref_traits.hpp> //used when macros of refmacros.hpp expand

namespace dds {
namespace core {

/**
 * @brief
 * Base class for reference-counted objects.
 *
 * All objects that have a reference-type have an associated shallow (polymorphic)
 * assignment operator that simply changes the value of the reference.
 * Furthermore, reference-types are safe, meaning that under no circumstances can
 * a reference point to an invalid object.
 * At any single point in time a reference can either refer to the null object or
 * to a valid object.
 *
 * The semantics for Reference types is defined by the DDS-PSM-Cxx class
 * dds::core::Reference. In the context of this specification the semantics implied
 * by the ReferenceType is mandatory, yet the implementation supplied as part of
 * this standard is provided to show one possible way of implementing this semantics.
 *
 * List of reference types:
 *
 * * Entity
 * * Condition
 * * GuardCondition
 * * ReadCondition
 * * QueryCondition
 * * Waitset
 * * DomainParticipant
 * * AnyDataWriter
 * * Publisher
 * * DataWriter
 * * AnyDataReader
 * * Subscriber
 * * DataReader
 * * SharedSamples
 * * AnyTopic
 * * Topic
 *
 * Instances of reference types are created using C++ constructors.
 * The trivial constructor is not defined for reference types; the only
 * alternative to properly constructing a reference is to initialize it to a
 * null reference by assigning dds::core::null.
 *
 * Resource management for some reference types might involve relatively
 * heavyweight operating-system resources (such as threads, mutexes,
 * and network sockets) in addition to memory.
 * These objects therefore provide a function close() that shall halt network
 * communication (in the case of entities) and dispose of any appropriate
 * operating-system resources.
 *
 * Users of this PSM are recommended to call close on objects of all reference
 * types once they are finished using them. In addition, implementations may
 * automatically close objects that they deem to be no longer in use,
 * subject to the following restrictions:
 *
 * * Any object to which the application has a direct reference
 *   (not including a WeakReference) is still in use.
 *
 * * Any object that has been explicitly retained is still in use.
 *
 * * The creator of any object that is still in use is itself still in use.
 *
 */
template<typename DELEGATE>
class Reference
{
public:

    DECLARE_TYPE_TRAITS(
        DELEGATE)

    /**
     * Creates a "null" Reference.
     *
     * @param null
     */
    explicit Reference(
            dds::core::null_type& n)
        : impl_(n)
    {
    }

    /**
     * Creates a Reference from another.
     *
     * @param ref the other reference
     */
    explicit Reference(
            const Reference& ref)
        : impl_(ref.impl_)
    {
    }
    
    /**
     * Assign a Reference from another.
     *
     * @param ref the other reference
     */
    Reference& operator = (
            const Reference& ref)
    {
        impl_ = ref.impl_;
        return *this;
    }

    /**
     * Creates a Reference from other Reference type safely.
     *
     * @param ref the other reference
     */
    template<typename D>
    explicit Reference(
            const Reference<D>& ref)
    {
        impl_ = std::dynamic_pointer_cast<DELEGATE_T>(ref.impl_);
        if (impl_ != ref.impl_)
        {
            throw
                IllegalOperationError(std::string("Attempted invalid cast: ") +
                        typeid(ref).name() + " to " + typeid(*this).name());
        }

    }

    /** @cond
     * The following two constructors create a dds Reference from a vendor
     * specific delegate.
     *
     * They are public, because the implementation of other delegates may
     * need to be able to create References in that manner.
     *
     * However, it shouldn't actually be part of the API. So, leave it from
     * the doxygen generated API documentation.
     */
    explicit Reference(
            DELEGATE_T* p)
        : impl_(p)
    {
    }

    explicit Reference(
            const DELEGATE_REF_T& p)
        : impl_(p)
    {
    }

    /** @endcond */

    /**
     * Destroys a Reference.
     *
     * There might be an associated garbage collection activity when
     * the current reference is not empty. When the underlaying delegate
     * is referenced by another Reference object as well, then that
     * delegate will not be destroyed.
     */
    ~Reference()
    {
    }

    /** @cond
     * Function possibly needed for delegate implementation, but not recommended
     * for application usage: exclude from the API documentation for clarity.
     *
     * Returns an object to the underlying delegate.
     */
    operator DELEGATE_REF_T() const
    {
        return impl_;
    }
    /** @endcond */

    /**
     * Compares two Reference objects and returns true if they are equal.
     *
     * Equality is based on the referential equality of the object being
     * pointed.
     *
     * @param ref the other Reference object
     * @return true when equal
     */
    template<typename R>
    bool operator ==(
            const R& ref) const
    {
        bool equal = false;
        if (this->is_nil() && ref.is_nil())
        {
            /* Both delegates are null. */
            equal = true;
        }
        else if (!this->is_nil() && !ref.is_nil())
        {
            /* Check delegates. */
            equal = (this->delegate() == ref.delegate());
        }
        return equal;
    }

    /**
     * Compares two Reference objects and returns true if they are not
     * equal.
     *
     * Inequality is based on the referential inequality of the object
     * being pointed to.
     *
     * @param ref the other Reference object
     * @return true when <i>not</i> equal
     */
    template<typename R>
    bool operator !=(
            const R& ref) const
    {
        // FRANAVA read something removing the this keyword.
        return !(*this == ref);
    }

    /**
     * Assign new referenced object to this dds reference.
     *
     * There might be an associated garbage collection activity when
     * the current reference is not empty.
     *
     * @return reference pointing to the new object.
     */
    template<typename D>
    Reference& operator =(
            const Reference<D>& that)
    {
        //To implement
        static_assert((is_base_of<DELEGATE_T, D>::value), "value error");
        if (this != (Reference*)&that)
        {
            *this = Reference<DELEGATE_T>(that);
        }
        return *this;

    }

    /** @copydoc dds::core::Reference::operator=(const Reference<D>& that) */
    template<typename R>
    Reference& operator =(
            const R& rhs)
    {
        static_assert(
            is_base_of< DELEGATE_T, typename R::DELEGATE_T>::value,
            "location: operator=()" );
        if (this != (Reference*)&rhs)
        {
            *this = Reference<DELEGATE_T>(rhs);
        }
        return *this;

    }

    /**
     * Special assignment operators that takes care of assigning
     * <i>dds::core::null</i> to this reference.
     *
     * When assigning null, there might be an associated garbage collection
     * activity.
     *
     * @return reference pointing to a null object.
     */

    Reference& operator =(
            const null_type nil)
    {
        this = nil;
        return this;
    }

    /**
     * Check if the referenced object is nil.
     *
     * In other words, check if the reference is pointing to a null object.
     *
     * @return true if the referenced object is null.
     */
    bool is_nil() const
    {
        return impl_.get() == 0;
    }

    /**
     * Special operator== used to check if this reference object
     * equals the <i>dds::core::null</i> reference.
     *
     * The null-check can be done like this:
     * @code{.cpp}
     * if (r == dds::core::null) {
     *    // Do not use the dds reference object r in its current state
     * }
     * @endcode
     *
     * @return true if this reference is null.
     */
    bool operator ==(
            const null_type) const
    {
        return this->is_nil();
    }

    /**
     * Special operator!= used to check if this reference object
     * does not equal the <i>dds::core::null</i> reference.
     *
     * The non-null-check can be done like this:
     * @code{.cpp}
     * if (r != dds::core::null) {
     *    // Use the dds reference object r
     * }
     * @endcode
     *
     * @return true if this reference is not null.
     */
    bool operator !=(
            const null_type) const
    {
        return !(this->is_nil());
    }

private:

    // -- disallow dynamic allocation for reference types
    void* operator new(
            size_t);

public:

    /** @cond
     * Functions possibly needed for delegate implementation, but not recommended
     * for application usage: exclude from the API documentation for clarity.
     *
     * Returns an object to the underlying delegate.
     */
    DELEGATE_REF_T& delegate()
    {
        return impl_;
    }

    const DELEGATE_REF_T& delegate() const
    {
        return impl_;
    }

    /** @endcond */

    /**
     * The operator->() is provided to be able to directly invoke
     * functions on the delegate.
     *
     * The decision to provide direct access to
     * the delegate was motivated by the need for providing a way that
     * was not invasive with respect to the CXXDDS API and yet would allow
     * for vendor-specific extension.
     * Thus vendor-specific extensions can be invoked on the Reference
     * and on all its subclasses as follows:
     * @code{.cpp}
     * my_dds_entity.standard_function();
     * my_dds_entity->vendor_specific_extension();
     * @endcode
     *
     * @return a reference to delegate.
     */
    DELEGATE* operator ->()
    {
        return impl_.get();

    }

    /** @copydoc dds::core::Reference::operator->() */
    const DELEGATE* operator ->() const
    {
        return impl_.get();
    }

    /** @cond
     * Functions possibly needed for delegate implementation, but not recommended
     * for application usage: exclude from the API documentation for clarity.
     *
     * Returns an object to the underlying delegate.
     */
    operator DELEGATE_REF_T& ()
    {
        return impl_;
    }

    operator const DELEGATE_REF_T& () const
    {
        return impl_;
    }
    /** @endcond */

protected:

    Reference()
    {
    }

    void set_ref(
            DELEGATE_T* p)
    {
        impl_.reset(p);
    }

protected:

    DELEGATE_REF_T impl_;
};


} //namespace core
} //namespace dds


/**
 * Special operator== used to check if this reference object
 * does not equal the <i>dds::core::null</i> reference.
 *
 * The non-null-check can be done like this:
 * @code{.cpp}
 * if (dds::core::null == r) {
 *    // Do not use the dds reference object r in its current state
 * }
 * @endcode
 *
 * @return true if this reference is not null.
 */
template<class D>
bool operator ==(
        dds::core::null_type,
        const dds::core::Reference<D>& r)
{
    return r.is_nil();
}

/**
 * Special operator!= used to check if this reference object
 * does not equal the <i>dds::core::null</i> reference.
 *
 * The non-null-check can be done like this:
 * @code{.cpp}
 * if (dds::core::null != r) {
 *    // Use the dds reference object r
 * }
 * @endcode
 *
 * @return true if this reference is not null.
 */
template<class D>
bool operator !=(
        dds::core::null_type,
        const dds::core::Reference<D>& r)
{
    return !r.is_nil();
}

#endif // OMG_DDS_CORE_REFERENCE_HPP_
