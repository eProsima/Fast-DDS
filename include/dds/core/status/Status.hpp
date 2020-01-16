/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef OMG_DDS_CORE_STATUS_STATUS_HPP_
#define OMG_DDS_CORE_STATUS_STATUS_HPP_

#include <dds/core/status/State.hpp>

#include <dds/core/Value.hpp>
#include <dds/core/InstanceHandle.hpp>
#include <dds/core/policy/CorePolicy.hpp>
#include <dds/core/status/detail/Status.hpp>

#include <cstdint>

namespace dds {
namespace core {
namespace status {

/**
 * \copydoc DCPS_Status_InconsistentTopic
 */
class InconsistentTopicStatus : public dds::core::Value<detail::InconsistentTopicStatus>
{
public:

    OMG_DDS_API InconsistentTopicStatus();

    /**
     * @return Total cumulative count of all inconsistent topics detected.
     */
    OMG_DDS_API int32_t total_count() const;

    /**
     * @return The incremental number of inconsistent topics since the last time
     * the listener was called or the status was read.
     */
    OMG_DDS_API int32_t total_count_change() const;
};

/**
 * \copydoc DCPS_Status_SampleLost
 */
class SampleLostStatus : public dds::core::Value<detail::SampleLostStatus>
{
public:

    OMG_DDS_API SampleLostStatus();

    /**
     * @return Total cumulative count of all samples lost across of instances of data
     * published under the Topic.
     */
    OMG_DDS_API int32_t total_count() const;

    /**
     * @return The incremental number of samples lost since the last time the listener
     * was called or the status was read.
     */
    OMG_DDS_API int32_t total_count_change() const;
};


/**
 * \copydoc DCPS_Status_SampleRejected
 */
class SampleRejectedStatus : public dds::core::Value<detail::SampleRejectedStatus>
{
public:

    OMG_DDS_API SampleRejectedStatus();

    /**
     * @return Total cumulative count of samples rejected by the DataReader.
     */
    OMG_DDS_API int32_t total_count() const;

    /**
     * @return The incremental number of samples rejected since the last time the
     * listener was called or the status was read.
     */
    OMG_DDS_API int32_t total_count_change() const;

    /**
     * @return Reason for rejecting the last sample rejected. If no samples have been
     * rejected, the reason is the special value NOT_REJECTED.
     */
    OMG_DDS_API const dds::core::status::SampleRejectedState last_reason() const;

    /**
     * @return Handle to the instance being updated by the last sample that was
     * rejected.
     */
    OMG_DDS_API const dds::core::InstanceHandle last_instance_handle() const;
};

/**
 * \copydoc DCPS_Status_LivelinessLost
 */
class LivelinessLostStatus : public dds::core::Value<detail::LivelinessLostStatus>
{
public:

    OMG_DDS_API LivelinessLostStatus();

    /**
     * @return Total cumulative number of times that a previously-alive DataWriter
     * became 'not alive' due to a failure to actively signal its liveliness within
     * its offered liveliness period. This count does not change when an
     * already not alive DataWriter simply remains not alive for another
     * liveliness period.
     */
    OMG_DDS_API int32_t total_count() const;

    /**
     * @return The change in total_count since the last time the listener was called or
     * the status was read.
     */
    OMG_DDS_API int32_t total_count_change() const;
};

/**
 * \copydoc DCPS_Status_LivelinessChanged
 */
class LivelinessChangedStatus : public dds::core::Value<detail::LivelinessChangedStatus>
{
public:

    OMG_DDS_API LivelinessChangedStatus();

    /**
     * @return The total number of currently active DataWriters that write the Topic
     * read by the DataReader. This count increases when a newly-matched
     * DataWriter asserts its liveliness for the first time or when a DataWriter
     * previously considered to be not alive reasserts its liveliness. The count
     * decreases when a DataWriter considered alive fails to assert its
     * liveliness and becomes not alive, whether because it was deleted
     * normally or for some other reason.
     */
    OMG_DDS_API int32_t alive_count() const;

