/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

/**
 * @file CongestionControlListener.hpp
 */

#ifndef FASTDDS_RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLLISTENER_HPP
#define FASTDDS_RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLLISTENER_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/congestion-control/CongestionControlStatus.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * @brief Listener used by the application to observe congestion-control and
 * flow-controller meta-information.
 *
 * It is set on the DomainParticipant through
 * DomainParticipant::set_congestion_control_listener and applies to the participant's
 * congestion controller (one per participant, covering all reliable writers and
 * evaluated per remote reader).
 *
 * Callbacks are invoked from the congestion-control evaluation thread. Default
 * implementations are empty no-ops, so the user only overrides the events of interest.
 *
 * @warning The listener object must outlive the participant it is registered on (it is
 * not copied). It must not be destroyed while the participant is enabled.
 */
class CongestionControlListener
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API CongestionControlListener()
    {
    }

    /**
     * @brief Destructor
     */
    FASTDDS_EXPORTED_API virtual ~CongestionControlListener()
    {
    }

    /**
     * @brief Called by every congestion-control algorithm whenever the per-reader
     * bandwidth limit changes (either an increase or a decrease).
     *
     * @param status The limit-update status.
     */
    FASTDDS_EXPORTED_API virtual void on_cc_limit_update(
            const CongestionControlLimitUpdateStatus& status)
    {
        static_cast<void>(status);
    }

    /**
     * @brief Called once per reader on every periodic evaluation cycle, regardless of
     * whether the limit changed.
     *
     * @param status The periodic per-reader status snapshot.
     */
    FASTDDS_EXPORTED_API virtual void on_cc_status_check(
            const CongestionControlStatus& status)
    {
        static_cast<void>(status);
    }

    /**
     * @brief Generic algorithm-specific channel. Each algorithm assigns its own meaning
     * to the @c {code, value} pair (see CongestionControlInfoCodes.hpp).
     *
     * @param status The algorithm-specific info.
     */
    FASTDDS_EXPORTED_API virtual void on_cc_info(
            const CongestionControlInfoStatus& status)
    {
        static_cast<void>(status);
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLLISTENER_HPP
