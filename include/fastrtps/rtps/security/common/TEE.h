#ifndef TEE_INIT_H_
#define TEE_INIT_H_

#include <dsec_ca.h>
#include <fastrtps/log/Log.h>

class TEE_Init
{
    public:

        TEE_Init()
        {
            instance = dsec_ca_instance_create(&session, &context);
            int32_t error_code = dsec_ca_instance_open(&instance);
            if (error_code != 0) {
                instanceOpenFailLog(error_code);
                abort();
            }
        }

        ~TEE_Init()
        {
            int32_t error_code = dsec_ca_instance_close(&instance);
            if (error_code != 0) {
                instanceCloseFailLog(error_code);
            }
        }

        struct dsec_instance instance;

    private:
        TEEC_Session session;
        TEEC_Context context;

        void instanceOpenFailLog(int32_t error_code);
        void instanceCloseFailLog(int32_t error_code);
};

extern TEE_Init tee;

#endif /* TEE_INIT_H_ */
