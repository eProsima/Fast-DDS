# **External Dependency Public API** `foonathan_memory`

This document lists the `foonathan_memory` API that *eprosima Fast DDS* uses.
This document will be updated if new features are included and used in *eprosima Fast DDS*.

## Allocators

1. Typedef [foonathan::memory::default_allocator](https://foonathan.net/memory/group__allocator.html#ga10acce2d854fc42fea7306e511d9cd10)
1. Typedef [foonathan::memory::heap_allocator](https://foonathan.net/memory/group__allocator.html#ga22bca7a15392be2aa9be773d838ec4f4)
1. Typedef [foonathan::memory::new_allocator](https://foonathan.net/memory/group__allocator.html#ga0203ba3d8ef46a65c504a6c98e3f7bb5)

## Allocator implementations

1. Class [foonathan::memory::memory_pool](https://foonathan.net/memory/classfoonathan_1_1memory_1_1memory__pool.html)
1. Struct [foonathan::memory::node_pool](https://foonathan.net/memory/classfoonathan_1_1memory_1_1memory__pool.html)

## Adapters and Wrappers

1. Class [foonathan::memory::binary_segregator](https://foonathan.net/memory/classfoonathan_1_1memory_1_1binary__segregator.html)


## Alias Templates

1. Class [foonathan::memory::unordered_map](https://foonathan.net/memory/classfoonathan_1_1memory_1_1unordered__map.html)
1. Class [foonathan::memory::map](https://foonathan.net/memory/classfoonathan_1_1memory_1_1map.html)
1. Class [foonathan::memory::set](https://foonathan.net/memory/classfoonathan_1_1memory_1_1set.html)

## Node sizes

1. Struct [foonathan::memory::map_node_size](https://foonathan.net/memory/structfoonathan_1_1memory_1_1map__node__size.html)
1. Struct [foonathan::memory::set_node_size](https://foonathan.net/memory/structfoonathan_1_1memory_1_1set__node__size.html)
1. Struct [foonathan::memory::unordered_map_node_size](https://foonathan.net/memory/structfoonathan_1_1memory_1_1unordered__map__node__size.html)

## Internal methods

When the library allocates memory blocks, some of this memory is reserved by `foonathan_memory` for internal uses and, consequently, is not available to the user.
The following methods provide information about the internal needs of `foonathan_memory`. 

1. foonathan::memory::detail::debug_fence_size
1. foonathan::memory::detail::max_alignment
1. foonathan::memory::detail::memory_block_stack::implementation_offset