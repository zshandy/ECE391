# A simple custom linux kernel

## Description

This kernel is developed as a school project.

It contains all the features a normal kernel contains.

There are some extra features:
  - file system
  - support for keyboard
  - support for mouse if GUI is implemented in the OS based on this kernel
  - support for schedueling (specifically 3 terminals)


## Instruction to Compile

To compile the kernel image:

``` bash 
make dep

sudo make
```
The make command will create two images, the kernel image "bootimg.img"
and the disk image "mp3.img".

The created disk image is provided for running kernel in QEMU emulator.
