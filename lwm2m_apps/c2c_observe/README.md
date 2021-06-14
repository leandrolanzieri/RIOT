# LwM2M Client-to-Client observe

This application shows how normal client-to-client communication is performed between two
LwM2M clients. Both clients have information and credentials to register to a server, using
PSK security mode. Also, they hold information and credentials about each other, to establish a
secure connection using PSK mode as well.

The application is split into client 1 and client 2. Depending on the configuration file used,
one client or the other is compiled. Configurations are chosen via the environmental variable
`KCONFIG_ADD_CONFIG=<path-to-config>`. The common configuration is placed in `app.config`, which is
applied automatically.

## Client 1 (observer)

It is meant to be the client that attempts an observation of a resource hosted by client 2.

Compile with:
```sh
env PKG_SOURCE_LOCAL_WAKAAMA="<path-to-wakaama>" KCONFIG_ADD_CONFIG="client_1.config" make clean all term
```

## Client 2 (host)

Hosts a resource that client 1 will attempt to observe. It will also have the correct access
rights instantiated to allow the observe (read) operation.

Compile with:
```sh
env PKG_SOURCE_LOCAL_WAKAAMA="<path-to-wakaama>" KCONFIG_ADD_CONFIG="client_2.config" make clean all term
```

## Usage
The there is a `lwm2m` shell command that should be used to perform all the operations.

1. `lwm2m start` will trigger the registration of the client to the server
2. `lwm2m obs` is used to request the observation of a resource, it should be used from client 2.
   For example: `lwm2m obs /3311/0/5850` will request to observe the status (resource 5850) of
   the instance 0 of the light control object (ID 3311) hosted by Client 1.
3. To only read (no observe option), alternatively use `lwm2m read`.
4. `lwm2m obj` will dump a tree representing all the objects in the client.
