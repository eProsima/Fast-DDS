# FAST DDS SECURITY RESOURCES

This directory contains several sample files needed to implement secure **TLS over TCP** communication.
These files are required to configure the TCP transport protocol with TLS in Fast DDS.

> :warning: Do not use these files in a real scenario. Generate your own certificates and parameters.

## COMMANDS

Following are the commands used to generate this example's keys and certificates

### Certification Authority (CA)

```sh
# Generate the Certificate Authority (CA) Private Key > ca.key
openssl ecparam -name prime256v1 -genkey -noout -out ca.key
# openssl ecparam -name prime256v1 -genkey | openssl ec -aes256 -out ca.key -passout pass:cakey # with password

# Generate the Certificate Authority Certificate > ca.crt
openssl req -new -x509 -sha256 -key ca.key -out ca.crt -days 1825 -config ca.cnf
# openssl req -new -x509 -sha256 -key ca.key -out ca.crt -days 1825 -config ca.cnf -passin pass:cakey # with password
```

### Fast DDS Certificate

```sh
# Generate the Fast DDS Certificate Private Key > fastdds.key
openssl ecparam -name prime256v1 -genkey -noout -out fastdds.key
# openssl ecparam -name prime256v1 -genkey | openssl ec -aes256 -out fastdds.key -passout pass:fastddspwd # with password

# Generate the Fast DDS Certificate Signing Request  > fastdds.csr
openssl req -new -sha256 -key fastdds.key -out fastdds.csr -config fastdds.cnf
# openssl req -new -sha256 -key fastdds.key -out fastdds.csr -config fastdds.cnf -passin pass:fastddspwd # with password

# Generate the Fast DDS Certificate (computed on the CA side) > fastdds.crt
openssl x509 -req -in fastdds.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out fastdds.crt -days 1825 -sha256
# openssl x509 -req -in fastdds.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out fastdds.crt -days 1825 -sha256 -passin pass:cakey # with password
```

### DH PARAMETERS

```sh
# Generate the Diffie-Hellman (DF) parameters to define how OpenSSL performs the DF key-exchange > dh_params.pem
openssl dhparam -out dh_params.pem 2048
```

## Use

```cpp
TCPv4TransportDescriptor recvDescriptor;
recvDescriptor.apply_security = true;
recvDescriptor.tls_config.password = "fastdds";
recvDescriptor.tls_config.cert_chain_file = "fastdds.crt";
recvDescriptor.tls_config.private_key_file = "fastdds.key";
recvDescriptor.tls_config.tmp_dh_file = "dh2048.pem";

TCPv4TransportDescriptor sendDescriptor;
sendDescriptor.apply_security = true;
sendDescriptor.tls_config.verify_file = "ca.crt";
```
