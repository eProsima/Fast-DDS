/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
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
 */

#ifndef OMG_DDS_SUB_DATA_STATE_HPP_
#define OMG_DDS_SUB_DATA_STATE_HPP_

#include <bitset>

#include <dds/core/types.hpp>
#include <fastrtps/rtps/common/CacheChange.h>


namespace dds {
namespace sub {
namespace status {

class SampleState;
class ViewState;
class InstanceState;
class DataState;


/**
 * @brief
 * Class to hold SampleState information.
 *
 * For each sample, the Data Distribution Service internally maintains a
 * sample_state specific to each DataReader. The sample_state can either be
 * READ_SAMPLE_STATE or NOT_READ_SAMPLE_STATE.
 * - <b><i>read</i></b>
 *      - The DataReader has already accessed that
 *        sample by means of read. Had the sample been accessed by take it would
 *        no longer be available to the DataReader.
 * - <b><i>not_read</i></b>
 *      - The DataReader has not accessed that sample before.
 *
 * @see @ref DCPS_Modules_Subscription_SampleInfo "SampleInfo" for more information
 */
class OMG_DDS_API SampleState : public std::bitset<OMG_DDS_STATE_BIT_COUNT>
{
public:

    /**
     * Convenience typedef for std::bitset<OMG_DDS_STATE_BIT_COUNT>.
     */
    typedef std::bitset<OMG_DDS_STATE_BIT_COUNT> MaskType;

    /**
     * Construct a SampleState with default MaskType.
     */
    SampleState()
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>()
    {
    }

    /**
     * Construct a SampleState with MaskType of i.
     *
     * @param i MaskType
     */
    explicit SampleState(
            uint32_t i)
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>(i)
    {
    }

    /**
     * Copy constructor.
     * Construct a SampleState with existing SampleState.
     *
     * @param src the SampleState to copy from
     */
    SampleState(
            const SampleState& src)
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>(src)
    {
    }

    /**
     * Construct a SampleState with existing MaskType.
     *
     * @param src the MaskType to copy from
     */
    SampleState(
            const MaskType& src)
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>(src)
    {
    }

    /**
     * Get the READ_SAMPLE_STATE.
     *
     * <i>read</i>
     *      - The DataReader has already accessed that
     *        sample by means of read. Had the sample been accessed by take it would
     *        no longer be available to the DataReader.
     *
     * @return the read SampleState
     */
    inline static const SampleState read()
    {
        return SampleState(0x0001 << 0u);
    }

    /**
     * Get the NOT_READ_SAMPLE_STATE.
     *
     *<i>not_read</i>
     *      - The DataReader has not accessed that sample before.
     *
     * @return the not_read SampleState
     */
    inline static const SampleState not_read()
    {
        return SampleState(0x0001 << 1u);
    }

    /**
     * Get any SampleState.
     *
     * Either the sample has already been read or not read.
     *
     * @return any SampleState
     */
    inline static const SampleState any()
    {
        return SampleState(0xffff);
    }

};

/**
 * @brief
 * Class to hold sample ViewState information.
 *
 * For each instance (identified by the key), the Data Distribution Service internally
 * maintains a view_state relative to each DataReader. The ViewSate can
 * either be NEW_VIEW_STATE or NOT_NEW_VIEW_STATE.
 * - <b><i>new_view</i></b>
 *      - Either this is the first time that the DataReader
 *        has ever accessed samples of that instance, or else that the DataReader has
 *        accessed previous samples of the instance, but the instance has since been reborn
 *        (i.e. become not-alive and then alive again).
 * - <b><i>not_new_view</i></b>
 *      - The DataReader has already accessed
 *        samples of the same instance and that the instance has not been reborn since.
 *
 * @see @ref DCPS_Modules_Subscription_SampleInfo "SampleInfo" for more information
 */
class OMG_DDS_API ViewState : public std::bitset<OMG_DDS_STATE_BIT_COUNT>
{
public:

    /**
     * Convenience typedef for std::bitset<OMG_DDS_STATE_BIT_COUNT>.
     */
    typedef std::bitset<OMG_DDS_STATE_BIT_COUNT> MaskType;

    /**
     * Construct a ViewState with default MaskType.
     */
    ViewState()
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>()
    {
    }

    /**
     * Construct a ViewState with MaskType of i.
     *
     * @param m the MaskType
     */
    explicit ViewState(
            uint32_t m)
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>(m)
    {
    }

    /**
     * Copy constructor.
     *
     * Construct a ViewState with existing ViewState.
     *
     * @param src the ViewState to copy from
     */
    ViewState(
            const ViewState& src)
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>(src)
    {
    }

