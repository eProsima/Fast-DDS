// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_1_3_DYNAMIC_TYPE_PTR_HPP
#define TYPES_1_3_DYNAMIC_TYPE_PTR_HPP

#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

template<>
std::function<void(DynamicType*)> dynamic_object_deleter(const DynamicType*);

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

namespace std
{

template<>
class shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>
    : public shared_ptr<void>
{
public:

    using element_type = const eprosima::fastrtps::types::v1_3::DynamicType;
    using base = shared_ptr<void>;

    constexpr shared_ptr() = default;

    explicit shared_ptr(const eprosima::fastrtps::types::v1_3::DynamicType* pA)
        : base(const_cast<eprosima::fastrtps::types::v1_3::DynamicType*>(pA), dynamic_object_deleter<element_type>) {}

    shared_ptr(const shared_ptr& r) noexcept
        : base(r) {}

    shared_ptr(shared_ptr&& r) noexcept
        : base(move(r)) {}

    template< class Y >
    shared_ptr(const shared_ptr<Y>& r, element_type* ptr) noexcept
        : base(r, const_cast<eprosima::fastrtps::types::v1_3::DynamicType*>(ptr)) {}

    template <class T, enable_if_t<is_convertible<element_type, T>::value, int> = 0>
    explicit shared_ptr(const weak_ptr<T>& r)
        : base(r) {}

    shared_ptr& operator=(const shared_ptr& r) noexcept
    {
        return static_cast<shared_ptr&>(base::operator=(r));
    }

    shared_ptr& operator=(shared_ptr&& r) noexcept
    {
        return static_cast<shared_ptr&>(base::operator=(move(r)));
    }

    void reset() noexcept
    {
        base::reset();
    }

    void reset(const eprosima::fastrtps::types::v1_3::DynamicType* pA)
    {
        base::reset(const_cast<eprosima::fastrtps::types::v1_3::DynamicType*>(pA),
                dynamic_object_deleter<element_type>);
    }

    element_type* get() const noexcept
    {
        return static_cast<element_type*>(base::get());
    }

    const eprosima::fastrtps::types::v1_3::DynamicType& operator*() const noexcept
    {
        return *get();
    }

    const eprosima::fastrtps::types::v1_3::DynamicType* operator->() const noexcept
    {
        return get();
    }
};

template<>
class shared_ptr<eprosima::fastrtps::types::v1_3::DynamicType>
    : public shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>
{
public:

    using element_type = eprosima::fastrtps::types::v1_3::DynamicType;
    using base = shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>;

    constexpr shared_ptr() = default;

    shared_ptr(const shared_ptr& r) noexcept
        : base(r) {}

    shared_ptr(shared_ptr<eprosima::fastrtps::types::v1_3::DynamicType>&& r) noexcept
        : base(move(r)) {}

    explicit shared_ptr(eprosima::fastrtps::types::v1_3::DynamicType* pA)
        : base(pA) {}

    template< class Y >
    shared_ptr(const shared_ptr<Y>& r, element_type* ptr) noexcept
        : base(r, ptr) {}

    template <class T, enable_if_t<is_convertible<element_type, T>::value, int> = 0>
    explicit shared_ptr(const weak_ptr<T>& r)
        : base(r) {}

    shared_ptr& operator=(const shared_ptr& r) noexcept
    {
        return static_cast<shared_ptr&>(base::operator=(r));
    }

    shared_ptr& operator=(shared_ptr&& r) noexcept
    {
        return static_cast<shared_ptr&>(base::operator=(move(r)));
    }

    eprosima::fastrtps::types::v1_3::DynamicType* get() const noexcept
    {
        return const_cast<eprosima::fastrtps::types::v1_3::DynamicType*>(base::get());
    }

    eprosima::fastrtps::types::v1_3::DynamicType& operator*() const noexcept
    {
        return const_cast<eprosima::fastrtps::types::v1_3::DynamicType&>(*get());
    }

    eprosima::fastrtps::types::v1_3::DynamicType* operator->() const noexcept
    {
        return get();
    }
};

template<>
class weak_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>
    : public weak_ptr<void>
{
public:

    using element_type = const eprosima::fastrtps::types::v1_3::DynamicType;
    using base = weak_ptr<void>;

    constexpr weak_ptr() noexcept = default;

    weak_ptr(const weak_ptr& r) noexcept
        : base(r) {}

    weak_ptr(weak_ptr&& r) noexcept
        : base(move(r)) {}

    weak_ptr(const shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>& r) noexcept
        : base(const_pointer_cast<eprosima::fastrtps::types::v1_3::DynamicType>(r)) {}

    weak_ptr& operator=( const weak_ptr& r) noexcept
    {
        return static_cast<weak_ptr&>(base::operator=(r));
    }

    weak_ptr& operator=( weak_ptr&& r) noexcept
    {
        return static_cast<weak_ptr&>(base::operator=(move(r)));
    }

    shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicType> lock() const noexcept
    {
        return static_pointer_cast<const eprosima::fastrtps::types::v1_3::DynamicType>(base::lock());
    }
};

template<>
class weak_ptr<eprosima::fastrtps::types::v1_3::DynamicType>
    : public weak_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>
{
public:

    using element_type = eprosima::fastrtps::types::v1_3::DynamicType;
    using base = weak_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>;

    weak_ptr(const weak_ptr& r) noexcept
        : base(r) {}

    weak_ptr(weak_ptr&& r) noexcept
        : base(move(r)) {}

    weak_ptr(const shared_ptr<eprosima::fastrtps::types::v1_3::DynamicType>& r) noexcept
        : base(r) {}

    shared_ptr<eprosima::fastrtps::types::v1_3::DynamicType> lock() const noexcept
    {
        return const_pointer_cast<eprosima::fastrtps::types::v1_3::DynamicType>(base::lock());
    }
};

} // std

#endif // TYPES_1_3_DYNAMIC_TYPE_PTR_HPP
