# External Dependency Quality Declaration `fastrtps`

This document is a declaration of software quality for the `foonathan_memory` external dependency, based on the
guidelines in [REP-2004](https://github.com/ros-infrastructure/rep/blob/rep-2004/rep-2004.rst).

Foonathan's memory is a C++ library to manage memory allocations that improves upon the STL.

This QD claims that the external dependency `foonathan_memory` qualifies to Quality Level 2 for it's use within *Fast
DDS*.

##  Version Policy [1]
### Version Scheme [1.i]
While `foonathan_memory` does not have a memory versioning scheme, our build tool `foonathan_memory_vendor` adheres to
[`semver`](https://semver.org/).

### Version Scheme [1.ii]
Regarding the previous point, `foonathan_memory_vendor` is currently at the stable version 1.0.0, having been
extensively shown to be stable whithin *Fast DDS*.

### Public API Declaration [1.iii]
TODO

### API Stability Within a Released ROS Distribution [1.iv]/[1.vi]
To avoid breaking API within a ROS distribution, `foonathan_memory_vendor` imports a specific version of
`foonathan_memory`.

### ABI Stability Within a Released ROS Distribution [1.v]/[1.vi]
To avoid breaking ABI within a ROS distribution, `foonathan_memory_vendor` imports a specific version of
`foonathan_memory`.

## Change Control Process [2]
