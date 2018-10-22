import sys
import msparser
import csv

stack = []
heap = []
msparser_data = msparser.parse_file(sys.argv[1])
for snapshot in msparser_data['snapshots']:
    if snapshot['mem_heap'] != 0:
        stack.append(snapshot['mem_stack'])
        heap.append(snapshot['mem_heap'])

with open(sys.argv[2], 'w+') as csv_file:
    csv_writer = csv.writer(csv_file, delimiter=',', quoting=csv.QUOTE_ALL)
    csv_writer.writerow(['stack', 'heap'])
    csv_writer.writerow([max(stack), max(heap)])