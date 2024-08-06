# Platform Support

This document reflects the level of support offered by *eprosima Fast DDS* on different platforms as per the following
definitions:

## Tier 1

Tier 1 platforms are subjected to our unit test suite and other testing tools on a frequent basis including continuous
integration jobs, nightly jobs, packaging jobs, and performance testing.
Errors or bugs discovered in these platforms are prioritized for correction by the development team.
Significant errors discovered in Tier 1 platforms can impact release dates and we strive to resolve all known high
priority errors in Tier 1 platforms prior to new version releases.

## Tier 2

Tier 2 platforms are subject to periodic CI testing which runs both builds and tests with publicly accessible results.
The CI is expected to be run at least within a week of relevant changes for the current release of *Fast DDS*.
Installation instructions should be available and up-to-date in order for a platform to be listed in this category.
Package-level binary packages may not be provided but providing a downloadable archive of the built workspace is
encouraged.
Errors may be present in released product versions for Tier 2 platforms.
Known errors in Tier 2 platforms will be addressed subject to resource availability on a best effort basis and may or
may not be corrected prior to new version releases.
One or more entities should be committed to continuing support of the platform.

## Tier 3

Tier 3 platforms are those for which community reports indicate that the release is functional.
The development team does not run the unit test suite or perform any other tests on platforms in Tier 3.
Community members may provide assistance with these platforms.

## Platforms

|Architecture|Ubuntu Noble (24.04)|Ubuntu Jammy (22.04)|MacOS Mojave (10.14)|Windows 10 (VS2019)|Windows 11 (VS2022)|Debian Buster (10)|Android 12 |Android 13 | QNX 7.1   |
|------------|--------------------|--------------------|--------------------|-------------------|-------------------|------------------|-----------|-----------|-----------|
|amd64       |Tier 3 [^a][^s]     |Tier 1 [^a][^s]     |Tier 1 [^s]         |Tier 1 [^a][^s]    |Tier 3 [^a][^s]    |Tier 3 [^s]       |Tier 3 [^s]|Tier 3 [^s]|Tier 3 [^s]|
|amd32       |                    |                    |Tier 3 [^a][^s]    |Tier 3 [^a][^s]    |                  |           |           |           |
|arm64       |Tier 3 [^a][^s]     |Tier 1 [^a][^s]     |                    |                   |                   |Tier 3 [^s]       |Tier 3 [^s]|Tier 3 [^s]|Tier 3 [^s]|
|arm32       |                    |Tier 3 [^s]         |                    |                   |                   |Tier 3 [^s]       |Tier 3 [^s]|Tier 3 [^s]|           |

[^a]: Binary releases are provided as a single archive per platform.
[^s]: Compilation from source.

## Compilers

Tier 1 compilers and minimum supported versions:

* GCC 11.4 [^d]
* Clang 15
* MSVC v142 (Visual Studio 2019)

[^d]: Using GCC's Thread Sanitizer flags in conjunction with GCC 11 to analyze Fast DDS threaded behavior produces
false positives on some uninstrumented synchronization calls.
Fast DDS's Thread Sanitizer Github Action uses GCC 12 for threading issues testing.
