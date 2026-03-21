// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file WLPListener.h
 *
 */

#ifndef _FASTDDS_RTPS_WLPLISTENER_H_
#define _FASTDDS_RTPS_WLPLISTENER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class WLP;
class RTPSReader;
struct CacheChange_t;

/**
 * Class WLPListener that receives the liveliness messages asserting the liveliness of remote endpoints.
 * @ingroup LIVELINESS_MODULE
 */
class WLPListener : public ReaderListener
{
public:

    /**
     * @brief Constructor
     * @param pwlp Pointer to the writer liveliness protocol
     */
    WLPListener(
            WLP* pwlp);

    /**
     * @brief Destructor
     */
    virtual ~WLPListener();

    /**
     * @brief Method call when this class is notified of a new cache change
     * @param reader The reader receiving the cache change
     * @param change The cache change
     */
    void on_new_cache_change_added(
            RTPSReader* reader,
            const CacheChange_t* const change) override;

private:

    /**
     * Separate the Key between the GuidPrefix_t and the liveliness Kind
     * @param key InstanceHandle_t to separate.
     * @param guidP GuidPrefix_t pointer to store the info.
     * @param liveliness Liveliness Kind Pointer.
     * @return True if correctly separated.
     */
    bool separateKey(
            InstanceHandle_t& key,
            GuidPrefix_t* guidP,
            dds::LivelinessQosPolicyKind* liveliness);

    /**
     * Compute the key from a CacheChange_t
     * @param change
     */
    bool computeKey(
            CacheChange_t* change);

    /**
     * @brief Check that the ParticipantMessageData kind is a valid one for WLP and extract the liveliness kind.
     *
     * @param [in] serialized_kind A pointer to the first octet of the kind array. The function assumes 4 elements
     *        in the array.
     * @param [out] liveliness_kind A reference to the LivelinessQosPolicyKind.
     *
     * @return True if the kind corresponds with one for WLP, false otherwise.
     */
    bool get_wlp_kind(
            const octet* serialized_kind,
            dds::LivelinessQosPolicyKind& liveliness_kind);

    //! A pointer to the writer liveliness protocol
    WLP* mp_WLP;

};

} /* namespace rtps */
} /* namespace eprosima */
} // namespace eprosima
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_WLPLISTENER_H_ */
