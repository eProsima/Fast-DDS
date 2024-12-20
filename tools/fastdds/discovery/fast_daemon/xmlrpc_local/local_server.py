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
