# An ARMv8 Virtual Platform (AVP64)

[![cmake](https://github.com/aut0/avp64/actions/workflows/cmake.yml/badge.svg?event=push)](https://github.com/aut0/avp64/actions/workflows/cmake.yml)
[![nightly](https://github.com/aut0/avp64/actions/workflows/nightly.yml/badge.svg?event=schedule)](https://github.com/aut0/avp64/actions/workflows/nightly.yml)
[![lint](https://github.com/aut0/avp64/actions/workflows/lint.yml/badge.svg?event=push)](https://github.com/aut0/avp64/actions/workflows/lint.yml)
[![style](https://github.com/aut0/avp64/actions/workflows/style.yml/badge.svg?event=push)](https://github.com/aut0/avp64/actions/workflows/style.yml)

This repository contains an ARMv8 multicore virtual platform.
It was built at the [Institute for Communication Technologies and Embedded Systems at RTWH Aachen University](https://www.ice.rwth-aachen.de/).
The following target software configurations were tested (see [avp64-sw](https://github.com/aut0/avp64_sw)):

- CoreMark
- Dhrystone
- Whetstone
- STREAM
- Linux single-, dual-, quad-, octa-core
- Xen single- and dual-core
- Zephyr application single-, dual-, quand-, octa-core

----

## Build & Installation

This project has a [GitHub action](.github/workflows/cmake.yml) that automatically builds the project and runs the tests.
The action contains all steps that are needed to compile the project on Ubuntu 22.04 and 24.04 for x86 and ARM based host machines.
It can be used as a guideline.
The needed steps are explained below:

1. Install the needed dependencies

    Ubuntu:

    ```bash
    sudo apt update
    sudo apt install git build-essential libelf-dev libsdl2-dev libvncserver-dev libslirp-dev
    ```

1. Clone git repository including submodules:

    ```bash
    git clone --recursive https://github.com/aut0/avp64
    ```

1. Chose directories for building and deployment:

    ```text
    <source-dir>  location of your repo copy,     e.g. /home/lukas/avp64
    <build-dir>   location to store object files, e.g. /home/lukas/avp64/BUILD
    <install-dir> output directory for binaries,  e.g. /opt/avp64
    ```

1. Configure and build the project using `cmake`. During configuration you must
   state whether or not to build the runner (vp executable) and the unit tests:

     - `-DAVP64_VP=[ON|OFF]`: build the VP executables (default: `ON`)
     - `-DAVP64_TESTS=[ON|OFF]`: build unit tests (default: `OFF`)

   Release and debug build configurations are controlled via the regular cmake parameters:

   ```bash
   mkdir -p <build-dir>
   cd <build-dir>
   cmake -DCMAKE_INSTALL_PREFIX=<install-dir> -DCMAKE_BUILD_TYPE=RELEASE <source-dir>
   cmake --build . -j `nproc`
   cmake --install .
   ```

   If building with `-DAVP64_TESTS=ON` you can run all unit tests using `make test` within `<build-dir>`.

1. After installation, the following new files should be present:

    ```bash
    <install-dir>/bin/avp64              # full VP executable program
    <install-dir>/bin/avp64_miminal      # minimal VP executable program
    <install-dir>/lib/libocx-qemu-arm.so
    ```

1. If the library `libocx-qemu-arm.so` cannot be found, add the lib folder to `LD_LIBRARY_PATH`:

    ```bash
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<install-dir>/lib/
    ```

----

## Run

To run the platform, a configuration and the corresponding target software files are required.
Examples can be found in the [avp64-sw](https://github.com/aut0/avp64_sw) repository.
The configuration files and target software can be placed in the [sw](sw/) folder to be automatically copied to the install directory on `make install`.
To fetch the latest buildroot Linux build, the [utils/fetch_buildroot_linux](./utils/fetch_buildroot_linux) script can be used.
This script downloads the latest prebuilt linux image and configuration files and places them in the [sw](sw/) folder.
This projects builds two VPs: `avp64` and `avp64_minimal`.
This minimal VP contains less peripherals and is simpler.
To use the minimal VP, the `_minimal` configs file (e.g., `buildroot_6_5_6-x1_minimal.cfg`) must be used.

Run the platform using a config file from the [sw](sw/) folder:

```bash
<install-dir>/bin/avp64 -f <install-dir>/sw/<config-file>
```

For more details on run parameters, you can use the `--help` option:

```bash
<install-dir>/bin/avp64 --help
```

The output should look like this:

```text
Usage: avp64 <arguments>
--config, -c <value>   Specify individual property values
--file, -f <value>     Load configuration from file
--help, -h             Prints this message
--license              Prints module license information
--list-models          Prints all supported models
--list-properties      Prints a list of all properties
--log-debug            Activate verbose debug logging
--log-file, -l <value> Send log output to file
--log-inscight         Send log output to InSCight database
--log-stdout           Send log output to stdout
--trace, -t <value>    Send tracing output to file
--trace-inscight       Send tracing output to InSCight
--trace-stdout         Send tracing output to stdout
--version              Prints module version information
```

To stop the platform, press <kbd>Ctrl</kbd> + <kbd>a</kbd> + <kbd>x</kbd> .

----

## Tracing

VCML has a tracing feature to trace TLM transactions that are sent during the simulation.
Tracing is available for all VCML-based protocols:

- TLM
- GPIO
- Clock
- PCI
- I2C
- SPI
- SD command
- SD data
- Serial
- Virtio
- Ethernet
- CAN
- USB

The VCML tracing documentation can be found [here](https://github.com/machineware-gmbh/vcml/blob/main/doc/tracing.md).
Tracing can be enabled on a per TLM socket basis.
For example, to trace the UART packets that are sent by the `system.uart0` peripheral via its `serial_tx` socket, you can use the folling command:

```bash
<install-dir>/bin/avp64                        \
    -f <install-dir>/sw/buildroot_6_6_6-x1.cfg \ # the configuration file to use
    -c system.uart0.serial_tx.trace=true       \ # enable tracing of the system.uart0.serial_tx socket
    --trace-stdout                               # send the traces to stdout
```

Since this produces a lot of output, it makes sense to store the traces in a trace file.
To do this, use `--trace <trace file>` instead of `--trace-stdout`:

```bash
<install-dir>/bin/avp64                        \
    -f <install-dir>/sw/buildroot_6_6_6-x1.cfg \ # the configuration file to use
    -c system.uart0.serial_tx.trace=true       \ # enable tracing of the system.uart0.serial_tx socket
    --trace my_trace_file.log                    # send the traces to a file
```

The content of the trace file will look like this:

```text
[SERIAL 0.001271278] system.uart0.serial_tx >> SERIAL TX [5b] (9600n8)
[SERIAL 0.001271312] system.uart0.serial_tx >> SERIAL TX [20] (9600n8)
[SERIAL 0.001271346] system.uart0.serial_tx >> SERIAL TX [20] (9600n8)
[SERIAL 0.001271380] system.uart0.serial_tx >> SERIAL TX [20] (9600n8)
[SERIAL 0.001271414] system.uart0.serial_tx >> SERIAL TX [20] (9600n8)
[SERIAL 0.001271448] system.uart0.serial_tx >> SERIAL TX [30] (9600n8)
...
```

To enable tracing for all socket of a peripheral, the `trace` property of the peripheral can be used:

```bash
<install-dir>/bin/avp64                        \
    -f <install-dir>/sw/buildroot_6_6_6-x1.cfg \ # the configuration file to use
    -c system.uart0.trace=true                 \ # enable tracing for all sockets of system.uart0
    --trace my_trace_file.log                    # send the traces to a file
```

You will then get the traces of all sockets of the `system.uart0` peripheral (`rst`, `in`, `serial_tx`, `serial_rx`, and `irq`):

```text
[GPIO 0.000000000] system.uart0.rst >> GPIO+
[GPIO 0.000000000] system.uart0.rst << GPIO+
[GPIO 0.000000000] system.uart0.rst >> GPIO-
[GPIO 0.000000000] system.uart0.rst << GPIO-
[TLM 0.001271270] system.uart0.in >> RD 0x00000018 [00 00 00 00] (TLM_INCOMPLETE_RESPONSE)
[TLM 0.001271270] system.uart0.fr >> RD 0x00000000 [00 00] (TLM_INCOMPLETE_RESPONSE)
[TLM 0.001271270] system.uart0.fr << RD 0x00000000 [90 00] (TLM_OK_RESPONSE)
[TLM 0.001271270] system.uart0.in << RD 0x00000018 [90 00 00 00] (TLM_OK_RESPONSE)
[TLM 0.001271278] system.uart0.in >> WR 0x00000000 [5b] (TLM_INCOMPLETE_RESPONSE)
[TLM 0.001271278] system.uart0.dr >> WR 0x00000000 [5b] (TLM_INCOMPLETE_RESPONSE)
[SERIAL 0.001271278] system.uart0.serial_tx >> SERIAL TX [5b] (9600n8)
...
```

----

## Logging

VCML-based model support logging.
The logging documentation can be found [here](https://github.com/machineware-gmbh/vcml/blob/main/doc/logging.md).
By default, logging is sent to `stdout`.
To redirect logging to a file, use the `--log-file <filename>`.
To enable debug logging, use `--log-debug`.
The log level can be adjusted on a per model basis.

```bash
<install-dir>/bin/avp64                        \
    -f <install-dir>/sw/buildroot_6_6_6-x1.cfg \ # the configuration file to use
    -c system.uart0.loglvl=debug               \ # enable debug logging for system.uart0
    --log-debug                                \ # enable debug logging in general
    --log-file my_log_file.log                   # send the log to a file
```

The log file looks like this:

```text
[I 0.000000000] system.term1: listening on port 52011
[I 0.000000000] system.term2: listening on port 52012
[I 0.000000000] system.term3: listening on port 52013
[D 0.000000000] created slirp ipv4 network 10.0.0.0/24
[D 0.000000000] created slirp ipv6 network fec0::
[I 0.000000000] system: starting infinite simulation using 100 us quantum
[I 0.000000000] system.cpu: listening for GDB connection on port 5555
[I 0.000000000] system.cpu.arm0: listening for GDB connection on port 52100
[D 0.023305928] system.uart0: FIFO enabled
[D 0.023311996] system.uart0: device enabled
[D 0.023313989] system.uart0: device disabled
[D 0.023323269] system.uart0: device enabled
...
```

----

## InSCight™ Simulation Database

By default (if the `SYSTEMC_HOME` environment variable is not set to point to a custom SystemC source), [MachineWare's SystemC kernel](https://github.com/machineware-gmbh/systemc) is used.
This kernel supports the InSCight™ database (more information here: [SystemC Fika](https://www.youtube.com/watch?v=BUoVm30XAIs)) to store simulation data in csv or SQLite-based databases.

To enable InSCight™, set the `INSCIGHT` environment variable before starting the simulation.
Possible values are

| INSCIGHT | Database                                   | Database Name            |
|:--------:|--------------------------------------------|--------------------------|
| csv      | one csv file per database table            | `<table_name>.<pid>.csv` |
| sqlite   | SQLite-based database with multiple tables | `sim.<pid>.db`           |

The databases are stored in the current working directory.

To store logging/tracing information in the database, use the `--log-inscight`, and`--trace-inscight` command-line options (see [Run Section](#run)).

Example command to run the VP with InSCight™ enabled and forwarded tracing messages:

```bash
INSCIGHT=sqlite <install-dir>/bin/avp64 -f <install-dir>/sw/buildroot_6_6_6-x1.cfg --trace-inscight
```

----

## Maintaining Multiple Builds

Debug builds (i.e. `-DCMAKE_BUILD_TYPE=DEBUG`) are intended for developers
that like to make changes on `avp64` and want to track down bugs.
Note that these builds operate significantly slower than optimized release
builds and should therefore not be used for VPs that are used productively,
e.g. for target software development.
To maintain both builds from a single source repository, try the following:

```bash
git clone --recursive https://github.com/aut0/avp64 && cd avp64

home=$PWD
for type in "DEBUG" "RELEASE"; do
    install="$home/BUILD/$type"
    build="$home/BUILD/$type/BUILD"
    mkdir -p $build && cd $build
    cmake -DCMAKE_BUILD_TYPE=$type -DCMAKE_INSTALL_PREFIX=$install $home
    cmake --build . -j `nproc`
done
```

Afterward, you can find the builds in:

```bash
<source-dir>/avp64/BUILD/DEBUG    # for the debug build or
<source-dir>/avp64/BUILD/RELEASE  # for the release build
```

----

## Tutorial

A baisc tutorial that shows how to debug the executed target software using [Visual Studio Code](https://code.visualstudio.com/) can be found in the [vscode-tutorial](./vscode-tutorial/) folder.
Run the [vscode-tutorial/setup.bash](./vscode-tutorial/setup.bash) script to download the Linux image and Linux Kernel Source files and setup Visual Studio Code.
See the corresponding [Readme](./vscode-tutorial/README.md) for further details.

----

## Documentation

The VCML documentation can be found [here](https://github.com/machineware-gmbh/vcml/tree/main/doc).

----

## License

This project is licensed under the MIT license - see the
[LICENSE](LICENSE) file for details.
