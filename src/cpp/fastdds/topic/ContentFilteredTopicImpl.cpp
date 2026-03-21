// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ContentFilteredTopicImpl.cpp
 */

#include "ContentFilteredTopicImpl.hpp"

#include <algorithm>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/subscriber/DataReaderImpl.hpp>
#include <fastdds/topic/ContentFilterUtils.hpp>
#include <fastdds/topic/TopicProxy.hpp>
#include <fastdds/utils/md5.hpp>
#include <rtps/messages/CDRMessage.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

bool ContentFilteredTopicImpl::is_relevant(
        const fastdds::rtps::CacheChange_t& change,
        const fastdds::rtps::GUID_t& reader_guid) const
{
    bool ret_val = true;

    if (!check_filter_signature(change, ret_val))
    {
        IContentFilter::FilterSampleInfo filter_info;
        filter_info.sample_identity.writer_guid(change.writerGUID);
        filter_info.sample_identity.sequence_number(change.sequenceNumber);
        FASTDDS_TODO_BEFORE(4, 0, "Use change.write_params.related_sample_identity()");
        filter_info.related_sample_identity = change.write_params.sample_identity();
        ret_val = filter_instance->evaluate(change.serializedPayload, filter_info, reader_guid);
    }

    return ret_val;
}

ReturnCode_t ContentFilteredTopicImpl::set_expression_parameters(
        const char* new_expression,
        const std::vector<std::string>& new_expression_parameters)
{
    TopicProxy* topic_impl = dynamic_cast<TopicProxy*>(related_topic->get_impl());
    assert(nullptr != topic_impl);
    const TypeSupport& type = topic_impl->get_type();

    DomainParticipantQos pqos;
    related_topic->get_participant()->get_qos(pqos);
    if (new_expression_parameters.size() > pqos.allocation().content_filter.expression_parameters.maximum )
    {
        EPROSIMA_LOG_ERROR(CONTENT_FILTERED_TOPIC, "Number of expression parameters exceeds maximum allocation limit: "
                << new_expression_parameters.size() << " > "
                << pqos.allocation().content_filter.expression_parameters.maximum);
        return RETCODE_BAD_PARAMETER;
    }

    LoanableSequence<const char*>::size_type n_params;
    n_params = static_cast<LoanableSequence<const char*>::size_type>(new_expression_parameters.size());
    LoanableSequence<const char*> filter_parameters(n_params);
    filter_parameters.length(n_params);
    while (n_params > 0)
    {
        n_params--;
        filter_parameters[n_params] = new_expression_parameters[n_params].c_str();
    }

    ReturnCode_t ret = filter_factory->create_content_filter(
        filter_property.filter_class_name.c_str(),
        related_topic->get_type_name().c_str(),
        type.get(), new_expression, filter_parameters, filter_instance);

    if (RETCODE_OK == ret)
    {
        filter_property.expression_parameters.assign(new_expression_parameters.begin(),
                new_expression_parameters.end());
        if (nullptr != new_expression)
        {
            filter_property.filter_expression = new_expression;
        }

        // Update filter signature
        update_signature();

        // Inform data readers
        for (DataReaderImpl* reader : readers_)
        {
            reader->filter_has_been_updated();
        }
    }

    return ret;
}

void ContentFilteredTopicImpl::update_signature()
{
    ContentFilterUtils::compute_signature(filter_property, filter_signature_, filter_signature_rti_connext_);
}

