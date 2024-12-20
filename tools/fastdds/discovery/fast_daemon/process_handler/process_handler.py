import os
import select
import signal
import subprocess
import threading

class ProcessHandler:
    def __init__(self, toolpath: str):
        self.processes = {}  # Dictionary to store process information
        # Path to the tool to be executed. It is stored in the object to avoid
        # passing it as an argument in every call and to enable daemon calls between
        # host and docker containers with --net=host option.
        self.toolpath = toolpath
        self._lock = threading.RLock()

    def __del__(self):
        # Ensure all processes are stopped when the object is deleted
        with self._lock:
            for domain in list(self.processes.keys()):
                self.stop_process(domain, 1)

    def _get_signal(self, sig: int):
        if sig == 0:
            return signal.SIGINT
        elif sig == 1:
            return signal.SIGTERM
        elif sig == 2:
            return signal.SIGKILL

    def run_process_nb(self, domain: int, command: list):
        """ Used for starting new servers. Commands 'start' and 'auto'."""
        with self._lock:
            if domain in self.processes:
                return f"Discovery server for Domain '{domain}' is already running."
            # Start a new subprocess in a non-blocking way
            command.insert(0, self.toolpath)
            process = subprocess.Popen(command,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE,
                                       preexec_fn=os.setsid)
            # Check output to see if the process started correctly or it issued an error
            try:
                streams, _, _ = select.select([process.stdout, process.stderr], [], [], 0.3)
                output = ''
                if streams:
                    for stream in streams:
                        output = os.read(stream.fileno(), 1024).decode('utf-8')
            except Exception as e:
                output = f"Error reading initial log: {e}"

            if 'started' in output:
                self.processes[domain] = process
            else:
                output = f"Error starting Server: {output}"
            return output

    def run_process_b(self, domain: int, command: list, check_server: bool):
        """ Used for modyfing running servers. Commands 'list', 'add', 'set'."""
        with self._lock:
            if check_server and domain not in self.processes:
                return f"Discovery server for Domain '{domain}' is not running."

            # Run the process in a blocking way (list, add, set)
            try:
                command.insert(0, self.toolpath)
                result = subprocess.run(command,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE,
                                        text=True,
                                        check=True)
                return result.stdout
            except subprocess.CalledProcessError as e:
                return e.stdout

    def stop_process(self, domain: int, sig: int):
        with self._lock:
            process = self.processes.get(domain)
            if not process:
                return f"Discovery Server for Domain ID '{domain}' not running."

            # Send signal to terminate the process group
            os.killpg(os.getpgid(process.pid), self._get_signal(sig))
            process.communicate(5)
            del self.processes[domain]
            return f"Discovery Server for Domain ID '{domain}' stopped."

    def list_processes(self):
        with self._lock:
            return {domain: proc.pid for domain, proc in self.processes.items()}

    def stop_all_processes(self, sig: int):
        ret = ''
        with self._lock:
            len_processes = len(self.processes)
            for idx, domain in enumerate(list(self.processes.keys())):
                ret += self.stop_process(domain, sig)
                ret += '' if idx == len_processes - 1 else '\n'
        return ret
