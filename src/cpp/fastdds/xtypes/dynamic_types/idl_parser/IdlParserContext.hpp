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

#include "AnnotationList.hpp"
#include "IdlAnnotations.hpp"
#include "IdlGrammar.hpp"
#include "IdlModule.hpp"
#include "IdlParserUtils.hpp"
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

class AnnotationsManager
{
public:

    using AnnotationInfo = std::pair<const Annotation*, AnnotationParameterValues>;
    using PendingAnnotations = std::vector<AnnotationInfo>;

    enum class AnnotationTargetKind
    {
        DISCRIMINATOR,
        MEMBER,
        TYPE
    };

    AnnotationsManager()
        : declared_annotations_(AnnotationList::from_builtin())
    {
    }

    static AnnotationTargetKind string_to_target_kind(
            const std::string& target)
    {
        if (target == "discriminator")
        {
            return AnnotationTargetKind::DISCRIMINATOR;
        }
        else if (target == "member")
        {
            return AnnotationTargetKind::MEMBER;
        }
        else if (target == "type")
        {
            return AnnotationTargetKind::TYPE;
        }
        else
        {
            throw std::runtime_error("Invalid annotation target kind: " + target);
        }
    }

    static std::string target_kind_to_string(
            AnnotationTargetKind target_kind)
    {
        switch (target_kind)
        {
            case AnnotationTargetKind::DISCRIMINATOR:
                return "discriminator";
            case AnnotationTargetKind::MEMBER:
                return "member";
            case AnnotationTargetKind::TYPE:
                return "type";
            default:
                throw std::runtime_error("Invalid annotation target kind.");
        }
    }

    const AnnotationList& declared_annotations() const
    {
        return declared_annotations_;
    }

    const PendingAnnotations& pending_type_annotations() const
    {
        return pending_type_annotations_;
    }

    const PendingAnnotations& pending_discriminator_annotations() const
    {
        return pending_discriminator_annotations_;
    }

    const std::map<std::string, PendingAnnotations>& pending_member_annotations() const
    {
        return pending_member_annotations_;
    }

    bool update_pending_annotations(
            std::map<std::string, std::string>& state);

    void set_target(
            std::map<std::string, std::string>& state);

    void reset()
    {
        pending_type_annotations_.clear();
        pending_discriminator_annotations_.clear();
        pending_member_annotations_.clear();
    }

protected:

    AnnotationList declared_annotations_;
    // Pending annotations
    PendingAnnotations pending_type_annotations_;
    PendingAnnotations pending_discriminator_annotations_;
    std::map<std::string, PendingAnnotations> pending_member_annotations_;
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
    CharType char_translation = CHAR;
    WideCharType wchar_type = WCHAR_T;

    // Results
    bool success = false;

    explicit Context(
            std::function<bool(DynamicTypeBuilder::_ref_type)> on_type_dcl_builder_created)
        : on_type_dcl_builder_created_(on_type_dcl_builder_created)
        , should_continue_(true)
    {
    }

    traits<DynamicType>::ref_type get_type(
            std::map<std::string, std::string>& state,
            const std::string& type)
    {
        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
        DynamicTypeBuilder::_ref_type type_builder;
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
            type_builder = factory->create_string_type(length);
            xtype = type_builder->build();
        }
        else if (type == "wstring")
        {
            uint32_t length = static_cast<uint32_t>(LENGTH_UNLIMITED);
            if (state.count("wstring_size"))
            {
                length = std::atoi(state["wstring_size"].c_str());
                state.erase("wstring_size");
            }
            type_builder = factory->create_wstring_type(length);
            xtype = type_builder->build();
        }
        else
        {
            type_builder = modules_.current()->get_builder(type);
            if (type_builder)
            {
                xtype = type_builder->build();
            }
        }

        return xtype;
    }

    void notify_declared_type(
            DynamicTypeBuilder::_ref_type type_builder)
    {
        should_continue_ = on_type_dcl_builder_created_(type_builder);
    }

    ModuleStack& modules()
    {
        return modules_;
    }

    AnnotationsManager& annotations()
    {
        return annotations_;
    }

    bool should_continue() const
    {
        return should_continue_;
    }

    void clear_context()
    {
        if (clear)
        {
            modules_.reset();
            annotations_.reset();
        }
    }

    ~Context()
    {
        clear_context();
    }

