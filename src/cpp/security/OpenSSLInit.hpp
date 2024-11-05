// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <memory>

#include <openssl/evp.h>
#include <openssl/engine.h>
#include <openssl/rand.h>
#include <openssl/err.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class OpenSSLInit
{
public:

    OpenSSLInit()
    {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        OpenSSL_add_all_algorithms();
#endif // if OPENSSL_VERSION_NUMBER < 0x10100000L
    }

    ~OpenSSLInit()
    {
#if OPENSSL_VERSION_NUMBER < 0x10000000L
        ERR_remove_state(0);
        ENGINE_cleanup();
#elif OPENSSL_VERSION_NUMBER < 0x10100000L
        ERR_remove_thread_state(NULL);
        ENGINE_cleanup();
#endif // if OPENSSL_VERSION_NUMBER < 0x10000000L
    }

    static std::shared_ptr<OpenSSLInit> get_instance()
    {
        static auto instance = std::make_shared<OpenSSLInit>();
        return instance;
    }
};

} // namespace security
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
