# Interprocess Shared Memory model

## Overview

This document describes a model for interprocess shared memory communication  of software components interchanging data messages. The objective of the model is to provide a shared memory based transport layer suitable for a Real-time Publish-Subscribe DDS (Data Distribution Service).

## Context

eProsima has received commercial proposals to implement a shared memory transport as an improvement of its FastRPTS product. The goals listed in this document are extracted from the customers proposals.

### Improvements over the standard network transports

* Reduce OS Kernel calls: This is unavoidable for transports like UDP / TCP (even with the loopback interface).
* Large message support: Network stacks needs to fragment large packets.
* Avoid data serialization / deserialization process: This is not possible in heterogeneous networks, but is possible in shared memory interprocess communication.
* Reduce memory copies: A buffer can be shared directly with several readers without additional copies.

### Objectives 

* Increase the communications performance of processes in the same machine.
* Create a portable shared memory library (Windows / Linux / MacOS).
* Documentation.
* Tests
* Examples / Tutorials.

## Architecture

### Design concepts

* **Segment**: Is a block of shared memory of fixed size that can be accessed from different processes. Shared memory segments have a global name so any process who knows the name can open the segment and map it in its address space.

* **SegmentId**: SegmentIds are names that uniquely identify shared memory segments, these names are 16 characters UUIDs.

* **Shared memory buffer**: Is a buffer allocated in the shared memory segment. 

* **Buffer descriptor**: Shared memory buffers can be referenced by buffers descriptors, these descriptors act like pointers to a buffer that can be copied between processes with a minimum cost. A descriptor contains the SegmentId and the offset to the data from the base of the segment.

* **Shared memory port**: Is a communication channel identified by a port_id(uint32_t number). Through this channel, buffer descriptors are sent to other processes. It has a ring-buffer, in shared memory, where descriptors are stored. The same port can by opened by several processes for reading and writting operations. Multiple listeners can be registered in a port to be notified when descriptors are pushed to the ring-buffer. Multiple data producers can push descriptors to a port. The port containts an atomic counter with the number of listeners registered on it, each position in the ring-buffer has also a counter initialized to the number of listeners, as listeners read the descriptor, decrement the counter, so the ring-buffer position will be considered free when the counter is zero. The port also has an interprocess condition variable where the listeners will wait for incoming descriptors.

* **Listener**: Listeners listen to descriptors pushed to a port. The Listener provides an interface for the application to wait and access to the data referenced by the descriptors. When a consumer pops a descriptor from the port listener, look at the descriptor's SegmentId field to open the origin shared memory segment (if not already opened in this process), once the origin segment is mapped locally, the consumer is able to access the data using the offset field contained in the descriptor.

* **SharedMemoryManager**: Applications instantiate this object in order to access the shared memory resources described above. At least one per process memory manager is required. The manager provides functionality for the application to create shared memory segments, alloc data buffers in the segments, push buffer descriptors to shared memory ports and create listeners associated to a port.

### Example scenario
![](interprocess_shared_mem1.png)

Let's study the above example. There are three processes with a SharedMemManager per process, every process creates its own shared memory segment intended to store the buffers that will be shared with other processes.

P1, P2 and P3 processes are participants in a RTPS-DDS environment. Discovery of participants is done using multicast so we have selected shared memory port0 as "multicast" port for discovery, therefore, first thing all participants do is to open shared memory port0. A listener attached to port0 is created too, by every participant, to read the incoming descriptors.

Each participant opens a port to receive unicast messages, ports 1, 2 and 3 respectively, and create listeners associated to those ports.

The first message the participants send is the multicast discovery message: "I'm here, and I am listening on portN". So they alloc a buffer in its local segment, write the message to that buffer and push the buffer's descriptor through the port0. Observe, in the global segment, how port0's ring-buffer store the descriptors to Data1a(P1), Data2a(P2), Data3a(P3). 

After the discovery phase, participants know the other participants and its "unicast" ports so they can push messages to specific participants.

Finally, let's observe how P1 share Data1b with P3 and Data1c with P2 and P3, this is done by pushing the buffer's descriptors to the corresponding ports, the descriptors are stored in the port's ring-buffer and listeners registered in these ports are notified.

### Design considerations

* **Minimize global interprocess locks**: Interprocess locks are dangerous because, in the case of one of the involved processes crash or hung while holding an interprocess lock, all the collaborating processes could be affected. In this design, the global segment is the critical area, most of accesses will be lock-free reading operations. Interprocess-locks will only be required when registering / unregistering a new listener in a port. Once the setup has been established, the risk of a process locking the whole port is minimized.

* **Scalable number of processes**: Every process creates its own shared memory segments to store the locally generated messages. This is more scalable than having a global segment shared between all the involved processes. 

* **Per application / process customizable memory usage**: Again, a local shared memory segment allows to adapt the size of the segments to the requirements of the application. Imagine for example an application sending two types of data: Video and status info. It could create a big segment to store video frames, and a small segment for the status messages.

### Future improvements

* **Fault tolerance**: As stated in the design considerations, the possibility of a process crashing holding interprocess resources is real. Implementing fault tolerance for these cases is not an easy task. Timeouts, keep alive checks and garbage collectors are some of the things that could be added to the initial design in order to achieve fault tolerance. This will be considered in future revisions.

### Mapping the design to FastRTPS transport layer

* **Locators**: LOCATOR_KIND_SHMEM (16) is defined to identify shared memory transport endpoints. Only the locator's port will be used to match the shared memory port_id.

* **SharedMemTransportDescriptor**: The following values can be customized:
    * segment_size: Size of the shared memory segment reserved by the transport.
    * port_queue_capacity: Size, in number of messages, of the shared memory port message queue.
    * port_overflow_policy: WAIT_ON_OVERFLOW or FAIL_ON_OVERFLOW.

    Non customizable values:
    * min_send_buffer_size = 1
    * max_message_size = 4.294.967.296

* **Default metatraffic multicast locator**: One locator, the port will be selected by the participant (will be the same as in the RTPS standard for UDP).

* **Default metatraffic unicast locator**: One locator, the port will be selected by the participant (default port_id will be the same as in the RTPS standard for UDP).

* **Default output locator**: There will be no default output locator.

* **OpenInputChannel**: A SharedMemChannelResource instance will be created. An opened channels vector is maintained, if the same input locator is opened several times the channel instance is reused.

* **OpenOutputChannel**: There will be only one SharedMemSenderResource instance per transport instance, an unordered_map of opened ports will be maintained by the SharedMemSenderResource object in order to match the destination locators to the shared memory ports.

## Class design
![](interprocess_shared_mem2.png)