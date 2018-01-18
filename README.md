#### My_dbg

This project implements a simple debugger for simple ELF binaries.
This debugger is fully cappable of debugging simple binaries on x86_64
architecture.

This project was made by grison_d for the LSE enrollment.

---
#### Installation

This project is compiled using a Makefile.
To install it just do as follow at the root of the project:

```
make
```

---
#### Usage

First, you must have a binary file to debug. Only then `my-dbg` will be
able to debug it.

```
./my-dbg <binary_file> [addtitional binary file arguments]
```

---
#### Commands

 - `info_regs`: display registers
 - `continue`: continue program being debugged
 - `quit`: quit the program
 - `info_memroy`: dipslay memory
 - `break <addr>`: add a breakpoint 
 - `step_instr`: single step the program being debugged
 - `examine <format> <size> <addr>`: print data at a specific address
 - `backtrace`: print the call trace at the current %rip
 - `tbrake <addr>` : add a temporary breakpoint
 - `next_instr`: single step over call instructions
 - `break_list`: list all breakpoints
 - `break_del <id>`: delete a breakpoint

---
#### Features

 -[X] Breakpoints listing
 -[X] Breakpoints deleting

---
#### Project's Directory Tree

```
+ subject-grison_d/
|-- Makefile
|-- README
+--+ src/
|  |-- bp_list.c
|  |-- breakpoint.c
|  |-- cmd.c
|  +--+ include/
|     |-- breakpoint.h
|     |-- cmd.h
|     +-- dbg.h
|  |-- info_cmd.c
|  +-- prompt.c
+-- TODO
```
