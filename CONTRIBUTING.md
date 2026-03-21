# Contribution Guidelines

The following documents constitutes a set of guidelines to which contributors must adhere.

* [Contributions Licensing](#contributions-licensing)
* [Developer Certificate of Origin](#developer-certificate-of-origin)
* [Code Coverage](#code-coverage)
* [Issues and Support](#issues-and-support)

## Contributions Licensing

Any contribution that you make to this repository will
be under the Apache 2 License, as dictated by that
[license](http://www.apache.org/licenses/LICENSE-2.0.html):

~~~
5. Submission of Contributions. Unless You explicitly state otherwise,
   any Contribution intentionally submitted for inclusion in the Work
   by You to the Licensor shall be under the terms and conditions of
   this License, without any additional terms or conditions.
   Notwithstanding the above, nothing herein shall supersede or modify
   the terms of any separate license agreement you may have executed
   with Licensor regarding such Contributions.
~~~

## Developer Certificate of Origin

Contributors must sign-off each commit by adding a `Signed-off-by: ...`
line to commit messages to certify that they have the right to submit
the code they are contributing to the project according to the
[Developer Certificate of Origin (DCO)](https://developercertificate.org/).

## Code Coverage

As stated in [QUALITY.md](QUALITY.md), all contributions to the project must increase line code coverage.
Because of this, contributors are asked to locally run a coverage assessment that ensures that code coverage has increased when compared to the latest execution of the [nightly coverage CI job](http://jenkins.eprosima.com:8080/job/nightly_fastdds_coverage_linux/).

## Issues and Support

*eProsima Fast DDS* developers welcome all contributions, and we will be grateful if you follow the guidelines below while contributing to this project.
Firs of all, there are several products related with Fast DDS.
Please, open the issue in the corresponding repository:

- [Fast DDS documentation](https://github.com/eProsima/Fast-DDS-docs/issues/new)
- [Fast DDS-Gen](https://github.com/eProsima/Fast-DDS-Gen/issues/new)
- [Foonathan memory vendor](https://github.com/eProsima/foonathan_memory_vendor/issues/new)
- [Fast CDR](https://github.com/eProsima/Fast-CDR/issues/new)
- [Shapes Demo](https://github.com/eProsima/ShapesDemo/issues/new)
- [ROS 2 Fast DDS RMW](https://github.com/ros2/rmw_fastrtps/issues/new)
- [Fast DDS Statistics Backend](https://github.com/eProsima/Fast-DDS-statistics-backend/issues/new)
- [Fast DDS Monitor](https://github.com/eProsima/Fast-DDS-monitor/issues/new)
- [DDS Router](https://github.com/eProsima/DDS-Router/issues/new)

Also, this project classifies user issues in the following categories.
Please, help us giving you a better support by opening your issue in the corresponding category.

### Issues

Fast DDS issues are understood as bug reports and may be opened by anyone [here](https://github.com/eProsima/Fast-DDS/issues/new/choose).
Consequently, this section handles malfunctions in the current documented behavior of the library or non-compliances to the DDS specification.
If you are unsure on whether your experienced behavior falls into this category, please open a ticket in the [Support](#support) discussion forum and, if it is a malfunction, an issue will be opened on your behalf with the provided report.

An issue template is provided and it is really appreciated if all related information is provided so the issue may be reproduced.
Otherwise, more information could be required in order to reproduce and solve the issue.
If this is the case, the issue will be labeled with `need more info`.
Please, be advised that in case that no response is received within a month, the issue would be closed due to inactivity.
The issue may be reopened if the required information is provided.

### Feature request

Feature requests and improvements suggestions should be opened in the Ideas [discussion forum](https://github.com/eProsima/Fast-DDS/discussions/new).
Please, remember to select the corresponding category while opening the discussion.
It is also encouraged to contact directly with [eProsima support team](https://github.com/eProsima/Fast-DDS#getting-help) for a feature evaluation.

### Q&A

Questions about *eProsima Fast DDS* behavior and features should be opened in the Q&A (Questions & Answers) [discussion forum](https://github.com/eProsima/Fast-DDS/discussions/new).
Please, remember to select the correct category while opening the discussion.
It is strongly advised to first consult [eProsima Fast DDS documentation](https://fast-dds.docs.eprosima.com/en/latest/) and previous Q&A in the forum.

### Support

Most user issues would fall in this category.
*eProsima Fast DDS* provides a lot of features and tuning the library for optimal behavior for each use case is not an easy task.
These issues should be opened in the Support [discussion forum](https://github.com/eProsima/Fast-DDS/discussions/new).
Please, remember to select the corresponding category while opening the discussion.
Please, take into account that *eProsima Fast DDS* provides official support for the Tier 1 platforms, architectures and compilers defined [here](https://github.com/eProsima/Fast-DDS/blob/master/PLATFORM_SUPPORT.md).
Any other support should be opened in the next section: [Unofficial support](#unofficial-support)

### Unofficial support

Any issue related with a non-officially supported platform, architecture and/or compiler should be opened in the Unofficial support [discussion forum](https://github.com/eProsima/Fast-DDS/discussions/new).
Please, remember to select the corresponding category while opening the discussion.
If official support is wanted for any platform, architecture and/or compiler, please contact directly with [eProsima support team](https://github.com/eProsima/Fast-DDS#getting-help) for an evaluation.
