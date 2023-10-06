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

#include <fastdds/dds/xtypes/type_representation/TypeObjectBuilder.hpp>

#include <fastdds/dds/xtypes/type_representation/TypeObject.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

void TypeObjectBuilder::minimal_annotation_type_filler(
            MinimalAnnotationType& minimal_annotation_type,
            const AnnotationTypeFlag& annotation_flag,
            const MinimalAnnotationHeader& header,
            const MinimalAnnotationParameterSeq& member_seq)
{
    static_cast<void>(minimal_annotation_type);
    static_cast<void>(annotation_flag);
    static_cast<void>(header);
    static_cast<void>(member_seq);
}

void TypeObjectBuilder::create_register_annotation_type_object(
        const std::string& annotation_name,
        const MinimalAnnotationType& minimal_annotation_type,
        const CompleteAnnotationType& complete_annotation_type)
{
    static_cast<void>(annotation_name);
    static_cast<void>(minimal_annotation_type);
    static_cast<void>(complete_annotation_type);
}

} // xtypes
} // dds
} // fastdds
} // eprosima
