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

#ifndef RTPS_XMLPARSER__XMLPARSERUTILS_HPP
#define RTPS_XMLPARSER__XMLPARSERUTILS_HPP

#include <string>

namespace tinyxml2 {
class XMLElement;
} // namespace tinyxml2

namespace eprosima {
namespace fastdds {
namespace xml {
namespace detail {

/**
 * @brief Get text from XML element.
 *
 * This method is equivalent to calling element->GetText() and constructing an std::string with the returned value.
 * It will perform processing of environmental variables.
 * This method will return an empty string in case of error.
 *
 * @param [in] element  XMLElement from where to extract its text.
 */
std::string get_element_text(
        tinyxml2::XMLElement* element);

/**
 * @brief Get text from XML element.
 *
 * This method is equivalent to calling element->GetText() and constructing an std::string with the returned value.
 * It will perform processing of environmental variables.
 * This method will return an empty string in case of error.
 *
 * @param [in]  element  XMLElement from where to extract its text.
 * @param [out] text     String where to store the resulting text.
 *
 * @return true on success.
 * @return false on error.
 */
inline bool get_element_text(
        tinyxml2::XMLElement* element,
        std::string& text)
{
    text = get_element_text(element);
    return !text.empty();
}

} // namespace detail
} // namespace xml
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_XMLPARSER__XMLPARSERUTILS_HPP
