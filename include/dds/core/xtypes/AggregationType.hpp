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
#include <dds/core/xtypes/Member.hpp>

#include <string>
#include <map>
#include <vector>
#include <cassert>

namespace dds {
namespace core {
namespace xtypes {

class AggregationType : public DynamicType
{
public:
    bool has_member(const std::string& name) const { return indexes_.count(name) != 0; }
    const std::vector<Member>& members() const { return members_; }
    const Member& member(size_t index) const { return members_[index]; }
    const Member& member(const std::string& name) const
    {
        return members_[indexes_.at(name)];
    }

protected:
    AggregationType(
            TypeKind kind,
            const std::string& name)
        : DynamicType(kind, name)
    {}

    Member& insert_member(const Member& member)
    {
        assert(indexes_.count(member.name()) == 0);
        indexes_.emplace(member.name(), members_.size());
        members_.emplace_back(std::move(member));
        return members_.back();
    }

private:
    std::map<std::string, size_t> indexes_;
    std::vector<Member> members_;
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_AGGREGATION_TYPE_HPP_
