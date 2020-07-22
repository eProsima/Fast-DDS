#include <openssl/evp.h>
#include <openssl/engine.h>
#include <openssl/rand.h>
#include <openssl/err.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class OpenSSLInit
{
public:

    OpenSSLInit()
    {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        OpenSSL_add_all_algorithms();
#endif
    }

    ~OpenSSLInit()
    {
#if OPENSSL_VERSION_NUMBER < 0x10000000L
        ERR_remove_state(0);
#elif OPENSSL_VERSION_NUMBER < 0x10100000L
        ERR_remove_thread_state(NULL);
#endif
        ENGINE_cleanup();
        RAND_cleanup();
        CRYPTO_cleanup_all_ex_data();
        ERR_free_strings();
        EVP_cleanup();
    }

};

} // namespace security
} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
