## SBUnix : CSE 506 Project

Professor: [Mike Ferdman](http://compas.cs.stonybrook.edu/~mferdman/)

Course webpage: [Operating systems](https://compas.cs.stonybrook.edu/courses/cse506-operating-systems-fall-17/)

### Project Description

* x86_64 Preemptive multitasking operating system with following functionalities:
  * Memory Subsystem (page descriptors, free list, page tables, kmalloc)
  * Process Subsystem (kernel threads, context switch)
  * User-level Subsystem (VMAs/vm_map_entrys, switch to ring 3, page faults)
  * I/O subsystem (syscalls, terminals, VFS, tarfs file access)
  * A Shell that
    * Supports changing current directory (cd /path)
    * Executes binaries interactively
    * Executes scripts (files that start with #!sbush)
    * Sets and uses PATH and PS1 variables
    * Launches background processes (ls &)

* The provided Makefile:
  1) Builds a kernel
  2) Copies it into rootfs/boot/kernel/kernel
  3) Creates an ISO CD image with the rootfs/ contents
  
  
* Commands supported by shell:
```
ps                  Process List
ls                  List directory contents
ls path             List directory contents of that path
cat [file path]     Prints file contents
                    example: cat file
cd                  Change Directory
ulimits             Displays system limits
sbush {filepath}    Execute shebang files. example: sbush /etc/test/script.sh
exit                Kills the current process
shutdown            Exits all the process.
echo $VAR           Displays the current value set in the argument variable
                    example - echo $PATH
export VAR=ABC      Export used to create/set value to environment variable
                    example: export PATH=/rootfs/bin
sleep {seconds}     Puts the shell to sleep for specified seconds
                    example: sleep 5
sleep {seconds} &   Puts the shell to sleep for specified seconds in background
                    example: sleep 5 &
kill -9 pid         Kill the process with that pid [Note: Only kill -9 supported]
```

* To boot the system in QEMU, run:
```
./start.sh
```

OR

```
qemu-system-x86_64 -curses -drive id=boot,format=raw,file=$USER.img,if=none -drive id=data,format=raw,file=$USER-data.img,if=none -device ahci,id=ahci -device ide-drive,drive=boot,bus=ahci.0 -device ide-drive,drive=data,bus=ahci.1 -gdb tcp::9999
```

* Explanation of parameters:
```
  -curses         use a text console (omit this to use default SDL/VNC console)
  -drive ...      connect a CD-ROM or hard drive with corresponding image
  -device ...     configure an AHCI controller for the boot and data disks
  -gdb tcp::9999  listen for "remote" debugging connections on port NNNN
  -S              wait for GDB to connect at startup
  -no-reboot      prevent reboot when OS crashes
```

* When using the -curses mode, switch to the qemu> console with ESC-2.

* To connect a remote debugger to the running qemu VM, from a different window:
gdb ./kernel

* At the (gdb) prompt, connect with:
target remote localhost:9999
