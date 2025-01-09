// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLMODULE_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLMODULE_HPP

#include <map>
#include <string>

#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

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

    // Module& create_submodule(
    //         const std::string& submodule)
    // {
    //     std::shared_ptr<Module> new_submodule(new Module(this, submodule));
    //     auto result = inner_.emplace(submodule, new_submodule);
    //     return *result.first->second.get();
    // }

    // std::shared_ptr<Module> submodule(
    //         const std::string& submodule)
    // {
    //     return inner_[submodule];
    // }

    // size_t submodule_size()
    // {
    //     return inner_.size();
    // }

    // using ModuleVisitor = std::function<void(const Module& mod)>;

    // void for_each_submodule(
    //         ModuleVisitor visitor,
    //         const Module* module,
    //         bool recursive = true) const
    // {
    //     for (const auto& inner : module->inner_)
    //     {
    //         visitor(*inner.second.get());
    //         if (recursive)
    //         {
    //             for_each_submodule(visitor, inner.second.get());
    //         }
    //     }
    // }

    // void for_each_submodule(
    //         ModuleVisitor visitor,
    //         bool recursive = true) const
    // {
    //     for_each_submodule(visitor, this, recursive);
    // }

    // void for_each(
    //         ModuleVisitor visitor) const
    // {
    //     visitor(*this);
    //     for_each_submodule(visitor, this);
    // }

    // bool has_submodule(
    //         const std::string& submodule) const
    // {
    //     return inner_.count(submodule) > 0;
    // }

    // Module& operator [] (
    //         const std::string& submodule)
    // {
    //     return *inner_[submodule];
    // }

    // const Module& operator [] (
    //         const std::string& submodule) const
    // {
    //     return *inner_.at(submodule);
    // }

    // const std::string& name() const
    // {
    //     return name_;
    // }

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
                || constants_.count(ident) > 0
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

    bool has_structure(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return false;
        }
        return module.first->structs_.count(module.second) > 0;
    }

    // v1_3::DynamicType& structure(
    //         const std::string& name)
    // {
    //     // Solve scope
    //     PairModuleSymbol module = resolve_scope(name);
    //     if (module.first == nullptr)
    //     {
    //         EPROSIMA_LOG_ERROR(IDLPARSER, "Cannot solve scope for structure '" << name << "'.");
    //         throw std::runtime_error("Cannot solve scope for structure '" + name + "'.");
    //     }

    //     auto it = module.first->structs_.find(module.second);
    //     if (it == module.first->structs_.end())
    //     {
    //         EPROSIMA_LOG_ERROR(IDLPARSER, "Cannot find structure '" << name << "'.");
    //         throw std::runtime_error("Cannot find structure '" + name + "'.");
    //     }

    //     return const_cast<v1_3::DynamicType&>(*it->second.get());
    // }

    // bool structure(
    //         v1_3::DynamicType& struct_type)
    // {
    //     if (struct_type.get_name().find("::") != std::string::npos)
    //     {
    //         return false; // Cannot add a symbol with scoped name.
    //     }

    //     std::string name = struct_type.get_name();
    //     std::string name_space = scope();
    //     // TODO set name of struct type?
    //     //struct_type.set_name(name_space + (name_space.empty() ? "" : "::") + name);
    //     auto result = structs_.emplace(
    //         name,
    //         Type(*this, struct_type));
    //     return result.second;
    // }

    // bool structure(
    //         v1_3::DynamicType&& struct_type,
    //         bool replace = false)
    // {
    //     if (struct_type.get_name().find("::") != std::string::npos)
    //     {
    //         return false; // Cannot add a symbol with scoped name.
    //     }

    //     if (replace)
    //     {
    //         auto it = structs_.find(struct_type.get_name());
    //         if (it != structs_.end())
    //         {
    //             structs_.erase(it);
    //         }
    //     }

    //     std::string name = struct_type.get_name();
    //     std::string name_space = scope();
    //     // TODO set name of struct type?
    //     //struct_type.set_name(name_space + (name_space.empty() ? "" : "::") + name);
    //     auto result = structs_.emplace(
    //         name,
    //         Type(*this, std::move(struct_type)));

    //     return result.second;
    // }

    bool structure(
            DynamicTypeBuilder::_ref_type builder,
            bool replace = false)
    {
        std::string name(builder->get_name());
        if (name.find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        if (replace)
        {
            auto it = structs_.find(name);
            if (it != structs_.end())
            {
                structs_.erase(it);
            }
        }

        std::string name_space = scope();
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        builder->get_descriptor(type_descriptor);
        type_descriptor->name(name_space + (name_space.empty() ? "" : "::") + name);
        auto result = structs_.emplace(name, builder);

        return result.second;
    }

    bool has_union(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return false;
        }
        return module.first->unions_.count(module.second) > 0;
    }

    // const v1_3::DynamicType union_switch(
    //         const std::string& name) const
    // {
    //     // Solve scope
    //     PairModuleSymbol module = resolve_scope(name);
    //     if (!module.first)
    //     {
    //         EPROSIMA_LOG_ERROR(IDLPARSER, "Cannot solve scope for union '" << name << "'.");
    //         throw std::runtime_error("Cannot solve scope for union '" + name + "' was already declared'.");
    //     }

    //     auto it = module.first->unions_.find(module.second);
    //     if (it == module.first->unions_.end())
    //     {
    //         EPROSIMA_LOG_ERROR(IDLPARSER, "Cannot find union '" << name << "'.");
    //         throw std::runtime_error("Cannot find union '" + name + "'.");
    //     }

    //     return *it->second.get();
    // }

    // v1_3::DynamicType union_switch(
    //         const std::string& name)
    // {
    //     // Solve scope
    //     PairModuleSymbol module = resolve_scope(name);
    //     if (!module.first)
    //     {
    //         EPROSIMA_LOG_ERROR(IDLPARSER, "Cannot solve scope for union '" << name << "'.");
    //         throw std::runtime_error("Cannot solve scope for union '" + name + "' was already declared'.");
    //     }

    //     auto it = module.first->unions_.find(module.second);
    //     if (it == module.first->unions_.end())
    //     {
    //         EPROSIMA_LOG_ERROR(IDLPARSER, "Cannot find union '" << name << "'.");
    //         throw std::runtime_error("Cannot find union '" + name + "'.");
    //     }

    //     return const_cast<v1_3::DynamicType&>(*it->second.get());
    // }

    bool union_switch(
            DynamicTypeBuilder::_ref_type builder)
    {
        std::string name(builder->get_name());
        if (name.find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        std::string name_space = scope();
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        builder->get_descriptor(type_descriptor);
        type_descriptor->name(name_space + (name_space.empty() ? "" : "::") + name);
        auto result = unions_.emplace(name, builder);

        return result.second;
    }

    // bool union_switch(
    //         v1_3::DynamicType&& union_type,
    //         bool replace = false)
    // {
    //     if (union_type.get_name().find("::") != std::string::npos)
    //     {
    //         return false; // Cannot add a symbol with scoped name.
    //     }

    //     if (replace)
    //     {
    //         auto it = unions_.find(union_type.get_name());
    //         if (it != unions_.end())
    //         {
    //             unions_.erase(it);
    //         }
    //     }

    //     std::string name = union_type.get_name();
    //     std::string name_space = scope();
    //     // TODO set name of union type?
    //     //union_type.set_name(name_space + (name_space.empty() ? "" : "::") + name);
    //     auto result = unions_.emplace(
    //         name,
    //         Type(*this, std::move(union_type)));
    //     return result.second;
    // }

    DynamicData::_ref_type constant(
            const std::string& name) const
    {
        DynamicData::_ref_type xdata;

        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return xdata;
        }

        auto it = module.first->constants_.find(module.second);
        if (it != module.first->constants_.end())
        {
            return it->second;
        }

        return xdata;
    }

    bool has_constant(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return false;
        }

        auto it = module.first->constants_.find(module.second);
        return it != module.first->constants_.end();
    }

    bool create_constant(
            const std::string& name,
            DynamicData::_ref_type xdata,
            bool replace = false,
            bool from_enumeration = false)
    {
        if (name.find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        if (replace)
        {
            auto it = constants_.find(name);
            if (it != constants_.end())
            {
                constants_.erase(it);
            }
        }

        auto result = constants_.emplace(name, xdata);
        if (result.second && from_enumeration)
        {
            from_enum_.push_back(name);
        }
        return result.second;
    }

    bool has_enum_32(
            const std::string& name) const
    {
        return enumerations_32_.count(name) > 0;
    }

    bool enum_32(
            const std::string& name,
            DynamicTypeBuilder::_ref_type builder,
            bool replace = false)
    {
        if (name.find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        if (replace)
        {
            auto it = enumerations_32_.find(name);
            if (it != enumerations_32_.end())
            {
                enumerations_32_.erase(it);
            }
        }

        std::string name_space = scope();
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        builder->get_descriptor(type_descriptor);
        type_descriptor->name(name_space + (name_space.empty() ? "" : "::") + name);
        auto result = enumerations_32_.emplace(name, builder);

        return result.second;
    }

    bool has_alias(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return false;
        }

        auto it = module.first->aliases_.find(module.second);
        return it != module.first->aliases_.end();
    }

    bool create_alias(
            const std::string& name,
            DynamicTypeBuilder::_ref_type builder)
    {
        if (name.find("::") != std::string::npos || has_alias(name))
        {
            return false; // Cannot define alias with scoped name (or already defined).
        }

        std::string name_space = scope();
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        builder->get_descriptor(type_descriptor);
        type_descriptor->name(name_space + (name_space.empty() ? "" : "::") + name);
        auto result = aliases_.emplace(name, builder);

        return result.second;
    }

    // Generic type builder retrieval
    DynamicTypeBuilder::_ref_type get_builder(
            const std::string& name,
            bool recursive = false)
    {
        DynamicTypeBuilder::_ref_type builder;

        if (recursive)
        {
            for (auto& pair : inner_)
            {
                std::shared_ptr<Module>& mod = pair.second;
                builder = mod->get_builder(name, recursive);
            }
        }

        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return builder;
        }

        // Check enums
        if (module.first->has_enum_32(module.second))
        {
            builder = module.first->enumerations_32_.at(module.second);
        }

        // Check structs
        if (module.first->has_structure(module.second))
        {
            builder = module.first->structs_.at(module.second);
        }

        // Check unions
        if (module.first->has_union(module.second))
        {
            builder = module.first->unions_.at(module.second);
        }

        // Check aliases
        if (module.first->has_alias(module.second))
        {
            builder = module.first->aliases_.at(module.second);
        }

        if (name.find("::") == 0)
        {
            // Scope ambiguity solver was originally used, add it to the retrieved DynamicTypeBuilder
            TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
            builder->get_descriptor(type_descriptor);
            type_descriptor->name(name);
        }

        // Check bitsets
        // TODO

        // Check bitmasks
        // TODO

        return builder;
    }

