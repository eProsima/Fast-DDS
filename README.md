eProsima Fast RTPS
====

Implementation of RTPS (Real Time Publish-Subscribe) Wire Protocol defined by the OMG (Object Management Group - http://www.omg.org/) that forms part of the DDS (Data Distribution Service - http://portals.omg.org/dds/) Interoperability Wire Protocol.

To download the product (installer & autoconf packages) please visit the  [eProsima Fast RTPS product page](http://www.eprosima.com/index.php/es/products-all/eprosima-fast-rtps)

[eProsima](www.eprosima.com) is a SME company focused on networking middleware with special attention to the OMG standard called Data Distribution Service for Real-time systems (DDS).
eProsima develops new features and plug-ins for DDS, interoperability tools, and personalized networking middleware solutions for its customers.

## Feature list
* Simple Participant and Endpoint Discovery Protocols; as well as a Static Endpoint Discovery Protocol.
* Two layers of user API's, to directly use RTPS entities or Publisher-Subscriber entities.
* Included Qos (included in the basic behaviour of the library):
  * Best Effort and Reliable Communication.
  * History Qos and ResourceLimits Qos.
  * Writer Liveliness Qos.
  * Partition Qos.
  * Durability Qos.
  * Shared Ownership Qos.
* Supported Qos (easily implemented with the provided API):
  * Exclusive Ownership Qos.
  * Time-Based Filter Qos.
  * Content-Based Filter Qos on the Subscriber side.

### New in Release 1.0.0
* Two layers of user API.
* Performance improvements.

### New in Release 0.5.0
* Writer Liveliness Protocol added.
* Static Endpoint Discovery Protocol adepted to support the definition of Qos. 

### New in Release 0.4.0
* New and improved API.
* Simple Endpoint Discovery Protocol.
* Latency and Throughput Tests included.

### New in Release 0.3.1

* Reliable and Best-Effort communication.
* Simple Participant Discovery Protocol.
* Static Endpoint Discovery Protocol.
