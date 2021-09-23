
# XTYPES EXAMPLE

This Fast DDS example

## Installation

This example requires an installation of the `xtypes` eProsima library <https://github.com/eProsima/xtypes>

Download the xTypes repository using the following command:

```sh
git clone https://github.com/eProsima/xtypes.git --recursive
```

If this repository is downloaded in `src` with the other Fast DDS dependencies, using colcon to compile Fast DDS
will automatically compile and install xtypes.
(Add it to fastrtps.repos in order to download it as another dependency)

If this repository is installed independetly to Fast DDS, make sure its -config.cmake file is inside the system `PATH`.

## Run

To launch this test open two different consoles:

```sh
./DDSHelloWorldExample publisher # DDSHelloWorldExample.exe publisher on windows
```

```sh
./DDSHelloWorldExample subscriber # DDSHelloWorldExample.exe subscriber on windows
```

The Publisher and Subscriber will use xTypes to dynamically create the type using the `xtypesExample.idl` file.
