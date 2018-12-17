- RDM: rdm-draft-alamos-lanzieri-runtime-configuration-architecture.md
- Title: Runtime configuration architecture
- Authors: José Álamos, Leandro Lanzieri
- Status: draft
- Type: Design
- Created: December 2018

## Abstract
This memo describes the proposed high-level architecture and
mechanisms to implement a runtime configuration system on a RIOT node.

A runtime configuration system is in charge of providing a mechanism to set and
get the values of parameters that are used during the execution of the firmware,
as well as a way to persist these values. Most of times these values are
per-node specific.

Examples of runtime configurations are:
- Transmission duty cycles
- Sensor thresholds
- Private-public keys
- System state variables

These parameters MAY have constraints, like an specific order to be applied
(due to interdependencies) or value boundaries.

The main advantages of having such a system are:
- Easy to apply per-node configurations during deployment
- No need to implement a special mechanism for per-node configurations during
  firmware updates (only in the case of migration)
- Common interface for modules to expose their runtime configuration and handle
  them
- Transparent interface for storing configuration parameters in non-volatile
  storage devices.

## Status
This document is currently under open discussion. This document is a product of
[Configuration Task
Force](https://github.com/RIOT-OS/RIOT/wiki/Configuration-Task-Force-(CTF)), and
aims to describe the architecture of a runtime configuration system. The content
of this document is licensed with a Creative Commons CC-BY-SA license.

## Terminology
This memo uses the [RFC2119](https://www.ietf.org/rfc/rfc2119.txt) terminology
and the following acronyms:

- RDM: RIOT Developer Memo
- CTF: RIOT's Configuration Task Force
- RCS: Runtime Configuration System

# 1. Introduction
This document specifies the proposed architecture by the
Configuration Task Force (CFT) to implement a secure and reliable Runtime
Configuration System (RCS), focusing in modularity, reusing of existing
technologies in RIOT (network stack, storage interface) and adhering to
standards.

# 2. Architecture
The RCS is formed by the RIOT Registry, one or more
storage facilities and a configuration manager with one or more interfaces. The
runtime configuration manager could in the future be replaced by a generic
manager.  ![](./files/rdm-draft-alamos-lanzieri-runtime-configuration-architecture/architecture.png "Runtime Configuration Architecture")

Essentially, modules register handlers for configuration group, in the RIOT
Registry, and are in charge of implementing those. This MAY include data
validation and any logic needed for applying or obtaining the parameters.

# 3. The RIOT Registry
The RIOT Registry is a module in charge of exposing an
**interface for handling persistent module configurations** (via registry handlers). The RIOT registry
allows, among others, to:
- Get or set a configuration parameter
- Commit a configuration group
- Load and set a group of configurations from a persistent storage device
- Export a set of configurations (e.g copy to a buffer, print, etc)

# 4. Storage facilities
Storage facilities SHOULD implement the **storage
interface** to allow the RIOT Registry to load, search and save configuration
parameters. From the point of view of the RIOT Registry all parameters are
key/value strings, it is responsibility of the storage facility to transform
that to the proper format for storage (e.g. lines separated by `\n` character in
a file).

If a storage implements any kind of encryption mechanism it SHOULD be
transparent to the RIOT Registry, and all that logic should be handled withing
the facility.

# 5. Configuration manager and interfaces
The configuration manager is a pseudo-module that allows a RIOT node to be
configured from one or more communication interfaces. Examples of these
communication interfaces could be UART, SPI or higher layers like PPP, IPv6,
UDP, CoAP, LWM2M, etc.

The configuration manager MAY provide an access control mechanism for
restricting access to the configurations. However, it can relay on security
implemented on the layers mentioned above (e.g CoAP and DTLS, CHAP on PPP, etc)
## Acknowledgements

## References
- [Mynewt OS config module
  documentation](https://mynewt.apache.org/latest/os/modules/config/config.html)

## Revisions
- Rev0: initial document

## Contact The authors of this memo can be contacted via email at
jose.alamos@haw-hamburg.de and leandro.lanzieri@haw-hamburg.de