    /**
     * @return The total count of currently DataWriters that write the Topic read by
     * the DataReader that are no longer asserting their liveliness. This count
     * increases when a DataWriter considered alive fails to assert its
     * liveliness and becomes not alive for some reason other than the normal
     * deletion of that DataWriter. It decreases when a previously not alive
     * DataWriter either reasserts its liveliness or is deleted normally.
     */
    OMG_DDS_API int32_t not_alive_count() const;

    /**
     * @return The change in the alive_count since the last time the listener was
     * called or the status was read.
     */
    OMG_DDS_API int32_t alive_count_change() const;

    /**
     * @return The change in the not_alive_count since the last time the listener was
     * called or the status was read.
     */
    OMG_DDS_API int32_t not_alive_count_change() const;

    /**
     * @return Handle to the last DataWriter whose change in liveliness caused this
     * status to change.
     */
    OMG_DDS_API const dds::core::InstanceHandle last_publication_handle() const;
};

/**
 * \copydoc DCPS_Status_OfferedDeadlineMissed
 */
class OfferedDeadlineMissedStatus : public dds::core::Value<detail::OfferedDeadlineMissedStatus>
{
public:

    OMG_DDS_API OfferedDeadlineMissedStatus();

    /**
     * @return Total cumulative number of offered deadline periods elapsed during
     * which a DataWriter failed to provide data. Missed deadlines
     * accumulate; that is, each deadline period the total_count will be
     * incremented by one.
     */
    OMG_DDS_API int32_t total_count() const;

    /**
     * @return The change in total_count since the last time the listener was called or
     * the status was read.
     */
    OMG_DDS_API int32_t total_count_change() const;

    /**
     * @return Handle to the last instance in the DataWriter for which an offered
     * deadline was missed.
     */
    OMG_DDS_API const dds::core::InstanceHandle last_instance_handle() const;
};

/**
 * \copydoc DCPS_Status_RequestedDeadlineMissed
 */
class RequestedDeadlineMissedStatus : public dds::core::Value<detail::RequestedDeadlineMissedStatus>
{
public:

    OMG_DDS_API RequestedDeadlineMissedStatus();

    /**
     * @return Total cumulative number of missed deadlines detected for any instance
     * read by the DataReader. Missed deadlines accumulate; that is, each
     * deadline period the total_count will be incremented by one for each
     * instance for which data was not received.
     */
    OMG_DDS_API int32_t total_count() const;

    /**
     * @return The incremental number of deadlines detected since the last time the
     * listener was called or the status was read.
     */
    OMG_DDS_API int32_t total_count_change() const;

    /**
     * @return Handle to the last instance in the DataReader for which a deadline was
     * detected.
     */
    OMG_DDS_API const dds::core::InstanceHandle last_instance_handle() const;
};


/**
 * \copydoc DCPS_Status_OfferedIncompatibleQoS
 */
class OfferedIncompatibleQosStatus : public dds::core::Value<detail::OfferedIncompatibleQosStatus>
{
public:

    OMG_DDS_API OfferedIncompatibleQosStatus();

    /**
     * @return Total cumulative number of times the concerned DataWriter
     * discovered a DataReader for the same Topic with a requested QoS that
     * is incompatible with that offered by the DataWriter.
     */
    OMG_DDS_API int32_t total_count() const;

    /**
     * @return The change in total_count since the last time the listener was called or
     * the status was read.
     */
    OMG_DDS_API int32_t total_count_change() const;

    /**
     * @return The PolicyId of one of the policies that was found to be
     * incompatible the last time an incompatibility was detected.
     */
    OMG_DDS_API dds::core::policy::QosPolicyId last_policy_id() const;

    /**
     * @return A list containing for each policy the total number of times that the
     * concerned DataWriter discovered a DataReader for the same Topic
     * with a requested QoS that is incompatible with that offered by the
     * DataWriter.
     */
    OMG_DDS_API const dds::core::policy::QosPolicyCountSeq policies() const;

    /**
     * @return A list containing for each policy the total number of times that the
     * concerned DataWriter discovered a DataReader for the same Topic
     * with a requested QoS that is incompatible with that offered by the
     * DataWriter.
     *
     * @param dst The destination QosPolicyCountSeq the policies will be returned to
     */
    OMG_DDS_API const dds::core::policy::QosPolicyCountSeq& policies(
            dds::core::policy::QosPolicyCountSeq& dst) const;
};

/**
 * \copydoc DCPS_Status_RequestedIncompatibleQoS
 */
class RequestedIncompatibleQosStatus : public dds::core::Value<detail::RequestedIncompatibleQosStatus>
{
public:

