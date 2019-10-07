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
 */

#ifndef OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_ITERATOR_HPP_
#define OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_ITERATOR_HPP_

#include <dds/core/xtypes/DynamicData.hpp>

#include <cstring>

namespace dds {
namespace core {
namespace xtypes {

class DynamicDataIterator
{
public:

};


    class Iterator
    {
    public:
        Iterator()
            : current_(nullptr)
        {}

        Iterator(DynamicData& data)
            : current_(&data)
        {
            if(current_->type().kind() != TypeKind::STRUCTURE_TYPE)
            {
                current_member_ = static_cast<const StructType&>(current_->type()).member_map().begin();
            }
        }

        Iterator& operator++()
        {
            //TODO
            return *this;
        }

        bool operator==(const Iterator& other)
        {
            //TODO
            (void) other;
            return true;
        }

        bool operator!=(const Iterator& other)
        {
            //TODO
            (void) other;
            return false;
        }

        DynamicData& operator*() { return *current_; }

    private:
        DynamicData* current_;
        std::map<std::string, StructMember>::const_iterator current_member_;
    };


} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_ITERATOR_HPP_
