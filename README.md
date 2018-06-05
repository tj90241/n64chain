# n64chain

This is a Nintendo 64 development toolchain based on GCC that do not depend on any proprietary Nintendo library.
Only thing that you need to provide is a 2kb bootcode compatible with your CIC to bootstrap the code.
This is not included in n64chain.

The library includes basic OS functionality supporting multi-threading through hardware interrupts on the
single MIPS 4300 CPU of the N64. Mainly written in C and MIPS assembly, it is expected to provide a
lightweight development environment and thus requires some low-level baby sitting.

## Building

To build the toolchain, first put your boot code named `header.bin` on the `libn64` folder. Run `build-linux64-toolchain.sh`
in the `tools` folder on a bash-compatible shell to start building the cross-compiler.
Prerequisites are MPFR and MPC with development headers, which can be installed via `apt install libmpfr-dev libmpc-dev`
on a debian based system. There is also a Windows copatible version `build-win64-toolchain.sh` that still requires a
UNIX-like environment to run.

After the custom GCC build is completed, you should be able to build the helloworld example by running `make` inside
helloworld folder or with `make -C helloworld` on project root. This will also build the libn64 dependency on the project root.
When it is complete you should have a `.z64` rom image ready to be run on a Nintendo 64. The same procedure applies for the
`rdpdemo` and `threadtest` examples.

## Basic N64 architecture

The N64 have a main MIPS4300 processor and a MIPS-like coprocessor called the Reality Co-processor (RCP). These two share
the RDMEM, albeit not sharing a common data bus (instead through DMA controllers and instructions). RCP has two major
sub-units namely the Reality Signal Processor (RSP) and Reality Display Processor (RDP).

Although use-case is dependent on the developer RSP, having an additional vector co-processor, is generally used for
processing 3D transformations and audio processing via microcodes (a program written with a subset of MIPS instructions)
and RDP is used for rasterizing the final image directly onto a display buffer. The latter uses custom 64-bit 'commands'
that instruct RDP to do various pixel manipulations. Using these resources in a balance is key to write performant
applications on N64.
