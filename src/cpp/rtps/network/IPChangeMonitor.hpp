/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef SRC_CPP_RTPS_NETWORK__IPCHANGEMONITOR_HPP
#define SRC_CPP_RTPS_NETWORK__IPCHANGEMONITOR_HPP

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * @brief Interface class for monitoring IP address changes.
 */
class IPChangeMonitorAPI
{
public:

    virtual ~IPChangeMonitorAPI() = default;

    /**
     * @brief Start monitoring for IP address changes.
     *
     * This function should be implemented by derived classes to initiate the monitoring process.
     * Calling this function multiple times is safe; subsequent calls will have no effect if monitoring is already active.
     */
    virtual void start_monitoring() = 0;

    /**
     * @brief Stop monitoring for IP address changes.
     *
     * This function should be implemented by derived classes to terminate the monitoring process.
     * Calling this function multiple times is safe; subsequent calls will have no effect if monitoring is already inactive.
     */
    virtual void stop_monitoring() = 0;
};

/**
 * @brief Base class for IP change monitoring implementations.
 *
 * This class provides a different implementation of the IPChangeMonitorAPI interface on different platforms.
 */
class IPChangeMonitorImpl : public IPChangeMonitorAPI
{
public:

    ~IPChangeMonitorImpl() override = default;

    void start_monitoring() override;

    void stop_monitoring() override;

protected:

    IPChangeMonitorImpl();

    /**
     * @brief Callback function invoked when an IP address change is detected.
     */
    virtual void ip_change_detected() = 0;

private:

    // Forward declaration of the implementation data
    struct IPChangeMonitorImplData;
    //! Pointer to the implementation data
    std::unique_ptr<IPChangeMonitorImplData> impl_data_;
};

/**
 * @brief Template class for monitoring IP address changes with a user-defined callback.
 */
template<typename Func>
class IPChangeMonitor : public IPChangeMonitorImpl
{
public:

    IPChangeMonitor(
            Func&& on_ip_change_callback)
        : IPChangeMonitorImpl()
        , on_ip_change_callback_(std::move(on_ip_change_callback))
    {
    }

    ~IPChangeMonitor() override
    {
        stop_monitoring();
    }

protected:

    void ip_change_detected() override
    {
        on_ip_change_callback_();
    }

private:

    Func on_ip_change_callback_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // SRC_CPP_RTPS_NETWORK__IPCHANGEMONITOR_HPP
