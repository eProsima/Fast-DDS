import os, subprocess, csv, ntpath

binaries = os.environ.get("PROFILING_BINS").split(';')

for binary in binaries:
    filename = ntpath.basename(binary)
    print (binary)
    print (filename)
    proc = subprocess.call(["valgrind", "--tool=massif","--time-unit=ms","--massif-out-file=massifLog_" + filename, binary,"100"])

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
