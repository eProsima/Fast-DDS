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
 * @file DomainParticipantFactoryQos.hpp
 *
 */

#ifndef FASTDDS_DDS_DOMAIN_QOS__DOMAINPARTICIPANTFACTORYQOS_HPP
#define FASTDDS_DDS_DOMAIN_QOS__DOMAINPARTICIPANTFACTORYQOS_HPP

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class DomainParticipantFactoryQos, contains all the possible Qos that can be set for a determined participant.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTDDS_QOS_MODULE
 */
class DomainParticipantFactoryQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API DomainParticipantFactoryQos()
    {
    }

    /**
     * @brief Destructor
     */
    FASTDDS_EXPORTED_API virtual ~DomainParticipantFactoryQos()
    {
    }

    bool operator ==(
            const DomainParticipantFactoryQos& b) const
    {
        return (this->shm_watchdog_thread_ == b.shm_watchdog_thread()) &&
               (this->file_watch_threads_ == b.file_watch_threads()) &&
               (this->entity_factory_ == b.entity_factory());
    }

    /**
     * Getter for EntityFactoryQosPolicy
     * @return EntityFactoryQosPolicy reference
     */
    const EntityFactoryQosPolicy& entity_factory() const
    {
        return entity_factory_;
    }

    /**
     * Getter for EntityFactoryQosPolicy
     * @return EntityFactoryQosPolicy reference
     */
    EntityFactoryQosPolicy& entity_factory()
    {
        return entity_factory_;
    }

    /**
     * Setter for EntityFactoryQosPolicy
     * @param entity_factory EntityFactoryQosPolicy
     */
    void entity_factory(
            const EntityFactoryQosPolicy& entity_factory)
    {
        entity_factory_ = entity_factory;
    }

    /**
     * Getter for SHM watchdog ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    rtps::ThreadSettings& shm_watchdog_thread()
    {
        return shm_watchdog_thread_;
    }

    /**
     * Getter for SHM watchdog ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    const rtps::ThreadSettings& shm_watchdog_thread() const
    {
        return shm_watchdog_thread_;
    }

    /**
     * Setter for the SHM watchdog ThreadSettings
     *
     * @param value New ThreadSettings to be set
     */
    void shm_watchdog_thread(
            const rtps::ThreadSettings& value)
    {
        shm_watchdog_thread_ = value;
    }

    /**
     * Getter for file watch related ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    rtps::ThreadSettings& file_watch_threads()
    {
        return file_watch_threads_;
    }

    /**
     * Getter for file watch related ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    const rtps::ThreadSettings& file_watch_threads() const
    {
        return file_watch_threads_;
    }

    /**
     * Setter for the file watch related ThreadSettings
     *
     * @param value New ThreadSettings to be set
     */
    void file_watch_threads(
            const rtps::ThreadSettings& value)
    {
        file_watch_threads_ = value;
    }

private:

    //!EntityFactoryQosPolicy, implemented in the library.
    EntityFactoryQosPolicy entity_factory_;

    //! Thread settings for the SHM watchdog thread
    rtps::ThreadSettings shm_watchdog_thread_;

    //! Thread settings for the file watch related threads
    rtps::ThreadSettings file_watch_threads_;

};

FASTDDS_EXPORTED_API extern const DomainParticipantFactoryQos PARTICIPANT_FACTORY_QOS_DEFAULT;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_DOMAIN_QOS__DOMAINPARTICIPANTFACTORYQOS_HPP
