# Communication over local Network Project 

# Objective: 
- Facilitates communication between two microcontrollers
- Users can send messages to anyone on the same network by selecting the node ID.
- Users can receive incoming messages.

## How to run program

1. These steps is done after installation of non OS libraries
2. Command:
    sudo apt-get -y --no-install-recommends install lbzip2 unzip g++ tcl less emacs-nox vim
    nano make gcc libusb-dev libreadline-dev minicom libusb-dev
    sudo apt-get install libusb-1.0
    sudo apt install binutils-arm-none-eabi gcc-arm-none-eabi gcc-arm-none-eabi-source
    libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
3.  Add the following line to /etc/apt/sources.list:
   Command:
          deb http://cz.archive.ubuntu.com/ubuntu focal main universe
4. Execute the following commands
   Command:
       sudo apt update
       sudo apt install libnewlib-nano-arm-none-eabi
       sudo apt install gdb-multiarch
       sudo apt install libtool pkg-config
5. Download openocd from the following link
   https://sourceforge.net/projects/openocd/files/openocd/0.10.0-rc2/
   And uncompress and navigate to its directory. Then apply the following commands
   respectively
    Command:
       ./configure
       make
       sudo make install
6. Install PICOS:
   - Make sure that you have the instructions in the previous section done
  successfully.

  -  Download OLSONET.tar.xz and unzip it in your Desktop.
  - Navigate to OLSONET/PICOS and execute the following commands:
  Command:
       ./deploy
  - Add the following command to the "~/.bashrc export PATH=~/bin:$PATH"
  To test that your installation is successful:
  - navigate to OLSONET/PICOS/Apps/EXAMPLES/INTRO/LEDS and execute
    the following commands:
  Command:
       mkmk CC1350_LAUNCHXL
       make
  - Run openocd using the following command
  Command:
       openocd -f board/ti_cc13x0_launchpad.cfg
  - Run gdb using the following commands
  Command:

           gdb-multiarch Image

           (gdb) target extended-remote localhost:3333
           (gdb) monitor reset halt
           (gdb) load
           (gdb) continue
