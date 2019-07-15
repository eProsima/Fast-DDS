#!/bin/sh

## Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http:##www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.

############################################################################################
# This script launches the allocation test example under callgrind, causing the generation 
# of several callgraph files:
#   - callgrind.out.args.1   events during endpoint matching
#   - callgrind.out.args.2   events while transmitting first sample
#   - callgrind.out.args.3   events while transmitting the rest of samples
#   - callgrind.out.args.4   events during endpoint unmatching
############################################################################################

# Run 'AllocationTest' using the memory_tools_interpose library:
LD_PRELOAD=/usr/local/lib/libmemory_tools_interpose.so:/usr/local/lib/libmemory_tools.so ./AllocationTest $* true
