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

#include <string>
#include <dds/core/xtypes/MemberType.hpp>

#define COND_EXCEP_THROW(EXPR, CONT) if (EXPR){ \
        throw IllegalOperationError(CONT); \
}


namespace dds {
namespace core {
namespace xtypes {
namespace detail {


class StructType
{
public:

    StructType()
    {
    }

    void name(
            const std::string& s)
    {
        name_ = s;
    }

    void member(
            const xtypes::MemberType& m)
    {
        members_.push_back(m);
    }

    void members(
            std::vector<xtypes::MemberType>& v)
    {
        members_.reserve( v.size() + members_.size() );

        for (auto it = v.begin(); v.end() != it; ++it)
        {
            members_.emplace_back(*it);
        }
    }

    template <typename MemberIter>
    void members(
            MemberIter& begin,
            MemberIter& end)
    {
        members_.reserve( (end - begin) + members_.size() );

        for (auto it = begin; end != it; ++it)
        {
            members_.emplace_back(*it);
        }
    }

    void annotation(
            xtypes::Annotation& a )
    {
        annotations_.emplace_back(a);
    }

    void annotations(
            std::vector<xtypes::Annotation>& v)
    {
        annotations_.reserve( v.size() + annotations_.size() );

        for (auto it = v.begin(); v.end() != it; ++it)
        {
            annotations_.emplace_back(*it);
        }
    }

    template <typename AnnotationIter>
    void annotations(
            AnnotationIter& begin,
            AnnotationIter& end)
    {
        annotations_.reserve( (end - begin) + annotations_.size() );

        for (auto it = begin; end != it; ++it)
        {
            annotations_.emplace_back(*it);
        }
    }

    const std::vector<xtypes::MemberType>& members() const noexcept
    {
        return members_;
    }

    const xtypes::MemberType& member(
            uint32_t id) const
    {
        COND_EXCEP_THROW(id >= members_.size(), "no such member_id could be found");

        return members_[id];
    }

    const xtypes::MemberType& member(
            const std::string& s) const
    {
        auto retval = find_if(
            members_.begin(),
            members_.end(),
            [&](const xtypes::MemberType& m){
                            return m.name() == s;
                        });

        COND_EXCEP_THROW(retval == members_.end(), "member" + s + "not found");
        return *retval;
    }

    const std::vector<xtypes::Annotation>& annotations()
    {
        return annotations_;
    }

    void remove_member(
            const xtypes::MemberType& m)
    {
        auto rv = find_if(
            members_.begin(),
            members_.end(),
            [&](xtypes::MemberType& t) {
                            return t.name() == m.name();
                        });

        COND_EXCEP_THROW(rv == members_.end(), "could not find " + m.name() + " member");
        members_.erase(rv);
    }

    void remove_annotation(
            const xtypes::Annotation& a)
    {
        auto rv = find_if(
            annotations_.begin(),
            annotations_.end(),
            [&]( xtypes::Annotation& aa)
                        {
                            return aa.akind() == a.akind();
                        });

        COND_EXCEP_THROW(rv == annotations_.end(), "could not find such annotation");
        annotations_.erase(rv);
    }

private:

    std::string name_;
    std::vector<xtypes::MemberType> members_;
    std::vector<xtypes::Annotation> annotations_;
};

} //namespace detail
} //namespace xtypes
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_XTYPES_DETAIL_STRUCT_TYPE_HPP_
