This document is a declaration of software quality for *eprosima Fast DDS* inspired on the guidelines provided in the [ROS 2 REP-2004 document](https://www.ros.org/reps/rep-2004.html).

# Quality Declaration

*eprosima Fast DDS* is a C++ implementation of the DDS (Data Distribution Service) standard of the OMG (Object Management Group).
eProsima Fast DDS implements the RTPS (Real Time Publish Subscribe) protocol, which provides publisher-subscriber communications over unreliable transports such as UDP,
as defined and maintained by the Object Management Group (OMG) consortium.

*eprosima Fast DDS* claims to be in the **Quality Level 1** category.

Below are the rationales, notes and caveats for this claim, organized by the requirements listed in the [Package Requirements for Quality Level 1 in REP-2004](https://www.ros.org/reps/rep-2004.html#package-requirements).

## Version Policy [1]

### Version Scheme [1.i]

The **Versioning Policy Declaration** for *eprosima Fast DDS* can be found [here](VERSIONING.md) and it adheres to [`semver`](https://semver.org/).

### Version Stability [1.ii]

*eprosima Fast DDS* is at a stable version, i.e. `>=1.0.0`.
The latest version can be found [here](https://github.com/eProsima/Fast-DDS/releases) and its change history can be found [here](versions.md).

### Public API Declaration [1.iii]

*eprosima Fast DDS* documentation is hosted on [Read the Docs](https://fast-dds.docs.eprosima.com). The documentation includes the [API Reference](https://fast-dds.docs.eprosima.com/en/latest/fastdds/api_reference/api_reference.html).

### API Stability Policy [1.iv]

*eprosima Fast DDS* will only break public API between major releases.

### ABI Stability Policy [1.v]

Any ABI break in *eprosima Fast DDS* will be done between minor versions and it should be clearly stated on the release notes.

## Change Control Process [2]

The stability of *eprosima Fast DDS* is ensured through review, CI and tests.
The change control process can be found in [CONTRIBUTING](https://github.com/eProsima/policies/blob/main/CONTRIBUTING.md).

All changes to *eprosima Fast DDS* occur through pull requests that are required to pass all CI tests.
In case of failure, only maintainers can merge the pull request, and only when there is enough evidence that the failure is unrelated to the change.
Additionally, all pull requests must have a positive review from one other contributor that did not author the pull request.

### Change Requests [2.i]

All changes will occur through a pull request.

### Contributor Origin [2.ii]

*eprosima Fast DDS* uses the [Developer Certificate of Origin (DCO)](https://developercertificate.org/) as its confirmation of contributor origin policy.
More information can be found in [CONTRIBUTING](https://github.com/eProsima/policies/blob/main/CONTRIBUTING.md)

### Peer Review Policy [2.iii]

All pull requests will be peer-reviewed by at least one other contributor who did not author the pull request. Approval is required before merging.

### Continuous Integration [2.iv]

All pull requests must pass CI to be considered for merging, unless maintainers consider that there is enough evidence that the failure is unrelated to the changes.
CI testing is automatically triggered by incoming pull requests.
Current nightly results can be seen here for all supported platforms:

* Linux [![Linux amd64 ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux/badge/icon?subject=%20%20%20Linux%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux)
* Linux-aarch64 [![Linux arm64 ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux_aarch64/badge/icon?subject=%20%20%20Linux-aarch64%20CI%20)](http://jenkins.eprosima.com:8080/view/Nightly/job/nightly_fastdds_sec_master_linux_aarch64/)
* Windows [![Windows ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v141/badge/icon?subject=%20%20%20%20Windows%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v141)
* Mac [![Mac ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac/badge/icon?subject=%20%20%20%20%20%20%20Mac%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac)

### Documentation Policy [2.v]

All pull requests must resolve related documentation changes before merging as stated in [CONTRIBUTING](https://github.com/eProsima/policies/blob/main/CONTRIBUTING.md).

## Documentation [3]

### Feature Documentation [3.i]

*eprosima Fast DDS* has a documented feature list hosted in [Read the Docs](https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/dds_layer.html#dds-layer).

### Public API Documentation [3.ii]

*eprosima Fast DDS* includes a complete API Reference which is hosted in [Read the Docs](https://fast-dds.docs.eprosima.com/en/latest/fastdds/api_reference/api_reference.html).

### License [3.iii]

The license for *eprosima Fast DDS* is Apache 2.0, and a summary can be found in each source file.
A full copy of the license can be found [here](LICENSE).

There is some third-party content included with *eprosima Fast DDS* which is distributed under the [Boost Software License version 1.0](http://www.boost.org/LICENSE_1_0.txt) and [Zlib license](https://github.com/leethomason/tinyxml2/blob/8c8293ba8969a46947606a93ff0cb5a083aab47a/readme.md#license).

### Copyright Statements [3.iv]

*eprosima Fast DDS* copyright holder provide a statement of copyright in each source file.

## Testing [4]

### Feature Testing [4.i]

Each feature in *eprosima Fast DDS* has corresponding tests which simulate typical usage, and they are located in the [`test` directory](test).
New features are required to have tests before being added as stated in [CONTRIBUTING](https://github.com/eProsima/policies/blob/main/CONTRIBUTING.md).
Current nightly results can be found here:

* Linux [![Linux ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux/badge/icon?subject=%20%20%20Linux%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux)
* Linux-aarch64 [![Linux arm64 ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux_aarch64/badge/icon?subject=%20%20%20Linux-aarch64%20CI%20)](http://jenkins.eprosima.com:8080/view/Nightly/job/nightly_fastdds_sec_master_linux_aarch64/)
* Windows [![Windows ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v141/badge/icon?subject=%20%20%20%20Windows%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v141)
* Mac [![Mac ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac/badge/icon?subject=%20%20%20%20%20%20%20Mac%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac)

### Public API Testing [4.ii]

Each part of the public API has tests, and new additions or changes to the public API require tests before being added.
The tests aim to cover typical usage. Currently, efforts are being made to improve our code coverage statistics in order to test corner cases.

### Coverage [4.iii]

[![Coverage](https://img.shields.io/jenkins/coverage/cobertura.svg?jobUrl=http%3A%2F%2Fjenkins.eprosima.com%3A8080%2Fjob%2Fnightly_fastdds_coverage_linux)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_coverage_linux)
*eProsima Fast DDS* aims to provide a line coverage **above 95%**.
*Fast DDS* code coverage policy comprises:
1. All contributions to *Fast DDS* must increase (or at least keep) current line coverage.
   This is done to ensure that the **95%** line coverage goal is eventually met.
1. Line coverage regressions are only permitted if properly justified and accepted by maintainers.
1. If the CI system reports a coverage regression after a pull request has been merged, the maintainers must study the case and decide how to proceed, mostly reverting the changes and asking for a more thorough testing of the committed changes.
1. External dependencies are excluded from the coverage report.
1. *Fast DDS* examples are excluded from the coverage report.
1. This policy is enforced through the [nightly Fast DDS coverage CI job](http://jenkins.eprosima.com:8080/job/nightly_fastdds_coverage_linux/).

As stated in [CONTRIBUTING.md](CONTRIBUTING.md), developers and contributors are asked to run a line coverage assessment locally before submitting a PR.

### Performance [4.iv]

*eprosima Fast DDS* provides a test directory specifically dedicated to performance that can be found [here](test/performance).
Performace tests are automatically run after merging to the master branch of the project.
If there has been any performance regression a notification is issued to maintainers that will study and decide how to proceed.

Latest latency performance test results can be accesed [here](http://jenkins.eprosima.com:8080/job/FastRTPS_latency_performance/lastBuild/) and latest throughput performance test results can be seen [here](http://jenkins.eprosima.com:8080/job/fastrtps_throughput_performance/lastBuild/).

Furthermore, [*eprosima benchmarking* project](https://github.com/eProsima/benchmarking) provides tools to run performance tests and a methodology to analyze and present the results:

* [Latency Performance Test Specification](https://github.com/eProsima/benchmarking/blob/master/fastrtps_automated_benchmark/latency/README.md)
* [Throughput Performance Test Specification](https://github.com/eProsima/benchmarking/blob/master/fastrtps_automated_benchmark/throughput/README.md)

### Linters and Static Analysis [4.v]

*eprosima Fast DDS* has a [code style](https://github.com/eProsima/cpp-style) that is enforced using [*uncrustify*](https://github.com/uncrustify/uncrustify).
Among the CI tests there are tests that ensures that every pull request is compliant with the code style.
The latest pull request results can be seen [here](http://jenkins.eprosima.com:8080/job/fastdds_github_uncrustify/lastBuild).
The tests only check files where changes have been made.
Therefore, the code style is only enforced in some files.
However, the tendency will be to homogenize the older source files to the code style.

*eprosima Fast DDS* uses Synopsis Coverity static code analysis and the latest results can be found [here](https://scan.coverity.com/projects/eprosima-fast-dds).

## Dependencies [5]

### Direct Runtime Dependencies [5.iii]

*eprosima Fast DDS* depends on the following packages:

- `libasio-dev`
- `libtinyxml2-dev`
- `fast-cdr`
- `foonathan_memory`

The first two dependencies are suggested to be installed for Linux using apt package manager, which would pull them from the Debian upstream.
Therefore, these dependencies can be considered Quality Level 1 following the [advantages of being packaged for Debian](https://wiki.debian.org/AdvantagesForUpstream).

**eProsima Fast CDR** Quality Declaration can be found [here](https://github.com/eProsima/Fast-CDR/blob/master/QUALITY.md). Currently, **eProsima Fast CDR** claims to be in the **Quality Level 1** category.

`foonathan_memory` Quality Declaration can be found [here](Quality_Declaration_foonathan_memory.md).
This declaration claims that, even though `foonathan_memory` does not meet several quality requirements, it is considered to fulfill the **Quality Level 1** requirements for its use within *eprosima Fast DDS* with the caveats explained in the declaration.

## Platform Support [6]

*eprosima Fast DDS* supports the following platforms and tests each change against all of them as can be seen in the current nightly results:

* Linux [![Linux ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux/badge/icon?subject=%20%20%20Linux%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux)
* Linux-aarch64 [![Linux arm64 ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_linux_aarch64/badge/icon?subject=%20%20%20Linux-aarch64%20CI%20)](http://jenkins.eprosima.com:8080/view/Nightly/job/nightly_fastdds_sec_master_linux_aarch64/)
* Windows [![Windows ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v141/badge/icon?subject=%20%20%20%20Windows%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_windows/label=windows-secure,platform=x64,toolset=v141)
* Mac [![Mac ci](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac/badge/icon?subject=%20%20%20%20%20%20%20Mac%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_fastdds_sec_master_mac)

More information about the supported platforms can be found in [PLATFORM_SUPPORT](PLATFORM_SUPPORT.md)

## Vulnerability Disclosure Policy [7.i]

*eprosima Fast DDS* vulnerability Disclosure Policy can be found [here](https://github.com/eProsima/policies/blob/main/VULNERABILITY.md)

# Current Status Summary

The chart below compares the requirements in the [REP-2004](https://www.ros.org/reps/rep-2004.html#quality-level-comparison-chart) with the current state of *eprosima Fast DDS*
|Number| Requirement| Current State |
|--|--|--|
|1| **Version policy** |---|
|1.i|Version Policy available |✓|
|1.ii|Stable version |✓|
|1.iii|Declared public API|✓|
|1.iv|API stability policy|✓|
|1.v|ABI stability policy|✓|
|2| **Change control process** |---|
|2.i| All changes occur on change request |✓|
|2.ii| Contributor origin (DCO, CLA, etc) |✓|
|2.iii| Peer review policy |✓|
|2.iv| CI policy for change requests |✓|
|2.v| Documentation policy for change requests |✓|
|3| **Documentation** | --- |
|3.i| Per feature documentation |✓|
|3.ii| Per public API item documentation |✓|
|3.iii| Declared License(s) |✓|
|3.iv| Copyright in source files|✓|
|3.v.a| Quality declaration linked to README |✓|
|3.v.b| Centralized declaration available for peer review |✓|
|4| **Testing** | --- |
|4.i| Feature items tests |✓|
|4.ii| Public API tests |✓|
|4.iii.a| Using coverage |✓|
|4.iii.b| Coverage policy |✓|
|4.iv.a| Performance tests (if applicable) |✓|
|4.iv.b| Performance tests policy|✓|
|4.v.a| Code style enforcement (linters)|✓|
|4.v.b| Use of static analysis tools |✓|
|5| **Dependencies** | --- |
|5.iii| Justifies quality use of dependencies |✓|
|6| **Platform support** | --- |
|6.i| Support targets Tier1 ROS platforms|✓|
|7| **Security** | --- |
|7.i| Vulnerability Disclosure Policy |✓|
