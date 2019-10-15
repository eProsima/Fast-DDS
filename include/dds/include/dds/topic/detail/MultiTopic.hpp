#ifndef OMG_DDS_TOPIC_DETAIL_MULTI_TOPIC_HPP_
#define OMG_DDS_TOPIC_DETAIL_MULTI_TOPIC_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
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
 */

#include <string>
#include <vector>

#include <dds/core/types.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/topic/detail/TopicDescription.hpp>
#include <dds/core/Query.hpp>

#ifdef OMG_DDS_MULTI_TOPIC_SUPPORT

namespace dds { namespace topic { namespace detail {


template <typename T>
class MultiTopic  : public org::opensplice::topic::TopicDescriptionDelegate
{
public:
    MultiTopic(
        const dds::domain::DomainParticipant& dp,
        const std::string& name,
        const std::string expression,
        const FWDIterator& params_begin,
        const FWDIterator& params_end)
        : ::dds::core::Reference< DELEGATE<T> >(
                new dds::topic::detail::MultiTopic<T>(dp, name, expression, params_begin, params_end))
    {
        ISOCPP_REPORT_STACK_DELEGATE_BEGIN();
        // Set the correct (IDL) type_name in the TopicDescription.
        dds::topic::detail::TopicDescription<T>::myTypeName = org::opensplice::topic::TopicTraits<T>::getTypeName();

        validate_filter();
    }

    virtual ~MultiTopic()
    {
        // Assume destructor of Filter attribute cleans up itself.
    }

private:
    void validate_filter()
    {
        q_expr expr = NULL;
        uint32_t length;
        std::vector<c_value> params;

        length = myFilter.parameters_length();
        if (length < 100) {
            expr = q_parse(myFilter.expression().c_str());
            if (!expr ) {
                ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR,
                        "filter_expression '%s' is invalid", myFilter.expression().c_str());
            }
        } else {
            ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR,
                    "Invalid number of filter_parameters '%d', maximum is 99", length);
        }

        params = reader_parameters();
        /* The function below does not exist yet, but probably a function like it will need to be developed. */
        if (!u_topicMultiExprValidate(dp.delegate()->get_user_handle(), myFilter.expression().c_str(), &params[0], params.size())) {
            ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR,
                    "filter_expression '%s' is invalid.", myFilter.expression().c_str());
        }
        q_dispose(expr);
        /* The params can be NULL, but os_free ignores those pesky NULLs. */
        os_free(params);
    }

public:
    std::string reader_expression() const
    {
        ISOCPP_REPORT_STACK_DELEGATE_BEGIN();
        return myFilter.expression();
    }

    c_value *reader_parameters() const
    {
        ISOCPP_REPORT_STACK_DELEGATE_BEGIN();
        c_value *params = NULL;
        uint32_t n, length;
        org::opensplice::topic::FilterDelegate::const_iterator paramIterator;

        length = myFilter.parameters_length();
        if (length != 0) {
            params = (c_value *)os_malloc(length * sizeof(struct c_value));
            for (n = 0, paramIterator = myFilter.begin(); n < length; n++, paramIterator++) {
                params[n] = c_stringValue(const_cast<char *>(paramIterator->c_str()));
            }
        }
        return params;
    }

    const std::string expression() const
    {
        ISOCPP_REPORT_STACK_DELEGATE_BEGIN();
        return myFilter.expression();
    }

    template <typename FWIterator>
    void expression_parameters(const FWIterator& params_begin, const FWIterator& params_end)
    {
        ISOCPP_REPORT_STACK_DELEGATE_BEGIN();
        myFilter.parameters(begin, end);
        validate_filter();
    }

    const dds::core::StringSeq expression_parameters() const
    {
        ISOCPP_REPORT_STACK_DELEGATE_BEGIN();
        return dds::core::StringSeq(myFilter.begin(), myFilter.end());
    }

private:
    dds::topic::Filter myFilter;
};

}
}
}
#endif /* OMG_DDS_MULTI_TOPIC_SUPPORT */

#endif /* OMG_DDS_TOPIC_DETAIL_MULTI_TOPIC_HPP_ */