    /**
     * Construct a ViewState with existing MaskType.
     *
     * @param src the MaskType to copy from
     */
    ViewState(
            const MaskType& src)
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>(src)
    {
    }

    /**
     * Get the NEW_VIEW_STATE.
     *
     * <i>new_view</i>
     *      - Either this is the first time that the DataReader
     *        has ever accessed samples of that instance, or else that the DataReader has
     *        accessed previous samples of the instance, but the instance has since been reborn
     *        (i.e. become not-alive and then alive again).
     *
     * @return the new_view ViewState
     */
    inline static const ViewState new_view()
    {
        return ViewState(0x0001 << 0u);
    }

    /**
     * Get the NOT_NEW_VIEW_STATE.
     *
     * <i>not_new_view</i>
     *      - The DataReader has already accessed
     *        samples of the same instance and that the instance has not been reborn since.
     *
     * @return the not_new_view ViewState
     */
    inline static const ViewState not_new_view()
    {
        return ViewState(0x0001 << 1u);
    }

    /**
     * Get any ViewState.
     *
     * Either the sample has already been seen or not seen.
     *
     * @return the any ViewState
     */
    inline static const ViewState any()
    {
        return ViewState(0xffff);
    }

};

/**
 * @brief
 * Class to hold sample InstanceState information.
 *
 * For each instance the Data Distribution Service internally maintains an
 * InstanceState. The InstanceState can be:
 * - <b><i>alive</i></b>, which indicates that
 *      - samples have been received for the instance
 *      - there are live DataWriter objects writing the instance
 *      - the instance has not been explicitly disposed of (or else samples have been
 *        received after it was disposed of)
 * - <b><i>not_alive_disposed</i></b>, which indicates that
 *     - the instance was disposed
 *       of by a DataWriter, either explicitly by means of the dispose operation or
 *       implicitly in case the autodispose_unregistered_instances field of the
 *       WriterDataLyfecycle QosPolicy equals TRUE when the instance gets
 *       unregistered, WriterDataLifecycle QosPolicy and no new
 *       samples for that instance have been written afterwards.
 * - <b><i>not_alive_no_writers</i></b>, which indicates that
 *      - the instance has been
 *        declared as not-alive by the DataReader because it detected that there are no live
 *        DataWriter objects writing that instance.
 *
 * @see @ref DCPS_Modules_Subscription_SampleInfo "SampleInfo" for more information
 */
class OMG_DDS_API InstanceState : public std::bitset<OMG_DDS_STATE_BIT_COUNT>
{
public:

    /**
     * Convenience typedef for std::bitset<OMG_DDS_STATE_BIT_COUNT>.
     */
    typedef std::bitset<OMG_DDS_STATE_BIT_COUNT> MaskType;

public:

    /**
     * Construct an InstanceState with no state flags set.
     */
    InstanceState()
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>()
    {
    }

    /**
     * Construct an InstanceState with an uint32_t m, representing a bit array.
     *
     * @param m the bit array to initialize the bitset with
     */
    explicit InstanceState(
            uint32_t m)
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>(m)
    {
    }

    /**
     * Copy constructor.
     *
     * Construct an InstanceState with existing InstanceState.
     *
     * @param src the InstanceState to copy from
     */
    InstanceState(
            const InstanceState& src)
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>(src)
    {
    }

    /**
     * Construct an InstanceState with existing MaskType.
     *
     * @param src the bitset to copy from
     */
    InstanceState(
            const MaskType& src)
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>(src)
    {
    }

    inline void operator =(
            eprosima::fastrtps::rtps::ChangeKind_t kind)
    {
        if (kind == eprosima::fastrtps::rtps::ALIVE)
        {
            *this = alive();
        }
        else if (kind == eprosima::fastrtps::rtps::NOT_ALIVE_DISPOSED)
        {
            *this = not_alive_disposed();
        }
        else if (kind == eprosima::fastrtps::rtps::NOT_ALIVE_UNREGISTERED)
        {
            *this = not_alive_no_writers();
        }
        else if (kind == eprosima::fastrtps::rtps::NOT_ALIVE_DISPOSED_UNREGISTERED)
        {
            *this = not_alive_mask();
        }
    }

    /**
     * Get ALIVE_INSTANCE_STATE.
     *
     * <i>alive</i>, which indicates that
     *      - samples have been received for the instance
     *      - there are live DataWriter objects writing the instance
     *      - the instance has not been explicitly disposed of (or else samples have been
     *        received after it was disposed of)
     *
     * @return the alive InstanceState
     */
    inline static const InstanceState alive()
    {
        return InstanceState(0x0001 << 0u);
    }

