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

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/dds/xtypes/dynamic_types/AnnotationDescriptor.hpp>
#include "AnnotationManager.hpp"
#include "DynamicTypeImpl.hpp"
#include "DynamicTypeBuilderFactoryImpl.hpp"

#include <tuple>

namespace eprosima {
namespace fastdds {
namespace dds {

std::pair<AnnotationManager::annotation_iterator, bool>
AnnotationManager::get_annotation(
        const std::string& name) const
{
    annotation_iterator it = annotation_.begin();

    for (; it != annotation_.end(); ++it)
    {
        const AnnotationDescriptorImpl& d = *it;
        if ( d.type() && d.type()->get_kind() != TK_NONE
                && 0 != strlen(d.type()->get_name())
                && 0 == name.compare(d.type()->get_name()))
        {
            return std::make_pair(it, true);
        }
    }

    return std::make_pair(it, false);
}

std::pair<AnnotationManager::annotation_iterator, bool>
AnnotationManager::get_annotation(
        std::size_t idx) const
{
    // zero based
    if (idx >= annotation_.size())
    {
        return std::make_pair(annotation_.end(), false);
    }

    auto it = annotation_.begin();
    std::advance(it, idx);
    return std::make_pair(it, true);
}

// Annotations application

bool AnnotationManager::annotation_is_optional() const
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(ANNOTATION_OPTIONAL_ID);
    if (found)
    {
        ObjectName value;
        //TODO(richiware) if (it->get_value(value, "value") == RETCODE_OK)
        {
            return value == CONST_TRUE;
        }
    }
    return false;
}

bool AnnotationManager::annotation_is_key() const
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(ANNOTATION_KEY_ID);
    if (!found)
    {
        std::tie(it, found) = get_annotation(ANNOTATION_EPKEY_ID);
    }
    if (found)
    {
        ObjectName value;
        //TODO(richiware) if (it->get_value(value) == RETCODE_OK)
        {
            return value == CONST_TRUE;
        }
    }
    return false;
}

bool AnnotationManager::annotation_is_must_understand() const
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(ANNOTATION_MUST_UNDERSTAND_ID);
    if (found)
    {
        ObjectName value;
        //TODO(richiware) if (it->get_value(value) == RETCODE_OK)
        {
            return value == CONST_TRUE;
        }
    }
    return false;
}

bool AnnotationManager::annotation_is_non_serialized() const
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(ANNOTATION_NON_SERIALIZED_ID);
    if (found)
    {
        ObjectName value;
        //TODO(richiware) if (it->get_value(value) == RETCODE_OK)
        {
            return value == CONST_TRUE;
        }
    }
    return false;
}

bool AnnotationManager::annotation_is_value() const
{
    return get_annotation(ANNOTATION_VALUE_ID).second;
}

bool AnnotationManager::annotation_is_default_literal() const
{
    return get_annotation(ANNOTATION_DEFAULT_LITERAL_ID).second;
}

bool AnnotationManager::annotation_is_position() const
{
    return get_annotation(ANNOTATION_POSITION_ID).second;
}

bool AnnotationManager::annotation_is_external() const
{
    return get_annotation(ANNOTATION_EXTERNAL_ID).second;
}

bool AnnotationManager::annotation_is_bit_bound() const
{
    return get_annotation(ANNOTATION_BIT_BOUND_ID).second;
}

// Annotations getters

ObjectName AnnotationManager::annotation_get_value() const
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(ANNOTATION_VALUE_ID);
    if (found)
    {
        ObjectName value;
        //TODO(richiware) if (it->get_value(value) == RETCODE_OK)
        {
            return value;
        }
    }
    return {};
}

ObjectName AnnotationManager::annotation_get_default() const
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(ANNOTATION_DEFAULT_ID);
    if (found)
    {
        ObjectName value;
        //TODO(richiware) if (it->get_value(value) == RETCODE_OK)
        {
            return value;
        }
    }
    return {};
}

