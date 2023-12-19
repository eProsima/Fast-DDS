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

#ifndef TYPELOOKUP_REQUEST_LISTENER_HPP_
#define TYPELOOKUP_REQUEST_LISTENER_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
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
 * Class TypeLookupRequestListener that receives the typelookup request messages of remote endpoints.
 * @ingroup TYPES_MODULE
 */
class TypeLookupRequestListener : public fastrtps::rtps::ReaderListener
{
public:

    /**
     * @brief Constructor
     * @param manager Pointer to the TypeLookupManager
     */
    TypeLookupRequestListener(
            TypeLookupManager* manager);

    /**
     * @brief Destructor
     */
    virtual ~TypeLookupRequestListener() override;

    /**
     * @brief Gets TypeObject from TypeObjectRegistry, creates and sends reply
     * @param request_id[in] The SampleIdentity of the request
     * @param request[in] The request data
     */
    void check_get_types_request(
            SampleIdentity request_id,
            const TypeLookup_getTypes_In& request);

    /**
     * @brief Gets type dependencies from TypeObjectRegistry, creates and sends reply
     * @param request_id[in] The SampleIdentity of the request
     * @param request[in] The request data
     */
    void check_get_type_dependencies_request(
            SampleIdentity request_id,
            const TypeLookup_getTypeDependencies_In& request);

    /**
     * @brief Method call when this class is notified of a new cache change
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
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* TYPELOOKUP_REQUEST_LISTENER_HPP_*/
