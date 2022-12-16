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
- Install "Ubuntu 14.04.2 LTS (Trusty Tahr) i386" image inside the VM.
- Run the following commands inside the VM:
    - sudo apt-get update
    - sudo apt-get -y install qemu gitk git-gui build-essential gcc-multilib
    - sudo ln -s /usr/bin/qemu-system-i386 /usr/bin/qemu
    - echo "set auto-load safe-path /" >> ~/.gdbinit
- Clone repository.
- Run make inside the repository to start the OS.
