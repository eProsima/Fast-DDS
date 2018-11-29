# eProsima Fast RTPS
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

<a href="http://www.eprosima.com"><img src="https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ" align="left" hspace="8" vspace="2" width="100" height="100" ></a>
*eprosima Fast RTPS* is a C++ implementation of the RTPS (Real Time Publish Subscribe) protocol, which provides publisher-subscriber communications over unreliable transports such as UDP, 
as defined and maintained by the Object Management Group (OMG) consortium. RTPS is also the wire interoperability protocol defined for the Data Distribution
Service (DDS) standard, again by the OMG. *eProsima Fast RTPS* holds the benefit of being standalone and up-to-date, as most vendor solutions either implement RTPS as a tool to implement 
DDS or use past versions of the specification.

Some of the main features of this library are:

* Configurable best-effort and reliable publish-subscribe communication policies for real-time
applications.
* Plug and play connectivity so that any new applications are automatically discovered by any other
members of the network.
* Modularity and scalability to allow continuous growth with complex and simple devices in the
network.
* Configurable network behavior and interchangeable transport layer: Choose the best protocol and
system input/output channel combination for each deployment.
* Two API Layers: a high-level Publisher-Subscriber one focused on usability and a lower-level Writer-Reader one that provides finer access to the inner workings of the RTPS protocol.

eProsima Fast RTPS has been adopted by multiple organizations in many sectors including these important cases:

* Robotics: ROS (Robotic Operating System) as their default middleware for ROS2.
* EU R&D: FIWARE Incubated GE.

## Installation Guide
You can get either a binary distribution of *eprosima Fast RTPS* or compile the library yourself from source.

### Installation from binaries
The latest, up to date binary release of *eprosima Fast RTPS* can be obtained from the <a href='http://www.eprosima.com'>company website</a>.

### Installation from Source
To compile *eprosima Fast RTPS* from source, at least Cmake version 2.8.12 is needed.
Clone the project from GitHub:

    $ git clone https://github.com/eProsima/Fast-RTPS
    $ cd Fast-RTPS
    $ mkdir build
    $ cd build

If you are on Linux, execute:

    $ cmake -DTHIRDPARTY=ON ..
    $ make
    $ make install

If you are on Windows, choose your version of Visual Studio:

    > cmake -G "Visual Studio 14 2015 Win64" -DTHIRDPARTY=ON ..
    > cmake --build . --target install
	
If you want to compile the performance tests, you will need to add the argument `-DPERFORMANCE_TESTS=ON` when calling Cmake.

## Documentation 

You can access the documentation online, which is hosted on [Read the Docs](http://eprosima-fast-rtps.readthedocs.io).

* [Start Page](http://eprosima-fast-rtps.readthedocs.io)
* [Installation manual](http://eprosima-fast-rtps.readthedocs.io/en/latest/requirements.html)
* [User manual](http://eprosima-fast-rtps.readthedocs.io/en/latest/introduction.html)
* [FastRTPSGen manual](http://eprosima-fast-rtps.readthedocs.io/en/latest/geninfo.html)
* [Release notes](http://eprosima-fast-rtps.readthedocs.io/notes.html)

## Getting Help

If you need support you can reach us by mail at `support@eProsima.com` or by phone at `+34 91 804 34 48`.


