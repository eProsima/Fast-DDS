# eProsima Fast DDS

[![FIWARE Robotics](https://nexus.lab.fiware.org/static/badges/chapters/robotics.svg)](https://www.fiware.org/developers/catalogue/)
[![License](https://img.shields.io/github/license/eProsima/Fast-RTPS.svg)](https://opensource.org/licenses/Apache-2.0)
[![Releases](https://img.shields.io/github/v/release/eProsima/Fast-RTPS?sort=semver)](https://github.com/eProsima/Fast-RTPS/releases)
[![Issues](https://img.shields.io/github/issues/eProsima/Fast-RTPS.svg)](https://github.com/eProsima/Fast-RTPS/issues)
[![Forks](https://img.shields.io/github/forks/eProsima/Fast-RTPS.svg)](https://github.com/eProsima/Fast-RTPS/network/members)
[![Stars](https://img.shields.io/github/stars/eProsima/Fast-RTPS.svg)](https://github.com/eProsima/Fast-RTPS/stargazers)
<br/>
[![Documentation badge](https://img.shields.io/readthedocs/eprosima-fast-rtps.svg)](https://eprosima-fast-rtps.readthedocs.io)
![Status](https://nexus.lab.fiware.org/static/badges/statuses/incubating.svg)
[![Linux ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux/badge/icon?subject=%20%20%20LinuX%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux)
[![Windows ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v141/badge/icon?subject=%20%20%20Windows%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v141)
[![MacOS ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac/badge/icon?subject=%20%20%20MacOS%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac)

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

* Robotics: ROS (Robotic Operating System) as their default middleware for ROS2.
* EU R&D: FIWARE Incubated GE.

This project is part of [FIWARE](https://www.fiware.org/). For more information check the FIWARE Catalogue entry for
[Robotics](https://github.com/Fiware/catalogue/tree/master/robotics).

<br/>

**Want us to share your project with the community?  
Write to Evaluation.Support@eprosima.com or mention @EProsima on Twitter.  
We are curious to get to know your use case!**

<br/>

## Supported platforms

* Linux [![Linux Build Status](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux/badge/icon?subject=ci)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux)
* Windows [![Windows Build Status](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v141/badge/icon?subject=ci)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v141)
* Mac [![Mac Build Status](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac/badge/icon?subject=ci)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac)

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

#### Colcon installation
*******************
[colcon](https://colcon.readthedocs.io) is a command line tool to build sets of software packages.
This section explains to use it to compile easily Fast-RTPS and its dependencies.
First install ROS2 development tools (colcon and vcstool):

```bash
pip install -U colcon-common-extensions vcstool
```

Download the repos file that will be used to download Fast RTPS and its dependencies:

```bash
$ wget https://raw.githubusercontent.com/eProsima/Fast-DDS/master/fastrtps.repos
$ mkdir src
$ vcs import src < fastrtps.repos
```

Finally, use colcon to compile all software:

```bash
$ colcon build
```

#### Manual installation
*******************
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
* [Installation manual](https://fast-dds.docs.eprosima.com/en/latest/requirements.html)
* [User manual](https://fast-dds.docs.eprosima.com/en/latest/introduction.html)
* [FastRTPSGen manual](https://fast-dds.docs.eprosima.com/en/latest/geninfo.html)
* [Release notes](https://fast-dds.docs.eprosima.com/en/latest/notes.html)

## Quick Demo

For those who want to try a quick demonstration of Fast-DDS libraries on Ubuntu, here is a way to launch an example application.

First, download and install **docker** application. Open a terminal and type the following command

	$ sudo apt-get install docker.io

Then, download the docker image file from https://eprosima.com/index.php/downloads-all

Load the image and run it:

	$ docker load -i ubuntu-fast-rtps.tar
	$ docker run -it ubuntu-fast-rtps

You can run as many images as you want and check the communication between them.

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
