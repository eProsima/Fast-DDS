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

/**
 * @file PersistenceFactory.cpp
 *
 */

#include "PersistenceService.h"
#include "SQLite3PersistenceService.h"

#include <fastrtps/rtps/attributes/PropertyPolicy.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

IPersistenceService* PersistenceFactory::create_persistence_service(const PropertyPolicy& property_policy)
{
    IPersistenceService* ret_val = nullptr;
    const std::string* plugin_property = PropertyPolicyHelper::find_property(property_policy, "dds.persistence.plugin");

    if (plugin_property != nullptr)
    {
        if (plugin_property->compare("builtin.SQLITE3") == 0)
        {
            const std::string* filename_property = PropertyPolicyHelper::find_property(property_policy, "dds.persistence.sqlite3.filename");
            const char* filename = (filename_property == nullptr) ?
                "persistence.db" : filename_property->c_str();
            ret_val = create_SQLite3_persistence_service(filename);
        }
    }

    return ret_val;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */


