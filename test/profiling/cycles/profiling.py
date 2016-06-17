import os, subprocess, csv, ntpath, glob

binaries = os.environ.get("PROFILING_BINS").split(';')

for binary in binaries:
    filename = ntpath.basename(binary)
    proc = subprocess.call(["valgrind", "--tool=exp-bbv","--instr-count-only=yes","--bb-out-file=bbLog_" + filename, binary, "10"])

    thread_logs = glob.glob('bbLog_*')

    thread_numbers = []
    thread_instruction_counts = []
    for thread_log in thread_logs:
        with open (thread_log, 'r') as f:
            lines = f.read().splitlines()
            thread_numbers.append(lines[2].split(' ')[-1])
            thread_instruction_counts.append(int(lines[4].split(' ')[-1]))
        os.remove(thread_log)

    total_instructions = sum(thread_instruction_counts) 
    with open('results_' + filename + '.csv','w') as results:
        writer = csv.writer(results)
        writer.writerow(['Context','Instructions'])
        writer.writerow(['Total', total_instructions])
        for number, count in zip(thread_numbers, thread_instruction_counts):
            writer.writerow(['Thread ' + number, str(count)])

