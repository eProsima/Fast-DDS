# **External Dependency Quality Declaration** `foonathan_memory`

This document is a declaration of software quality for the `foonathan_memory` external dependency, inspired on the
guidelines provided in the [ROS 2 REP-2004 document](https://ros.org/reps/rep-2004.html).

Foonathan's [`memory`](https://github.com/foonathan/memory) is a C++ library to manage memory allocations that improves upon the STL.
*eprosima Fast DDS* started using this library following the advice of ROS's and eProsima's partner, Apex.AI.

This Quality Declaration claims that the external dependency `foonathan_memory` qualifies to Quality Level 1 category for its use within *eprosima Fast DDS*.
This document assesses the quality risks that can be introduced by the use of this library in *eprosima Fast DDS*.

Below are the rationales, notes and caveats for this claim, organized by the requirements listed in the [Package Requirements for Quality Level 1 in REP-2004](https://www.ros.org/reps/rep-2004.html#package-requirements).

##  Version Policy [1]

### Version Scheme [1.i]

`foonathan_memory` does not have a declared versioning scheme.

The latest release can be found [here](https://github.com/foonathan/memory/releases) and the release notes can be found in the [CHANGELOG](https://github.com/foonathan/memory/blob/main/CHANGELOG.md).
Even though the current version is `< 1.0.0`, `foonathan_memory` is in [maintenance mode](https://www.jonathanmueller.dev/project/).

*eprosima Fast DDS* ensures `foonathan_memory` version stability by pinning to a specific [release](https://github.com/foonathan/memory/releases/tag/v0.7-3).
Both *eprosima* `foonathan_memory_vendor` utility and *eprosima Fast DDS* CI tests use this specific release.
The specific tag was released January 11th, 2023.

*eprosima Fast DDS* maintainers will keep a continuous watch over new releases in order to assess the impact they could have over *eprosima Fast DDS*.
Any `bugfix` or security vulnerability corrected that affects the API used by *eprosima Fast DDS* will be analyzed by the maintainers.
Decision about updating the used commit/release rests in *eprosima Fast DDS* maintainers.

### Version Stability [1.ii]

`foonathan_memory` is stable and in [maintenance mode](https://www.jonathanmueller.dev/project/).

### Public API Declaration [1.iii]

`foonathan_memory` public API is defined in its [documentation](https://memory.foonathan.net/).
Additionally, *eprosima Fast DDS* has clearly stated the API used by the project [here](Public_API_foonathan_memory.md).

### API and ABI Stability Policy [1.iv]/[1.v]

`foonathan_memory` does not provide any versioning policy.
However, by pinning *eprosima Fast DDS* CI tests and `foonathan_memory_vendor` utility to a particular release ensures API and ABI stability within *eprosima Fast DDS* project.

## Change Control Process [2]

`foonathan_memory` does not have a stated change control process.
Nevertheless, only when the pinned commit/release is updated is *eprosima Fast DDS* affected.
The change control process for the  update of the `foonathan_memory_vendor` utility follows [eProsima Contributing guidelines](https://github.com/eProsima/policies/blob/main/CONTRIBUTING.md).

### Change Requests [2.i]

`foonathan_memory` does not require that changes occur through a pull request.
Many changes are committed directly by its maintainer.

`foonathan_memory_vendor` requires that all changes occur through a pull request.

### Contributor Origin [2.ii]

`foonathan_memory` does not have a confirmation of contributor origin policy.

`foonathan_memory_vendor` uses DCO as its confirmation of contributor origin policy. More information can be found [here](https://github.com/eProsima/policies/blob/main/CONTRIBUTING.md).

### Peer Review Policy [2.iii]

`foonathan_memory` does not state its peer review policy, but each pull request is at least reviewed by the maintainer.

All `foonathan_memory_vendor` pull requests will be peer reviewed and need at least one approved review to be merged.

### Continuous Integration [2.iv]

`foonathan_memory` changes run CI tests and the latest results can be seen [here](https://github.com/foonathan/memory/actions/workflows/main_ci.yml)

Additionally, eProsima CI runs `foonathan_memory` tests nightly in all *eprosima Fast DDS* [Tier 1 platforms](PLATFORM_SUPPORT.md).
Latest results can be found here:

* Linux [![Linux ci](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_linux/badge/icon?subject=%20%20%20Linux%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_linux)
* Linux-aarch64 [![Linux arm64 ci](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_linux_aarch64/badge/icon?subject=%20%20%20Linux-aarch64%20CI%20)](http://jenkins.eprosima.com:8080/view/Nightly/job/nightly_foonathan_memory_master_linux_aarch64/)
* Windows [![Windows ci](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_windows/label=windows-secure,platform=x64,toolset=v142/badge/icon?subject=%20%20%20%20Windows%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_windows/label=windows-secure,platform=x64,toolset=v142)
* Mac [![Mac ci](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_mac/badge/icon?subject=%20%20%20%20%20%20%20Mac%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_mac)

`foonathan_memory_vendor` does not provide CI tests being only an utility providing CMake files that configure `foonathan_memory`.
If ROS 2 dependencies are found, `foonathan_memory_vendor` could run ROS 2 linters and copyright tests as can be seen in the [nightly CI results](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/testReport/foonathan_memory_vendor/).

### Documentation Policy [2.v]

`foonathan_memory` does not have a stated documentation policy.

## Documentation [3]

### Feature Documentation [3.i]

`foonathan_memory` provides a [feature list](https://memory.foonathan.net/index.html) with descriptions of its main features.

### Public API Documentation [3.ii]

`foonathan_memory` has embedded API documentation that is generated using [Doxygen](https://www.doxygen.nl/index.html) and is [hosted](https://memory.foonathan.net/namespacefoonathan_1_1memory.html) alongside the feature documentation.

Additionally, *eprosima Fast DDS* provides a [document](Public_API_foonathan_memory.md) stating the API used within the project.

### License [3.iii]

The license for `foonathan_memory` is Zlib.
A summary statement is provided in each source file and a full copy can be found in the [LICENSE](https://raw.githubusercontent.com/foonathan/memory/main/LICENSE) file.

### Copyright Statements [3.iv]

The copyright holders each provide a statement of copyright in each source file in `foonathan_memory`.

## Testing [4]

### Feature and Public API Testing [4.i]/[4.ii]

`foonathan_memory` provides tests which simulate typical usage located in the [`test`](https://github.com/foonathan/memory/tree/main/test) directory.

Specifically, the API used by *eprosima Fast DDS* is tested in the following tests:

* Allocators: [test/default_allocator.cpp](https://github.com/foonathan/memory/blob/v0.7-3/test/default_allocator.cpp)
* Allocator implementations: [test/memory_pool.cpp](https://github.com/foonathan/memory/blob/v0.7-3/test/memory_pool.cpp)
* Adapters and Wrappers: [test/segregator.cpp](https://github.com/foonathan/memory/blob/v0.7-3/test/segregator.cpp)
* Alias templates: `foonathan_memory` does not provide any tests to check this functionality.
Regardless, *eprosima Fast DDS* tests these features in the [PersistenceTests](https://github.com/eProsima/Fast-DDS/tree/master/test/unittest/rtps/persistence) and the [WriterProxyTests](https://github.com/eProsima/Fast-DDS/tree/master/test/unittest/rtps/reader).

### Coverage [4.iii]

`foonathan_memory` tracks testing coverage on [codecov](https://codecov.io/github/foonathan/memory).

Apart from this, *eprosima Fast DDS* ensures that every feature and API used within the library has been tested by running its own [coverage analysis](http://jenkins.eprosima.com:8080/job/nightly_fastdds_coverage_linux/), which completely covers all API used by Fast DDS.
In order to change the `foonathan_memory` commit/release used in Fast DDS, maintainers must ensure that all new API is tested accordingly.

### Performance [4.iv]

`foonathan_memory` does not conduct performance tests.

*eprosima Fast DDS* will run its [performance tests](test/performance) and analyze their results before deciding to change the pinned commit/release to prevent performance regression.
Being pinned to a specific release, any performance regression in *eprosima Fast DDS* could not be blamed on `foonathan_memory`.

### Linters and Static Analysis [4.v]

`foonathan_memory` does not conduct tests with linters or any static analysis tool.
As `foonathan_memory` is not maintained by eProsima, these requirements should be waved.

## Dependencies [5]

`foonathan_memory` does not have any dependencies outside of the C++ standard library.

## Platform Support [6.i]

*eprosima Fast DDS* ensures `foonathan_memory` support for the same platforms defined in [PLATFORM_SUPPORT](PLATFORM_SUPPORT.md).
Current nightly results can be found here:

* Linux [![Linux ci](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_linux/badge/icon?subject=%20%20%20Linux%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_linux)
* Linux-aarch64 [![Linux arm64 ci](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_linux_aarch64/badge/icon?subject=%20%20%20Linux-aarch64%20CI%20)](http://jenkins.eprosima.com:8080/view/Nightly/job/nightly_foonathan_memory_master_linux_aarch64/)
* Windows [![Windows ci](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_windows/label=windows-secure,platform=x64,toolset=v142/badge/icon?subject=%20%20%20%20Windows%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_windows/label=windows-secure,platform=x64,toolset=v142)
* Mac [![Mac ci](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_mac/badge/icon?subject=%20%20%20%20%20%20%20Mac%20CI%20)](http://jenkins.eprosima.com:8080/job/nightly_foonathan_memory_master_mac)

## Vulnerability Disclosure Policy [7.i]

Even though `foonathan_memory` does not provide a Vulnerability Disclosure Policy, eProsima will apply its [Vulnerability Disclosure Policy](https://github.com/eProsima/policies/blob/main/VULNERABILITY.md) for any security issue within the scope of *eprosima Fast DDS* project.

# Current Status Summary

The chart below compares the requirements in the [REP-2004](https://www.ros.org/reps/rep-2004.html#quality-level-comparison-chart) with the current state of `foonathan_memory` with the specifics dealt in this document to guarantee its quality level.

■ = requirement not properly fulfilled but dealt with.

● = requirement waved

|Number| Requirement| Current State |
|--|--|--|
|1| **Version policy** |---|
|1.i|Version Policy available |■|
|1.ii|Stable version |✓|
|1.iii|Declared public API|✓|
|1.iv|API stability policy|■|
|1.v|ABI stability policy|■|
|2| **Change control process** |---|
|2.i| All changes occur on change request |■|
|2.ii| Contributor origin (DCO, CLA, etc) |■|
|2.iii| Peer review policy |■|
|2.iv| CI policy for change requests |■|
|2.v| Documentation policy for change requests |■|
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
|4.iii.a| Using coverage |■|
|4.iii.b| Coverage policy |✓|
|4.iv.a| Performance tests (if applicable) |■|
|4.iv.b| Performance tests policy|✓|
|4.v.a| Code style enforcement (linters)|●|
|4.v.b| Use of static analysis tools |●|
|5| **Dependencies** | --- |
|5.iii| Justifies quality use of dependencies |✓|
|6| **Platform support** | --- |
|6.i| Support targets Tier1 ROS platforms|✓|
|7| **Security** | --- |
|7.i| Vulnerability Disclosure Policy |✓|
