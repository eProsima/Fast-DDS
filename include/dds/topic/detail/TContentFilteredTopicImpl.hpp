/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef OSPL_DDS_TOPIC_TCONTENTFILTEREDTOPIC_HPP_
#define OSPL_DDS_TOPIC_TCONTENTFILTEREDTOPIC_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/topic/TContentFilteredTopic.hpp>

// Implementation

namespace dds
{
namespace topic
{
template <typename T, template <typename Q> class DELEGATE>
ContentFilteredTopic<T, DELEGATE>::ContentFilteredTopic(const Topic<T>& topic,
                                                        const std::string& name,
                                                        const dds::topic::Filter& filter) :
        ::dds::core::Reference< DELEGATE<T> >(
                new dds::topic::detail::ContentFilteredTopic<T>(topic, name, filter))
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(topic);
    this->delegate()->init(::dds::core::Reference< DELEGATE<T> >::impl_);
}

template <typename T, template <typename Q> class DELEGATE>
ContentFilteredTopic<T, DELEGATE>::~ContentFilteredTopic()
{
    // Nothing to be done yet....
}

template <typename T, template <typename Q> class DELEGATE>
const std::string& ContentFilteredTopic<T, DELEGATE>::filter_expression() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    return this->delegate()->filter_expression();
}

template <typename T, template <typename Q> class DELEGATE>
const dds::core::StringSeq ContentFilteredTopic<T, DELEGATE>::filter_parameters() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    return this->delegate()->filter_parameters();
}

template <typename T, template <typename Q> class DELEGATE>
template <typename FWDIterator>
void ContentFilteredTopic<T, DELEGATE>::filter_parameters(const FWDIterator& begin, const FWDIterator& end)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    this->delegate()->filter_parameters(begin, end);
}

template <typename T, template <typename Q> class DELEGATE>
const dds::topic::Topic<T>& ContentFilteredTopic<T, DELEGATE>::topic() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    return this->delegate()->topic();
}


}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_TCONTENTFILTEREDTOPIC_HPP_ */
