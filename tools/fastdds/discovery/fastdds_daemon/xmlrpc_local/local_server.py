# Copyright 2020 Open Source Robotics Foundation, Inc.
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

import socket
import struct
# Import SimpleXMLRPCRequestHandler to re-export it.
from xmlrpc.server import SimpleXMLRPCRequestHandler  # noqa
from xmlrpc.server import SimpleXMLRPCServer

import psutil


def get_local_ipaddrs():
    return [
        addr.address
        for _, addrs in psutil.net_if_addrs().items()
        for addr in addrs
        if addr.family == socket.AF_INET
    ]


class LocalXMLRPCServer(SimpleXMLRPCServer):

    allow_reuse_address = False

    def server_bind(self):
        # Prevent listening socket from lingering in TIME_WAIT state after close()
        self.socket.setsockopt(
            socket.SOL_SOCKET, socket.SO_LINGER, struct.pack('ii', 1, 0))
        super(LocalXMLRPCServer, self).server_bind()

    def get_request(self):
        # Prevent accepted socket from lingering in TIME_WAIT state after close()
        sock, addr = super(LocalXMLRPCServer, self).get_request()
        sock.setsockopt(
            socket.SOL_SOCKET, socket.SO_LINGER, struct.pack('ii', 1, 0))
        return sock, addr

    def verify_request(self, request, client_address):
        if client_address[0] not in get_local_ipaddrs():
            return False
        return super(LocalXMLRPCServer, self).verify_request(request, client_address)
