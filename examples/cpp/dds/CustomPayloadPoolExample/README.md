# Objetive

This example has been developed to allow setting the endpoints' payload pool from the DDS layer.
In that way, a custom payload pool is mandatory to be implemented, and passed to the endpoints in the publisher and subscriber implementations.

# Launch

To launch this test open two different consoles:

In the first one launch: `./CustomPayloadPoolExample publisher` (or `CustomPayloadPoolExample.exe publisher` on Windows).
In the second one: `./CustomPayloadPoolExample subscriber` (or `CustomPayloadPoolExample.exe subscriber` on Windows).

The endpoints will match and communicate with the custom payload pool.
