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
#ifndef EPROSIMA_DDS_CORE_XTYPES_DETAIL_DYNAMIC_TYPE_HPP_
#define EPROSIMA_DDS_CORE_XTYPES_DETAIL_DYNAMIC_TYPE_HPP_

#include <string>
#include <dds/core/xtypes/TypeKind.hpp>
#include <functional>
#include <dds/core/xtypes/Annotation.hpp>

namespace dds {
namespace core {
namespace xtypes {
namespace detail {

class DynamicType
{
public:

    const std::string& name() const noexcept
    {
        return name_;
    }

    const TypeKind& kind() const noexcept
    {
        return kind_;
    }

    void name(
            const std::string& name)
    {
        name_ = name;
    }

    void kind(
            const TypeKind& kind)
    {
        kind_ = kind;
    }

    void annotation(
            xtypes::Annotation& a)
    {
        ann_.push_back(a);
    }

    void annotation(
            const std::vector<xtypes::Annotation>& annotations)
    {
        ann_.reserve(annotations.size() + ann_.size());
        for (auto it = annotations.begin(); it != annotations.end(); ++it)
        {
            ann_.emplace_back(*it);
        }
    }

    template<typename AnnoIter>
    void annotation(
            AnnoIter begin,
            AnnoIter end)
    {
        ann_.reserve(ann_.size() + ( end - begin) );
        for (auto it = begin; it != end; ++it)
        {
            ann_.emplace_back(*it);
        }
    }

    const std::vector<xtypes::Annotation>& annotations() const
    {
        return ann_;
    }

private:

    std::string name_;
    TypeKind kind_;
    std::vector<xtypes::Annotation> ann_;
};

} //namespace detail
} //namespace xtypes
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_XTYPES_DETAIL_DYNAMIC_TYPE_HPP_
