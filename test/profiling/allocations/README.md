## Requisites

This test uses [OSRF testing tools v1.4.0](https://github.com/osrf/osrf_testing_tools_cpp/tree/1.4.0) memory_tools package.
OSRF testing tools should be installed before building this test.

## Usage

Note: This test needs the xml file stored in the sources directory.
You need to copy this file to the folder where you will execute the test executable.
The CMakeList.txt file also copies this file to the folder used to build the test.

To launch this test open two different consoles:

First launch the entity being profiled (publisher or subscriber) with
```
LD_PRELOAD=/usr/local/lib/libmemory_tools_interpose.so:/usr/local/lib/libmemory_tools.so ./AllocationTest <entity> <profile> true
```

Then launch its counterpart with
```
./AllocationTest <entity> <profile>
```

### Arguments

```
./AllocationTest <entity> [profile] [wait_unmatch]
```

First argument is mandatory and should have the value `publisher` or `subscriber` indicating the kind of entity to
profile.

Second argument is optional, defaults to `tl_be` and indicates the kind of qos to load from the XML file.

|         ||
|---------|-----------------------------|
| `tl_be` | transient-local best-effort |
| `tl_re` | transient-local reliable    |
| `vo_be` | volatile best-effort        |
| `vo_re` | volatile reliable           |

Third argument is optional, defaults to false, and indicates whether the test should wait for unmatching or not.

### Result

This test generates a CSV file containing the number of allocations and deallocations in each phase.
The name of the CSV file follows next rule:

```
alloc_test_<entity>_<profile>.csv
```

## Generating plot

This test comes with a python script which shows in a plot the allocations registered in a CSV file.

```
python3 allocation_plot.py <csv file> [0,1,2,3]
```

The last argument is the phases you want to see in the plot.