    /**
     * Get NOT_ALIVE_DISPOSED_INSTANCE_STATE.
     *
     * <i>not_alive_disposed</i>, which indicates that
     *     - the instance was disposed
     *       of by a DataWriter, either explicitly by means of the dispose operation or
     *       implicitly in case the autodispose_unregistered_instances field of the
     *       WriterDataLyfecycle QosPolicy equals TRUE when the instance gets
     *       unregistered, WriterDataLifecycle QosPolicy and no new
     *       samples for that instance have been written afterwards.
     *
     * @return the not_alive_disposed InstanceState
     */
    inline static const InstanceState not_alive_disposed()
    {
        return InstanceState(0x0001 << 1u);
    }

    /**
     * Get NOT_ALIVE_NO_WRITERS_INSTANCE_STATE.
     *
     * <i>not_alive_no_writers</i>, which indicates that
     *      - the instance has been
     *        declared as not-alive by the DataReader because it detected that there are no live
     *        DataWriter objects writing that instance.
     *
     * @return the not_alive_no_writers InstanceState
     */
    inline static const InstanceState not_alive_no_writers()
    {
        return InstanceState(0x0001 << 2u);
    }

    /**
     * Get not_alive mask
     *
     * <i>not_alive</i> = not_alive_disposed | not_alive_no_writers:
     * - <i>not_alive_disposed</i>, which indicates that
     *     - the instance was disposed
     *       of by a DataWriter, either explicitly by means of the dispose operation or
     *       implicitly in case the autodispose_unregistered_instances field of the
     *       WriterDataLyfecycle QosPolicy equals TRUE when the instance gets
     *       unregistered, WriterDataLifecycle QosPolicy and no new
     *       samples for that instance have been written afterwards.
     * - <i>not_alive_no_writers</i>, which indicates that
     *      - the instance has been
     *        declared as not-alive by the DataReader because it detected that there are no live
     *        DataWriter objects writing that instance.
     *
     * @return the not_alive_mask InstanceState
     */
    inline static const InstanceState not_alive_mask()
    {
        return not_alive_disposed() | not_alive_no_writers();
    }

    /**
     * Get any InstanceState.
     *
     * This Instance is either in existence or not in existence.
     *
     * @return the any InstanceState
     */
    inline static const InstanceState any()
    {
        return InstanceState(0xffff);
    }

};

/**
 * @brief
 * Class to hold sample DataState information.
 *
 * The DataState is part of dds::sub::SampleInfo but can also be used as data filter for
 * @ref dds::sub::DataReader "DataReaders".
 *
 * The DataState contains the following:
 *  - The sample_state of the Data value (i.e., if the sample has already been READ or NOT_READ by that same DataReader).
 *  - The view_state of the related instance (i.e., if the instance is NEW, or NOT_NEW for that DataReader).
 *  - The instance_state of the related instance (i.e., if the instance is ALIVE, NOT_ALIVE_DISPOSED, or NOT_ALIVE_NO_WRITERS).
 *
 * @see @ref DCPS_Modules_Subscription_SampleInfo "SampleInfo" for more information
 */
class DataState
{
public:

    /**
     * Create a DataState instance.
     *
     * Construct a DataState with:
     * - SampleState::any
     * - ViewState::any
     * - InstanceState::any
     *
     */
    DataState()
    //        : ss_(SampleState::any()),
    //          vs_(ViewState::any()),
    //          is_(InstanceState::any())
    {
    }

    /**
     * Create a DataState instance.
     *
     * Construct a DataState with:
     * - Given SampleState ss
     * - ViewState::any
     * - InstanceState::any
     *
     * @param ss the SampleState to construct DataState from
     */
    /* implicit */ DataState(
            const SampleState& ss)
        : ss_(ss)
        //          vs_(ViewState::any()),
        //          is_(InstanceState::any())
    {
    }

    /**
     * Create a DataState instance.
     *
     * Construct a DataState with:
     * - SampleState::any
     * - Given ViewState vs
     * - InstanceState::any
     *
     * @param vs the ViewState to construct DataState from
     */
    /* implicit */ DataState(
            const ViewState& vs)
    /*ss_(SampleState::any()),*/: vs_(vs)
        //          is_(InstanceState::any())
    {
    }

    /**
     * Create a DataState instance.
     *
     * Construct a DataState with:
     * - SampleState::any
     * - ViewState::any
     * - Given InstanceState is
     *
     * @param is InstanceState to construct DataState from
     */
    /* implicit */ DataState(
            const InstanceState& is)
    /*ss_(SampleState::any()),
       vs_(ViewState::any()),*/: is_(is)
    {
    }

