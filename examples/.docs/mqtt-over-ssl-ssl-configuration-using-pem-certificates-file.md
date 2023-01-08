# MQTT over SSL - using Self-signed PEM certificates file

1. ThingsBoard CE/PE SSL configuration using PEM certificates file.

   Refer [here](https://thingsboard.io/docs/user-guide/mqtt-over-ssl/#ssl-configuration-using-pem-certificates-file)

   Configure the following environment variables via [configuration](https://thingsboard.io/docs/user-guide/install/config/) file, docker-compose or kubernetes scripts. We will use thingsboard.conf for example:

   If ThingsBoard is installed on Linux as a monolithic application, you may specify the environment variables in the thingsboard.conf file:

   ```bash
   sudo nano /usr/share/thingsboard/conf/thingsboard.conf
   ```

   ```bash
   ...
   export MQTT_SSL_ENABLED=true
   export MQTT_SSL_CREDENTIALS_TYPE=PEM
   export MQTT_SSL_PEM_CERT=server.pem
   export MQTT_SSL_PEM_KEY=server_key.pem
   export MQTT_SSL_PEM_KEY_PASSWORD=secret
   ...
   ```

   where:

   * **MQTT_SSL_ENABLED** - Enable/disable SSL support;
   * **MQTT_SSL_CREDENTIALS_TYPE** - Server credentials type. PEM - pem certificate file; KEYSTORE - java keystore;
   * **MQTT_SSL_PEM_CERT** - Path to the server certificate file. Holds server certificate or certificate chain, may also include server private key;
   * **MQTT_SSL_PEM_KEY** - Path to the server certificate private key file. Optional by default. Required if the private key is not present in server certificate file;
   * **MQTT_SSL_PEM_KEY_PASSWORD** - Optional server certificate private key password.
   * After completing the setup, start or restart the ThingsBoard server.

1. Self-signed certificates generation - PEM certificate file

   See [PEM certificate file](https://thingsboard.io/docs/user-guide/mqtt-over-ssl/#pem-certificate-file)

   Use instructions below to generate your own certificate files. Useful for tests, but time consuming and not recommended for production.

   Note This step requires Linux based OS with openssl installed.

   To generate a server self-signed PEM certificate and private key, use the following command:

   ```bash
   openssl ecparam -out server_key.pem -name secp256r1 -genkey
   openssl req -new -key server_key.pem -x509 -nodes -days 365 -out server.pem 
   ```

   You can also add -nodes (short for no DES) if you don’t want to protect your private key with a passphrase. Otherwise, it will prompt you for “at least a 4 character” password.

   The **days** parameter (365) you can replace with any number to affect the expiration date. It will then prompt you for things like “Country Name”, but you can just hit Enter and accept the defaults.

   Add -subj `/CN=localhost` to suppress questions about the contents of the certificate (replace localhost with your desired domain).

   Self-signed certificates are not validated with any third party unless you import them to the browsers previously. If you need more security, you should use a certificate signed by a certificate authority (CA).

   Make sure the certificate files are reachable by ThingsBoard process:

   * Linux: use `/etc/thingsboard/conf` folder. Make sure the files - server_key.pem & server.pem have same permissions as thingsboard.conf; Use relative file path, e.g. keystore.p12;

1. Copy `server.pem` and rename it to `main\mqtt_thingsboard_server_cert.pem`.
