/*
 * Copyright 2010, Object Management Group, Inc.
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

#ifndef OMG_DDS_CORE_XTYPES_AGGREGATION_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_AGGREGATION_TYPE_HPP_

#include <dds/core/xtypes/DynamicType.hpp>

#include <string>
#include <map>

namespace dds {
namespace core {
namespace xtypes {

template<typename MemberImpl>
class AggregationType : public DynamicType
{
public:
    size_t member_count() const { return members_.size(); }
    const MemberImpl& member(const std::string& name) const { return members_.at(name); }

protected:
    AggregationType(
            TypeKind kind,
            const std::string& name)
        : DynamicType(kind, name)
    {}

    MemberImpl& insert_member(const std::string& name, MemberImpl&& member)
    {
        return members_.emplace(name, std::move(member)).first->second;
    }

private:
    std::map<std::string, MemberImpl> members_;
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_AGGREGATION_TYPE_HPP_
