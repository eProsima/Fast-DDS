// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSERCONTEXT_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSERCONTEXT_HPP

#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

#include "IdlModule.hpp"
#include "IdlPreprocessor.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

using namespace tao::TAO_PEGTL_NAMESPACE;

/**
 * @brief Class representing a hierarchy of nested modules, allowing to manage the current module context during parsing.
 * @note This class is used to keep track of the current module scope when parsing IDL attributes, types, and other elements.
 *       It is intended to be always non-empty, starting with a root module representing the global scope.
 *       The current module can be pushed and popped to navigate through nested modules.
 *       The root module is created upon instantiation and cannot be popped.
 */
class ModuleStack
{
public:

    ModuleStack()
    {
        // Initialize the stack with a global scope module (root)
        stack_.push(std::make_shared<Module>());
    }

    std::shared_ptr<Module> current() const
    {
        return stack_.top();
    }

    std::shared_ptr<Module> push(
            const std::string& submodule)
    {
        auto current = stack_.top();

        if (!current->has_submodule(submodule))
        {
            current->create_submodule(submodule);
        }

        auto new_module = current->submodule(submodule);
        stack_.push(new_module);

        return new_module;
    }

    void pop()
    {
        if (stack_.size() == 1)
        {
            EPROSIMA_LOG_ERROR(IDLPARSER, "Cannot pop the root module.");
            return;
        }

        stack_.pop();
    }

    void reset()
    {
        while (stack_.size() > 1)
        {
            stack_.pop();
        }
    }

private:

    std::stack<std::shared_ptr<Module>, std::vector<std::shared_ptr<Module>>> stack_;
};

class Context
    : public PreprocessorContext
{
public:

    enum CharType
    {
        CHAR,
        UINT8,
        INT8
    };

    enum WideCharType
    {
        WCHAR_T,
        CHAR16_T
    };

    // Config
    bool ignore_case = false;
    bool clear = true;
    bool allow_keyword_identifiers = false;
    bool ignore_redefinition = false;
    bool should_continue = true;
    CharType char_translation = CHAR;
    WideCharType wchar_type = WCHAR_T;
    std::function<bool(DynamicTypeBuilder::_ref_type)> on_builder_created = [](DynamicTypeBuilder::_ref_type) { return true; };

    // Results
    bool success = false;
    std::string target_type_name;

    traits<DynamicType>::ref_type get_type(
            std::map<std::string, std::string>& state,
            const std::string& type)
    {
        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        DynamicTypeBuilder::_ref_type builder;
        DynamicType::_ref_type xtype;

        if (type == "boolean")
        {
            xtype = factory->get_primitive_type(TK_BOOLEAN);
        }
        else if (type == "int8")
        {
            xtype = factory->get_primitive_type(TK_INT8);
        }
        else if (type == "uint8")
        {
            xtype = factory->get_primitive_type(TK_UINT8);
        }
        else if (type == "int16")
        {
            xtype = factory->get_primitive_type(TK_INT16);
        }
        else if (type == "uint16")
        {
            xtype = factory->get_primitive_type(TK_UINT16);
        }
        else if (type == "int32")
        {
            xtype = factory->get_primitive_type(TK_INT32);
        }
        else if (type == "uint32")
        {
            xtype = factory->get_primitive_type(TK_UINT32);
        }
        else if (type == "int64")
        {
            xtype = factory->get_primitive_type(TK_INT64);
        }
        else if (type == "uint64")
        {
            xtype = factory->get_primitive_type(TK_UINT64);
        }
        else if (type == "float")
        {
            xtype = factory->get_primitive_type(TK_FLOAT32);
        }
        else if (type == "double")
        {
            xtype = factory->get_primitive_type(TK_FLOAT64);
        }
        else if (type == "long double")
        {
            xtype = factory->get_primitive_type(TK_FLOAT128);
        }
        else if (type == "char")
        {
            xtype = factory->get_primitive_type(TK_CHAR8);
        }
        else if (type == "wchar" || type == "char16")
        {
            xtype = factory->get_primitive_type(TK_CHAR16);
        }
        else if (type == "string")
        {
            uint32_t length = static_cast<uint32_t>(LENGTH_UNLIMITED);
            if (state.count("string_size"))
            {
                length = std::atoi(state["string_size"].c_str());
                state.erase("string_size");
            }
            builder = factory->create_string_type(length);
            xtype = builder->build();
        }
        else if (type == "wstring")
        {
            uint32_t length = static_cast<uint32_t>(LENGTH_UNLIMITED);
            if (state.count("wstring_size"))
            {
                length = std::atoi(state["wstring_size"].c_str());
                state.erase("wstring_size");
            }
            builder = factory->create_wstring_type(length);
            xtype = builder->build();
        }
        else
        {
            builder = modules_.current()->get_builder(type);
            if (builder)
            {
                xtype = builder->build();
            }
        }

        return xtype;
    }

    std::vector<std::string> split_string(
            const std::string& str,
            char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream ss(str);
        while (std::getline(ss, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    DynamicTypeBuilder::_ref_type builder;

    ModuleStack& modules()
    {
        return modules_;
    }

    void clear_context()
    {
        if (clear)
        {
            modules_.reset();
        }
    }

    ~Context()
    {
        clear_context();
    }

private:

    ModuleStack modules_;

};

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSERCONTEXT_HPP