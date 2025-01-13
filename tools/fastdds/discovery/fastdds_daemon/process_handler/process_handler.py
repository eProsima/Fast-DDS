# Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

import os
import re
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
        print(f"Process Handler initialized with toolpath: {self.toolpath}")

    def __del__(self):
        # Ensure all processes are stopped when the object is deleted
        with self._lock:
            for domain in list(self.processes.keys()):
                self.stop_process(domain, 1)

    def write_file(self, path: str, content: str):
        with open(path, 'a') as f:
            f.write(content)

    def _get_signal(self, sig: int):
        if sig == 0:
            return signal.SIGINT
        elif sig == 1:
            return signal.SIGTERM
        elif sig == 2:
            return signal.SIGKILL

    def run_process_nb(self, domain: int, command: list):
        """ Used for starting new servers. Commands 'start' and 'auto'."""
        self.write_file("/tmp/server_works.txt", f"\nRunning process non-blocking")
        with self._lock:
            self.write_file("/tmp/server_works.txt", f"\nRunning process non-blocking with lock")
            if domain in self.processes:
                self.write_file("/tmp/server_works.txt", f"\nDiscovery server for Domain ID '{domain}' already running\n")
                return f"Discovery server for Domain '{domain}' is already running."
            # Start a new subprocess in a non-blocking way
            command.insert(0, self.toolpath)
            self.write_file("/tmp/server_works.txt", f"\nRunning command: {command}")
            process = subprocess.Popen(command,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE,
                                       preexec_fn=os.setsid)
            # Check output to see if the process started correctly or it issued an error
            try:
                output = os.read(process.stdout.fileno(), 1024).decode('utf-8')
            except Exception as e:
                self.write_file("/tmp/server_works.txt", f"\nError reading initial log: {e}")
                output = f"Error reading initial log: {e}"

            if 'started' in output:
                self.write_file("/tmp/server_works.txt", f"\nDiscovery Server for Domain ID '{domain}' started.\n")
                self.processes[domain] = process
            else:
                # Strip ANSI colors from the error message
                stderr = os.read(process.stderr.fileno(), 1024).decode('utf-8')
                ansi_escape = re.compile(r'\x1b[^m]*m')
                stripped_output = ansi_escape.sub('', stderr)
                self.write_file("/tmp/server_works.txt", f"\nError starting Server: {stripped_output}\n")
                output = f"Error starting Server: {stripped_output}"
            return output

    def run_process_b(self, domain: int, command: list, check_server: bool):
        """ Used for modyfing running servers. Commands 'list', 'add', 'set'."""
        self.write_file("/tmp/server_works.txt", '\nRunning process blocking')
        with self._lock:
            if check_server and domain not in self.processes:
                self.write_file("/tmp/server_works.txt", f"\nDiscovery server for Domain ID '{domain}' not running\n")
                return f"Discovery server for Domain '{domain}' is not running."

            # Run the process in a blocking way (list, add, set)
            try:
                command.insert(0, self.toolpath)
                self.write_file("/tmp/server_works.txt", f"\nRunning command: {command}")
                result = subprocess.run(command,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE,
                                        text=True,
                                        check=True)
                self.write_file("/tmp/server_works.txt", f"\nCommand output: {result.stdout}\n")
                return result.stdout
            except subprocess.CalledProcessError as e:
                self.write_file("/tmp/server_works.txt", f"\nError running command: {e}\n")
                return e.stdout

    def stop_process(self, domain: int, sig: int):
        self.write_file("/tmp/server_works.txt", '\nStopping process')
        with self._lock:
            self.write_file("/tmp/server_works.txt", '\nStopping process with lock')
            process = self.processes.get(domain)
            self.write_file("/tmp/server_works.txt", f"\n Getting process '{process.pid}'")
            for p in self.processes:
                self.write_file("/tmp/server_works.txt", f"\n Processes: {p}")
            if not process:
                self.write_file("/tmp/server_works.txt", f"\nDiscovery Server for Domain ID '{domain}' not running.\n")
                return f"Discovery Server for Domain ID '{domain}' not running."

            # Send signal to terminate the process group
            os.killpg(os.getpgid(process.pid), self._get_signal(sig))
            self.write_file("/tmp/server_works.txt", f"\n Kill signal sent to process {os.getpgid(process.pid)} with signal {self._get_signal(sig)}")
            process.wait(10)
            del self.processes[domain]
            self.write_file("/tmp/server_works.txt", f"\nDiscovery Server for Domain ID '{domain}' stopped.\n")
            return f"Discovery Server for Domain ID '{domain}' stopped."

    def list_processes(self):
        with self._lock:
            return {domain: proc.pid for domain, proc in self.processes.items()}

    def stop_all_processes(self, sig: int):
        ret = ''
        self.write_file("/tmp/server_works.txt", '\nStopping all processes')
        with self._lock:
            self.write_file("/tmp/server_works.txt", '\nStopping all processes with lock')
            len_processes = len(self.processes)
            for idx, domain in enumerate(list(self.processes.keys())):
                ret += self.stop_process(domain, sig)
                ret += '' if idx == len_processes - 1 else '\n'
        return ret
