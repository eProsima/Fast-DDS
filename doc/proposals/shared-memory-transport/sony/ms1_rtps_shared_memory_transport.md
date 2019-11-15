# Overview

DDSi-RTPS(Real-Time Publish-Subscribe) is the **protocol** that enable to support vendor implementation. And RTPS specifies implementaion and message packet format and other stuff as well. On this proposal, it will descibe how we could integrate **shared memory as transport** for RTPS layer based on.

# Objective

Implement a shared Memory transport using the current Fast-RTPS transport API & Architecture: The transport will be used as any other transport (including serialization and RTPS encapsulation). This approach is used in other DDS implementations such as commercial release for OpenSplice and RTI Connext DDS. This is mostly related to ROS2 RMW transport, but it can be used in any DDS application or product.

# Requirements

- Multi-Platform support (Windows, Linux and MacOS) along with ROS2.
- Design and Architecture Description
- Source Code [Fast-RTPS](https://github.com/eProsima/Fast-RTPS)
- Documents for user manual and tutorials.
- ROS2/RMW compatible interface (do not break ROS2 userland)
- Latency/Bandwidth improvement results.
- Intra/Inter-Process Communication

# Architecture / Design

## Interoperability

There is no gurantee that the whole system is constructed with eProsima Fast-RTPS, so multiple DDS implementaions may be used in a distributed system.
To keep the interoperability between different vendor DDS implementations, the shared memory transport feature has to be implemented with the following:

- All communications must take place using RTPS Messages.
- All implementations must implement the RTPS Message Receiver.
- Simple Participant and Endpoint Discovery Protocols.

## Discovery

Participant and Endpoint Discovery(PDP, EDP such as metatrrafic transmittion) will be kept as is in the current code, since this negotiation does not require shared memory use cases (all of the RTPS protocol stays put).

## Memory Management

- Dynamic Configuration
  Shared memory transport should be dynamically configured when necessary, that is, when the reader exists on the same host system than the writer. This is mandatory to reduce unnecessary message passing via shared memory.

  This can be detected using the GuidPrefix_t information in the locator, which includes vendor id, host id, process id and participant id.

  ```
  struct RTPS_DllAPI GuidPrefix_t
  ```

- Lifecycle
  Shared memory lifecycle is managed by the writer corresponding to HistoryCache more likely QoS setting.

- Owner
  Shared memory is owned by the writer. The writer is responsible of managing the shared memory (creation and removal) according to the aforementioned lifecycle.

- Version
  Versioning must be used to check if the implementation can use shared memory feature or not.

## Shared Memory Framework

Since this whole project is targeted to be multi-platform, it is mandatory not to use system dependent shared memory frameworks or primitives.

- C/C++ implementation
  - Primitive baseline implementation, can be used for multi-platform.
  - But most parts need to be implemented by our own.
  - Also good affinity to intra-process, just use std::shared_ptr.
- POSIX IPC shared memory (shm_open)
  - This is only for Unix system, so not good affinity for Windows/Mac.
- **boost::interprocess**
  - Surely multi-platform, this is generic and up-to-date interfaces.
  - Generic shared memory interface
  - Emulation layer available for windows and system v(xsi) if necessary.
  - Memory mapped file (this can be useful to use ramfs to refresh system restart or reboot)
  - Shared memory range can be truncated dynamically.
  - Container mapping with allocator
  - File locking (this could be useful to exclude access to the memory mapped shared memory)
  - Writer has read_write mode and reader only has read_only mode.
  - ***But cannot be extremly optimized via system calls, such as hugetlb to reduce TLB miss hit and pagefaults.***


## Event Notification

Writer needs to notify right after the shared memory is ready to be read on reader side. Then reader will be notified that data is ready to read out.

- RTPS extensible message
  - Submessages with ID's 0x80 to 0xff (inclusive) are vendor-specific
  - But probably we should avoid sending messages via network.
- **boost::interprocess**
  - Condition: after data is set by writer, notify the subscribers via condition variable on shared memory (named under topic name?).
  - Semaphore: after data is set by writer, post to notify the subscribers via semaphore on shared memory. Can be used to control the number of subscribers to read via wait API (named under topic name?).

## Security

- Encryption / Decryption:
  - Shared memory will be used as transport only, so encrypted data will be stored on shared memory. Therefore, this does not affect anything.
- Access Permission:
  - Can we support access permission? Like file system based access permission? This needs to be considered.
  - Since security implementation is different among platforms, boost::interprocess does not try to standardize the access permission control. This is responsibility of the implementation.

## Quality of Service

Since this is just a new implementation of the transport layer for shared memory, where all the rest of the RTPS protocol stays put, we expect full compatibility of configuration with Fast-RTPS.

Nevertheless, this needs to be further considered during the detailed design and implementation.

# Considerations

## Intra Process Communication

There is an intra-process communication feature in ROS2 rclcpp and rcl layer (basically zero-copy based). This has nothing to do with DDS implementation, but needs to be considered to ensure this feature doesn't break when implementing the DDS shared memory transport.

Refer to [Intra-Process-Communication](https://index.ros.org//doc/ros2/Tutorials/Intra-Process-Communication/)

Intra-process communication does not support QoS specification, e.g) RMW_QOS_POLICY_HISTORY_KEEP_ALL, !RMW_QOS_POLICY_DURABILITY_VOLATILE is not supported. 
Intra-process communication uses a ring buffer that is internally implemented [mapped_ring_buffer.hpp](https://github.com/ros2/rclcpp/blob/master/rclcpp/include/rclcpp/mapped_ring_buffer.hpp).

**this implementataion will stay on rclcpp for other implementations, but will be decricated for eProsima Fast-RTPS use-cases basically.**

## Container Boundary

Using containers basically means to divide the user space into independent sections, so shared memory should not be used beyond the container boundary.

This can be actually done just checking IP addresses to see if both participants are in the same host or not.

So we don't expect addtional requirements, it just appears to be another network interface.

# Reference

[ROSConJP 2019 Lightning Talk Presentation](https://discourse.ros.org/uploads/short-url/1SbbxgRCiM6NH2BuSCqNAe0aogx.pdf)
[Boost.Interprocess](https://www.boost.org/doc/libs/1_71_0/doc/html/interprocess.html)

