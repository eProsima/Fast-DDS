// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataWriterListener.hpp
 */

#ifndef FASTDDS_DDS_PUBLISHER__DATAWRITERLISTENER_HPP
#define FASTDDS_DDS_PUBLISHER__DATAWRITERLISTENER_HPP

#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DataWriter;

/**
 * Class DataWriterListener, allows the end user to implement callbacks triggered by certain events.
 * @ingroup FASTDDS_MODULE
 */
class FASTDDS_EXPORTED_API DataWriterListener
{
public:

    /**
     * @brief Constructor
     */
    DataWriterListener()
    {
    }

    /**
     * @brief Destructor
     */
    virtual ~DataWriterListener()
    {
    }

    /**
     * This method is called when the DataWriter is matched (or unmatched) against an endpoint.
     *
     * @param writer Pointer to the associated DataWriter
     * @param info Information regarding the matched DataReader
     */
    virtual void on_publication_matched(
            DataWriter* writer,
            const PublicationMatchedStatus& info)
    {
        static_cast<void>(writer);
        static_cast<void>(info);
    }

    /**
     * A method called when a deadline is missed
     *
     * @param writer Pointer to the associated DataWriter
     * @param status The deadline missed status
     */
    virtual void on_offered_deadline_missed(
            DataWriter* writer,
            const OfferedDeadlineMissedStatus& status)
    {
        static_cast<void>(writer);
        static_cast<void>(status);
    }

    /**
     * A method called when an incompatible QoS is offered
     *
     * @param writer Pointer to the associated DataWriter
     * @param status The deadline missed status
     */
    virtual void on_offered_incompatible_qos(
            DataWriter* writer,
            const OfferedIncompatibleQosStatus& status)
    {
        static_cast<void>(writer);
        static_cast<void>(status);
    }

    /**
     * @brief Method called when the liveliness of a DataWriter is lost
     *
     * @param writer Pointer to the associated DataWriter
     * @param status The liveliness lost status
     */
    virtual void on_liveliness_lost(
            DataWriter* writer,
            const LivelinessLostStatus& status)
    {
        static_cast<void>(writer);
        static_cast<void>(status);
    }

    /**
     * @brief Method called when a sample has been removed unacknowledged
     *
     * @param writer Pointer to the associated DataWriter
     * @param instance Handle to the instance the sample was removed from
     */
    virtual void on_unacknowledged_sample_removed(
            DataWriter* writer,
            const InstanceHandle_t& instance)
    {
        static_cast<void>(writer);
        static_cast<void>(instance);
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_PUBLISHER__DATAWRITERLISTENER_HPP
