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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/utils/QosConverters.hpp>

#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <fastdds/log/LogResources.hpp>
#include <rtps/history/TopicPayloadPoolRegistry.hpp>
#include <rtps/RTPSDomainImpl.hpp>
#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>
#include <utils/shared_memory/SharedMemWatchdog.hpp>
#include <utils/SystemInfo.hpp>
#include <xmlparser/XMLEndpointParser.h>
#include <xmlparser/XMLProfileManager.h>

using namespace eprosima::fastdds::xmlparser;

using eprosima::fastdds::rtps::RTPSDomain;
using eprosima::fastdds::rtps::RTPSParticipant;

namespace eprosima {
namespace fastdds {
namespace dds {

DomainParticipantFactory::DomainParticipantFactory()
    : default_xml_profiles_loaded(false)
    , default_domain_id_(0)
    , default_participant_qos_(PARTICIPANT_QOS_DEFAULT)
    , topic_pool_(rtps::TopicPayloadPoolRegistry::instance())
    , rtps_domain_(rtps::RTPSDomainImpl::get_instance())
    , log_resources_(detail::get_log_resources())
{
}

DomainParticipantFactory::~DomainParticipantFactory()
{
    {
        std::lock_guard<std::mutex> guard(mtx_participants_);
        for (auto it : participants_)
        {
            for (auto pit : it.second)
            {
                pit->disable();
                delete pit;
            }
        }
        participants_.clear();
    }

    // Deletes DynamicTypes and TypeObject factories
    dds::DynamicDataFactory::delete_instance();
    dds::DynamicTypeBuilderFactory::delete_instance();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Log::KillThread();
}

DomainParticipantFactory* DomainParticipantFactory::get_instance()
{
    return get_shared_instance().get();
}

std::shared_ptr<DomainParticipantFactory> DomainParticipantFactory::get_shared_instance()
{
    // Note we need a custom deleter, since the destructor is protected.
    static std::shared_ptr<DomainParticipantFactory> instance(
        new DomainParticipantFactory(),
        [](DomainParticipantFactory* p)
        {
            delete p;
        });
    return instance;
}

ReturnCode_t DomainParticipantFactory::delete_participant(
        DomainParticipant* part)
{
    using PartVectorIt = std::vector<DomainParticipantImpl*>::iterator;
    using VectorIt = std::map<DomainId_t, std::vector<DomainParticipantImpl*>>::iterator;

    if (part != nullptr)
    {
        std::lock_guard<std::mutex> guard(mtx_participants_);
#ifdef FASTDDS_STATISTICS
        // Delete builtin statistics entities
        statistics::dds::DomainParticipantImpl* stat_part_impl =
                static_cast<statistics::dds::DomainParticipantImpl*>(part->impl_);
        stat_part_impl->delete_statistics_builtin_entities();
#endif // ifdef FASTDDS_STATISTICS
        if (part->has_active_entities())
        {
            return RETCODE_PRECONDITION_NOT_MET;
        }

        VectorIt vit = participants_.find(part->get_domain_id());

        if (vit != participants_.end())
        {
            for (PartVectorIt pit = vit->second.begin(); pit != vit->second.end();)
            {
                if ((*pit)->get_participant() == part
                        || (*pit)->get_participant()->guid() == part->guid())
                {
                    (*pit)->disable();
                    delete (*pit);
                    PartVectorIt next_it = vit->second.erase(pit);
                    pit = next_it;
                    break;
                }
                else
                {
                    ++pit;
                }
            }
            if (vit->second.empty())
            {
                participants_.erase(vit);
            }
            return RETCODE_OK;
        }
    }
    return RETCODE_ERROR;
}

DomainParticipant* DomainParticipantFactory::create_participant(
        DomainId_t did,
        const DomainParticipantQos& qos,
        DomainParticipantListener* listener,
        const StatusMask& mask)
{
    load_profiles();

    const DomainParticipantQos& pqos = (&qos == &PARTICIPANT_QOS_DEFAULT) ? default_participant_qos_ : qos;

    DomainParticipant* dom_part = new DomainParticipant(mask);
#ifndef FASTDDS_STATISTICS
    DomainParticipantImpl* dom_part_impl = new DomainParticipantImpl(dom_part, did, pqos, listener);
#else
    statistics::dds::DomainParticipantImpl* dom_part_impl =
            new statistics::dds::DomainParticipantImpl(dom_part, did, pqos, listener);
#endif // FASTDDS_STATISTICS

    if (fastdds::rtps::GUID_t::unknown() != dom_part_impl->guid())
    {
        {
            std::lock_guard<std::mutex> guard(mtx_participants_);
            using VectorIt = std::map<DomainId_t, std::vector<DomainParticipantImpl*>>::iterator;
            VectorIt vector_it = participants_.find(did);

            if (vector_it == participants_.end())
            {
                // Insert the vector
                std::vector<DomainParticipantImpl*> new_vector;
                auto pair_it = participants_.insert(std::make_pair(did, std::move(new_vector)));
                vector_it = pair_it.first;
            }

            vector_it->second.push_back(dom_part_impl);
        }

        if (factory_qos_.entity_factory().autoenable_created_entities)
        {
            if (RETCODE_OK != dom_part->enable())
            {
                delete_participant(dom_part);
                return nullptr;
            }
        }
    }
    else
    {
        delete dom_part_impl;
        return nullptr;
    }

    return dom_part;
}

DomainParticipant* DomainParticipantFactory::create_participant(
        const DomainParticipantExtendedQos& extended_qos,
        DomainParticipantListener* listener,
        const StatusMask& mask)
{
    return create_participant(extended_qos.domainId(), extended_qos, listener, mask);
}

DomainParticipant* DomainParticipantFactory::create_participant_with_default_profile()
{
    return create_participant_with_default_profile(nullptr, StatusMask::none());
}

DomainParticipant* DomainParticipantFactory::create_participant_with_default_profile(
        DomainParticipantListener* listener,
        const StatusMask& mask)
{
    load_profiles();
    return create_participant(default_domain_id_, default_participant_qos_, listener, mask);
}

DomainParticipant* DomainParticipantFactory::create_participant_with_profile(
        DomainId_t did,
        const std::string& profile_name,
        DomainParticipantListener* listener,
        const StatusMask& mask)
{
    load_profiles();

    // TODO (Miguel C): Change when we have full XML support for DDS QoS profiles
    ParticipantAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillParticipantAttributes(profile_name, attr))
    {
        DomainParticipantQos qos = default_participant_qos_;
        utils::set_qos_from_attributes(qos, attr.rtps);
        return create_participant(did, qos, listener, mask);
    }

