# eProsima Fast DDS

[![FIWARE Robotics](https://nexus.lab.fiware.org/static/badges/chapters/robotics.svg)](https://www.fiware.org/developers/catalogue/)
[![License](https://img.shields.io/github/license/eProsima/Fast-RTPS.svg)](https://opensource.org/licenses/Apache-2.0)
[![Releases](https://img.shields.io/github/v/release/eProsima/Fast-RTPS?sort=semver)](https://github.com/eProsima/Fast-RTPS/releases)
[![Issues](https://img.shields.io/github/issues/eProsima/Fast-RTPS.svg)](https://github.com/eProsima/Fast-RTPS/issues)
[![Forks](https://img.shields.io/github/forks/eProsima/Fast-RTPS.svg)](https://github.com/eProsima/Fast-RTPS/network/members)
[![Stars](https://img.shields.io/github/stars/eProsima/Fast-RTPS.svg)](https://github.com/eProsima/Fast-RTPS/stargazers)
[![Twitter Follow](https://img.shields.io/twitter/follow/eprosima?style=social)](https://twitter.com/EProsima)
<br/>
[![Documentation badge](https://img.shields.io/readthedocs/eprosima-fast-rtps.svg)](https://eprosima-fast-rtps.readthedocs.io)
![Status](https://nexus.lab.fiware.org/static/badges/statuses/incubating.svg)
[![Linux ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux/badge/icon?subject=%20%20%20Linux%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux)
[![Linux arm64 ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux_aarch64/badge/icon?subject=%20%20%20Linux-aarch64%20CI%20)](http://jenkins.eprosima.com:8080/view/Nightly/job/nightly_fastdds_sec_master_linux_aarch64/)
[![Windows ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v142/badge/icon?subject=%20%20%20%20Windows%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v142)
[![Mac ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac/badge/icon?subject=%20%20%20%20%20%20%20Mac%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac)
[![Coverage](https://img.shields.io/jenkins/coverage/cobertura.svg?jobUrl=http%3A%2F%2Fjenkins.eprosima.com%3A8080%2Fjob%2Fnightly_fastdds_coverage_linux)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_coverage_linux)

<a href="http://www.eprosima.com"><img src="https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ" align="left" hspace="8" vspace="2" width="100" height="100" ></a>

*eprosima Fast DDS* (formerly Fast RTPS) is a C++ implementation of the DDS (Data Distribution Service) standard of the OMG (Object Management Group). eProsima Fast DDS implements the RTPS (Real Time Publish Subscribe) protocol, which provides publisher-subscriber communications over unreliable transports such as UDP,
as defined and maintained by the Object Management Group (OMG) consortium. RTPS is also the wire interoperability protocol defined for the Data Distribution
Service (DDS) standard. *eProsima Fast DDS* expose an API to access directly the RTPS protocol, giving the user full access to the protocol internals.

Some of the main features of this library are:

* Configurable best-effort and reliable publish-subscribe communication policies for real-time
applications.
* Plug and play connectivity so that any new applications are automatically discovered by any other
members of the network.
* Modularity and scalability to allow continuous growth with complex and simple devices in the
network.
* Configurable network behavior and interchangeable transport layer: Choose the best protocol and
system input/output channel combination for each deployment.
* Two API Layers: a high-level Publisher-Subscriber one focused on usability (DDS) and a lower-level Writer-Reader one that provides finer access to the inner workings of the RTPS protocol.

*eProsima Fast DDS* has been adopted by multiple organizations in many sectors including these important cases:

* Robotics: ROS (Robotic Operating System) as their default middleware for ROS2 until and including the
  latest long term release Foxy Fitzroy.
* EU R&D: FIWARE Incubated GE.

This project is part of [FIWARE](https://www.fiware.org/). For more information check the FIWARE Catalogue entry for
[Robotics](https://github.com/Fiware/catalogue/tree/master/robotics).

## Want us to share your project with the community?

Write to evaluation.support@eprosima.com or mention @EProsima on Twitter.
We are curious to get to know your use case!

## Supported platforms

* Linux [![Linux ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux/badge/icon?subject=%20%20%20Linux%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux)
* Linux-aarch64 [![Linux arm64 ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux_aarch64/badge/icon?subject=%20%20%20Linux-aarch64%20CI%20)](http://jenkins.eprosima.com:8080/view/Nightly/job/nightly_fastdds_sec_master_linux_aarch64/)
* Windows [![Windows ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v142/badge/icon?subject=%20%20%20%20Windows%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v142)
* Mac [![Mac ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac/badge/icon?subject=%20%20%20%20%20%20%20Mac%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac)

## Installation Guide
You can get either a binary distribution of *eprosima Fast DDS* or compile the library yourself from source.

### Installation from binaries
The latest, up to date binary release of *eprosima Fast DDS* can be obtained from the <a href='http://www.eprosima.com'>company website</a>.

### Installation from Source

#### Dependencies

##### Asio and TinyXML2 libraries

On Linux, you can install these libraries using the package manager of your Linux distribution.
For example, on Ubuntu you can install them by using its package manager with the next command.

```bash
sudo apt install libasio-dev libtinyxml2-dev
```

On Windows, you can install these libraries using [Chocolatey](https://chocolatey.org).
First, download the following chocolatey packages from this
[ROS2 Github repository](https://github.com/ros2/choco-packages/releases/latest).

* asio.1.12.1.nupkg
* tinyxml2.6.0.0.nupkg

Once these packages are downloaded, open an administrative shell and execute the following command:

```batch
choco install -y -s <PATH\TO\DOWNLOADS\> asio tinyxml2
```

Please replace `<PATH\TO\DOWNLOADS>` with the folder you downloaded the packages to.

##### Libp11 library

Libp11 provides PKCS#11 support for openSSL. This is an optional dependency,
that is needed only when *eprosima Fast DDS* is used with security and PKCS#11 URLs.

On Linux, you can install libp11 using the package manager of your Linux distribution.
For example, on Ubuntu you can install them by using its package manager with the next command.

```bash
sudo apt install libp11-dev libengine-pkcs11-openssl
```

On Windows, you can download and compile the library from this
[ROS2 Github repository](https://github.com/OpenSC/libp11).
Follow the instructions on the repository to compile it on your platform.

#### Colcon installation

[colcon](https://colcon.readthedocs.io) is a command line tool to build sets of software packages.
This section explains to use it to compile easily Fast-RTPS and its dependencies.
First install ROS2 development tools (colcon and vcstool):

```bash
pip install -U colcon-common-extensions vcstool
```

Download the repos file that will be used to download Fast RTPS and its dependencies:

```bash
$ mkdir fastdds_ws
$ cd fastdds_ws
$ wget https://raw.githubusercontent.com/eProsima/Fast-DDS/master/fastrtps.repos
$ mkdir src
$ vcs import src < fastrtps.repos
```

Finally, use colcon to compile all software:

```bash
$ colcon build
```

#### Manual installation

Before compiling manually Fast DDS you need to clone the following dependencies and compile them using
[CMake](https://cmake.org).

* [Fast CDR](https://github.com/eProsima/Fast-CDR.git)

    ```bash
    $ git clone https://github.com/eProsima/Fast-CDR.git
    $ mkdir Fast-CDR/build && cd Fast-CDR/build
    $ cmake ..
    $ cmake --build . --target install
    ```

* [Foonathan memory](https://github.com/foonathan/memory)

    ```bash
    $ git clone https://github.com/eProsima/foonathan_memory_vendor.git
    $ cd foonathan_memory_vendor
    $ mkdir build && cd build
    $ cmake ..
    $ cmake --build . --target install
    ```

Once all dependencies are installed, you will be able to compile and install Fast DDS.

```bash
$ git clone https://github.com/eProsima/Fast-DDS.git
$ mkdir Fast-DDS/build && cd Fast-DDS/build
$ cmake ..
$ cmake --build . --target install
```


## Documentation

You can access the documentation online, which is hosted on [Read the Docs](https://fast-dds.docs.eprosima.com).

* [Start Page](https://fast-dds.docs.eprosima.com)
* [Installation manual](https://fast-dds.docs.eprosima.com/en/latest/installation/binaries/binaries_linux.html)
* [User manual](https://fast-dds.docs.eprosima.com/en/latest/fastdds/getting_started/getting_started.html)
* [Fast DDS-Gen manual](https://fast-dds.docs.eprosima.com/en/latest/fastddsgen/introduction/introduction.html)
* [Release notes](https://fast-dds.docs.eprosima.com/en/latest/notes/notes.html)

## Quality Declaration

*eprosima Fast DDS* claims to be in the **Quality Level 1** category based on the guidelines provided by
[ROS 2](https://ros.org/reps/rep-2004.html).
See the [Quality Declaration](https://github.com/eProsima/Fast-DDS/blob/master/QUALITY.md) for more details.

## Quick Demo

eProsima provides the eProsima Fast DDS Suite Docker image for those who want a quick demonstration of Fast-DDS running on an Ubuntu platform. It can be downloaded from [eProsima's downloads page](https://eprosima.com/index.php/downloads-all).

This Docker image was built for Ubuntu 20.04 (Focal Fossa).

To run this container you need **Docker installed**. From a terminal run the following command

	$ sudo apt-get install docker.io

Load the docker image:

	$ docker load -i ubuntu-fastdds-suite:<FastDDS-Version>.tar
	$ docker tag ubuntu-fastdds-suite:<FastDDS-Version> ubuntu-fastdds-suite:latest

Run the eProsima Fast DDS Suite Docker container:

    $ xhost local:root
    $ docker run -it --privileged -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix \
    ubuntu-fastdds-suite:<FastDDS-Version>

This Docker Image comes bundled with the following:

### Shapes Demo

eProsima Shapes Demo is an application in which Publishers and Subscribers are shapes of different
colors and sizes moving on a board. Each shape refers to its own topic: Square, Triangle or Circle.
A single instance of the eProsima Shapes Demo can publish on or subscribe to several topics at a
time.

You can read more about this application on the
[Shapes Demo documentation page](https://eprosima-shapes-demo.readthedocs.io/).

To run this application once inside the Docker container run:

    $ ShapesDemo

eProsima Shapes Demo usage information can be found on the
[Shapes Demo First Steps page](https://eprosima-shapes-demo.readthedocs.io/en/latest/first_steps/first_steps.html).

### Fast DDS Monitor

eProsima Fast DDS Monitor is a graphical desktop application aimed at monitoring DDS environments
deployed using the *eProsima Fast DDS* library. Thus, the user can monitor in real time the status
of publication/subscription communications between DDS entities. They can also choose from a wide
variety of communication parameters to be measured (latency, throughput,packet loss, etc.), as well
as record and compute in real time statistical measurements on these parameters (mean, variance,
standard deviation, etc.).

You can read more about this application on the
[Fast DDS Monitor documentation page](https://fast-dds-monitor.readthedocs.io/).

To run this application once inside the Docker container run:

    $ fastdds_monitor

eProsima Fast DDS Monitor usage information can be found on the
[Fast DDS Monitor User Manual](
https://fast-dds-monitor.readthedocs.io/en/latest/rst/user_manual/initialize_monitoring.html).


### Fast DDS libraries and Examples

Included in this Docker container is a set of binary examples that showcase several functionalities of the
Fast DDS libraries. These examples' path can be accessed from a terminal by typing

    $ goToExamples

From this folder you can access all examples, both for DDS and RTPS. We detail the steps to launch two such
examples below.

To launch the Hello World example (a minimal example that will perform a Publisher/Subscriber match and start
sending samples) you could run:

    $ goToExamples
    $ cd HelloWorldExample/bin
    $ tmux new-session "./HelloWorldExample publisher 0 1000" \; \
    split-window "./HelloWorldExample subscriber" \; \
    select-layout even-vertical

This example is not constrained to the current instance. It's possible to run several instances of this
container to check the communication between them by running the following from each container.

    $ goToExamples
    $ cd HelloWorldExample/bin
    $ ./HelloWorldExample publisher

or

    $ goToExamples
    $ cd HelloWorldExample/bin
    $ ./HelloWorldExample subscriber

Another example you could launch is the Benchmark example. This example creates either a Publisher or a Subscriber and
on a successful match starts sending samples. After a few seconds the process that launched the Publisher will show
a report with the number of samples transmitted.

On the subscriber side, run:

    $ goToExamples
    $ cd Benchmark/bin
    $ ./Benchmark subscriber udp

On the publisher side, run:

    $ goToExamples
    $ cd Benchmark/bin
    $ ./Benchmark publisher udp


## Getting Help

If you need support you can reach us by mail at `support@eProsima.com` or by phone at `+34 91 804 34 48`.




---

<!--
    ROSIN acknowledgement from the ROSIN press kit
    @ https://github.com/rosin-project/press_kit
-->

<a href="http://rosin-project.eu">
  <img src="http://rosin-project.eu/wp-content/uploads/rosin_ack_logo_wide.png"
       alt="rosin_logo" height="60" >
</a>

Supported by ROSIN - ROS-Industrial Quality-Assured Robot Software Components.
More information: <a href="http://rosin-project.eu">rosin-project.eu</a>

<img src="http://rosin-project.eu/wp-content/uploads/rosin_eu_flag.jpg"
     alt="eu_flag" height="45" align="left" >

This project has received funding from the European Union’s Horizon 2020
research and innovation programme under grant agreement no. 732287.
