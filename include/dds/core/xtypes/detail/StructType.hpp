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

#ifndef EPROSIMA_DDS_CORE_XTYPES_DETAIL_STRUCT_TYPE_HPP_
#define EPROSIMA_DDS_CORE_XTYPES_DETAIL_STRUCT_TYPE_HPP_

#include <dds/core/xtypes/detail/DynamicType.hpp>
#include <dds/core/xtypes/MemberType.hpp>

#include <string>

#define COND_EXCEP_THROW(EXPR, CONT) if(EXPR) {\
    throw IllegalOperationError(CONT);\
}


namespace dds {
namespace core {
namespace xtypes {
namespace detail {


class StructType : public DynamicType
{
public:
    StructType(
            const std::string& name)
        : DynamicType(name, TypeKind::STRUCTURE_TYPE)
        , parent_(null)
    {}

    StructType(
            const std::string& name,
            const xtypes::DynamicType& parent)
        : DynamicType(name, TypeKind::STRUCTURE_TYPE)
        , parent_(parent)
    {}

    StructType(
            const std::string& name,
            const xtypes::DynamicType& parent,
            const std::vector<xtypes::MemberType>& members)
        : DynamicType(name, TypeKind::STRUCTURE_TYPE)
        , parent_(parent)
        , members_(members)
    {}

    template<typename MemberIter>
    StructType(
            const std::string& name,
            const xtypes::DynamicType& parent,
            const MemberIter begin,
            const MemberIter end)
        : DynamicType(name, TypeKind::STRUCTURE_TYPE)
        , parent_(parent)
        , members_(begin, end)
    {}

    StructType(
            const std::string& name,
            const xtypes::DynamicType& parent,
            const std::vector<xtypes::MemberType>& members,
            const xtypes::Annotation& annotation)
        : DynamicType(name, TypeKind::STRUCTURE_TYPE, annotation)
        , parent_(parent)
        , members_(members)
    {}

    StructType(
            const std::string& name,
            const xtypes::DynamicType& parent,
            const std::vector<xtypes::MemberType>& members,
            const std::vector<xtypes::Annotation>& annotations)
        : DynamicType(name, TypeKind::STRUCTURE_TYPE, annotations)
        , parent_(parent)
        , members_(members)
    {}

    template<typename MemberIter, typename AnnotationIter>
    StructType(
            const std::string& name,
            const xtypes::DynamicType& parent,
            const MemberIter members_begin,
            const MemberIter members_end,
            const AnnotationIter annotations_begin,
            const AnnotationIter annotations_end)
        : DynamicType(name, TypeKind::STRUCTURE_TYPE, annotations_begin, annotations_end)
        , parent_(parent)
        , members_(members_begin, members_end)
    {}

    const xtypes::DynamicType& parent() const { return parent_; }
    const std::vector<xtypes::MemberType>& members() const { return members_; }

    const xtypes::MemberType& member(
            uint32_t id)const
    {
        COND_EXCEP_THROW(id >= members_.size(), "no such member id could be found");
        return members_[id];
    }

    const xtypes::MemberType& member(
            const std::string& name) const
    {
        auto retval = find_if(
                        members_.begin(),
                        members_.end(),
                        [&](const xtypes::MemberType& m) { return m.name() == name; } );

        COND_EXCEP_THROW(retval == members_.end(), "member" + name + "not found");
        return *retval;
    }

    void add_member(
            const xtypes::MemberType& member)
    {
        members_.emplace_back(member);
    }

    void add_annotation(
            xtypes::Annotation& annotation)
    {
        add_annotation(annotation);
    }

private:
    xtypes::DynamicType parent_;
    std::vector<xtypes::MemberType> members_;
};

} //namespace detail
} //namespace xtypes
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_XTYPES_DETAIL_STRUCT_TYPE_HPP_
