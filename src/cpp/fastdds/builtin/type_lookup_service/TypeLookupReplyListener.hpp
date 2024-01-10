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
 * @file TypeLookupReplyListener.hpp
 *
 */

#ifndef _FASTDDS_TYPELOOKUP_SERVICE_REPLY_LISTENER_HPP_
#define _FASTDDS_TYPELOOKUP_SERVICE_REPLY_LISTENER_HPP_

#include <fastrtps/rtps/reader/ReaderListener.h>

#include <fastdds/builtin/type_lookup_service/detail/TypeLookupTypes.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSReader;
struct CacheChange_t;

} // namespace rtps
} // namespace fastrtp

namespace fastdds {
namespace dds {
namespace builtin {

class TypeLookupManager;

/**
 * Class TypeLookupReplyListener that receives the typelookup request messages of remote endpoints.
 * @ingroup TYPES_MODULE
 */
class TypeLookupReplyListener : public fastrtps::rtps::ReaderListener
{
public:

    /**
     * @brief Constructor
     * @param manager Pointer to the TypeLookupManager
     */
    TypeLookupReplyListener(
            TypeLookupManager* manager);

    /**
     * @brief Destructor
     */
    virtual ~TypeLookupReplyListener() override;

    /**
     * @brief Registers TypeIdentifier and TypeObject in TypeObjectRegistry.
     * Also notifies all callbacks for the type and removes the current SampleIdentity from the list.
     * @param request_id[in] The SampleIdentity of the request
     * @param reply[in] The reply data
     */
    void check_get_types_reply(
            const SampleIdentity& request_id,
            const TypeLookup_getTypes_Out& reply);

    /**
     * @brief Checks if all dependencies are solved.
     * If they are not, sends next request and adds it to the list.
     * If they are, sends get_types request and adds it to the list.
     * Also removes the current SampleIdentity from the list.
     * @param request_id[in] The SampleIdentity of the request
     * @param type_server[in] GuidPrefix corresponding to the remote participant which TypeInformation is being solved.
     * @param reply[in] The reply data
     */
    void check_get_type_dependencies_reply(
            const SampleIdentity& request_id,
            const fastrtps::rtps::GuidPrefix_t type_server,
            const TypeLookup_getTypeDependencies_Out& reply);

    /**
     * @brief Method called when this class is notified of a new cache change
     * @param reader The reader receiving the cache change
     * @param change The cache change
     */
    void onNewCacheChangeAdded(
            fastrtps::rtps::RTPSReader* reader,
            const fastrtps::rtps::CacheChange_t* const change) override;

private:

    //! A pointer to the typelookup manager
    TypeLookupManager* typelookup_manager_;
};

} /* namespace builtin */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif /* _FASTDDS_TYPELOOKUP_SERVICE_REPLY_LISTENER_HPP_*/