uint16_t AnnotationManager::annotation_get_position() const
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(ANNOTATION_POSITION_ID);
    if (found)
    {
        ObjectName value;
        //TODO(richiware) if (it->get_value(value) == RETCODE_OK)
        {
            return static_cast<uint16_t>(std::stoi(value.c_str()));
        }
    }
    return static_cast<uint16_t>(-1);
}

uint16_t AnnotationManager::annotation_get_bit_bound() const
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(ANNOTATION_BIT_BOUND_ID);

    if (found)
    {
        ObjectName value;
        //TODO(richiware) if (it->get_value(value) == RETCODE_OK)
        {
            return static_cast<uint16_t>(std::stoi(value.c_str()));
        }
    }
    return 32; // Default value
}

/* Ancillary method for setters
 * @param id annotation name
 * @param C functor that checks if the annotation should be modified: bool(const AnnotationDescriptorImpl&)
 * @param M functor that modifies the annotation if present: void(AnnotationDescriptorImpl&)
 */
template<typename C, typename M>
void AnnotationManager::annotation_set(
        const std::string& id,
        const C& c,
        const M& m)
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(id);
    if (!found)
    {
        AnnotationDescriptorImpl descriptor;
        /* TODO(richiware)
           descriptor.type(
            DynamicTypeBuilderFactoryImpl::get_instance().create_annotation_primitive(id));
         */
        m(descriptor);
        apply_annotation(descriptor);
    }
    else if (c(*it))
    {
        // Reinsert because order may be modified
        AnnotationDescriptorImpl descriptor(std::move(*it));
        it = annotation_.erase(it);
        m(descriptor);
        //TODO(richiware) annotation_.insert(it, std::move(descriptor));
    }

    std::tie(it, found) = get_annotation(id);
    assert(found);
}

//! Specialization of the above template to simple values
void AnnotationManager::annotation_set(
        const std::string& id,
        const char* new_val)
{
    annotation_set(
        id,
        [new_val](const AnnotationDescriptorImpl& d) -> bool
        {
            ObjectName val;
            // TODO(richiware) d.get_value(val, "value");
            return 0 != val.compare(new_val);
        },
        [new_val](AnnotationDescriptorImpl& d)
        {
            d.set_value("value", new_val);
        });
}

void AnnotationManager::annotation_set(
        const std::string& id,
        const std::string& new_val)
{
    return annotation_set(id, new_val.c_str());
}

// Annotations setters

void AnnotationManager::annotation_set_optional(
        bool optional)
{
    annotation_set(ANNOTATION_OPTIONAL_ID, optional ? "true" : "false");
}

void AnnotationManager::annotation_set_key(
        bool key)
{
    annotation_set(ANNOTATION_KEY_ID, key ? "true" : "false");
}

void AnnotationManager::annotation_set_must_understand(
        bool must_understand)
{
    annotation_set(ANNOTATION_MUST_UNDERSTAND_ID, must_understand ? "true" : "false");
}

void AnnotationManager::annotation_set_non_serialized(
        bool non_serialized)
{
    annotation_set(ANNOTATION_NON_SERIALIZED_ID, non_serialized ? "true" : "false");
}

void AnnotationManager::annotation_set_value(
        const ObjectName& value)
{
    annotation_set(ANNOTATION_VALUE_ID, value);
}

void AnnotationManager::annotation_set_default(
        const ObjectName& default_value)
{
    annotation_set(ANNOTATION_DEFAULT_ID, default_value);
}

void AnnotationManager::annotation_set_default_literal()
{
    annotation_set(ANNOTATION_DEFAULT_LITERAL_ID, "true");
}

void AnnotationManager::annotation_set_position(
        uint16_t position)
{
    annotation_set(ANNOTATION_POSITION_ID, std::to_string(position));
}

void AnnotationManager::annotation_set_bit_bound(
        uint16_t bit_bound)
{
    annotation_set(ANNOTATION_BIT_BOUND_ID, std::to_string(bit_bound));
}

void AnnotationManager::annotation_set_extensibility(
        const ObjectName& extensibility)
{
    annotation_set(ANNOTATION_EXTENSIBILITY_ID, extensibility);
}

void AnnotationManager::annotation_set_mutable()
{
    annotation_set(ANNOTATION_MUTABLE_ID, CONST_TRUE);
}

