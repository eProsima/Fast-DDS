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
        # Dictionary to store process information: domain -> process
        self.processes = {}
        # Dictionary to store remote connections of each domain: domain -> connection
        self.remote_connections = {}
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

    def run_process_nb(self, domain: int, command: list, easy_mode: str):
        """
        Used for starting new servers. Commands 'start' and 'auto'.
        Param @easy_mode indicates the remote_connection that should be used.
        """
        with self._lock:
            if domain in self.processes:
                if easy_mode != self.remote_connections[domain]:
                    return f"Error: DS for Domain '{domain}' already points to '{self.remote_connections[domain]}'."
                return f"Discovery server for Domain '{domain}' is already running."
            # Start a new subprocess in a non-blocking way
            command.insert(0, self.toolpath)
            process = subprocess.Popen(command,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE,
                                       preexec_fn=os.setsid)
            # Check output to see if the process started correctly or if it issued an error
            try:
                output = os.read(process.stdout.fileno(), 1024).decode('utf-8')
            except Exception as e:
                output = f"Error reading initial log: {e}"

            if 'started' in output:
                self.processes[domain] = process
                self.remote_connections[domain] = easy_mode
            else:
                # Strip ANSI colors from the error message
                stderr = os.read(process.stderr.fileno(), 1024).decode('utf-8')
                ansi_escape = re.compile(r'\x1b[^m]*m')
                stripped_output = ansi_escape.sub('', stderr)
                output = f"Error starting Server: {stripped_output}"
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

                output = result.stdout
                # When the command received matches the Command.SET value (4), the remote connection needs to be modified
                if command[1] == '4':
                    # Remote connections are always the last element of the command
                    # Cpp tool should fail if more than one argument is received
                    remote_connection = ''
                    if len(command) > 4:
                        ip_re = r"(\d{1,3}(?:\.\d{1,3}){3})"
                        match = re.search(ip_re, command[-1])
                        if match:
                            ip = match.group(1)
                            remote_connection = ip
                            output += f"Remote connection modified to {command[-1]}. Use this value for the ROS2_EASY_MODE. "
                        else:
                            output += f"Not able to modify the remote connection to {command[-1]}. "
                    self.remote_connections[domain] = remote_connection
                return output
            except subprocess.CalledProcessError as e:
                # Strip ANSI colors from the error message
                ansi_escape = re.compile(r'\x1b[^m]*m')
                stripped_output = ansi_escape.sub('', e.stderr)
                output = f"Command not executed: {stripped_output}"
                return output

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
