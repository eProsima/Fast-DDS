To launch this test open two different consoles:

1. In the first one launch:

   ```c++
   ./LargeDataTCPExample publisher client 100 200000
   ```

When launching the publisher, you can configure the sending frequency (100 above) and the size in bytes of the message to send (200KB above).

1. In the second one launch:

   ```c++
   ./LargeDataTCPExample subscriber server
   ```
