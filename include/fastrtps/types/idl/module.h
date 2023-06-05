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

#ifndef TYPES_IDL_MODULE_H_
#define TYPES_IDL_MODULE_H_

#include <fastrtps/types/idl/type.h>

#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/TypesBase.h>

#include <string>
#include <map>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace idl {

class Module
    : public std::enable_shared_from_this<Module>
{

protected:

    using PairModuleSymbol = std::pair<const Module*, std::string>;

public:

    Module()
        : outer_(nullptr)
        , name_("")
    {
    }

    Module(
            const Module& other) = delete;

    std::string scope() const
    {
        if (outer_ != nullptr && !outer_->scope().empty())
        {
            return outer_->scope() + "::" + name_;
        }
        return name_;
    }

    bool has_symbol(
            const std::string& ident,
            bool extend = true) const
    {
        bool has_it = structs_.count(ident) > 0
                || unions_.count(ident) > 0
                || aliases_.count(ident) > 0
                //|| constants_.count(ident) > 0
                || enumerations_32_.count(ident) > 0
                || inner_.count(ident) > 0;

        if (has_it)
        {
            return true;
        }
        if (extend && outer_ != nullptr)
        {
            return outer_->has_symbol(ident, extend);
        }

        return false;
    }

    bool structure(
            v1_3::DynamicType&& struct_type,
            bool replace = false)
    {
        if (struct_type.get_name().find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        if (replace)
        {
            auto it = structs_.find(struct_type.get_name());
            if (it != structs_.end())
            {
                structs_.erase(it);
            }
        }

        std::string name = struct_type.get_name();
        std::string name_space = scope();
        // TODO set name of struct type?
        //struct_type.set_name(name_space + (name_space.empty() ? "" : "::") + name);
        auto result = structs_.emplace(
            name,
            Type(*this, std::move(struct_type)));

        return result.second;
    }

    bool union_switch(
            v1_3::DynamicType&& union_type,
            bool replace = false)
    {
        if (union_type.get_name().find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        if (replace)
        {
            auto it = unions_.find(union_type.get_name());
            if (it != unions_.end())
            {
                unions_.erase(it);
            }
        }

        std::string name = union_type.get_name();
        std::string name_space = scope();
        // TODO set name of union type?
        //union_type.set_name(name_space + (name_space.empty() ? "" : "::") + name);
        auto result = unions_.emplace(
            name,
            Type(*this, std::move(union_type)));
        return result.second;
    }

protected:

    std::map<std::string, Type> aliases_;
    std::map<std::string, Type> constants_types_;
    //std::map<std::string, v1_3::DynamicData> constants_;
    std::vector<std::string> from_enum_;
    std::map<std::string, Type> enumerations_32_;
    std::map<std::string, Type> structs_;
    std::map<std::string, Type> unions_;
    //std::map<std::string, std::shared_ptr<AnnotationType>> annotations_;
    Module* outer_;
    std::map<std::string, std::shared_ptr<Module>> inner_;
    std::string name_;
};

} // namespace idl
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_IDL_MODULE_H_
