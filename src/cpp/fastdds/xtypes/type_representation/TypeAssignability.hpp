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

#include <cstddef>
#include <list>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

class MinimalStructMember;
class MinimalStructType;
class MinimalTypeObject;
class TypeIdentifier;
class TypeObject;

class TypeAssignability
{
public:

    /*!
     * @brief Check if the TypeIdentifier t1 is assignable from TypeIdentifier t2.
     * @param t1 TypeIdentifier to check if is assignable from t2.
     * @param t2 TypeIdentifier to check if is assignable to t1.
     * @return true if t1 is assignable from t2, false otherwise.
     */
    static bool type_assignable_from(
            const TypeIdentifier& t1,
            const TypeIdentifier& t2);

private:

    static bool type_assignable_from_(
            const TypeIdentifier& t1,
            const TypeIdentifier& t2);

    /*!
     * @brief Check if the MinimalTypeObject t1 is assignable from MinimalTypeObject t2.
     * @param t1 MinimalTypeObject to check if is assignable from t2.
     * @param t2 MinimalTypeObject to check if is assignable to t1.
     * @return true if t1 is assignable from t2, false otherwise.
     */
    static bool minimal_typeobject_assignable_from(
            const MinimalTypeObject& t1,
            const MinimalTypeObject& t2);

    /*!
     * @brief Check if the MinimalStructType t1 is assignable from MinimalStructType t2.
     * @param t1 MinimalStructType to check if is assignable from t2.
     * @param t2 MinimalStructType to check if is assignable to t1.
     * @return true if t1 is assignable from t2, false otherwise.
     */
    static bool struct_type_assignable_from(
            const MinimalStructType& t1,
            const MinimalStructType& t2);

    static void generate_all_struct_members(
            std::list<const MinimalStructMember*>& list,
            const MinimalStructType& type);

    /*!
     * @brief Get the MinimalTypeObject for a given TypeIdentifier.
     * @param type_id TypeIdentifier to get the MinimalTypeObject for.
     * @param minimal_object Reference to store the resulting MinimalTypeObject.
     * @return true if the MinimalTypeObject was successfully retrieved, false otherwise.
     */
    static bool get_minimal_type_object(
            TypeIdentifier type_id,
            MinimalTypeObject& minimal_object);
};

} // namespace xtypes
} // namespace dds
} // namespace fastdds
} // namespace eprosima
