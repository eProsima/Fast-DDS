# TCP HELLO WORLD

## How to run it

To launch this test open two different consoles:

In the first one launch: ./DDSHelloWorldExampleTCP publisher (or DDSHelloWorldExampleTCP.exe publisher on windows).
In the second one: ./DDSHelloWorldExampleTCP subscriber (or DDSHelloWorldExampleTCP.exe subscriber on windows).

This example includes additional options to show the capabilities of the TCP Transport on Fast DDS,
such as WAN and TLS. In this example the publisher will work as a TCP server and the subscriber as a
TCP client.

## Arguments

First argument is `publisher` or `subscriber` and then the rest of arguments are read unordered

```sh
Usage: DDSHelloWorldExampleTCP <publisher|subscriber>

General options:
  -h            --help              Produce help message.
  -t            --tls               Use TLS.

Publisher options:
  -s <num>,     --samples=<num>     Number of samples (0, default, infinite).
  -i <num>,     --interval=<num>    Time between samples in milliseconds
                                    (Default: 100).
  -a <address>, --address=<address> Public IP Address of the publisher (Default: None).
  -p <num>,     --port=<num>        Physical Port to listening incoming
                                    connections (Default: 5100).

Subscriber options:
  -a <address>, --address=<address> IP Address of the publisher (Default: 127.0.0.1).
  -p <num>,     --port=<num>        Physical Port where the publisher is
                                    listening for connections (Default: 5100).
```

## WAN Example

```sh
# Public WAN address and port of this host (port must be open in router)
DDSHelloWorldExampleTCP publisher -a 80.88.150.120 -p 5500

# Public WAN address and port of the publisher
DDSHelloWorldExampleTCP subscriber -a 80.88.150.120 -p 5500
```

## TLS Example

```sh
# Generate CA certificate
openssl ecparam -name prime256v1 -genkey | openssl ec -aes256 -out cakey.pem -passout pass:cakey # Generate CA private key
openssl req -new -x509 -sha256 -key cakey.pem -out cacert.pem -days 3650 -config ca.cnf -passin pass:cakey # Generate CA certificate

# Generate server certificate
openssl ecparam -name prime256v1 -genkey | openssl ec -aes256 -out serverkey.pem -passout pass:test # Generate server private key
openssl req -new -sha256 -key serverkey.pem -out server.csr -config server.cnf -passin pass:test # Generate server certificate request
openssl x509 -req -in server.csr -CA cacert.pem -CAkey cakey.pem -CAcreateserial -out servercert.pem -days 1000 -sha256 -passin pass:cakey # Generate signed server certiticate
openssl dhparam -out dh2048.pem 2048 # Generate Diffie-Hellman parameters

# Launch in localhost
DDSHelloWorldExampleTCP publisher -t
DDSHelloWorldExampleTCP subscriber -t
```