    OMG_DDS_API RequestedIncompatibleQosStatus();

    /**
     * @return Total cumulative number of times the concerned DataReader
     * discovered a DataWriter for the same Topic with an offered QoS that
     * was incompatible with that requested by the DataReader.
     */
    OMG_DDS_API int32_t total_count() const;

    /**
     * @return The change in total_count since the last time the listener was called or
     * the status was read.
     */
    OMG_DDS_API int32_t total_count_change() const;

    /**
     * @return The QosPolicyId of one of the policies that was found to be
     * incompatible the last time an incompatibility was detected.
     */
    OMG_DDS_API dds::core::policy::QosPolicyId last_policy_id() const;

    /**
     * @return A list containing for each policy the total number of times that the
     * concerned DataReader discovered a DataWriter for the same Topic
     * with an offered QoS that is incompatible with that requested by the
     * DataReader.
     */
    OMG_DDS_API const dds::core::policy::QosPolicyCountSeq policies() const;

    /**
     * @return A list containing for each policy the total number of times that the
     * concerned DataReader discovered a DataWriter for the same Topic
     * with an offered QoS that is incompatible with that requested by the
     * DataReader.
     *
     * @param dst The destination QosPolicyCountSeq the policies will be returned to
     */
    OMG_DDS_API const dds::core::policy::QosPolicyCountSeq& policies(
            dds::core::policy::QosPolicyCountSeq& dst) const;
};

/**
 * \copydoc DCPS_Status_PublicationMatched
 */
class PublicationMatchedStatus : public dds::core::Value<detail::PublicationMatchedStatus>
{
public:

    OMG_DDS_API PublicationMatchedStatus();

    /**
     * @return Total cumulative count the concerned DataWriter discovered a
     * "match" with a DataReader. That is, it found a DataReader for the
     * same Topic with a requested QoS that is compatible with that offered
     * by the DataWriter.
     */
    OMG_DDS_API int32_t total_count() const;

    /**
     * @return The change in total_count since the last time the listener was called or
     * the status was read.
     */
    OMG_DDS_API int32_t total_count_change() const;

    /**
     * @return The number of DataReaders currently matched to the concerned
     * DataWriter.
     */
    OMG_DDS_API int32_t current_count() const;

    /**
     * @return The change in current_count since the last time the listener was called
     * or the status was read.
     */
    OMG_DDS_API int32_t current_count_change() const;

    /**
     * @return Handle to the last DataReader that matched the DataWriter causing the
     * status to change.
     */
    OMG_DDS_API const dds::core::InstanceHandle last_subscription_handle() const;
};

/**
 * \copydoc DCPS_Status_SubscriptionMatched
 */
class SubscriptionMatchedStatus : public dds::core::Value<detail::SubscriptionMatchedStatus>
{
public:

    OMG_DDS_API SubscriptionMatchedStatus();

    /**
     * @return Total cumulative count the concerned DataReader discovered a
     * "match" with a DataWriter. That is, it found a DataWriter for the same
     * Topic with a requested QoS that is compatible with that offered by the
     * DataReader.
     */
    OMG_DDS_API int32_t total_count() const;

    /**
     * @return The change in total_count since the last time the listener was called or
     * the status was read.
     */
    OMG_DDS_API int32_t total_count_change() const;

    /**
     * @return The number of DataWriters currently matched to the concerned
     * DataReader.
     */
    OMG_DDS_API int32_t current_count() const;

    /**
     * @return The change in current_count since the last time the listener was called
     * or the status was read.
     */
    OMG_DDS_API int32_t current_count_change() const;

    /**
     * @return Handle to the last DataWriter that matched the DataReader causing the
     * status to change.
     */
    OMG_DDS_API const dds::core::InstanceHandle last_publication_handle() const;
};


class DataAvailableStatus
{
    // empty
};


class DataOnReadersStatus
{
    // empty
};

// This trait is used to get the state associated with each status
template<typename STATUS>
StatusMask get_status();

} //namespace status
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_STATUS_STATUS_HPP_
