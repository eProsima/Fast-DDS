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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_ANNOTATIONLIST_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_ANNOTATIONLIST_HPP

#include <exception>
#include <map>
#include <memory>
#include <string>

#include <fastdds/dds/log/Log.hpp>

#include "IdlAnnotations.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

/**
 * @brief Class used to manage a list of annotations
 */
class AnnotationList
{
public:

    AnnotationList() = default;

    /**
     * @brief Create an AnnotationList with built-in annotations.
     *
     * @return An AnnotationList containing all the built-in annotations.
     */
    static AnnotationList from_builtin()
    {
        AnnotationList list;

        // @id
        auto id_annotation = std::make_shared<IdAnnotation>();
        if (!id_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @id annotation.");
            throw std::runtime_error("Failed to initialize @id annotation.");
        }
        list.add_annotation(id_annotation);

        // @optional
        auto optional_annotation = std::make_shared<OptionalAnnotation>();
        if (!optional_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @optional annotation.");
            throw std::runtime_error("Failed to initialize @optional annotation.");
        }
        list.add_annotation(optional_annotation);

        // @position
        auto position_annotation = std::make_shared<PositionAnnotation>();
        if (!position_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @position annotation.");
            throw std::runtime_error("Failed to initialize @position annotation.");
        }
        list.add_annotation(position_annotation);

        // @extensibility
        auto extensibility_annotation = std::make_shared<ExtensibilityAnnotation>();
        if (!extensibility_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @extensibility annotation.");
            throw std::runtime_error("Failed to initialize @extensibility annotation.");
        }
        list.add_annotation(extensibility_annotation);

        // @final
        list.add_annotation(std::make_shared<FinalAnnotation>());

        // @appendable
        list.add_annotation(std::make_shared<AppendableAnnotation>());

        // @mutable
        list.add_annotation(std::make_shared<MutableAnnotation>());

        // @key
        auto key_annotation = std::make_shared<KeyAnnotation>();
        if (!key_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @key annotation.");
            throw std::runtime_error("Failed to initialize @key annotation.");
        }
        list.add_annotation(key_annotation);

        // @default_literal
        list.add_annotation(std::make_shared<DefaultLiteralAnnotation>());

        // @default
        auto default_annotation = std::make_shared<DefaultAnnotation>();
        if (!default_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @default annotation.");
            throw std::runtime_error("Failed to initialize @default annotation.");
        }
        list.add_annotation(default_annotation);

        // @bit_bound
        auto bit_bound_annotation = std::make_shared<BitBoundAnnotation>();
        if (!bit_bound_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @bit_bound annotation.");
            throw std::runtime_error("Failed to initialize @bit_bound annotation.");
        }
        list.add_annotation(bit_bound_annotation);

        // @external
        auto external_annotation = std::make_shared<ExternalAnnotation>();
        if (!external_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @external annotation.");
            throw std::runtime_error("Failed to initialize @external annotation.");
        }
        list.add_annotation(external_annotation);

        // @nested
        auto nested_annotation = std::make_shared<NestedAnnotation>();
        if (!nested_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @nested annotation.");
            throw std::runtime_error("Failed to initialize @nested annotation.");
        }
        list.add_annotation(nested_annotation);

        // @try_construct
        auto try_construct_annotation = std::make_shared<TryConstructAnnotation>();
        if (!try_construct_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @try_construct annotation.");
            throw std::runtime_error("Failed to initialize @try_construct annotation.");
        }
        list.add_annotation(try_construct_annotation);

        // @value
        auto value_annotation = std::make_shared<ValueAnnotation>();
        if (!value_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @value annotation.");
            throw std::runtime_error("Failed to initialize @value annotation.");
        }
        list.add_annotation(value_annotation);

        // TODO: Add other built-in annotations when supported
        // @autoid (Unsupported)
        // @must_understand (Unsupported)
        // @range (Unsupported)
        // @min (Unsupported)
        // @max (Unsupported)
        // @unit (Unsupported)
        // @verbatim (Unsupported)
        // @service (Unsupported)
        // @oneway (Unsupported)
        // @ami (Unsupported)

        return list;
    }

    /**
     * @brief Add a new annotation to the list.
     *
     * @param annotation The annotation to add.
     */
    void add_annotation(
            const std::shared_ptr<Annotation>& annotation)
    {
        assert(annotation != nullptr);
        assert(!annotation->get_name().empty());

        const std::string& name = annotation->get_name();

        if (has_annotation(name))
        {
            EPROSIMA_LOG_WARNING(IDL_PARSER,
                    "Ignoring annotation '" << name
                                            << "': already exists in the list.");
        }
        else
        {
            annotations_[name] = annotation;
            EPROSIMA_LOG_INFO(IDL_PARSER, "Added annotation '" << name << "'");
        }
    }

    /**
     * @brief Delete an annotation from the list.
     *
     * @param annotation_name The name of the annotation to delete.
     */
    void delete_annotation(
            const std::string& annotation_name)
    {
        auto it = annotations_.find(annotation_name);

        if (it != annotations_.end())
        {
            EPROSIMA_LOG_INFO(IDL_PARSER, "Deleting annotation '" << annotation_name << "'");
            annotations_.erase(it);
        }
        else
        {
            EPROSIMA_LOG_WARNING(IDL_PARSER, "Annotation '" << annotation_name
                                                            << "' not found in the list.");
        }
    }

    /**
     * @brief Check if an annotation with the given name exists in the list.
     *
     * @param annotation_name The name of the annotation to check.
     */
    bool has_annotation(
            const std::string& annotation_name) const
    {
        auto it = annotations_.find(annotation_name);
        return it != annotations_.end();
    }

    /**
     * @brief Get an annotation by its name.
     *
     * @param annotation_name The name of the annotation to retrieve.
     * @return A reference to the annotation with the given name, or `nullptr` if not found.
     */
    const Annotation* get_annotation(
            const std::string& annotation_name) const
    {
        auto it = annotations_.find(annotation_name);

        if (it != annotations_.end())
        {
            return it->second.get();
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Annotation '" << annotation_name
                                                          << "' not found in the list.");
            return nullptr;
        }
    }

    /**
     * @brief Apply a callable type to each annotation in the list.
     */
    template<typename Func>
    void for_each(
            Func&& func) const
    {
        for (const auto& annotation : annotations_)
        {
            func(annotation.second);
        }
    }

    template<typename Func>
    void for_each(
            Func&& func)
    {
        for (auto& annotation : annotations_)
        {
            func(annotation.second);
        }
    }

protected:

    std::map<std::string, std::shared_ptr<Annotation>> annotations_;
};

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_ANNOTATIONLIST_HPP