# Request-Reply example

This example illustrates how to achieve a Client/Server communication between two applications using the *Fast DDS*
request-reply mechanism.
More information in [Fast DDS documentation](https://fast-dds.docs.eprosima.com/en/latest/fastdds/use_cases/request_reply/request_reply.html).

To launch this example open two different consoles:

1. In the first one launch: `./DDSCalculatorServer` (or `DDSCalculatorServer.exe` on Windows).
1. In the second one: `./DDSCalculatorClient 3 + 3` (or `DDSCalculatorClient.exe 3 + 3` on windows).
