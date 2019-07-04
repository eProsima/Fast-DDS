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
 * @file DomainParticipantFactory.cpp
 *
 */

#include <fastdds/domain/DomainParticipantFactory.hpp>
#include <fastrtps/rtps/RTPSDomain.h>

#include <fastdds/domain/DomainParticipant.hpp>
#include "DomainParticipantImpl.hpp"

#include <fastrtps/utils/eClock.h>
#include <fastrtps/log/Log.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/TypeObjectFactory.h>

using namespace eprosima::fastrtps::xmlparser;

using eprosima::fastrtps::ParticipantAttributes;
using eprosima::fastrtps::Log;

using eprosima::fastrtps::rtps::RTPSDomain;
using eprosima::fastrtps::rtps::RTPSParticipant;

namespace eprosima {
namespace fastdds {

class DomainParticipantFactoryReleaser
{
public:
    ~DomainParticipantFactoryReleaser()
    {
        DomainParticipantFactory::delete_instance();
    }
};

static DomainParticipantFactoryReleaser s_releaser;
static DomainParticipantFactory* g_instance = nullptr;

DomainParticipantFactory::DomainParticipantFactory()
    : default_xml_profiles_loaded(false)
{
}

DomainParticipantFactory::~DomainParticipantFactory()
{
    {
        std::lock_guard<std::mutex> guard(mtx_participants_);
        for (auto it : participants_)
        {
            delete it.second;
        }
        participants_.clear();
    }

    // Deletes DynamicTypes and TypeObject factories
    fastrtps::types::DynamicTypeBuilderFactory::delete_instance();
    fastrtps::types::DynamicDataFactory::delete_instance();
    fastrtps::types::TypeObjectFactory::delete_instance();
    XMLProfileManager::DeleteInstance();

    fastrtps::eClock::my_sleep(100);
    fastrtps::Log::KillThread();
}

DomainParticipantFactory* DomainParticipantFactory::get_instance()
{
    if (g_instance == nullptr)
    {
        g_instance = new DomainParticipantFactory();
    }
    return g_instance;
}

bool DomainParticipantFactory::delete_instance()
{
    if (g_instance != nullptr)
    {
        delete g_instance;
        g_instance = nullptr;
        return true;
    }
    return false;
}

ReturnCode_t DomainParticipantFactory::delete_participant(
        DomainParticipant* part)
{
    if(part->contains_entity(part->get_instance_handle(),true))
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }
    if (part != nullptr)
    {
        std::lock_guard<std::mutex> guard(mtx_participants_);

        auto it = participants_.find(part->get_domain_id());

        if (it != participants_.end())
        {
            delete it->second;
            participants_.erase(it);
            return ReturnCode_t::RETCODE_OK;
        }
    }
    return ReturnCode_t::RETCODE_ERROR;
}

DomainParticipant* DomainParticipantFactory::create_participant(
        const std::string& participant_profile,
        DomainParticipantListener* listen)
{
    if (false == default_xml_profiles_loaded)
    {
        XMLProfileManager::loadDefaultXMLFile();
        default_xml_profiles_loaded = true;
    }

    ParticipantAttributes participant_att;
    if ( XMLP_ret::XML_ERROR == XMLProfileManager::fillParticipantAttributes(participant_profile, participant_att))
    {
        logError(DOMAIN_PARTICIPANT_FACTORY, "Problem loading profile '" << participant_profile << "'");
        return nullptr;
    }

    return create_participant(participant_att, listen);
}

DomainParticipant* DomainParticipantFactory::create_participant(
        const ParticipantAttributes& att,
        DomainParticipantListener* listen)
{
    uint8_t domain_id = static_cast<uint8_t>(att.rtps.builtin.domainId);
    if (lookup_participant(domain_id) == nullptr)
    {
        DomainParticipant* pubsubpar = new DomainParticipant();
        DomainParticipantImpl* pspartimpl = new DomainParticipantImpl(att, pubsubpar, listen);
        RTPSParticipant* part = RTPSDomain::createParticipant(att.rtps, &pspartimpl->rtps_listener_);

        if (part == nullptr)
        {
            logError(DOMAIN_PARTICIPANT_FACTORY, "Problem creating RTPSParticipant");
            delete pspartimpl;
            return nullptr;
        }

        pspartimpl->rtps_participant_ = part;

        {
            std::lock_guard<std::mutex> guard(mtx_participants_);
            participants_[domain_id] = pspartimpl;
        }
        return pubsubpar;
    }

    logError(DOMAIN_PARTICIPANT_FACTORY, "A Participant already exists in domain: " << att.rtps.builtin.domainId);
    return nullptr;
}

DomainParticipant* DomainParticipantFactory::lookup_participant(
        uint8_t domain_id) const
{
    std::lock_guard<std::mutex> guard(mtx_participants_);

    auto it = participants_.find(domain_id);
    if (it != participants_.end())
    {
        return it->second->get_participant();
    }

    return nullptr;
}

ReturnCode_t DomainParticipantFactory::get_default_participant_qos(
        ParticipantAttributes& participant_attributes) const
{
    if (false == default_xml_profiles_loaded)
    {
        XMLProfileManager::loadDefaultXMLFile();
        default_xml_profiles_loaded = true;
    }

    XMLProfileManager::getDefaultParticipantAttributes(participant_attributes);
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DomainParticipantFactory::set_default_participant_qos(
        const fastrtps::ParticipantAttributes &participant_qos)
{
    // TODO XMLProfileManager::setDefault...
    (void)participant_qos;
    logError(DOMAIN_PARTICIPANT_FACTORY, "Not implemented.");
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

bool DomainParticipantFactory::load_XML_profiles_file(
        const std::string& xml_profile_file)
{
    if (false == default_xml_profiles_loaded)
    {
        XMLProfileManager::loadDefaultXMLFile();
        default_xml_profiles_loaded = true;
    }

    if ( XMLP_ret::XML_ERROR == XMLProfileManager::loadXMLFile(xml_profile_file))
    {
        logError(DOMAIN, "Problem loading XML file '" << xml_profile_file << "'");
        return false;
    }
    return true;
}

} /* namespace fastrtps */
} /* namespace eprosima */
