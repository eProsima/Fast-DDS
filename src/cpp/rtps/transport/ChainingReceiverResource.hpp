#ifndef TRANSPORT_CHAININGRECEIVERRESOURCE_HPP
#define TRANSPORT_CHAININGRECEIVERRESOURCE_HPP

#include <fastdds/rtps/transport/TransportReceiverInterface.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class ChainingTransport;

class ChainingReceiverResource : public TransportReceiverInterface
{
public:

    ChainingReceiverResource(
            ChainingTransport& transport,
            TransportReceiverInterface* low_receiver_resource)
        : transport_(transport)
        , low_receiver_resource_(low_receiver_resource)
    {
    }

    virtual ~ChainingReceiverResource() = default;

    /**
     * Method to be called by the transport when receiving data.
     * @param data Pointer to the received data.
     * @param size Number of bytes received.
     * @param localLocator Locator identifying the local endpoint.
     * @param remote_locator Locator identifying the remote endpoint.
     */
    void OnDataReceived(
            const fastrtps::rtps::octet* data,
            const uint32_t size,
            const fastrtps::rtps::Locator_t& local_locator,
            const fastrtps::rtps::Locator_t& remote_locator) override
    {
        transport_.receive(low_receiver_resource_, data, size, local_locator, remote_locator);
    }

private:

    ChainingTransport& transport_;
    TransportReceiverInterface* low_receiver_resource_ = nullptr;
};


}
}
}

#endif // TRANSPORT_CHAININGRECEIVERRESOURCE_HPP
