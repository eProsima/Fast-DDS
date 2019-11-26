#ifndef OMG_DDS_CORE_DETAIL_QOS_PROVIDER_HPP_
#define OMG_DDS_CORE_DETAIL_QOS_PROVIDER_HPP_


/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dds/core/detail/TQosProviderImpl.hpp>
#include <org/opensplice/core/QosProviderDelegate.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
   namespace core {
      namespace detail {
         typedef dds::core::TQosProvider<org::opensplice::core::QosProviderDelegate> QosProvider;
      }
   }
}

/** @endcond */

#endif /* OMG_DDS_CORE_DETAIL_QOS_PROVIDER_HPP_ */
