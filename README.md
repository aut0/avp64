# An ARMv8 Virtual Platform (AVP64)
This repository contains an ARMv8 multi-core virtual platform.
It was build at the Institute for Communication Techonlogies and Embedded Systems at RTWH Aachen University.
The following target software configurations were tested (see [config](config/)):
- CoreMark
- Dhrystone
- Linux single- and dualcore
- Xen single- and dualcore

----
## Build & Installation
In order to build `avp64`, you need a working installation of `vcml` and its components.
For that please follow the installation guideline of `vcml` which can be found [here](https://github.com/machineware-gmbh/vcml).

1. Clone git repository including submodules:
    ```
    git clone --recurse-submodules https://github.com/aut0/avp64 
    ```
	
2. Chose directories for building and deployment:
    ```
    <source-dir>  location of your repo copy,     e.g. /home/lukas/avp64
    <build-dir>   location to store object files, e.g. /home/lukas/avp64/BUILD
    <install-dir> output directory for binaries,  e.g. /opt/avp64
    ```

3. Configure and build the project using `cmake`. During configuration you must
   state whether or not to build the unit tests:
     * `-DBUILD_TESTS=[ON|OFF]`: build unit tests (default: `OFF`)

   Ensure that the enviroment variable `SYSTEMC_HOME` is correctly set.  
   Release and debug build configurations are controlled via the regular
   parameters:
   ```
   mkdir -p <build-dir>
   cd <build-dir>
   cmake -DCMAKE_INSTALL_PREFIX=<install-dir> -DCMAKE_BUILD_TYPE=RELEASE <source-dir>
   make -j 4
   sudo make install
   ```
   If building with `-DBUILD_TESTS=ON` you can run all unit tests using
   `make test` within `<build-dir>`.

4. After installation, the following new files should be present:
    ```
    <install-dir>/bin/avp64                # executable program
    <install-dir>/config/                  # configuration files 
    <install-dir>/lib/libocx-qemu-arm.so 
    ```

5. If the library `libocx-qemu-arm.so` cannot be found, add the lib folder to `LD_LIBRARY_PATH`:
    ```
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/avp64/lib/
    ```

----
## Run
Run the platform using a [config](config/) file:	
```
<install-dir>/bin/avp64 -f <install-dir>/config/<config-file>
```
For more details on run parameters please look [here](https://github.com/machineware-gmbh/vcml).  

To install tested software which belongs to the provided [configuration files](config/), follow the installation guide of [avp64_sw](https://github.com/aut0/avp64_sw).  
After the installation, set the enviroment variable `AVP64_SW` to the path of the `avp64_sw` repository:  
```
export AVP64_SW=<software-repository>
```
Execute the setup script, which is in `avp64/`, to copy and integrate the installed software:  
```
./setup.sh
```

----
## Maintaining Multiple Builds
Debug builds (i.e. `-DCMAKE_BUILD_TYPE=DEBUG`) are intended for developers
that like to make changes on `avp64` and want to track down bugs.
Note that these builds operate significantly slower than optimized release
builds and should therefore not be used for VPs that are used productively,
e.g. for target software development.   
Ensure that the enviroment variable `SYSTEMC_HOME` is correctly set.  
To maintain both builds from a single source repository, try the following:
```
git clone --recurse-submodules https://github.com/aut0/avp64 && cd avp64  

home=$PWD
for type in "DEBUG" "RELEASE"; do
    install="$home/BUILD/$type"
    build="$home/BUILD/$type/BUILD"
    mkdir -p $build && cd $build
    cmake -DCMAKE_BUILD_TYPE=$type -DCMAKE_INSTALL_PREFIX=$install $home
    make install
done
```
Afterwards, you can find the builds in:
```
<source-dir>/avp64/BUILD/DEBUG    # for the debug build or
<source-dir>/avp64/BUILD/RELEASE  # for the release build
```

----
## Documentation
The VCML documentation can be found 
[here](https://github.com/machineware-gmbh/vcml).

----
## License
This project is licensed under the MIT license - see the
[LICENSE](LICENSE) file for details.
