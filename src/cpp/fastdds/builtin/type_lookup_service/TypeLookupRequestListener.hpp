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
 * @file TypeLookupRequestListener.hpp
 *
 */

#ifndef _FASTDDS_TYPELOOKUP_SERVICE_REQUEST_LISTENER_HPP_
#define _FASTDDS_TYPELOOKUP_SERVICE_REQUEST_LISTENER_HPP_

#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include <fastrtps/rtps/reader/ReaderListener.h>

#include <fastdds/builtin/type_lookup_service/detail/TypeLookupTypes.hpp>
#include <fastdds/xtypes/type_representation/TypeIdentifierWithSizeHashSpecialization.h>

namespace std {

template <>
struct hash<eprosima::fastdds::dds::xtypes::TypeIdentifierSeq>
{
    std::size_t operator ()(
            const eprosima::fastdds::dds::xtypes::TypeIdentifierSeq& k) const
    {
        std::size_t hash_value = 0;
        for (const auto& id : k)
        {
            hash_value ^= (static_cast<size_t>(id.equivalence_hash()[0]) << 16) |
                    (static_cast<size_t>(id.equivalence_hash()[1]) << 8) |
                    (static_cast<size_t>(id.equivalence_hash()[2]));
        }
        return hash_value;
    }

};

} // std

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
 * Class TypeLookupRequestListener that receives the typelookup request messages of remote endpoints.
 * @ingroup TYPES_MODULE
 */
class TypeLookupRequestListener : public fastrtps::rtps::ReaderListener
{
public:

    /**
     * @brief Constructor.
     * @param manager Pointer to the TypeLookupManager.
     */
    TypeLookupRequestListener(
            TypeLookupManager* manager);

    /**
     * @brief Destructor.
     */
    virtual ~TypeLookupRequestListener() override;

    /**
     * @brief Gets TypeObject from TypeObjectRegistry, creates and sends reply.
     * @param request_id[in] The SampleIdentity of the request.
     * @param request[in] The request data.
     */
    void check_get_types_request(
            SampleIdentity request_id,
            const TypeLookup_getTypes_In& request);

    /**
     * @brief Gets type dependencies from TypeObjectRegistry, creates and sends reply.
     * @param request_id[in] The SampleIdentity of the request.
     * @param request[in] The request data.
     */
    void check_get_type_dependencies_request(
            SampleIdentity request_id,
            const TypeLookup_getTypeDependencies_In& request);

    /**
     * @brief Creates a TypeLookup_getTypeDependencies_Out using continuation points to manage size.
     * @param id_seq[in] Sequence of TypeIdentifiers for which dependencies are needed.
     * @param type_dependencies[in] The full list of dependencies of the type.
     * @param continuation_point[in] The continuation point of the previous request.
     * @return The reply containing the dependent types.
     */
    TypeLookup_getTypeDependencies_Out prepare_get_type_dependencies_response(
            const xtypes::TypeIdentifierSeq& id_seq,
            const std::unordered_set<xtypes::TypeIdentfierWithSize>& type_dependencies,
            const std::vector<uint8_t>& continuation_point);

    /**
     * @brief Method call when this class is notified of a new cache change.
     * @param reader The reader receiving the cache change.
     * @param change The cache change.
     */
    void onNewCacheChangeAdded(
            fastrtps::rtps::RTPSReader* reader,
            const fastrtps::rtps::CacheChange_t* const change) override;

private:

    //! A pointer to the typelookup manager.
    TypeLookupManager* typelookup_manager_;

    //! Mutex to protect access to requests_with_continuation_.
    std::mutex requests_with_continuation_mutex_;

    //! Collection of the requests that needed continuation points.
    std::unordered_map <xtypes::TypeIdentifierSeq,
            std::unordered_set<xtypes::TypeIdentfierWithSize>> requests_with_continuation_;
};

} /* namespace builtin */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif /* _FASTDDS_TYPELOOKUP_SERVICE_REQUEST_LISTENER_HPP_*/
