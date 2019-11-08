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

namespace eprosima {
namespace fastrtps {
namespace rtps {

    class RTPSWriter;

/**
 * @brief Class RTPSDomainImpl, contains the private implementation of the RTPSDomain
 * @ingroup RTPS_MODULE
 */
class RTPSDomainImpl
{
public:

    /**
     * Check whether intraprocess delivery is enabled or not.
     *
     * @returns true when intraprocess delivery is enabled, false otherwise.
     */
    static bool is_intraprocess_enabled()
    {
        return false;
    }

    static RTPSWriter* find_local_writer(
        const GUID_t& /* writer_guid */ )
    {
        return nullptr;
    }
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
