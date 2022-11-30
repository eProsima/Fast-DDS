// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LocatorList.hpp
 */

#ifndef _FASTDDS_RTPS_COMMON_LOCATORLIST_HPP_
#define _FASTDDS_RTPS_COMMON_LOCATORLIST_HPP_

#include <fastrtps/fastrtps_dll.h>

#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/LocatorsIterator.hpp>

#include <vector>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <algorithm>

namespace eprosima {
namespace fastdds {
namespace rtps {

/// Iterator to iterate over a vector of locators.
typedef std::vector<Locator>::iterator LocatorListIterator;
/// Constant iterator to iterate over a vector of locators.
typedef std::vector<Locator>::const_iterator LocatorListConstIterator;

/**
 * Adapter class that provides a LocatorsIterator interface from a LocatorListConstIterator
 */
class Locators : public LocatorsIterator
{
public:

    /// Constructor
    Locators(
            const LocatorListConstIterator& it)
        : it_(it)
    {
    }

    /// Copy constructor
    Locators(
            const Locators& other)
        : it_(other.it_)
    {
    }

    LocatorsIterator& operator ++()
    {
        ++it_;
        return *this;
    }

    bool operator ==(
            const LocatorsIterator& other) const
    {
        return it_ == static_cast<const Locators&>(other).it_;
    }

    bool operator !=(
            const LocatorsIterator& other) const
    {
        return it_ != static_cast<const Locators&>(other).it_;
    }

    const Locator& operator *() const
    {
        return (*it_);
    }

private:

    LocatorListConstIterator it_;
};

/**
 * Class LocatorList, a Locator vector that doesn't allow duplicates.
 * @ingroup COMMON_MODULE
 */
class LocatorList
{
public:

    using value_type = typename std::vector<Locator>::value_type;

    /// Constructor
    RTPS_DllAPI LocatorList()
    {
    }

    /// Destructor
    RTPS_DllAPI ~LocatorList()
    {
    }

    /// Copy constructor
    RTPS_DllAPI LocatorList(
            const LocatorList& list)
        : m_locators(list.m_locators)
    {
    }

    /// Move constructor
    RTPS_DllAPI LocatorList(
            LocatorList&& list)
        : m_locators(std::move(list.m_locators))
    {
    }

    /// Copy assignment
    RTPS_DllAPI LocatorList& operator =(
            const LocatorList& list)
    {
        m_locators = list.m_locators;
        return *this;
    }

    /// Move assignment
    RTPS_DllAPI LocatorList& operator =(
            LocatorList&& list)
    {
        m_locators = std::move(list.m_locators);
        return *this;
    }

    /// Equal to operator
    RTPS_DllAPI bool operator ==(
            const LocatorList& locator_list) const
    {
        if (locator_list.m_locators.size() == m_locators.size())
        {
            bool returnedValue = true;

            for (auto it = locator_list.m_locators.begin(); returnedValue &&
                    it != locator_list.m_locators.end(); ++it)
            {
                returnedValue = false;

                for (auto it2 = m_locators.begin(); !returnedValue && it2 != m_locators.end(); ++it2)
                {
                    if (*it == *it2)
                    {
                        returnedValue = true;
                    }
                }
            }

            return returnedValue;
        }

        return false;
    }

    /**
     * @brief Return an iterator to the beginning.
     *
     * @return LocatorListIterator iterator to the first locator.
     */
    RTPS_DllAPI LocatorListIterator begin()
    {
        return m_locators.begin();
    }

    /**
     * @brief Return an iterator to the end.
     *
     * @return LocatorListIterator iterator to the element following the last element.
     */
    RTPS_DllAPI LocatorListIterator end()
    {
        return m_locators.end();
    }

    /**
     * @brief Return a constant iterator to the beginning.
     *
     * @return LocatorListConstIterator iterator to the first locator.
     */
    RTPS_DllAPI LocatorListConstIterator begin() const
    {
        return m_locators.begin();
    }

    /**
     * @brief Return a constant iterator to the end.
     *
     * @return LocatorListConstIterator iterator to the element following the last element.
     */
    RTPS_DllAPI LocatorListConstIterator end() const
    {
        return m_locators.end();
    }

    /**
     * @brief Return the number of locators.
     *
     * @return size_t The number of locators in the container.
     */
    RTPS_DllAPI size_t size() const
    {
        return m_locators.size();
    }

    /**
     * @brief Replace the contents of the container.
     *
     * @param list New content to be saved into the container.
     * @return LocatorList& reference to the container with the replaced content.
     */
    RTPS_DllAPI LocatorList& assign(
            const LocatorList& list)
    {
        if (!(*this == list))
        {
            m_locators = list.m_locators;
        }
        return *this;
    }

    /**
     * @brief Erase all locators from the container.
     */
    RTPS_DllAPI void clear()
    {
        return m_locators.clear();
    }

    /**
     * @brief Reserve storage increasing the capacity of the vector.
     *
     * @param num new capacity of the vector, in number of elements.
     */
    RTPS_DllAPI void reserve(
            size_t num)
    {
        return m_locators.reserve(num);
    }

