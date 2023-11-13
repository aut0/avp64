# Visual Studio Code (VSCode) Integration

## Setup

1. Install the needed tools
    
    Ubuntu:

    ```bash
    sudo apt update
    sudo apt install gdb-multiarch git cmake build-essential lldb         # development tools
    sudo apt install libelf-dev libsdl2-dev libvncserver-dev libslirp-dev # dependencies
    ```

1. Run the [setup script](setup.bash) to fetch the compiled Linux kernel from [avp64_sw](https://github.com/aut0/avp64_sw) and the source files for debugging

    ```bash
    <avp64 root>/vscode/setup.bash
    ```
1. Install the VSCode extensions

    * In the end of the setup script, VSCode opens
    * Import the [avp64.code-profile](avp64.code-profile) by following the instructions [here](https://code.visualstudio.com/docs/editor/profiles#_import). This install the required extensions

1. Configure the project

    * Press <kbd>F1</kbd> and type _CMake: Configure_
    * Select the C++ compiler

1. Build the project

    * Press <kbd>F1</kbd> and type _CMake: Build_

1. Install the project

    * The project is configured to be installed at `<avp64 root>/build/{debug|release}/`. (This setting can be changed in the [\<avp64 root\>/.vscode/settings.json](../.vscode/settings.json) file)
    * To install the project, press <kbd>F1</kbd> and type _CMake: Install_


## Start AVP64

1. Open the Debug view (<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>D</kbd>)
1. At the top, after the green arrow, select _Debug AVP64_
1. Press the green arrow to start AVP64
1. Change to the terminal window (<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>`</kbd>) to see the the output of avp64.
    The VP waits for a debugger to connect.
    Sample output:

    ```text
            SystemC 2.3.3-MachineWare GmbH --- Nov 10 2023 18:03:26
            Copyright (c) 1996-2018 by all Contributors,
            ALL RIGHTS RESERVED
    [I 0.000000000] system.term0: listening on port 52010
    [I 0.000000000] system.term1: listening on port 52011
    [I 0.000000000] system.term2: listening on port 52012
    [I 0.000000000] system.term3: listening on port 52013
    [D 0.001000000] created slirp ipv4 network 10.0.0.0/24
    [D 0.001000000] created slirp ipv6 network fec0::
    [I 0.132000000] system: starting infinite simulation using 100 us quantum
    [I 0.138000000] system.arm0: waiting for GDB connection on port 52100
    ```

1. Connect the debugger by going back to the Debug view (<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>D</kbd>)
1. At the top, after the green arrow, select _Debug Target SW_
1. Press the green arrow to connect the debugger
1. You can control the target using the control panel at the top.
    You can use the dropdown menu to switch between debugging avp64 and the target software.

    <img src="assets/control_panel.png" width="400">