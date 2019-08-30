/*
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
 *
*/

#ifndef EPROSIMA_DDS_TOPIC_DETAIL_CONTENTFILTEREDTOPIC_HPP_
#define EPROSIMA_DDS_TOPIC_DETAIL_CONTENTFILTEREDTOPIC_HPP_

#include <string>
#include <vector>

#include <dds/core/detail/conformance.hpp>
#include <dds/core/types.hpp>
#include <dds/topic/Topic.hpp>
#include <dds/topic/Filter.hpp>
//TODO: Fix when TopicDescriptionDelegate and ScopedLock are implemented
//#include <org/opensplice/topic/TopicDescriptionDelegate.hpp>
//#include <org/opensplice/core/ScopedLock.hpp>

//#include "u_topic.h"
//#include "v_kernelParser.h"

//#ifdef OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace topic {
namespace detail {

template<typename T>
//class ContentFilteredTopic  : public virtual org::opensplice::topic::TopicDescriptionDelegate
//{
//public:
//    ContentFilteredTopic(
//        const dds::topic::Topic<T>& topic,
//        const std::string& name,
//        const dds::topic::Filter& filter)
//        : org::opensplice::topic::TopicDescriptionDelegate(topic.domain_participant(), name, topic.type_name()),
//          myTopic(topic),
//          myFilter(filter)
//    {
//        ISOCPP_REPORT_STACK_DDS_BEGIN(topic);
//        validate_filter();
//        topic.delegate()->incrNrDependents();
//        this->myParticipant.delegate()->add_cfTopic(*this);
//    }

//    virtual ~ContentFilteredTopic()
//    {
//        if (!this->closed) {
//            try {
//                this->close();
//            } catch (...) {
//                /* Empty: the exception throw should have already traced an error. */
//            }
//        }
//    }

//    virtual void close()
//    {
//        ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
//        org::opensplice::core::ScopedObjectLock scopedLock(*this);

//        myTopic.delegate()->decrNrDependents();

//        // Remove the ContentFilteredTopic from the list of topics in its participant.
//        this->myParticipant.delegate()->remove_cfTopic(*this);

//        org::opensplice::core::ObjectDelegate::close();
//    }

//    void
//    init(org::opensplice::core::ObjectDelegate::weak_ref_type weak_ref)
//    {
//        ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);

//        /* Set weak_ref before passing ourselves to other isocpp objects. */
//        this->set_weak_ref(weak_ref);
//        /* Register topic at participant. */
//        this->myParticipant.delegate()->add_cfTopic(*this);
//    }

//private:
//    void validate_filter()
//    {
//        q_expr expr = NULL;
//        uint32_t length;
//        std::vector<c_value> params;

//        length = myFilter.parameters_length();
//        if (length < 100) {
//            expr = v_parser_parse(myFilter.expression().c_str());
//            if (!expr ) {
//                ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR,
//                        "filter_expression '%s' is invalid", myFilter.expression().c_str());
//            }
//        } else {
//            ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR,
//                    "Invalid number of filter_parameters '%d', maximum is 99", length);
//        }

//        u_topic uTopic = (u_topic)(myTopic.delegate()->get_user_handle());

//        params = reader_parameters();
//        if (!u_topicContentFilterValidate2(uTopic, expr, params.empty() ? NULL : &params[0], params.size())) {
//            q_dispose(expr);
//            ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR,
//                    "filter_expression '%s' is invalid.", myFilter.expression().c_str());
//        }
//        q_dispose(expr);
//    }

//public:
//    std::string reader_expression() const
//    {
//        ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
//        std::string rExpr;

//        rExpr += "select * from ";
//        rExpr += myTopic.name();
//        rExpr += " where ";
//        rExpr += myFilter.expression();
//        return rExpr;
//    }

//    std::vector<c_value> reader_parameters() const
//    {
//        ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
//        std::vector<c_value> params;
//        uint32_t n, length;
//        org::opensplice::topic::FilterDelegate::const_iterator paramIterator;

//        length = myFilter.parameters_length();
//        if (length != 0) {
//            for (n = 0, paramIterator = myFilter.begin(); n < length; n++, paramIterator++) {
//                params.push_back(c_stringValue(const_cast<char *>(paramIterator->c_str())));
//            }
//        }
//        return params;
//    }

//    /**
//    *  @internal Accessor to return the topic filter.
//    * @return The dds::topic::Filter in effect on this topic.
//    */
//    const dds::topic::Filter& filter() const
//    {
//        return myFilter;
//    }

//    /**
//     *  @internal Sets the filter parameters for this content filtered topic.
//     * @param begin The iterator holding the first string param
//     * @param end The last item in the string iteration
//     */
//    template<typename FWIterator>
//    void filter_parameters(const FWIterator& begin, const FWIterator& end)
//    {
//        ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
//        ISOCPP_THROW_EXCEPTION(ISOCPP_UNSUPPORTED_ERROR, "Changing of Filter parameters is currently not supported.");
//        myFilter.parameters(begin, end);
//        validate_filter();
//    }

//    const dds::topic::Topic<T>& topic() const
//    {
//        return myTopic;
//    }

//    const std::string& filter_expression() const
//    {
//        ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
//        return myFilter.expression();
//    }

//    const dds::core::StringSeq filter_parameters() const
//    {
//        ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
//        return dds::core::StringSeq(myFilter.begin(), myFilter.end());
//    }

//private:
//    dds::topic::Topic<T> myTopic;
//    dds::topic::Filter myFilter;
//};
class ContentFilteredTopic { };

} //namespace detail
} //namespace topic
} //namespace dds)

/** @endcond */

//#endif //OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

#endif //EPROSIMA_DDS_TOPIC_DETAIL_CONTENTFILTEREDTOPIC_HPP_