protected:

    std::map<std::string, DynamicTypeBuilder::_ref_type> aliases_;
    // std::map<std::string, Type> constants_types_;
    std::map<std::string, DynamicData::_ref_type> constants_;
    std::vector<std::string> from_enum_;
    std::map<std::string, DynamicTypeBuilder::_ref_type> enumerations_32_;
    std::map<std::string, DynamicTypeBuilder::_ref_type> structs_;
    std::map<std::string, DynamicTypeBuilder::_ref_type> unions_;
    //std::map<std::string, std::shared_ptr<AnnotationType>> annotations_;
    Module* outer_;
    std::map<std::string, std::shared_ptr<Module>> inner_;
    std::string name_;

    Module(
            Module* outer,
            const std::string& name)
        : outer_(outer)
        , name_(name)
    {
    }

    // Auxiliar method to resolve scoped. It will return the Module up to the last "::" by calling
    // recursively resolving each scoped name, looking for the scope path, and the symbol name without the scope.
    // If the path cannot be resolved, it will return nullptr as path, and the full given symbol name.
    PairModuleSymbol resolve_scope(
            const std::string& symbol_name) const
    {
        return resolve_scope(symbol_name, symbol_name, true);
    }

    PairModuleSymbol resolve_scope(
            const std::string& symbol_name,
            const std::string& original_name,
            bool first = false) const
    {
        if (!first && symbol_name == original_name)
        {
            // Loop trying to resolve the name. Abort failing.
            PairModuleSymbol pair;
            pair.first = nullptr;
            pair.second = original_name;
            return pair;
        }

        std::string name = symbol_name;
        // Solve scope
        if (symbol_name.find("::") != std::string::npos) // It is an scoped name
        {
            if (symbol_name.find("::") == 0) // Looking for root
            {
                if (outer_ == nullptr) // We are the root, now go down.
                {
                    return resolve_scope(symbol_name.substr(2), original_name);
                }
                else // We are not the root, go up, with the original name.
                {
                    return outer_->resolve_scope(original_name, original_name, true);
                }
            }
            else // not looking for root
            {
                std::string inner_scope = symbol_name.substr(0, symbol_name.find("::"));
                // Maybe the current scope its me?
                if (inner_scope == name_)
                {
                    std::string innest_scope = inner_scope.substr(0, inner_scope.find("::"));
                    if (inner_.count(innest_scope) > 0)
                    {
                        std::string inner_name = symbol_name.substr(symbol_name.find("::") + 2);
                        const auto& it = inner_.find(innest_scope);
                        PairModuleSymbol result = it->second->resolve_scope(inner_name, original_name);
                        if (result.first != nullptr)
                        {
                            return result;
                        }
                    }
                }
                // Do we have a inner scope that matches?
                if (inner_.count(inner_scope) > 0)
                {
                    std::string inner_name = symbol_name.substr(symbol_name.find("::") + 2);
                    const auto& it = inner_.find(inner_scope);
                    return it->second->resolve_scope(inner_name, original_name);
                }
                // Try going back
                if (outer_ != nullptr && first)
                {
                    return outer_->resolve_scope(original_name, original_name, true);
                }
                // Unknown scope
                PairModuleSymbol pair;
                pair.first = nullptr;
                pair.second = original_name;
                return pair;
            }
        }

        if (has_symbol(name, false))
        {
            return std::make_pair<const Module*, std::string>(this, std::move(name));
        }

        if (outer_ != nullptr)
        {
            return outer_->resolve_scope(symbol_name, original_name, true);
        }

        // Failed, not found
        PairModuleSymbol pair;
        pair.first = nullptr;
        pair.second = original_name;
        return pair;
    }

}; // class Module

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLMODULE_HPP
