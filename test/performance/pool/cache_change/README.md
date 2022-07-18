# CACHE CHANGE POOL

This directory provides a test to measure the performance of the CacheChangePool.

## Importance

The CacheChangePool is an important part of the core of Fast DDS as it is used along the whole
hot-path and it could be a great candidate for performance tuning.

## Tests done

## Measures

| Configuration | Creation | Minimum reserve | High reserve | High release | High reserve after release | Release all | Destruction |
|---------------|----------|-----------------|--------------|--------------|----------------------------|-------------|-------------|
|               |          |                 |              |              |                            |             |             |

### Local machine

* x86_64
* 64-bit
* 12 CPUs
* Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz

**Results**
