#!/usr/bin/env sh

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

# Concatenate all arguments using '_' as separator
old="$IFS"
IFS='_'
str="$*"
IFS=$old

# Run 'AllocationTest' under callgrind using the correct parameters:
#   --dump-before          to force dump when callgrind_dump() is called
#   --zero-before          to reset the counters when callgrind_zero_count() is called
#   --callgrind-out-file   to include the arguments on the name of the generated files
valgrind --tool=callgrind --dump-before=*callgrind_dump* --zero-before=*callgrind_zero_count* --callgrind-out-file=callgrind.out.$str ./AllocationTest $* true

# Remove the base dump file, as we only want the explicitly created dumps
rm callgrind.out.$str

