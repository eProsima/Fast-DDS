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

typedef std::vector<Locator>::iterator LocatorListIterator;
typedef std::vector<Locator>::const_iterator LocatorListConstIterator;

/**
 * Adapter class that provides a LocatorsIterator interface from a LocatorListConstIterator
 */
class Locators : public LocatorsIterator
{
public:

    Locators(
            const LocatorListConstIterator& it)
        : it_(it)
    {
    }

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
 * Class LocatorList, a Locator vector that doesn't avoid duplicates.
 * @ingroup COMMON_MODULE
 */
class LocatorList
{
public:

    RTPS_DllAPI LocatorList()
    {
    }

    RTPS_DllAPI ~LocatorList()
    {
    }

    RTPS_DllAPI LocatorList(
            const LocatorList& list)
        : m_locators(list.m_locators)
    {
    }

    RTPS_DllAPI LocatorList(
            LocatorList&& list)
        : m_locators(std::move(list.m_locators))
    {
    }

    RTPS_DllAPI LocatorList& operator =(
            const LocatorList& list)
    {
        m_locators = list.m_locators;
        return *this;
    }

    RTPS_DllAPI LocatorList& operator =(
            LocatorList&& list)
    {
        m_locators = std::move(list.m_locators);
        return *this;
    }

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

    RTPS_DllAPI LocatorListIterator begin()
    {
        return m_locators.begin();
    }

    RTPS_DllAPI LocatorListIterator end()
    {
        return m_locators.end();
    }

    RTPS_DllAPI LocatorListConstIterator begin() const
    {
        return m_locators.begin();
    }

    RTPS_DllAPI LocatorListConstIterator end() const
    {
        return m_locators.end();
    }

    RTPS_DllAPI size_t size() const
    {
        return m_locators.size();
    }

    RTPS_DllAPI LocatorList& assign(
            const LocatorList& list)
    {
        if (!(*this == list))
        {
            m_locators = list.m_locators;
        }
        return *this;
    }

    RTPS_DllAPI void clear()
    {
        return m_locators.clear();
    }

    RTPS_DllAPI void reserve(
            size_t num)
    {
        return m_locators.reserve(num);
    }

    RTPS_DllAPI void resize(
            size_t num)
    {
        return m_locators.resize(num);
    }

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

    RTPS_DllAPI void push_back(
            const LocatorList& locList)
    {
        for (auto it = locList.m_locators.begin(); it != locList.m_locators.end(); ++it)
        {
            this->push_back(*it);
        }
    }

    RTPS_DllAPI bool empty() const
    {
        return m_locators.empty();
    }

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

    RTPS_DllAPI void swap(
            LocatorList& locatorList)
    {
        this->m_locators.swap(locatorList.m_locators);
    }

private:

    std::vector<Locator> m_locators;
};

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
            logWarning(LOCATOR_LIST, "Error deserializing LocatorList");
        }

        input.exceptions(excp_mask);
    }

    return input;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif /* _FASTDDS_RTPS_COMMON_LOCATORLIST_HPP_ */
