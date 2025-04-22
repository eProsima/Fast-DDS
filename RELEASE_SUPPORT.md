# Release support

Please, refer to the [master branch](https://github.com/eProsima/Fast-DDS/blob/master/RELEASE_SUPPORT.md) for the latest version of this document.

*eProsima Fast DDS* maintains several releases with different support cycles.
Major releases are scarce and they are made when public API is broken (more information about *eProsima Fast DDS* versioning can be found [here](https://github.com/eProsima/Fast-DDS/blob/master/VERSIONING.md)).
Latest major release is v3.0.0 (August 2024), which introduced the DDS-XTypes 1.3.
Refer to the [migration guide](https://github.com/eProsima/Fast-DDS/blob/master/UPGRADING.md) if looking for hints moving from Fast DDS 2.x to v3.0.0.

Each quarter, a new Fast DDS minor version with new features is released.
By default, *eProsima Fast DDS* minor releases have a lifecycle of **6 months**.
However, the life of certain branches can be extended depending on costumer and user needs.
Within the support period of any minor version, there may be several patch releases that either add new functionalities in an ABI compatible way or fix possible issues.
Before the minor release end of life (EOL) date, a final patch release including the latest fixes will be made available if there are any changes not included in any previous patch release of the same minor version.
Long Term Supported (LTS) versions may have an extended supported period for only critical issues and security fixes, where no other ABI compatible neither bug fixes will be backported.
This period applies since the end of standard support date to the end of life (EOL) date.

## *eProsima Fast DDS* currently supported versions and their support cycles

|Version|Version branch|Latest Release|Release Date|End of Standard Support Date|EOL Date|
|-------|--------------|--------------|------------|----------------------------|--------|
|3.2|[3.2.x](https://github.com/eProsima/Fast-DDS/tree/3.2.x) (LTS)|v3.2.0|March 2025|March 2026 [^*]|March 2026 [^*]|
|3.1|[3.1.x](https://github.com/eProsima/Fast-DDS/tree/3.1.x)|[v3.1.2](https://github.com/eProsima/Fast-DDS/releases/tag/v3.1.2)|October 2024|April 2025|April 2025|
|2.14|[2.14.x](https://github.com/eProsima/Fast-DDS/tree/2.14.x) (LTS)|[v2.14.4](https://github.com/eProsima/Fast-DDS/releases/tag/v2.14.4)|March 2024|March 2025 [^*]|March 2025 [^*]|
|2.10|[2.10.x](https://github.com/eProsima/Fast-DDS/tree/2.10.x) (LTS)|[v2.10.6](https://github.com/eProsima/Fast-DDS/releases/tag/v2.10.6)|March 2023|May 2025 [^*]|May 2025|
|2.6|[2.6.x](https://github.com/eProsima/Fast-DDS/tree/2.6.x) (LTS)|[v2.6.10](https://github.com/eProsima/Fast-DDS/releases/tag/v2.6.10)|March 2022|July 2024|May 2025[^*]|

[^*]: Support may be extended.

## *eProsima Fast DDS* previously supported versions.

|Version|Version branch|Latest Release|Release Date|EOL Date|
|-------|--------------|--------------|------------|--------|
|3.0|[3.0.x](https://github.com/eProsima/Fast-DDS/tree/3.0.x)|[v3.0.2](https://github.com/eProsima/Fast-DDS/releases/tag/v3.0.2)|August 2024|February 2025|
|2.13|[2.13.x](https://github.com/eProsima/Fast-DDS/tree/2.13.x)|[v2.13.6](https://github.com/eProsima/Fast-DDS/releases/tag/v2.13.6)|December 2023|July 2024|
|2.12|[2.12.x](https://github.com/eProsima/Fast-DDS/tree/2.12.x)|[v2.12.2](https://github.com/eProsima/Fast-DDS/releases/tag/v2.12.2)|September 2023|March 2024|
|2.11|[2.11.x](https://github.com/eProsima/Fast-DDS/tree/2.11.x)|[v2.11.3](https://github.com/eProsima/Fast-DDS/releases/tag/v2.11.3)|July 2023|January 2024|
|2.9|[2.9.x](https://github.com/eProsima/Fast-DDS/tree/2.9.x)|[v2.9.2](https://github.com/eProsima/Fast-DDS/releases/tag/v2.9.2)|December 2022|July 2023|
|2.8|[2.8.x](https://github.com/eProsima/Fast-DDS/tree/2.8.x)|[v2.8.2](https://github.com/eProsima/Fast-DDS/releases/tag/v2.8.2)|September 2022|March 2023|
|2.7|[2.7.x](https://github.com/eProsima/Fast-DDS/tree/2.7.x)|[v2.7.2](https://github.com/eProsima/Fast-DDS/releases/tag/v2.7.2)|July 2022|January 2023|
|2.5|[2.5.x](https://github.com/eProsima/Fast-DDS/tree/2.5.x)|[v2.5.2](https://github.com/eProsima/Fast-DDS/releases/tag/v2.5.2)|December 2021|June 2022|
|2.4|[2.4.x](https://github.com/eProsima/Fast-DDS/tree/2.4.x)|[v2.4.2](https://github.com/eProsima/Fast-DDS/releases/tag/v2.4.2)|September 2021|March 2022|
|2.3|[2.3.x](https://github.com/eProsima/Fast-DDS/tree/2.3.x)|[v2.3.6](https://github.com/eProsima/Fast-DDS/releases/tag/v2.3.6)|March 2021|November 2022|
|2.2|[2.2.x](https://github.com/eProsima/Fast-DDS/tree/2.2.x)|[v2.2.1](https://github.com/eProsima/Fast-DDS/releases/tag/v2.2.1)|January 2021|February 2022|
|2.1|[2.1.x](https://github.com/eProsima/Fast-DDS/tree/2.1.x)|[v2.1.4](https://github.com/eProsima/Fast-DDS/releases/tag/v2.1.4)|November 2020|May 2023|
|2.0|[2.0.x](https://github.com/eProsima/Fast-DDS/tree/2.0.x)|[v2.0.3](https://github.com/eProsima/Fast-DDS/releases/tag/v2.0.3)|June 2020|February 2022|
|1.10|[1.10.x](https://github.com/eProsima/Fast-DDS/tree/1.10.x)|[v1.10.1](https://github.com/eProsima/Fast-DDS/releases/tag/v1.10.1)|April 2020|February 2022|
|1.9|[1.9.x](https://github.com/eProsima/Fast-DDS/tree/1.9.x)|[v1.9.5](https://github.com/eProsima/Fast-DDS/releases/tag/v1.9.5)|August 2019|February 2022|
|1.8|[1.8.x](https://github.com/eProsima/Fast-DDS/tree/1.8.x)|[v1.8.5](https://github.com/eProsima/Fast-DDS/releases/tag/v1.8.5)|May 2019|February 2022|
|1.7|[1.7.x](https://github.com/eProsima/Fast-DDS/tree/1.7.x)|[v1.7.3](https://github.com/eProsima/Fast-DDS/releases/tag/v1.7.3)|December 2018|February 2022|
