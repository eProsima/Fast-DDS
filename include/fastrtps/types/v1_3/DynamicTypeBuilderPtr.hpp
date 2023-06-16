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

#ifndef TYPES_1_3_DYNAMIC_TYPE_BUILDER_PTR_HPP
#define TYPES_1_3_DYNAMIC_TYPE_BUILDER_PTR_HPP

#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

template<>
std::function<void(const DynamicTypeBuilder*)> dynamic_object_deleter(const DynamicTypeBuilder*);

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

namespace std
{

template<>
class shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>
    : public shared_ptr<const void>
{
public:

    using element_type = const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder;
    using base = shared_ptr<const void>;

    constexpr shared_ptr() = default;

    explicit shared_ptr(const element_type* pA)
        : base(pA, dynamic_object_deleter(pA)) {}

    shared_ptr(const shared_ptr& r) noexcept
        : base(r) {}

    shared_ptr(shared_ptr&& r) noexcept
        : base(move(r)) {}

    template< class Y >
    shared_ptr(const shared_ptr<Y>& r, element_type* ptr) noexcept
        : base(r, ptr) {}

    template <class T, enable_if_t<is_convertible<T*, element_type*>::value, int> = 0>
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
        base::reset(pA, dynamic_object_deleter(pA));
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
class shared_ptr<eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>
    : public shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>
{
public:

    using element_type = eprosima::fastrtps::types::v1_3::DynamicTypeBuilder;
    using base = shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>;

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

    template <class T, enable_if_t<is_convertible<T*, element_type*>::value, int> = 0>
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
class weak_ptr<const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>
    : public weak_ptr<const void>
{
public:

    using element_type = const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder;
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
class weak_ptr<eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>
    : public weak_ptr<const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>
{
public:

    using element_type = eprosima::fastrtps::types::v1_3::DynamicTypeBuilder;
    using base = weak_ptr<const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>;

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

#endif // TYPES_1_3_DYNAMIC_TYPE_BUILDER_PTR_HPP
