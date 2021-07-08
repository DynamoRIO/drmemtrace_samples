Sample x86_64 trace directory contents:
+ drmemtrace.threadsig.468608.6496.dir/trace/* == the trace itself, with a
  separate file per application thread.
+ drmemtrace.threadsig.468608.6496.dir/raw/modules.log == the list of libraries
  mapped into the application address space.
+ binaries/ == binaries for the librares listed in the modules.log file.
+ threadsig.cpp == the source code of the traced application.
+ README.txt == this file.

The memory address tracer we use is part of the "drcachesim" open-source
tool [1], which is part of the DynamoRIO dynamic binary instrumentation
framework [2].

The trace format used by analysis tools is the memref_t structure [3].  A
simple analysis tool to look at is basic_counts.cpp [4].

A human-readable view of the sample trace highlighting thread switches and
a signal handler:
--------------------------------------------------
  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type view 2>&1 | less
  <...>
  T468608   0x0000000000467c45  4c 8b 54 24 08       mov    0x08(%rsp), %r10
  T468608     read  8 byte(s) @ 0x7fff9f5fd9b0
  T468608   0x0000000000467c4a  b8 38 00 00 00       mov    $0x00000038, %eax
  T468608   0x0000000000467c4f  0f 05                syscall
  ------------------------------------------------------------
  T468610 <marker: timestamp 13270239527782712>
  T468610 <marker: tid 468610 on core 0>
  T468610   0x0000000000467c51  48 85 c0             test   %rax, %rax
  T468610   0x0000000000467c54  7c 13                jl     $0x0000000000467c69
  T468610   0x0000000000467c56  74 01                jz     $0x0000000000467c59
  T468610   0x0000000000467c59  31 ed                xor    %ebp, %ebp
  T468610   0x0000000000467c5b  58                   pop    %rax
  T468610     read  8 byte(s) @ 0x7f669dc77e70
  T468610   0x0000000000467c5c  5f                   pop    %rdi
  T468610     read  8 byte(s) @ 0x7f669dc77e78
  T468610   0x0000000000467c5d  ff d0                call   %rax
  <...>
  T468608   0x0000000000405376  64 c7 04 25 18 00 00 movl   $0x00000001, %fs:0x18
  T468608                       00 01 00 00 00
  T468608     write 4 byte(s) @ 0x4eb898
  T468608   0x0000000000405382  45 31 c0             xor    %r8d, %r8d
  T468608   0x0000000000405385  eb 2f                jmp    $0x00000000004053b6
  T468608 <marker: kernel xfer from 0x4053b6 to handler>
  T468608 <marker: timestamp 13270239527784929>
  T468608 <marker: tid 468608 on core 2>
  T468608   0x000000000040257d  55                   push   %rbp
  T468608     write 8 byte(s) @ 0x7fff9f5fd330
  T468608   0x000000000040257e  48 89 e5             mov    %rsp, %rbp
  T468608   0x0000000000402581  89 7d fc             mov    %edi, -0x04(%rbp)
  T468608     write 4 byte(s) @ 0x7fff9f5fd32c
  T468608   0x0000000000402584  48 89 75 f0          mov    %rsi, -0x10(%rbp)
  T468608     write 8 byte(s) @ 0x7fff9f5fd320
  T468608   0x0000000000402588  48 89 55 e8          mov    %rdx, -0x18(%rbp)
  T468608     write 8 byte(s) @ 0x7fff9f5fd318
  T468608   0x000000000040258c  83 7d fc 1a          cmp    -0x04(%rbp), $0x1a
  T468608     read  4 byte(s) @ 0x7fff9f5fd32c
  T468608   0x0000000000402590  75 0f                jnz    $0x00000000004025a1
  T468608   0x0000000000402592  8b 05 5c 0f 0e 00    mov    <rel> 0x00000000004e34f4, %eax
  T468608     read  4 byte(s) @ 0x4e34f4
  T468608   0x0000000000402598  83 c0 01             add    $0x01, %eax
  T468608   0x000000000040259b  89 05 53 0f 0e 00    mov    %eax, <rel> 0x00000000004e34f4
  T468608     write 4 byte(s) @ 0x4e34f4
  T468608   0x00000000004025a1  90                   nop
  T468608   0x00000000004025a2  5d                   pop    %rbp
  T468608     read  8 byte(s) @ 0x7fff9f5fd330
  T468608   0x00000000004025a3  c3                   ret
  T468608     read  8 byte(s) @ 0x7fff9f5fd338
  T468608   0x0000000000407bb0  48 c7 c0 0f 00 00 00 mov    $0x0000000f, %rax
  T468608   0x0000000000407bb7  0f 05                syscall
  T468608 <marker: timestamp 13270239527784936>
  T468608 <marker: tid 468608 on core 2>
  T468608 <marker: syscall xfer from 0x407bb9>
  T468608 <marker: timestamp 13270239527792923>
  T468608 <marker: tid 468608 on core 2>
  T468608   0x00000000004053b6  80 bd 7c ff ff ff 00 cmp    -0x84(%rbp), $0x00
  T468608     read  1 byte(s) @ 0x7fff9f5fda4c
  <...>
--------------------------------------------------

It is a series of instruction fetch, data fetch, and metadata entries.  The
fetches contain addresses and sizes.  The addresses are all virtual.  The metadata
"markers" indicate things like which core a thread executed on, timestamps,
an arriving signal causing a PC discontinuity, etc.

For using a trace in a core simulator, you will want to obtain the opcodes,
as is done with the "view" tool above.
These are not part of the base trace.  They are obtained by decoding the
instruction fetch addresses from the binaries.  Library support makes this
straightforward.  A simple tool that does this is opcode_mix.cpp [5].  It
uses library routines to read the "modules.log" file, which contains the
mappings of the binary and libraries from the traced execution, and map
those binaries into the address space, allowing examining the instruction
bytes.  The modules.log file is included here, along with all of the
binaries referenced there.  For the vdso the raw bytes are embedded in the
file and mapped in just like the binaries.

Other aspects of the trace which help core simulation are discussed in our
documentation [6].

The sample trace here is small but includes corner cases of thread
interleavings, signals, and "non-fetched" instructions.  The trace is from
the "threadsig" application whose source code is included.  It is an
adaptation of [7] with extra threads, signals, and string loop
instructions.

The trace was generated by running:
  $ bin64/drrun -t drcachesim -offline -- ./threadsig 20 200000

Counts of event types in the trace:
  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type basic_counts
  Basic counts tool results:
  Total counts:
      15892282 total (fetched) instructions
          8958 total unique (fetched) instructions
      12600072 total non-fetched instructions
             0 total prefetches
      15221311 total data loads
      17411584 total data stores
             0 total icache flushes
             0 total dcache flushes
            21 total threads
         22056 total scheduling markers
           114 total transfer markers
             0 total function id markers
             0 total function return address markers
             0 total function argument markers
             0 total function return value markers
            63 total other markers
  Thread 468610 counts:
        790315 (fetched) instructions
           346 unique (fetched) instructions
  <...>

Top opcodes in the trace:
  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type opcode_mix -module_file drmemtrace.threadsig.[0-9]*.dir/raw/modules.log
  Opcode mix tool results:
         28492354 : total executed instructions
         12800000 :  rep movs
          2400134 :   vmovdqa
          1610000 :       cmp
          1600378 :   vmovdqu
          1007481 :       add
          1000184 :     movsd
  <...>

You can run any of the commands above yourself by downloading DynamoRIO [8].

References from above:

[1] = http://dynamorio.org/page_drcachesim.html
[1] = http://dynamorio.org
[3] = http://dynamorio.org/union__memref__t.html
(source: https://github.com/DynamoRIO/dynamorio/blob/master/clients/drcachesim/common/memref.h)
[4] = https://github.com/DynamoRIO/dynamorio/blob/master/clients/drcachesim/tools/basic_counts.cpp
[5] = https://github.com/DynamoRIO/dynamorio/blob/master/clients/drcachesim/tools/opcode_mix.cpp
[6] = https://dynamorio.org/sec_drcachesim_core.html
[7] = https://github.com/DynamoRIO/dynamorio/blob/master/suite/tests/pthreads/ptsig.c
[8] = https://github.com/DynamoRIO/dynamorio/releases/latest