    /**
     * Create a DataState instance.
     *
     * Construct a DataState with:
     * - Given SampleState ss
     * - Given ViewState vs
     * - Given InstanceState is
     *
     * @param ss SampleState
     * @param vs ViewState
     * @param is InstanceState
     */
    DataState(
            const SampleState& ss,
            const ViewState& vs,
            const InstanceState& is)
        : ss_(ss)
        , vs_(vs)
        , is_(is)
    {
    }

    /**
     * Set SampleState.
     *
     * @param ss SampleState
     */
    DataState& operator <<(
            const SampleState& ss)
    {
        ss_ = ss;
        return *this;
    }

    /**
     * Set InstanceState.
     *
     * @param is InstanceState
     */
    DataState& operator <<(
            const InstanceState& is)
    {
        is_ = is;
        return *this;
    }

    /**
     * Set ViewState.
     *
     * @param vs ViewState
     */
    DataState& operator <<(
            const ViewState& vs)
    {
        vs_ = vs;
        return *this;
    }

    /**
     * Get SampleState.
     *
     * @param ss SampleState
     * @return the DataState
     */
    const DataState& operator >>(
            SampleState& ss) const
    {
        ss = ss_;
        return *this;
    }

    /**
     * Get InstanceState.
     *
     * @param is InstanceState
     * @return the DataState
     */
    const DataState& operator >>(
            InstanceState& is) const
    {
        is = is_;
        return *this;
    }

    /**
     * Get ViewState.
     *
     * @param vs ViewState
     * @return the DataState
     */
    const DataState& operator >>(
            ViewState& vs) const
    {
        vs = vs_;
        return *this;
    }

    /**
     * Check if this DataState is equal with another
     *
     * @return true if equal
     */
    bool operator ==(
            const DataState& o) const
    {
        return ((ss_ == o.ss_) && (vs_ == o.vs_) && (is_ == o.is_));
    }

    /**
     * Check if this DataState is not equal with another
     *
     * @return true if not equal
     */
    bool operator !=(
            const DataState& o) const
    {
        return !operator ==(o);
    }

    /**
     * Get SampleState.
     *
     * @return the SampleState
     */
    const SampleState& sample_state() const
    {
        return ss_;
    }

    /**
     * Set SampleState.
     *
     * @param ss SampleState
     */
    void sample_state(
            const SampleState& ss)
    {
        *this << ss;
    }

    /**
     * Get InstanceState.
     *
     * @return the InstanceState
     */
    const InstanceState& instance_state() const
    {
        return is_;
    }

    /**
     * Set InstanceState.
     *
     * @param is InstanceState
     */
    void instance_state(
            const InstanceState& is)
    {
        *this << is;
    }

    /**
     * Get ViewState.
     *
     * @return the ViewState
     */
    const ViewState& view_state() const
    {
        return vs_;
    }

    /**
     * Set ViewState.
     *
     * @param vs ViewState
     */
    void view_state(
            const ViewState& vs)
    {
        *this << vs;
    }

    /**
     * Create a DataState instance.
     *
     * Return a DataState with:
     * - SampleState::any
     * - ViewState::any
     * - InstanceState::any
     *
     * @return the any DataState
     */
    static DataState any();
    //    {
    //        return DataState(SampleState::any(),
    //                         ViewState::any(),
    //                         InstanceState::any());
    //    }

    /**
     * Create a DataState instance.
     *
     * Return a DataState with:
     * - SampleState::not_read
     * - ViewState::any
     * - InstanceState::alive
     *
     * @return the new_data DataState
     */
    static DataState new_data();
    //    {
    //        return DataState(SampleState::not_read(),
    //                         ViewState::any(),
    //                         InstanceState::alive());
    //    }

    /**
     * Create a DataState instance.
     *
     * Return a DataState with:
     * - SampleState::any
     * - ViewState::any
     * - InstanceState::alive
     *
     * @return the any_data DataState
     */
    static DataState any_data();
    //    {
    //        return DataState(SampleState::any(),
    //                         ViewState::any(),
    //                         InstanceState::alive());
    //    }

    /**
     * Create a DataState instance.
     *
     * Return a DataState with:
     * - SampleState::any
     * - ViewState::new_view
     * - InstanceState::alive
     *
     * @return the new_instance DataState
     */
    static DataState new_instance();
    //    {
    //        return DataState(SampleState::any(),
    //                         ViewState::new_view(),
    //                         InstanceState::alive());
    //    }

private:

    SampleState ss_;
    ViewState vs_;
    InstanceState is_;

};

} //namespace status
} //namespace sub
} //namespace dds

#endif //OMG_DDS_SUB_DATA_STATE_HPP_
