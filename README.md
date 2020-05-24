# SoftwareCache

## This Program
This program is a software implementation of a single level write-back and write-allocate cache with 24-bit addresses. Users can specify cache size, block size, and associativity of the cache. Using a trace file of commands, the program will output if it is a hit or a miss, and the value to be loaded if the command is a load.

## Set up
In order to compile the C code, you need a C compiler program. I used the GNU Compiler Collection (GCC) from [here](https://gcc.gnu.org/). Once installed, the program can be compiled by running 
```
gcc -o cachesim cachesim.c
```

## Running the program
The program takes in 4 input arguments in the following format:

./cachesim \<trace-file\> \<cache-size-kB\> \<associativity\> \<block-size\>

All numbers are in decimal format

\<trace-file\>: Filename (full path) of the memory access trace file, extension .txt.

\<cache-size-kB\>: Total capacity of the cache in kB. Must be a power of 2 between 2 and 2048.

\<associativity\>: The set associativity. Must be a power of 2.

\<block-size\>: The size of the cache blocks in Bytes. Must be a power of 2 between 2 and 512.

## Assumptions
Addresses are 24-bits, and thus addresses range from 0 to 2^24-1. The machine is byte-addressed and big-endian. The cache size,
associativity, block size, and access size must all be powers of 2. Cache size cannot be larger than 2MB, block
size cannot be larger than 64B, and no access can be larger than the block size and cannot exceed 8 bytes.
No cache access can span multiple blocks.

## Trace File Format
The trace file will specify a single load or store on each line, the 24-bit address (hex), the size of the access in decimal, and the value to be written if it is a store (hex). For example, here is example.txt:
```
store 0xd53170 4 7d2f13ac
load  0xd53172 1
store 0xd53170 2 f0b1
store 0x1a25bb 2 c77a
load  0xd53170 4
load  0x12 2
store 0x23 8 d687eb9f1bc687ec
```

## Example Output
The following is an example of a 1 MB, 4-way cache with 32-byte blocks:
```
$ ./cachesim traces/example.txt 1024 4 32
store 0xd53170 miss
load 0xd53172 hit 13
store 0xd53170 hit
store 0x1a25bb miss
load 0xd53170 hit f0b113ac
load 0x12 miss 0000
store 0x23 miss
```

## Acknowledgements
This is an adaptation of the cache simulator assignment by Duke Professor Alvin Lebeck. The original assignment has students building a program to simulate a cache. This project takes this simulator and creates an actual software implementation of the simulated cache. Trace files, file io, command line arguments, and specifications of the cache were given by Dr. Lebeck. Everything else is original work.
