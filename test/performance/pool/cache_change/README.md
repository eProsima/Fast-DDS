# CACHE CHANGE POOL

This directory provides a test to measure the performance of the CacheChangePool.

## Importance

The CacheChangePool is an important part of the core of Fast DDS as it is used along the whole
hot-path and it could be a great candidate for performance tuning.

## Tests

This test compares different configurations and how much time it takes to different operations over the CacheChangePool.
It creates a Pool, reserver different number of changes and release them
to obtain the time relative to add less or more numbers of changes, and release them.

### Values

The `low_reserve` is to reserve `1` CacheChange.
The `high_reserve` is to reserve `2048` CacheChanges.

### Test definition

The test is composed of the following steps:

- **creation**
  - Time spent in creating the Pool
- **low_reserve**
  - Time spent in reserving a low amount of CacheChanges
- **high_reserve**
  - Time spent in reserving a high amount of CacheChanges
- **high_release**
  - Time spent in releasing a high amount of CacheChanges
- **high_reserve_after_release**
  - Time spent in reserving a high amount of CacheChanges after releasing them
- **all_release**
  - Time spent in releasing all CacheChanges (high and low amount)
- **destruction**
  - Time spent in destroying the Pool

### Configurations

- **preallocated initialized**
  - PREALLOCATED_MEMORY_MODE
  - initialized to maximum size
- **preallocated not initialized**
  - PREALLOCATED_MEMORY_MODE
  - initialized to 0
- **dynamic reserve**
  - DYNAMIC_RESERVE_MEMORY_MODE
- **dynamic reuse**
  - DYNAMIC_REUSABLE_MEMORY_MODE

### Local machine

The results in files `old_cache_change_pool_performance_results.csv` and `new_cache_change_pool_performance_results.csv`
are calculated in a machine with this capabilities:

- 20.04.1-Ubuntu
- x86_64
- 64-bit
- 12 CPUs
- Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz
- Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz
