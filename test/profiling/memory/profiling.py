# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os, subprocess, csv, ntpath

binaries = os.environ.get("PROFILING_BINS").split(';')
valgrind = os.environ.get("VALGRIND_BIN")

for binary in binaries:
    filename = ntpath.basename(binary)
    print (binary)
    print (filename)
    proc = subprocess.call([valgrind, "--tool=massif","--time-unit=ms","--massif-out-file=massifLog_" + filename, binary,"100"])

    heap_values = []
    heap_tree_tags = []
    times = []

    with open('massifLog_' + filename,'r') as log:
        lines = log.read().splitlines()
        for line in lines:
            tokens = line.split('=')
            if tokens[0] == "mem_heap_B":
                heap_values.append(tokens[1])
            if tokens[0] == "heap_tree":
                heap_tree_tags.append(tokens[1])
            if tokens[0] == "time":
                times.append(tokens[1])

    with open('results_' + filename + '.csv','w') as results:
        writer = csv.writer(results)
        writer.writerow(['Type','Time(ms)','Heap(b)'])
        for snapshot in zip(times,heap_values,heap_tree_tags):
            if snapshot[2] == "peak":
                writer.writerow(['Peak', snapshot[0], snapshot[1]])

        midpoint = int(times[-1])/2
        tenth = int(times[-1])/10
        mid_range_heap_values = []
        for snapshot in zip(times,heap_values,heap_tree_tags):
            added_beginning_of_range = False
            if (int(snapshot[1]) >= (midpoint - tenth) and int(snapshot[1]) <= (midpoint + tenth)):
                mid_range_heap_values.append(snapshot[2])

        if len(mid_range_heap_values) < 2:
            writer.writerow(['Mid Range Growth',2*tenth,0])
        else:
            writer.writerow(['Mid Range Growth',2*tenth,mid_range_heap_values[-1] - mid_range_heap_values[0]])
