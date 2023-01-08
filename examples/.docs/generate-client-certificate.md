# Generate Client certificate

Refer [here](https://thingsboard.io/docs/user-guide/certificates/)

1. Generate Client certificate

   Use the following command to generate the self-signed private key and x509 certificate. The command is based on the openssl tool which is most likely already installed on your workstation:

   To generate the RSA based key and certificate, use:

   ```bash
   openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 365 -nodes
   ```

   The output of the command will be a private key file `key.pem` and a public certificate `cert.pem`. We will use them in next steps.

1. Rename `cert.pem` to `client.crt`; Copy it to `main\client.crt` in your ESP32 source code;

1. Rename `key.pem` to `client.key`; Copy it to `main\client.key` in your ESP32 source code;

