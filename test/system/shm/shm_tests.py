from subprocess import Popen, PIPE
from time import sleep
from signal import SIGINT
from threading  import Thread
from queue import Queue, Empty

def enqueue_output(out, queue):
    for line in iter(out.readline, b''):
        print(line)
        queue.put(line)
    out.close()

if __name__ == "__main__":
    
    MAX_ITERATIONS = 25
    TIMEOUT = 10

    print("Launching subscriber process")
    subscriber_process = Popen(['./SHMRecoveryTest','subscriber', '--transport=shm'], stdout=PIPE, universal_newlines=True)
    subscriber_queue = Queue()
    subscriber_thread = Thread(target=enqueue_output, args=[subscriber_process.stdout, subscriber_queue])
    subscriber_thread.daemon = True
    subscriber_thread.start()
    current_iteration = 0; 

    while (current_iteration <= MAX_ITERATIONS):
        current_iteration += 1
        print("Launching iteration number " + str(current_iteration))
        publisher_process = Popen(['./SHMRecoveryTest','publisher', '--transport=shm'], stdout=PIPE, universal_newlines=True)
        elapsed_cycles = 0
        matched = False
        while (elapsed_cycles <= TIMEOUT and not matched):
            try:
                subscriber_output = subscriber_queue.get_nowait()
            except Empty:
                elapsed_cycles+=1
                sleep(1)
            else:
                elapsed_cycles = 0
                if (" matched" in subscriber_output):
                    matched = True
                    publisher_process.wait()

        # Waited 40 seconds with no match. Test failed.
        if (elapsed_cycles >= TIMEOUT):
            exit(1)
    # Completed all iterations. Test succeded.
    subscriber_process.send_signal(SIGINT)
    exit(0)
        

    