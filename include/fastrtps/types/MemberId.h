// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_MEMBER_ID_H
#define TYPES_MEMBER_ID_H

#include <fastrtps/types/TypesBase.h>

namespace eprosima{
namespace fastrtps{
namespace types{

class DynamicType;

class TypeSupport
{
public:
	//int register_type(DomainParticipant domain, std::string type_name);
    std::string get_type_name();
	DynamicType get_type();
};

class DynamicTypeSupport : public TypeSupport
{
public:
    static DynamicTypeSupport create_type_support(DynamicType type);
    static ResponseCode delete_type_support(DynamicTypeSupport type_support);

    //ResponseCode register_type(DomainParticipant participant, std::string type_name);
    std::string get_type_name();
	DynamicType get_type();
};

typedef uint32_t MemberId;

#define MEMBER_ID_INVALID = 0;

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_MEMBER_ID_H
