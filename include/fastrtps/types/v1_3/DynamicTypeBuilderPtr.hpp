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

#include <fastrtps/types/v1_3/DynamicTypeBuilder.hpp>
#include <fastrtps/types/v1_3/DynamicTypePtr.hpp>
#include <fastrtps/types/v1_3/MemberDescriptor.hpp>

#include <string>
#include <type_traits>

namespace std
{

template<>
struct default_delete<eprosima::fastrtps::types::v1_3::DynamicTypeBuilder> {
    void operator()(const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder* pA) const noexcept
    {
        auto& inst = eprosima::fastrtps::types::v1_3::DynamicTypeBuilderFactory::get_instance();
        inst.delete_type(pA);
    }
};

template<>
class shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>
    : public shared_ptr<const void>
{
public:

    using element_type = const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder;
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
class shared_ptr<eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>
    : public shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>
{
public:

    using element_type = eprosima::fastrtps::types::v1_3::DynamicTypeBuilder;
    using base = shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>;

    using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;
    using MemberId = eprosima::fastrtps::types::v1_3::MemberId;
    using MemberDescriptor = eprosima::fastrtps::types::v1_3::MemberDescriptor;
    using DynamicType = eprosima::fastrtps::types::v1_3::DynamicType;

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

    // ancillary methods

    std::shared_ptr<const DynamicType> build() const
    {
        if(*this)
        {
            return std::shared_ptr<const DynamicType> {get()->build()};
        }
        return {};
    }

    template<typename S,
             typename = typename std::enable_if<std::is_convertible<S, std::string>::value>::type>
    ReturnCode_t add_member(
            const MemberId& id,
            const S& name)
    {
        if(*this)
        {
            MemberDescriptor md;
            md.set_id(id);
            md.set_name(get_null_terminated(name));

            return get()->add_member(md);
        }
        return {};
    }

    template<typename S,
             typename = typename std::enable_if<std::is_convertible<S, std::string>::value>::type>
    ReturnCode_t add_member(
            uint32_t index,
            const S& name)
    {
        if(*this)
        {
            MemberDescriptor md;
            md.set_index(index);
            md.set_name(get_null_terminated(name));

            return get()->add_member(md);
        }
        return {};
    }

    template<typename S,
             typename D,
             typename = typename std::enable_if<
                 std::is_convertible<S, std::string>::value ||
                 std::is_constructible<const std::shared_ptr<const DynamicType>, D>::value ||
                 std::is_constructible<const DynamicType&, D>::value>::type>
    ReturnCode_t add_member(
            uint32_t index,
            const S& name,
            const D& type)
    {
        if(*this)
        {
            MemberDescriptor md;
            md.set_index(index);
            md.set_name(get_null_terminated(name));
            set_descriptor_type(md, type);

            return get()->add_member(md);
        }
        return {};
    }

    template<typename S,
             typename D,
             typename = typename std::enable_if<
                 std::is_convertible<S, std::string>::value ||
                 std::is_constructible<const std::shared_ptr<const DynamicType>, D>::value ||
                 std::is_constructible<const DynamicType&, D>::value>::type>
    ReturnCode_t add_member(
            const MemberId& id,
            const S& name,
            const D& type)
    {
        if(*this)
        {
            MemberDescriptor md;
            md.set_id(id);
            md.set_name(get_null_terminated(name));
            set_descriptor_type(md, type);

            return get()->add_member(md);
        }
        return {};
    }

    template<typename S1,
             typename D,
             typename S2,
             typename = typename std::enable_if<
                 std::is_convertible<S1, std::string>::value ||
                 std::is_convertible<S2, std::string>::value ||
                 std::is_constructible<const std::shared_ptr<const DynamicType>, D>::value ||
                 std::is_constructible<const DynamicType&, D>::value>::type>
    ReturnCode_t add_member(
            const MemberId& id,
            const S1& name,
            const D& type,
            const S2& defaultValue)
    {
        if(*this)
        {
            MemberDescriptor md;
            md.set_id(id);
            md.set_name(get_null_terminated(name));
            set_descriptor_type(md, type);
            md.set_default_value(get_null_terminated(defaultValue));

            return get()->add_member(md);
        }
        return {};
    }

    template<typename S1,
             typename D,
             typename S2,
             typename C,
             typename = typename std::enable_if<
                 std::is_convertible<S1, std::string>::value ||
                 std::is_convertible<S2, std::string>::value ||
                 std::is_constructible<const std::shared_ptr<const DynamicType>, D>::value ||
                 std::is_constructible<const DynamicType&, D>::value>::type,
             typename T = typename C::value_type,
             typename = decltype(*std::declval<C>().data()),
             typename = typename std::enable_if<std::is_convertible<T, uint32_t>::value>::type>
    ReturnCode_t add_member(
            const MemberId& id,
            const S1& name,
            const D& type,
            const S2& defaultValue,
            const C& labels)
    {
        if(*this)
        {
            MemberDescriptor md;
            md.set_id(id);
            md.set_name(get_null_terminated(name));
            set_descriptor_type(md, type);
            md.set_default_value(get_null_terminated(defaultValue));
            md.set_labels(labels.data(), static_cast<uint32_t>(labels.size()));

            return get()->add_member(md);
        }
        return {};
    }

protected:

    static const char* const get_null_terminated(const char* const name)
    {
        return name;
    }

    static const char* const get_null_terminated(const std::string& name)
    {
        return name.c_str();
    }

    static void set_descriptor_type(MemberDescriptor& md, const std::shared_ptr<const DynamicType> sp)
    {
        md.set_type(*sp); // avoid ownership transfer
    }

    static void set_descriptor_type(MemberDescriptor& md, const DynamicType* p)
    {
        md.set_type(p); // enforce ownership transfer
    }

    static void set_descriptor_type(MemberDescriptor& md, const DynamicType& r)
    {
        md.set_type(r); // avoid ownership transfer
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
