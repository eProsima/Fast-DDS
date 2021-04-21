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
 * @file StatisticsCommon.hpp
 */

#ifndef _FASTDDS_STATISTICS_RTPS_STATISTICSCOMMON_HPP_
#define _FASTDDS_STATISTICS_RTPS_STATISTICSCOMMON_HPP_

#include <memory>
#include <type_traits>

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/statistics/IListeners.hpp>
#include <fastrtps/utils/TimedMutex.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {

// Members are private details
struct StatisticsAncillary;

class StatisticsListenersImpl
{
    std::unique_ptr<StatisticsAncillary> members_;

protected:

    /*
     * Create a class A auxiliary structure
     * @return true if successfully created
     */
    template<class A>
    void init_statistics()
    {
        static_assert(
            std::is_base_of<StatisticsAncillary, A>::value,
            "Auxiliary structure must derive from StatisticsAncillary");

        if (!members_)
        {
            members_.reset(new A);
        }
    }

    /*
     * Returns the auxiliary members
     * @return The specialized auxiliary structure for each class
     */
    StatisticsAncillary* get_aux_members() const;

    /*
     * Add a listener to receive statistics backend callbacks
     * @param listener
     * @return true if successfully added
     */
    bool add_statistics_listener_impl(
            std::shared_ptr<fastdds::statistics::IListener> listener);

    /*
     * Remove a listener from receiving statistics backend callbacks
     * @param listener
     * @return true if successfully removed
     */
    bool remove_statistics_listener_impl(
            std::shared_ptr<fastdds::statistics::IListener> listener);

    /*
     * Lambda function to traverse the listener collection
     * @param f function object to apply to each listener
     * @return function object after being applied to each listener
     */
    template<class Function>
    Function for_each_listener(
            Function f);

    /*
     * Retrieve endpoint mutexes from derived class
     * @return defaults to the endpoint mutex
     */
    virtual fastrtps::RecursiveTimedMutex& get_statistics_mutex() = 0;

    /*
     * Retrieve the GUID_t from derived class
     * @return endpoint GUID_t
     */
    virtual const fastrtps::rtps::GUID_t& get_guid() = 0;
};

// Members are private details
struct StatisticsWriterAncillary;

class StatisticsWriterImpl
    : protected StatisticsListenersImpl
{

    /*
     * Create the auxiliary structure
     * @return true if successfully created
     */
    StatisticsWriterAncillary* get_members() const;

    /*
     * Retrieve endpoint mutexes from derived class
     * @return defaults to the endpoint mutex
     */
    fastrtps::RecursiveTimedMutex& get_statistics_mutex() final;

    /*
     * Retrieve the GUID_t from derived class
     * @return endpoint GUID_t
     */
    const fastrtps::rtps::GUID_t& get_guid() final;

protected:

    /*
     * Constructor. Mandatory member initialization.
     */
    StatisticsWriterImpl();

    // TODO: methods for listeners callbacks

    //! Report a DATA message is sent
    void on_data();

    //! Report a DATA_FRAG message is sent
    void on_data_frag();

};

// Members are private details
struct StatisticsReaderAncillary;

class StatisticsReaderImpl
    : protected StatisticsListenersImpl
{
    friend class RTPSMessageGroup;

    /*
     * Create the auxiliary structure
     * TODO: enable when a member is added to StatisticsReaderAncillary
     * @return true if successfully created
     */
    StatisticsReaderAncillary* get_members() const
    {
        return nullptr;
    }

    /*
     * Retrieve endpoint mutexes from derived class
     * @return defaults to the endpoint mutex
     */
    fastrtps::RecursiveTimedMutex& get_statistics_mutex() final;

    /*
     * Retrieve the GUID_t from derived class
     * @return endpoint GUID_t
     */
    const fastrtps::rtps::GUID_t& get_guid() final;

protected:

    /*
     * Constructor. Mandatory member initialization.
     */
    StatisticsReaderImpl();

    // TODO: methods for listeners callbacks

    //! Report a DATA message is sent
    void on_acknack(
            int32_t count);
};

} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_STATISTICS_RTPS_STATISTICSCOMMON_HPP_