    /**
     * @brief Resize the container to contain @c num locators.
     *        If the current size is greater than @c num, the container is reduced to its first @c num locators.
     *        If the current size is less than count, additional default-inserted locators are appended.
     *
     * @param num new size of the container.
     */
    RTPS_DllAPI void resize(
            size_t num)
    {
        return m_locators.resize(num);
    }

    /**
     * @brief Add locator to the end if not found within the list.
     *
     * @param loc locator to be appended.
     */
    RTPS_DllAPI void push_back(
            const Locator& loc)
    {
        bool already = false;
        for (LocatorListIterator it = this->begin(); it != this->end(); ++it)
        {
            if (loc == *it)
            {
                already = true;
                break;
            }
        }
        if (!already)
        {
            m_locators.push_back(loc);
        }
    }

    /**
     * @brief Add several locators to the end if not already present within the list.
     *
     * @param locList LocatorList with the locators to be appended.
     */
    RTPS_DllAPI void push_back(
            const LocatorList& locList)
    {
        for (auto it = locList.m_locators.begin(); it != locList.m_locators.end(); ++it)
        {
            this->push_back(*it);
        }
    }

    /**
     * @brief Check that the container has no locators.
     *
     * @return true if the container is empty. False otherwise.
     */
    RTPS_DllAPI bool empty() const
    {
        return m_locators.empty();
    }

    /**
     * @brief Erase the specified locator from the container.
     *
     * @param loc Locator to be removed.
     */
    RTPS_DllAPI void erase(
            const Locator& loc)
    {
        auto it = std::find(m_locators.begin(), m_locators.end(), loc);
        if (it != m_locators.end())
        {
            m_locators.erase(it);
        }
    }

    FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastrtps::rtps::LocatorList::contains(const Locator&)",
            "Unused method.")
    RTPS_DllAPI bool contains(
            const Locator& loc)
    {
        for (LocatorListIterator it = this->begin(); it != this->end(); ++it)
        {
            if (IsAddressDefined(*it))
            {
                if (loc == *it)
                {
                    return true;
                }
            }
            else
            {
                if (loc.kind == (*it).kind && loc.port == (*it).port)
                {
                    return true;
                }
            }
        }

        return false;
    }

    /**
     * @brief Check that every locator contained in the list is not LOCATOR_KIND_INVALID.
     *
     * @return true if all locators are valid. False otherwise.
     */
    RTPS_DllAPI bool isValid() const
    {
        for (LocatorListConstIterator it = this->begin(); it != this->end(); ++it)
        {
            if (!IsLocatorValid(*it))
            {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief exchange the content of the container.
     *
     * @param locatorList container to exchange the contents with.
     */
    RTPS_DllAPI void swap(
            LocatorList& locatorList)
    {
        this->m_locators.swap(locatorList.m_locators);
    }

    // Check if there are specific transport locators associated
    // the template parameter is the locator kind (e.g. LOCATOR_KIND_UDPv4)
    template<int kind> bool has_kind() const
    {
        for (auto& loc : m_locators)
        {
            if ( kind == loc.kind )
            {
                return true;
            }
        }

        return false;
    }

private:

    std::vector<Locator> m_locators;
};

/**
 * @brief Insertion operator: serialize a locator list.
 *        The deserialization format is [locator1,locator2,...,locatorN].
 *        Each individual locator within the list must follow the serialization format explained in the locator insertion
 *        operator.
 *
 * @param output Output stream where the serialized locator list is appended.
 * @param locList Locator list to be serialized/inserted.
 * @return \c std::ostream& Reference to the output stream with the serialized locator list appended.
 */
inline std::ostream& operator <<(
        std::ostream& output,
        const LocatorList& locList)
{
    output << "[";
    if (!locList.empty())
    {
        output << *(locList.begin());
        for (auto it = locList.begin() + 1; it != locList.end(); ++it)
        {
            output << "," << *it;
        }
    }
    else
    {
        output << "_";
    }
    output << "]";
    return output;
}

/**
 * @brief Extraction operator: deserialize a list of locators.
 *        The serialization format is [locator1,locator2,...,locatorN].
 *        Each individual locator within the list must follow the deserialization format explained in the locator
 *        extraction operator.
 *
 * @param input Input stream where the locator list to be deserialized is located.
 * @param locList Locator list where the deserialized locators are saved.
 * @return \c std::istream& Reference to the input stream after extracting the locator list.
 */
inline std::istream& operator >>(
        std::istream& input,
        LocatorList& locList)
{
    std::istream::sentry s(input);
    locList = LocatorList();

    if (s)
    {
        char punct;
        Locator loc;
        std::ios_base::iostate excp_mask = input.exceptions();

        try
        {
            input.exceptions(excp_mask | std::ios_base::failbit | std::ios_base::badbit);

            // Get [
            input >> punct;

            while (punct != ']')
            {
                input >> loc >> punct;
                if (loc.kind != LOCATOR_KIND_INVALID)
                {
                    locList.push_back(loc);
                }
            }
        }
        catch (std::ios_base::failure& )
        {
            locList.clear();
            EPROSIMA_LOG_WARNING(LOCATOR_LIST, "Error deserializing LocatorList");
        }

        input.exceptions(excp_mask);
    }

    return input;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif /* _FASTDDS_RTPS_COMMON_LOCATORLIST_HPP_ */
