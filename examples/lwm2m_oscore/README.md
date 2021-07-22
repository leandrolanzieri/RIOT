# LwM2M example client using OSCORE

This application starts a
[LwM2M](https://wiki.openmobilealliance.org/display/TOOL/What+is+LwM2M) client
on the node with instances of the following objects:
- [Security object](http://www.openmobilealliance.org/tech/profiles/LWM2M_Security-v1_0.xml)
- [Server object](http://www.openmobilealliance.org/tech/profiles/LWM2M_Server-v1_0.xml)
- [Device object](http://www.openmobilealliance.org/tech/profiles/LWM2M_Device-v1_0_3.xml)
- [OSCORE object](https://raw.githubusercontent.com/OpenMobileAlliance/lwm2m-registry/prod/21.xml)

The application is based on the Eclipse Wakaama
[example client](https://github.com/eclipse/wakaama/tree/master/examples/client).

## Usage

### Setting up a LwM2M Test Server
This has been tested against the
[`oscore` branch of the Leshan server](https://github.com/eclipse/leshan/tree/oscore).

There is currently some issue with matching IPv6 addresses to the proper OSCORE security context,
which can be patched with the following:

```diff
diff --git a/leshan-server-cf/src/main/java/org/eclipse/leshan/server/californium/registration/RegisterResource.java b/leshan-server-cf/src/main/java/org/eclipse/leshan/server/californium/registration/RegisterResource.java
index 7ce55a9d..616ac5cd 100644
--- a/leshan-server-cf/src/main/java/org/eclipse/leshan/server/californium/registration/RegisterResource.java
+++ b/leshan-server-cf/src/main/java/org/eclipse/leshan/server/californium/registration/RegisterResource.java
@@ -18,6 +18,8 @@ package org.eclipse.leshan.server.californium.registration;
 
 import static org.eclipse.leshan.core.californium.ResponseCodeUtil.toCoapResponseCode;
 
+import java.net.URI;
+import java.net.URISyntaxException;
 import java.util.EnumSet;
 import java.util.HashMap;
 import java.util.List;
@@ -136,9 +138,10 @@ public class RegisterResource extends LwM2mCoapResource {
             OSCoreCtx clientCtx = db.getContext(exchange.advanced().getCryptographicContextID());
 
             try {
-                db.addContext(request.getScheme() + "://"
-                        + request.getSourceContext().getPeerAddress().getHostString().toString(), clientCtx);
-            } catch (OSException e) {
+                URI clientUri = new URI(request.getScheme(),
+                        request.getSourceContext().getPeerAddress().getHostString(), "", "");
+                db.addContext(clientUri.toString(), clientCtx);
+            } catch (OSException | URISyntaxException e) {
                 LOG.error("Failed to update OSCORE Context for registering client.", request, e);
             }
         }
```

Build the project with `mvn clean install  -DskipTests` and run the server demo, it should start
a webserver on `localhost:8080`. A security context should be registered for the client, in the
"Security" section. Choose `riot` as Client endpoint, and OSCORE in the security mode.
Add the credentials in hexadecimal, keeping in mind that (at the time of writing) sender ID and
recipient ID are not swapped in relation to the client, so what is configured as "Sender ID" in RIOT
should be "Sender ID" in Leshan as well.

The security credentials seem not to be persisted in the server, so whenever you start it, delete
the node security context and create it again.

### Running the client
The server address is set by the application, during the instantiation of the Security object.
It can be set via `menuconfig` or the environmental variable `LWM2M_SERVER_URI`. It should be
reachable from the node, e.g. either running on native with a tap interface or as a mote connected
to a
[border router](https://github.com/RIOT-OS/RIOT/tree/master/examples/gnrc_border_router).

#### Configure, compile and run

The Wakaama package can be configured via Kconfig. Its options are placed
under `Packages > Configure Wakaama LwM2M`. There is also an application-specific configuration
menu. There the Server URI and OSCORE credentials can be set. To access the configuration interface
run:
```
make menuconfig
```

For debugging purposes there are two types of messages that can be enabled:
- The lwm2m client adaptation debug can be enabled by setting `ENABLE_DEBUG` in
  `lwm2m_client.c` and `lwm2m_client_connection.c` to 1
- The wakaama internal logging can be enabled by setting
  `CONFIG_LWM2M_WITH_LOGS`, either on `menuconfig` or by adding it to CFLAGS
  (`CFLAGS += -DCONFIG_LWM2M_WITH_LOGS`).

For memory allocation the TLSF package is being used, with a private heap. If
memory usage has to be tweaked the heap size can be modified via the option
`CONFIG_LWM2M_TLSF_BUFFER`.

To compile run:

```shell
BOARD=<board> make all flash term
```

#### Limitations

For now we are using uoscore package to provide OSCORE functionalities. There seems to be an issue
with the parsing of CoAP queries, so using a long endpoint name does not work. The application has
configured the value `riot`. The library has not implemented replay protection nor the appendix B.2
functionality yet.

#### Shell commands
- `lwm2m start`: Starts the LwM2M by configuring the module and registering to
  the server.