bool ContentFilteredTopicImpl::check_filter_signature(
        const fastdds::rtps::CacheChange_t& change,
        bool& filter_result) const
{
    // Empty expressions always pass the filter
    if (filter_property.filter_expression.empty())
    {
        filter_result = true;
        return true;
    }

    // Find PID_CONTENT_FILTER_INFO on inline QoS
    if (0 == change.inline_qos.length || nullptr == change.inline_qos.data)
    {
        return false;
    }

    bool found = false;

    // This will be called for every parameter found on the inline QoS of the received change.
    // If a ContentFilterInfo parameter is found, it will traverse the filter signatures trying to find one that
    // matches our own signature, in which case found is set to true, and filter_result is set to the result present
    // on the ContentFilterInfo parameter for that signature.
    auto parameter_process = [&](
        fastdds::rtps::CDRMessage_t* msg,
        const ParameterId_t pid,
        uint16_t plength)
            {
                if (PID_CONTENT_FILTER_INFO == pid)
                {
                    uint32_t num_bitmaps = 0;
                    uint32_t num_signatures = 0;
                    bool valid;

                    // Validate and consume length for numBitmaps and numSignatures
                    valid = 8 < plength;
                    if (!valid)
                    {
                        return true;
                    }
                    plength -= 8;

                    // Read and validate numBitmaps
                    valid &= fastdds::rtps::CDRMessage::readUInt32(msg, &num_bitmaps);
                    valid &= num_bitmaps * 4 <= plength;
                    if (!valid || 0 == num_bitmaps)
                    {
                        return true;
                    }

                    // Save starting position of bitmaps and skip them
                    uint32_t bitmap_pos = msg->pos;
                    msg->pos += num_bitmaps * 4;
                    plength -= static_cast<uint16_t>(num_bitmaps * 4);

                    // Read and validate numSignatures
                    valid &= fastdds::rtps::CDRMessage::readUInt32(msg, &num_signatures);
                    valid &= num_signatures * 16 <= plength;
                    if (!valid || 0 == num_signatures || ((num_signatures + 31) / 32) != num_bitmaps)
                    {
                        return true;
                    }

                    // Lookup our own signature
                    uint32_t i;
                    for (i = 0; i < num_signatures; ++i)
                    {
                        if (std::equal(filter_signature_.begin(), filter_signature_.end(), msg->buffer + msg->pos))
                        {
                            found = true;
                            break;
                        }
                        else if (0x01 == change.writerGUID.guidPrefix.value[0] &&
                                0x01 == change.writerGUID.guidPrefix.value[1] &&
                                std::equal(filter_signature_rti_connext_.begin(), filter_signature_rti_connext_.end(),
                                msg->buffer + msg->pos))
                        {
                            found = true;
                            break;
                        }
                        msg->pos += 16;
                    }

                    // Signature found, set filter result from bitmap
                    if (found)
                    {
                        uint32_t bitmap_idx = i / 32;
                        uint32_t bitmask = 1 << (31 - (i & 31));
                        uint32_t bitmap = 0;

                        msg->pos = bitmap_pos + bitmap_idx * 4;
                        fastdds::rtps::CDRMessage::readUInt32(msg, &bitmap);
                        filter_result = 0 != (bitmap & bitmask);
                    }
                }

                return true;
            };

    uint32_t qos_size = 0;
    fastdds::rtps::CDRMessage_t msg(change.inline_qos);
    ParameterList::readParameterListfromCDRMsg(msg, parameter_process, false, qos_size);

    return found;
}

void ContentFilterUtils::compute_signature(
        const rtps::ContentFilterProperty& filter_property,
        std::array<uint8_t, 16>& filter_signature)
{
    MD5 md5_rtps;

    md5_rtps.init();

    // Add content_filtered_topic_name
    {
        const char* str = filter_property.content_filtered_topic_name.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
    }
    // Add related_topic_name
    {
        const char* str = filter_property.related_topic_name.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
    }
    // Add filter_class_name
    {
        const char* str = filter_property.filter_class_name.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
    }
    // Add filter_expression
    {
        const char* str = filter_property.filter_expression.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
    }
    // Add expression_parameters
    for (const auto& param : filter_property.expression_parameters)
    {
        const char* str = param.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
    }
    md5_rtps.finalize();

    std::copy_n(md5_rtps.digest, filter_signature.size(), filter_signature.begin());
}

void ContentFilterUtils::compute_signature(
        const rtps::ContentFilterProperty& filter_property,
        std::array<uint8_t, 16>& filter_signature_rtps,
        std::array<uint8_t, 16>& filter_signature_rti_connext)
{
    MD5 md5_rtps;
    MD5 md5_connext;

    md5_rtps.init();
    md5_connext.init();

    // Add content_filtered_topic_name
    {
        const char* str = filter_property.content_filtered_topic_name.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
        md5_connext.update(str, slen);
    }
    // Add related_topic_name
    {
        const char* str = filter_property.related_topic_name.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
        md5_connext.update(str, slen);
    }
    // Add filter_class_name
    {
        const char* str = filter_property.filter_class_name.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
        md5_connext.update(str, slen);
    }
    // Add filter_expression
    size_t n_params = filter_property.expression_parameters.size();
    {
        const char* str = filter_property.filter_expression.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
        md5_connext.update(str, (0 == n_params) ? slen - 1 : slen);
    }
    // Add expression_parameters
    size_t i = 0;
    for (const auto& param : filter_property.expression_parameters)
    {
        const char* str = param.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
        ++i;
        md5_connext.update(str, i == n_params ? slen - 1 : slen);
    }
    md5_rtps.finalize();
    md5_connext.finalize();

    std::copy_n(md5_rtps.digest, filter_signature_rtps.size(), filter_signature_rtps.begin());
    std::copy_n(md5_connext.digest, filter_signature_rti_connext.size(), filter_signature_rti_connext.begin());
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
