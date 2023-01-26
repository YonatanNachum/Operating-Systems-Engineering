# CS236376: Operating Systems Engineering

Faculty of Computer Science, Technion.

## Introduction
We implemented, from scratch, a minimalistic yet fully functional operating system that supports virtual memory, kernel and user modes, system calls, threads, context switches, interrupts, interprocess communication, coordination of concurrent activities, file system I/O, networking, VGA and mouse.

## Contents
- Lab 1: Booting a PC
    - PC Bootstrap
    - The Boot Loader
    - The Kernel
- Lab 2: Memory Management
    - Physical Page Management
    - Virtual Memory
    - Kernel Address Space
- Lab 3: User Environments
    - User Environments and Exception Handling
    - Page Faults, Breakpoints Exceptions, and System Calls
- Lab 4: Preemptive Multitasking
    - Multiprocessor Support and Cooperative Multitasking
    - Copy-on-Write Fork
    - Preemptive Multitasking and Inter-Process communication (IPC)
- Lab 5: File System, Spawn and Shell
    - File System: Disk Access, Block Cache, Block Bitmap, File Operations, File system interface
    - Spawning Processes: Sharing library state across fork and spawn
    - Keyboard Interface
    - Shell Commands
- Lab 6: Network Driver
    - Initialization and transmitting packets
    - Receiving packets and the web server
    - EEPROM MAC
    - "Zero Copy"
- Project:
    - VGA
    - Mouse
    - Drawer
    - Games: PingPong, Tic-Tac-Toe

## How To Run
- Install VM
- Install "[Ubuntu 14.04.2 LTS (Trusty Tahr) i386](http://old-releases.ubuntu.com/releases/14.04.2/)" image inside the VM.
- Run the following commands inside the VM:
```bash
sudo apt-get update
sudo apt-get -y install qemu gitk git-gui build-essential gcc-multilib
sudo ln -s /usr/bin/qemu-system-i386 /usr/bin/qemu
echo "set auto-load safe-path /" >> ~/.gdbinit
```
- Clone repository.
- Run the following inside the repository to start the OS.
### Games
```bash
make qemu-nox
# Run inside qemu to excute different tasks
pong # Start ping pong
tictactoe # Start tic tac toe
```
### Network
```bash
make which-ports # Show a web page served by the HTTP server running inside JOS.

# Echo test - a daemon called echosrv that sets up an echo server running on port 7
# that will echo back anything sent over a TCP connection.
make E1000_DEBUG=TX,TXERR,RX,RXERR,RXFILTER run-echosrv # First terminal
make nc-7 # Second terminal
```

## Results
| Tic Tac Toe      | Ping Pong      |
|------------|-------------|
|<img width="634" alt="Screen Shot 2023-01-25 at 17 36 42" src="https://user-images.githubusercontent.com/62839801/214607383-5d0e1314-32ef-43fb-9280-9c24fa132833.png">|<img width="640" alt="Screen Shot 2023-01-25 at 17 39 02" src="https://user-images.githubusercontent.com/62839801/214607507-cdf6e153-a975-4398-972f-207013976c5d.png">|
