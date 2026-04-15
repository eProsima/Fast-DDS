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

## *eProsima Fast DDS Pro* currently supported versions and their support cycles

|Version|Version branch|Latest Release|Release Date|End of Standard Support Date|EOL Date|
|-------|--------------|--------------|------------|----------------------------|--------|
|3.6|[3.6.x](https://github.com/eProsima/Fast-DDS-Pro/tree/pro/3.6.x) (LTS)|[v3.6.0](https://github.com/eProsima/Fast-DDS-Pro/releases/tag/v3.6.0)|April 2026|April 2027 [^*]|April 2027 [^*]|

[^*]: Support may be extended.

A detailed view of the features of each supported version is available at [Fast DDS documentation](https://fast-dds.docs.eprosima.com/en/latest/notes/previous_versions/supported_versions.html).
