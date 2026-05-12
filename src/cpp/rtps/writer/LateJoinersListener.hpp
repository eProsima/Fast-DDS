/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

/**
 * @file LateJoinersListener.hpp
 */

#ifndef FASTDDS_RTPS_WRITER__LATEJOINERSLISTENER_HPP
#define FASTDDS_RTPS_WRITER__LATEJOINERSLISTENER_HPP

namespace eprosima {
namespace fastdds {
namespace rtps {

struct CacheChange_t;
class ReaderProxyData;

/**
 * Interface class for listeners invoked on events associated to late joiners.
 */
class LateJoinersListener
{
protected:

    LateJoinersListener() = default;

public:

    virtual ~LateJoinersListener() = default;

    /**
     * @brief Method called when the writer discovers a late joiner.
     *
     * @param rdata ReaderProxyData of the late joiner that has been discovered.
     */
    virtual void on_late_joiner_added(
            const ReaderProxyData& rdata)
    {
        static_cast<void>(rdata);
    }

    /**
     * @brief Method called to preprocess a change for a late joiner prior to delivery.
     *
     * @param change Change to preprocess.
     * @param rdata ReaderProxyData of the late joiner for which the change will be preprocessed.
     */
    virtual void preprocess_change_for_late_joiner(
            CacheChange_t& change,
            const ReaderProxyData& rdata)
    {
        static_cast<void>(change);
        static_cast<void>(rdata);
    }

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_RTPS_WRITER__LATEJOINERSLISTENER_HPP
