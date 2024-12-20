import discovery.fast_daemon.daemon as daemon
import xmlrpc.client

def run_request_nb(domain, command) -> str:
    server_url = daemon.get_xmlrpc_server_url()
    with xmlrpc.client.ServerProxy(server_url) as proxy:
        return(proxy.run_process_nb(domain, command))

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
