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


namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

class MinimalStructType;
class MinimalTypeObject;
class TypeIdentifier;
class TypeObject;

class TypeAssignability
{
public:

    static bool type_assignable_from(
            const TypeIdentifier& t1,
            const TypeIdentifier& t2);

private:

    static bool inheritance_type_assignable_from(
            const TypeIdentifier& t1,
            const TypeIdentifier& t2);

    static bool minimal_typeobject_assignable_from(
            const MinimalTypeObject& t1,
            const MinimalTypeObject& t2);

    static bool struct_type_assignable_from(
            const MinimalStructType& t1,
            const MinimalStructType& t2);

    static bool typeobject_assignable_from(
            const TypeObject& t1,
            const TypeObject& t2);

};

} // namespace xtypes
} // namespace dds
} // namespace fastdds
} // namespace eprosima