void AnnotationManager::annotation_set_final()
{
    annotation_set(ANNOTATION_FINAL_ID, CONST_TRUE);
}

void AnnotationManager::annotation_set_appendable()
{
    annotation_set(ANNOTATION_APPENDABLE_ID, CONST_TRUE);
}

void AnnotationManager::annotation_set_nested(
        bool nested)
{
    annotation_set(ANNOTATION_NESTED_ID, nested ? CONST_TRUE : CONST_FALSE);
}

void AnnotationManager::annotation_set_external(
        const ObjectName& type_name)
{
    annotation_set(ANNOTATION_EXTERNAL_ID, type_name);
}

ReturnCode_t AnnotationManager::apply_annotation(
        const AnnotationDescriptorImpl& descriptor)
{
    /*TODO(richiware)
       if (descriptor.is_consistent())
       {
        annotation_.insert(descriptor);
        return RETCODE_OK;
       }
       else
       {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error applying annotation. The input descriptor isn't consistent.");
        return RETCODE_BAD_PARAMETER;
       }
     */
}

ReturnCode_t AnnotationManager::apply_annotation(
        const std::string& annotation_name,
        const ObjectName& key,
        const ObjectName& value)
{
    annotation_set(
        annotation_name,
        [&key, &value](const AnnotationDescriptorImpl& d) -> bool
        {
            ObjectName val;
            //TODO(richiware) d.get_value(val, key);
            return val != value;
        },
        [&key, &value](AnnotationDescriptorImpl& d)
        {
            d.set_value(key, value);
        });

    return RETCODE_OK;
}

bool AnnotationManager::annotation_is_extensibility() const
{
    return get_annotation(ANNOTATION_EXTENSIBILITY_ID).second;
}

bool AnnotationManager::annotation_is_mutable() const
{
    return get_annotation(ANNOTATION_MUTABLE_ID).second ||
           annotation_get_extensibility().compare(EXTENSIBILITY_MUTABLE) == 0;
}

bool AnnotationManager::annotation_is_final() const
{
    return get_annotation(ANNOTATION_FINAL_ID).second ||
           annotation_get_extensibility().compare(EXTENSIBILITY_FINAL) == 0;
}

bool AnnotationManager::annotation_is_appendable() const
{
    return get_annotation(ANNOTATION_APPENDABLE_ID).second ||
           annotation_get_extensibility().compare(EXTENSIBILITY_APPENDABLE) == 0;
}

bool AnnotationManager::annotation_is_nested() const
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(ANNOTATION_NESTED_ID);

    if (found)
    {
        ObjectName value;
        //TODO(richiware) if (it->get_value(value) == RETCODE_OK)
        {
            return value == CONST_TRUE;
        }
    }

    return false;
}

// Annotation getters
ObjectName AnnotationManager::annotation_get_extensibility() const
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(ANNOTATION_EXTENSIBILITY_ID);
    if (found)
    {
        ObjectName value;
        //TODO(richiware) if (it->get_value(value) == RETCODE_OK)
        {
            return value;
        }
    }
    return {};
}

ObjectName AnnotationManager::annotation_get_external_typename() const
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(ANNOTATION_EXTERNAL_ID);
    if (found)
    {
        ObjectName value;
        //TODO(richiware) if (it->get_value(value) == RETCODE_OK)
        {
            return value;
        }
    }
    return {};
}

std::size_t AnnotationManager::get_annotation_count() const
{
    return annotation_.size();
}

ReturnCode_t AnnotationManager::get_annotation(
        AnnotationDescriptor& annotation,
        uint32_t index) const noexcept
{
    AnnotationManager::annotation_iterator it;
    bool found;
    std::tie(it, found) = get_annotation(index);

    if (found)
    {
        //TODO(richiware) annotation = it->get_descriptor();
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

bool AnnotationManager::key_annotation() const
{
    for (auto anIt = annotation_.begin(); anIt != annotation_.end(); ++anIt)
    {
        //TODO(richiware) if ((*anIt).key_annotation())
        {
            return true;
        }
    }
    return false;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
