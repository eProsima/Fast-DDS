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

#include <fastrtps/types/v1_3/DynamicType.hpp>
#include <fastrtps/types/v1_3/DynamicTypeBuilderFactory.hpp>

namespace std
{

template<>
struct default_delete<eprosima::fastrtps::types::v1_3::DynamicType> {
    void operator()(const eprosima::fastrtps::types::v1_3::DynamicType* pA) const noexcept
    {
        auto& inst = eprosima::fastrtps::types::v1_3::DynamicTypeBuilderFactory::get_instance();
        inst.delete_type(pA);
    }
};

template<>
class shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>
    : public shared_ptr<const void>
{
public:

    using element_type = const eprosima::fastrtps::types::v1_3::DynamicType;
    using base = shared_ptr<const void>;

    constexpr shared_ptr() = default;

    explicit shared_ptr(element_type* pA)
        : base(pA, default_delete<element_type>{}) {}

    shared_ptr(const shared_ptr& r) noexcept
        : base(r) {}

    shared_ptr(shared_ptr&& r) noexcept
        : base(move(r)) {}

    template< class Y >
    shared_ptr(const shared_ptr<Y>& r, element_type* ptr) noexcept
        : base(r, ptr) {}

    template <class T, typename enable_if<is_convertible<T*, element_type*>::value, int>::type = 0>
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

    void reset(element_type* pA)
    {
        base::reset(pA, default_delete<element_type>{});
    }

    element_type* get() const noexcept
    {
        return static_cast<element_type*>(base::get());
    }

    element_type& operator*() const noexcept
    {
        return *get();
    }

    element_type* operator->() const noexcept
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

    shared_ptr(shared_ptr&& r) noexcept
        : base(move(r)) {}

    explicit shared_ptr(element_type* pA)
        : base(pA) {}

    template< class Y >
    shared_ptr(const shared_ptr<Y>& r, element_type* ptr) noexcept
        : base(r, ptr) {}

    template <class T, typename enable_if<is_convertible<T*, element_type*>::value, int>::type = 0>
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

    element_type* get() const noexcept
    {
        return const_cast<element_type*>(base::get());
    }

    element_type& operator*() const noexcept
    {
        return const_cast<element_type&>(*get());
    }

    element_type* operator->() const noexcept
    {
        return get();
    }
};

template<>
class weak_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>
    : public weak_ptr<const void>
{
public:

    using element_type = const eprosima::fastrtps::types::v1_3::DynamicType;
    using base = weak_ptr<const void>;

    constexpr weak_ptr() noexcept = default;

    weak_ptr(const weak_ptr& r) noexcept
        : base(r) {}

    weak_ptr(weak_ptr&& r) noexcept
        : base(move(r)) {}

    weak_ptr(const shared_ptr<element_type>& r) noexcept
        : base(r) {}

    weak_ptr& operator=(const weak_ptr& r) noexcept
    {
        base::operator=(r);
        return *this;
    }

    weak_ptr& operator=(weak_ptr&& r) noexcept
    {
        base::operator=(move(r));
        return *this;
    }

    shared_ptr<element_type> lock() const noexcept
    {
        return static_pointer_cast<element_type>(base::lock());
    }
};

template<>
class weak_ptr<eprosima::fastrtps::types::v1_3::DynamicType>
    : public weak_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>
{
public:

    using element_type = eprosima::fastrtps::types::v1_3::DynamicType;
    using base = weak_ptr<const eprosima::fastrtps::types::v1_3::DynamicType>;

    constexpr weak_ptr() noexcept = default;

    weak_ptr(const weak_ptr& r) noexcept
        : base(r) {}

    weak_ptr(weak_ptr&& r) noexcept
        : base(move(r)) {}

    weak_ptr(const shared_ptr<element_type>& r) noexcept
        : base(r) {}

    weak_ptr& operator=(const weak_ptr& r) noexcept
    {
        base::operator=(r);
        return *this;
    }

    weak_ptr& operator=(weak_ptr&& r) noexcept
    {
        base::operator=(move(r));
        return *this;
    }

    shared_ptr<element_type> lock() const noexcept
    {
        return const_pointer_cast<element_type>(base::lock());
    }
};

} // std

#endif // TYPES_1_3_DYNAMIC_TYPE_PTR_HPP
