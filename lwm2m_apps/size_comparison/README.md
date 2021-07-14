# Binary Comparison application

The goal of this application is to be minimal to allow a comparison of the binary size changes
introduced by the different features.

## Usage of the automated measurement

Execute `riot_dissector.py`. It will use by default the configuration in `config.yaml`, which
indicates which application profiles to run (i.e. which configuration file should be applied on
the different builds) and how to group the generated information.

```
$ ./riot_dissector.py
INFO:root:Using cache for board iotlab-m3 with profile No extensions at /tmp/iotlab-m3_no-extensions.json
INFO:root:Using cache for board iotlab-m3 with profile Client-to-Client extension at /tmp/iotlab-m3_client-to-client-extension.json
INFO:root:Using cache for board iotlab-m3 with profile Client-to-Client and Authorization extensions at /tmp/iotlab-m3_client-to-client-and-authorization-extensions.json
INFO:root:CoAP:{'flash': [2981, 3149, 3149], 'ram': [4, 4, 4]}
INFO:root:LwM2M Core:{'flash': [11002, 11018, 11026], 'ram': [5124, 5124, 5124]}
INFO:root:Utils:{'flash': [5103, 5747, 6187], 'ram': [280, 280, 280]}
INFO:root:Connection handling:{'flash': [1741, 2711, 3027], 'ram': [3640, 3640, 3640]}
INFO:root:Security object:{'flash': [1394, 1474, 1474], 'ram': [414, 414, 414]}
INFO:root:Server object:{'flash': [1109, 1345, 1345], 'ram': [104, 272, 272]}
INFO:root:Other objects:{'flash': [2805, 2873, 2901], 'ram': [481, 521, 521]}
INFO:root:LwM2M Client to client:{'flash': [0, 1190, 1200], 'ram': [0, 0, 0]}
INFO:root:LwM2M Authorization Request:{'flash': [0, 0, 912], 'ram': [0, 0, 0]}
```

## Options

### Typical LwM2M

This mode has typical LwM2M features. No extensions. Will have DTLS support and access control.

No extra configuration is needed, `app.config` is applied automatically.



### Client-to-client extension

This mode allows client-to-client communication, but no features related to the dynamic
third party authorization. This means no request, no server hints response.

### Client-to-client with third party authorization

This is like the previous one, but implements:
- unauthorized resource request
- server hints response
- server authorization request
