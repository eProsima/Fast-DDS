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
#
# Modifications:
# - Adapted to work with Fast DDS CLI
#
# Original copyright retained below:
#
# Copyright 2021 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0

import copyreg
import os
import pickle
import platform
import socket
import subprocess
import sys
import threading

from discovery.fastdds_daemon.helpers import wait_for


class PicklerForProcess(pickle.Pickler):
    """
    Pickle objects for subprocess.

    This `pickle.Pickler` subclass sends serialized objects
    to the given process through its `stdin` pipe. It can
    serialize and send inheritable sockets and file descriptors.
    """

    def __init__(self, process, *args, **kwargs):
        """
        See `pickle.Pickler` class for further reference.

        :param process: a `subprocess.Popen` instance.
          It is assumed its `stdin` attribute is an open pipe.
        """
        super().__init__(process.stdin, *args, **kwargs)
        self.process = process
        self.dispatch_table = copyreg.dispatch_table.copy()
        self.dispatch_table[socket.socket] = self.reduce_socket
        self.dispatch_table[threading.Event] = self.reduce_event

    def reduce_event(self, obj):
        return threading.Event, ()

    @staticmethod
    def load_socket(data):
        if platform.system() == 'Windows':
            return socket.fromshare(data)
        return socket.socket(fileno=data)

    def reduce_socket(self, obj):
        if platform.system() == 'Windows':
            data = obj.share(self.process.pid)
        else:
            data = obj.fileno()
        return PicklerForProcess.load_socket, (data,)

    def dump(self, *args, **kwargs):
        super().dump(*args, **kwargs)
        self.process.stdin.flush()


def main():
    """
    Execute incoming, serialized callable.

    See `daemonize()` for further reference.
    """
    callable_ = pickle.load(sys.stdin.buffer)
    sys.stdin.close()
    os.close(0)  # force C stream close
    return callable_()


def daemonize(callable_, tags={}, timeout=None, debug=False):
    """
    Spawn a callable object as a daemon.

    The callable object is pickled and sent for execution
    via the `stdin` pipe, which is closed immediately after
    deserialization succeeds. The callable object may hold
    inheritable sockets and file descriptors.

    :param callable_: callable object to be daemonized.
    :param tags: optional key-value pairs to show up as
      command-line '--key value' arguments of the daemon
      process. Useful for identification in the OS process
      list.
    :param timeout: optional duration, in seconds, to wait
      for the daemon to be ready. Non-positive durations will
      result in an indefinite wait.
    :param debug: if `True`, the daemon process will not be
      detached and share both `stdout` and `stderr` streams
      with its parent process.
    """
    # Use daemon `main()` function
    prog = f'from {__name__} import main; main()'
    cmd = [sys.executable, '-c', prog]
    for name, value in tags.items():
        flag = '--' + name.replace('_', '-')
        cmd += [flag, str(value)]
    kwargs = {}
    if platform.system() == 'Windows':
        if not debug:
            kwargs.update(creationflags=subprocess.DETACHED_PROCESS)
        # Avoid showing cmd windows for subprocess
        si = subprocess.STARTUPINFO()
        si.dwFlags = subprocess.STARTF_USESHOWWINDOW
        si.wShowWindow = subprocess.SW_HIDE
        kwargs['startupinfo'] = si
        # Don't keep handle of current working directory in daemon process
        kwargs.update(cwd=os.environ.get('SYSTEMROOT', None))

    kwargs['stdin'] = subprocess.PIPE
    if not debug:
        kwargs['stdout'] = subprocess.DEVNULL
        kwargs['stderr'] = subprocess.DEVNULL
    # Don't close inheritable file descriptors,
    # the given callable object may be carrying
    # some.
    kwargs['close_fds'] = False

    # Spawn child process
    env = os.environ.copy()
    env['PYTHONPATH'] = ':'.join(sys.path)
    process = subprocess.Popen(cmd, env=env, **kwargs, preexec_fn=os.setsid)

    # Send serialized callable object through stdin pipe
    pickler = PicklerForProcess(process)
    pickler.dump(callable_)

    if timeout is not None:
        # Wait for daemon to be ready by
        # monitoring when the stdin pipe
        # is closed
        def daemon_ready():
            try:
                pickler.dump(None)
                return False
            except OSError:
                return True
        if not wait_for(daemon_ready, timeout):
            process.terminate()
            raise RuntimeError(
                'Timed out waiting for '
                'daemon to become ready'
            )
    # Make sure the daemon process is still alive
    # (and that the stdin pipe was purposefully closed)
    # before returning.

    if process.poll() is not None:
        rc = process.returncode
        print('Daemonize process died with return code', rc)
        raise RuntimeError(
            'Daemon process died '
            f'with returncode {rc}'
        )
    return
