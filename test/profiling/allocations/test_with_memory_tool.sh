#!/usr/bin/env sh

## Copyright 2018-2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
# This script launches the allocation test example under the interpose library of
# osrf_testing_tools for one qos profile received as argument:
#   
#   tl_be: transient-local best-effort" << std::endl
#   tl_re: transient-local reliable" << std::endl
#   vo_be: volatile best-effort" << std::endl
#   vo_re: volatile reliable" << std::endl;
# 
# Following files will be generated:
#   alloc_test_publisher_$1.csv    Allocations count summary for publisher
#   alloc_test_subscriber_$1.csv   Allocations count summary for subscriber
############################################################################################

LD_PRELOAD=/usr/local/lib/libmemory_tools_interpose.so:/usr/local/lib/libmemory_tools.so ./AllocationTest publisher $1 true &
sleep 1
./AllocationTest subscriber $1

sleep 5

LD_PRELOAD=/usr/local/lib/libmemory_tools_interpose.so:/usr/local/lib/libmemory_tools.so ./AllocationTest subscriber $1 true &
sleep 1
./AllocationTest publisher $1
