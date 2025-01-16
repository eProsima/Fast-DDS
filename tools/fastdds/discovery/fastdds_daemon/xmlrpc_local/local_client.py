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

import discovery.fastdds_daemon.daemon as daemon
import xmlrpc.client

def run_request_nb(domain, command, remote) -> str:
    server_url = daemon.get_xmlrpc_server_url()
    with xmlrpc.client.ServerProxy(server_url) as proxy:
        return(proxy.run_process_nb(domain, command, remote))

def run_request_b(domain, command, check_server) -> str:
    server_url = daemon.get_xmlrpc_server_url()
    with xmlrpc.client.ServerProxy(server_url) as proxy:
        return(proxy.run_process_b(domain, command, check_server))

def stop_request(domain, signal) -> str:
    server_url = daemon.get_xmlrpc_server_url()
    with xmlrpc.client.ServerProxy(server_url) as proxy:
        return(proxy.stop_process(domain, signal))

def stop_all_request(signal) -> str:
    server_url = daemon.get_xmlrpc_server_url()
    with xmlrpc.client.ServerProxy(server_url) as proxy:
        return(proxy.stop_all_processes(signal))