private:

    ModuleStack modules_;
    AnnotationsManager annotations_;
    std::function<bool(DynamicTypeBuilder::_ref_type)> on_type_dcl_builder_created_;
    bool should_continue_;

};

bool AnnotationsManager::update_pending_annotations(
        std::map<std::string, std::string>& state)
{
    if (!state.count("annotation_names") || !state.count("annotation_params") || !state.count("annotation_target"))
    {
        EPROSIMA_LOG_ERROR(IDLPARSER, "Unable to update pending annotations: missing required state keys.");
        return false;
    }

    std::vector<std::string> ann_names = utils::split_string(state["annotation_names"], ';');
    std::vector<std::string> ann_params = utils::split_string(state["annotation_params"], ';');

    // The number of annotation names and parameters should match
    if (ann_names.size() != ann_params.size())
    {
        EPROSIMA_LOG_ERROR(IDLPARSER, "Annotation names and parameters count mismatch: "
                << ann_names.size() << " names vs " << ann_params.size() << " parameters.");
        return false;
    }

    auto add_to_pending_ann = [this, &ann_names, &ann_params](PendingAnnotations& pending_ann)
            {
                for (size_t i = 0; i < ann_names.size(); i++)
                {
                    const Annotation* declared_ann = declared_annotations_.get_annotation(ann_names[i]);
                    if (!declared_ann)
                    {
                        EPROSIMA_LOG_ERROR(IDLPARSER, "Annotation '" << ann_names[i] << "' is not declared.");
                        return false;
                    }

                    AnnotationParameterValues param_values = AnnotationParameterValues::from_string(ann_params[i]);
                    pending_ann.push_back(std::make_pair(declared_ann, param_values));
                }

                return true;
            };

    bool success = true;
    switch (string_to_target_kind(state["annotation_target"]))
    {
        case AnnotationTargetKind::TYPE:
        {
            success = add_to_pending_ann(pending_type_annotations_);
            break;
        }
        case AnnotationTargetKind::MEMBER:
        {
            if (!state.count("annotation_member_name") || state["annotation_member_name"].empty())
            {
                EPROSIMA_LOG_ERROR(IDLPARSER, "Invalid annotation member name");
                return false;
            }
            const std::string& member_name = state["annotation_member_name"];
            pending_member_annotations_[member_name] = PendingAnnotations();
            success = add_to_pending_ann(pending_member_annotations_[member_name]);
            break;
        }
        case AnnotationTargetKind::DISCRIMINATOR:
        {
            success = add_to_pending_ann(pending_discriminator_annotations_);
            break;
        }
    }

    // Info about pending annotations consumed, delete it
    state.erase("annotation_names");
    state.erase("annotation_params");
    state.erase("annotation_target");
    state.erase("annotation_member_name");

    return success;
}

void AnnotationsManager::set_target(
        std::map<std::string, std::string>& state)
{
    // Determine the annotation's target (type or a member of a type) if it is not already set.
    if (!state.count("annotation_target"))
    {
        AnnotationTargetKind kind;
        if (state.count("struct_name") && !state["struct_name"].empty())
        {
            kind = AnnotationTargetKind::MEMBER;
            // We don't know yet the struct's member name
            state["annotation_member_name"] = "";

        }
        else if (state.count("union_name") && !state["union_name"].empty())
        {
            if (state["union_discriminant"].empty())
            {
                kind = AnnotationTargetKind::DISCRIMINATOR;
            }
            else
            {
                // We don't know yet the union's member name
                kind = AnnotationTargetKind::MEMBER;
            }
        }
        else if (state.count("enum_name") && !state["enum_name"].empty())
        {
            kind = AnnotationTargetKind::MEMBER;
            // We don't know yet the enum's member name
            state["annotation_member_name"] = "";
        }
        else
        {
            kind = AnnotationTargetKind::TYPE;
        }

        state["annotation_target"] = target_kind_to_string(kind);
    }
}

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSERCONTEXT_HPP