    return nullptr;
}

DomainParticipant* DomainParticipantFactory::create_participant_with_profile(
        const std::string& profile_name,
        DomainParticipantListener* listener,
        const StatusMask& mask)
{
    load_profiles();

    // TODO (Miguel C): Change when we have full XML support for DDS QoS profiles
    ParticipantAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillParticipantAttributes(profile_name, attr))
    {
        DomainParticipantQos qos = default_participant_qos_;
        utils::set_qos_from_attributes(qos, attr.rtps);
        return create_participant(attr.domainId, qos, listener, mask);
    }

    return nullptr;
}

DomainParticipant* DomainParticipantFactory::lookup_participant(
        DomainId_t domain_id) const
{
    std::lock_guard<std::mutex> guard(mtx_participants_);

    auto it = participants_.find(domain_id);
    if (it != participants_.end() && it->second.size() > 0)
    {
        return it->second.front()->get_participant();
    }

    return nullptr;
}

std::vector<DomainParticipant*> DomainParticipantFactory::lookup_participants(
        DomainId_t domain_id) const
{
    std::lock_guard<std::mutex> guard(mtx_participants_);

    std::vector<DomainParticipant*> result;
    auto it = participants_.find(domain_id);
    if (it != participants_.end())
    {
        const std::vector<DomainParticipantImpl*>& v = it->second;
        for (auto pit = v.begin(); pit != v.end(); ++pit)
        {
            result.push_back((*pit)->get_participant());
        }
    }

    return result;
}

ReturnCode_t DomainParticipantFactory::get_default_participant_qos(
        DomainParticipantQos& qos) const
{
    qos = default_participant_qos_;
    return RETCODE_OK;
}

const DomainParticipantQos& DomainParticipantFactory::get_default_participant_qos() const
{
    return default_participant_qos_;
}

ReturnCode_t DomainParticipantFactory::set_default_participant_qos(
        const DomainParticipantQos& qos)
{
    if (&qos == &PARTICIPANT_QOS_DEFAULT)
    {
        reset_default_participant_qos();
        return RETCODE_OK;
    }

    ReturnCode_t ret_val = DomainParticipantImpl::check_qos(qos);
    if (RETCODE_OK != ret_val)
    {
        return ret_val;
    }
    DomainParticipantImpl::set_qos(default_participant_qos_, qos, true);
    return RETCODE_OK;
}

ReturnCode_t DomainParticipantFactory::get_participant_qos_from_profile(
        const std::string& profile_name,
        DomainParticipantQos& qos) const
{
    ParticipantAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillParticipantAttributes(profile_name, attr, false))
    {
        qos = default_participant_qos_;
        utils::set_qos_from_attributes(qos, attr.rtps);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantFactory::get_participant_qos_from_xml(
        const std::string& xml,
        DomainParticipantQos& qos) const
{
    ParticipantAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_participant_attributes_from_xml(xml, attr, false))
    {
        qos = default_participant_qos_;
        utils::set_qos_from_attributes(qos, attr.rtps);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantFactory::get_participant_qos_from_xml(
        const std::string& xml,
        DomainParticipantQos& qos,
        const std::string& profile_name) const
{
    if (profile_name.empty())
    {
        EPROSIMA_LOG_ERROR(DDS_DOMAIN, "Provided profile name must be non-empty");
        return RETCODE_BAD_PARAMETER;
    }

    ParticipantAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_participant_attributes_from_xml(xml, attr, true, profile_name))
    {
        qos = default_participant_qos_;
        utils::set_qos_from_attributes(qos, attr.rtps);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantFactory::get_default_participant_qos_from_xml(
        const std::string& xml,
        DomainParticipantQos& qos) const
{
    ParticipantAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_default_participant_attributes_from_xml(xml, attr, true))
    {
        qos = default_participant_qos_;
        utils::set_qos_from_attributes(qos, attr.rtps);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantFactory::get_participant_extended_qos_from_profile(
        const std::string& profile_name,
        DomainParticipantExtendedQos& extended_qos) const
{
    ParticipantAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillParticipantAttributes(profile_name, attr, false))
    {
        extended_qos = default_participant_qos_;
        utils::set_extended_qos_from_attributes(extended_qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantFactory::get_participant_extended_qos_from_xml(
        const std::string& xml,
        DomainParticipantExtendedQos& extended_qos) const
{
    ParticipantAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_participant_attributes_from_xml(xml, attr, false))
    {
        extended_qos = default_participant_qos_;
        utils::set_extended_qos_from_attributes(extended_qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantFactory::get_participant_extended_qos_from_xml(
        const std::string& xml,
        DomainParticipantExtendedQos& extended_qos,
        const std::string& profile_name) const
{
    if (profile_name.empty())
    {
        EPROSIMA_LOG_ERROR(DDS_DOMAIN, "Provided profile name must be non-empty");
        return RETCODE_BAD_PARAMETER;
    }

    ParticipantAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_participant_attributes_from_xml(xml, attr, true, profile_name))
    {
        extended_qos = default_participant_qos_;
        utils::set_extended_qos_from_attributes(extended_qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantFactory::get_default_participant_extended_qos_from_xml(
        const std::string& xml,
        DomainParticipantExtendedQos& extended_qos) const
{
    ParticipantAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_default_participant_attributes_from_xml(xml, attr, true))
    {
        extended_qos = default_participant_qos_;
        utils::set_extended_qos_from_attributes(extended_qos, attr);
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t DomainParticipantFactory::get_participant_extended_qos_from_default_profile(
        DomainParticipantExtendedQos& extended_qos) const
{
    ParticipantAttributes attr;
    XMLProfileManager::getDefaultParticipantAttributes(attr);
    utils::set_extended_qos_from_attributes(extended_qos, attr);
    return RETCODE_OK;
}

ReturnCode_t DomainParticipantFactory::load_profiles()
{
    // NOTE: This could be done with a bool atomic to avoid taking the mutex in most cases, however the use of
    // atomic over mutex is not deterministically better, and this way is easier to read and understand.

    // Only load profiles once, if not, wait for profiles to be loaded
    std::lock_guard<std::mutex> _(default_xml_profiles_loaded_mtx_);
    if (!default_xml_profiles_loaded)
    {
        SystemInfo::set_environment_file();
        XMLProfileManager::loadDefaultXMLFile();

        // Change as already loaded
        default_xml_profiles_loaded = true;

        // Only change factory qos when not explicitly set by the user
        if (factory_qos_ == PARTICIPANT_FACTORY_QOS_DEFAULT)
        {
            XMLProfileManager::getDefaultDomainParticipantFactoryQos(factory_qos_);
        }

        // Only change default participant qos when not explicitly set by the user
        if (default_participant_qos_ == PARTICIPANT_QOS_DEFAULT)
        {
            reset_default_participant_qos();
        }
        // Take the default domain id from the default participant profile
        ParticipantAttributes attr;
        XMLProfileManager::getDefaultParticipantAttributes(attr);
        default_domain_id_ = attr.domainId;

        RTPSDomain::set_filewatch_thread_config(factory_qos_.file_watch_threads(), factory_qos_.file_watch_threads());
    }

    return RETCODE_OK;
}

ReturnCode_t DomainParticipantFactory::load_XML_profiles_file(
        const std::string& xml_profile_file)
{
    if (XMLP_ret::XML_ERROR == XMLProfileManager::loadXMLFile(xml_profile_file))
    {
        EPROSIMA_LOG_ERROR(DDS_DOMAIN, "Problem loading XML file '" << xml_profile_file << "'");
        return RETCODE_ERROR;
    }
    return RETCODE_OK;
}

ReturnCode_t DomainParticipantFactory::load_XML_profiles_string(
        const char* data,
        size_t length)
{
    if (XMLP_ret::XML_ERROR == XMLProfileManager::loadXMLString(data, length))
    {
        EPROSIMA_LOG_ERROR(DDS_DOMAIN, "Problem loading XML string");
        return RETCODE_ERROR;
    }
    return RETCODE_OK;
}

ReturnCode_t DomainParticipantFactory::check_xml_static_discovery(
        std::string& xml_file)
{
    xmlparser::XMLEndpointParser parser;
    if (XMLP_ret::XML_OK != parser.loadXMLFile(xml_file))
    {
        EPROSIMA_LOG_ERROR(DDS_DOMAIN, "Error parsing xml file");
        return RETCODE_ERROR;
    }
    return RETCODE_OK;
}

ReturnCode_t DomainParticipantFactory::get_qos(
        DomainParticipantFactoryQos& qos) const
{
    qos = factory_qos_;
    return RETCODE_OK;
}

ReturnCode_t DomainParticipantFactory::set_qos(
        const DomainParticipantFactoryQos& qos)
{
    ReturnCode_t ret_val = check_qos(qos);
    if (RETCODE_OK != ret_val)
    {
        return ret_val;
    }
    if (!can_qos_be_updated(factory_qos_, qos))
    {
        return RETCODE_IMMUTABLE_POLICY;
    }
    set_qos(factory_qos_, qos, false);
    return RETCODE_OK;
}

xtypes::ITypeObjectRegistry& DomainParticipantFactory::type_object_registry()
{
    return rtps_domain_->type_object_registry();
}

void DomainParticipantFactory::reset_default_participant_qos()
{
    // TODO (Miguel C): Change when we have full XML support for DDS QoS profiles
    DomainParticipantImpl::set_qos(default_participant_qos_, PARTICIPANT_QOS_DEFAULT, true);
    if (true == default_xml_profiles_loaded)
    {
        ParticipantAttributes attr;
        XMLProfileManager::getDefaultParticipantAttributes(attr);
        utils::set_qos_from_attributes(default_participant_qos_, attr.rtps);
    }
}

void DomainParticipantFactory::set_qos(
        DomainParticipantFactoryQos& to,
        const DomainParticipantFactoryQos& from,
        bool first_time)
{
    (void) first_time;
    //As all the Qos can always be updated and none of them need to be sent
    to = from;

    rtps::SharedMemWatchdog::set_thread_settings(to.shm_watchdog_thread());
}

ReturnCode_t DomainParticipantFactory::check_qos(
        const DomainParticipantFactoryQos& qos)
{
    (void) qos;
    //There is no restriction by the moment with the contained Qos
    return RETCODE_OK;
}

bool DomainParticipantFactory::can_qos_be_updated(
        const DomainParticipantFactoryQos& to,
        const DomainParticipantFactoryQos& from)
{
    (void) to;
    (void) from;
    //All the DomainParticipantFactoryQos can be updated
    return true;
}

void DomainParticipantFactory::participant_has_been_deleted(
        DomainParticipantImpl* part)
{
    std::lock_guard<std::mutex> guard(mtx_participants_);
    auto it = participants_.find(part->get_domain_id());
    if (it != participants_.end())
    {
        for (auto pit = it->second.begin(); pit != it->second.end();)
        {
            if ((*pit) == part || (*pit)->guid() == part->guid())
            {
                pit = it->second.erase(pit);
            }
            else
            {
                ++pit;
            }
        }
        if (it->second.empty())
        {
            participants_.erase(it);
        }
    }
}

ReturnCode_t DomainParticipantFactory::get_library_settings(
        LibrarySettings& library_settings) const
{
    rtps_domain_->get_library_settings(library_settings);
    return RETCODE_OK;
}

ReturnCode_t DomainParticipantFactory::set_library_settings(
        const LibrarySettings& library_settings)
{
    if (rtps_domain_->set_library_settings(library_settings))
    {
        return RETCODE_OK;
    }
    return RETCODE_PRECONDITION_NOT_MET;
}

ReturnCode_t DomainParticipantFactory::get_dynamic_type_builder_from_xml_by_name(
        const std::string& type_name,
        DynamicTypeBuilder::_ref_type& type_builder)
{
    if (type_name.empty())
    {
        return RETCODE_BAD_PARAMETER;
    }
    if (XMLP_ret::XML_OK != XMLProfileManager::getDynamicTypeBuilderByName(type_builder, type_name))
    {
        return RETCODE_NO_DATA;
    }
    return RETCODE_OK;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
