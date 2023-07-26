Sample x86_64 trace directory contents:
+ drmemtrace.threadsig.2765542.9400.dir/trace/* == the trace itself, with a
  separate file per application thread.
+ binaries/threadsig == the executable that was traced, made available
  for experimentation and comparison in new traces.
+ threadsig.cpp == the source code of the traced application.
+ README.txt == this file.

The memory address tracer we use is part of the "drcachesim" open-source
tracing and analysis framework [1], which is part of the DynamoRIO dynamic
binary instrumentation framework [2].

The trace format used by analysis tools is described in online docs [3].  A
simple analysis tool to look at is basic_counts.cpp [4].

A human-readable view of the sample trace highlighting thread switches and
a signal handler:
--------------------------------------------------
$ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type view 2>&1 | less
Output format:
<--record#-> <--instr#->: <---tid---> <record details>
------------------------------------------------------------
           1           0:     2765542 <marker: version 5>
           2           0:     2765542 <marker: filetype 0xe40>
           3           0:     2765542 <marker: cache line size 64>
           4           0:     2765542 <marker: chunk instruction count 10000000>
           5           0:     2765542 <marker: page size 4096>
<...>
       47742       37769:     2765543 ifetch       6 byte(s) @ 0x0000000000404a78 0f 84 5b 01 00 00    jz     $0x0000000000404bd9 (untaken)
       47743       37770:     2765543 ifetch       5 byte(s) @ 0x0000000000404a7e 48 8b 5c 24 08       mov    0x08(%rsp), %rbx
       47744       37770:     2765543 read         8 byte(s) @ 0x00007fdf64e2adc8 by PC 0x0000000000404a7e
       47745       37771:     2765543 ifetch       5 byte(s) @ 0x0000000000404a83 be 18 00 00 00       mov    $0x00000018, %esi
       47746       37772:     2765543 ifetch       5 byte(s) @ 0x0000000000404a88 b8 11 01 00 00       mov    $0x00000111, %eax
       47747       37773:     2765543 ifetch       7 byte(s) @ 0x0000000000404a8d 48 8d bb e0 02 00 00 lea    0x000002e0(%rbx), %rdi
       47748       37774:     2765543 ifetch       2 byte(s) @ 0x0000000000404a94 0f 05                syscall
       47749       37774:     2765543 <marker: system call 273>
------------------------------------------------------------
       47750       37774:     2765542 <marker: timestamp 13334804716609227>
       47751       37774:     2765542 <marker: tid 2765542 on core 7>
       47752       37775:     2765542 ifetch       1 byte(s) @ 0x000000000040257d 55                   push   %rbp
       47753       37775:     2765542 write        8 byte(s) @ 0x00007ffd90c13950 by PC 0x000000000040257d
       47754       37776:     2765542 ifetch       3 byte(s) @ 0x000000000040257e 48 89 e5             mov    %rsp, %rbp
       47755       37777:     2765542 ifetch       3 byte(s) @ 0x0000000000402581 89 7d fc             mov    %edi, -0x04(%rbp)
<...>
       47865       37842:     2765542 ifetch       5 byte(s) @ 0x0000000000402bcf e8 da 01 00 00       call   $0x0000000000402dae
       47866       37842:     2765542 write        8 byte(s) @ 0x00007ffd90c14638 by PC 0x0000000000402bcf
       47867       37842:     2765542 <marker: kernel xfer from 0x402dae to handler>
       47868       37842:     2765542 <marker: timestamp 13334804716613173>
       47869       37842:     2765542 <marker: tid 2765542 on core 7>
       47870       37843:     2765542 ifetch       1 byte(s) @ 0x000000000040257d 55                   push   %rbp
       47871       37843:     2765542 write        8 byte(s) @ 0x00007ffd90c13930 by PC 0x000000000040257d
       47872       37844:     2765542 ifetch       3 byte(s) @ 0x000000000040257e 48 89 e5             mov    %rsp, %rbp
       47873       37845:     2765542 ifetch       3 byte(s) @ 0x0000000000402581 89 7d fc             mov    %edi, -0x04(%rbp)
       47874       37845:     2765542 write        4 byte(s) @ 0x00007ffd90c1392c by PC 0x0000000000402581
       47875       37846:     2765542 ifetch       4 byte(s) @ 0x0000000000402584 48 89 75 f0          mov    %rsi, -0x10(%rbp)
       47876       37846:     2765542 write        8 byte(s) @ 0x00007ffd90c13920 by PC 0x0000000000402584
       47877       37847:     2765542 ifetch       4 byte(s) @ 0x0000000000402588 48 89 55 e8          mov    %rdx, -0x18(%rbp)
       47878       37847:     2765542 write        8 byte(s) @ 0x00007ffd90c13918 by PC 0x0000000000402588
       47879       37848:     2765542 ifetch       4 byte(s) @ 0x000000000040258c 83 7d fc 1a          cmp    -0x04(%rbp), $0x1a
       47880       37848:     2765542 read         4 byte(s) @ 0x00007ffd90c1392c by PC 0x000000000040258c
       47881       37849:     2765542 ifetch       2 byte(s) @ 0x0000000000402590 75 0f                jnz    $0x00000000004025a1 (untaken)
       47882       37850:     2765542 ifetch       6 byte(s) @ 0x0000000000402592 8b 05 5c 0f 0e 00    mov    <rel> 0x00000000004e34f4, %eax
       47883       37850:     2765542 read         4 byte(s) @ 0x00000000004e34f4 by PC 0x0000000000402592
       47884       37851:     2765542 ifetch       3 byte(s) @ 0x0000000000402598 83 c0 01             add    $0x01, %eax
       47885       37852:     2765542 ifetch       6 byte(s) @ 0x000000000040259b 89 05 53 0f 0e 00    mov    %eax, <rel> 0x00000000004e34f4
       47886       37852:     2765542 write        4 byte(s) @ 0x00000000004e34f4 by PC 0x000000000040259b
       47887       37853:     2765542 ifetch       1 byte(s) @ 0x00000000004025a1 90                   nop
       47888       37854:     2765542 ifetch       1 byte(s) @ 0x00000000004025a2 5d                   pop    %rbp
       47889       37854:     2765542 read         8 byte(s) @ 0x00007ffd90c13930 by PC 0x00000000004025a2
       47890       37854:     2765542 <marker: indirect branch target 0x407bb0>
       47891       37855:     2765542 ifetch       1 byte(s) @ 0x00000000004025a3 c3                   ret
       47892       37855:     2765542 read         8 byte(s) @ 0x00007ffd90c13938 by PC 0x00000000004025a3
       47893       37856:     2765542 ifetch       7 byte(s) @ 0x0000000000407bb0 48 c7 c0 0f 00 00 00 mov    $0x0000000f, %rax
       47894       37857:     2765542 ifetch       2 byte(s) @ 0x0000000000407bb7 0f 05                syscall
       47895       37857:     2765542 <marker: system call 15>
       47896       37857:     2765542 <marker: timestamp 13334804716613183>
       47897       37857:     2765542 <marker: tid 2765542 on core 7>
       47898       37857:     2765542 <marker: syscall xfer from 0x407bb9>
       47899       37857:     2765542 <marker: timestamp 13334804716613190>
       47900       37857:     2765542 <marker: tid 2765542 on core 7>
       47901       37858:     2765542 ifetch       1 byte(s) @ 0x0000000000402dae 55                   push   %rbp
       47902       37858:     2765542 write        8 byte(s) @ 0x00007ffd90c14630 by PC 0x0000000000402dae
<...>
--------------------------------------------------

It is a series of user-mode instruction fetch, data fetch, and metadata
entries.  The fetches contain addresses and sizes and, for instruction
fetches, instruction encodings.  The addresses are all virtual (it is
possible to [gather physical addresses in some
circumstances](https://dynamorio.org/sec_drcachesim_phys.html)).
The metadata "markers" indicate things like which core a thread executed
on, timestamps, an arriving signal causing a PC discontinuity, etc.

For using a trace in a core simulator, the provided instruction encodings
can be decoded into opcodes and operands as is done with the "view" tool above.
This can be done with DynamoRIO's decoder, as in the provided sample tool
opcode_mix.cpp [5].  Other aspects of the trace which help core simulation
are discussed in our documentation [6].

The sample trace here is small but includes corner cases of thread
interleavings, signals, and "non-fetched" instructions.  The trace is from
the "threadsig" application whose source code is included.  It is an
adaptation of [7] with extra threads, signals, and string loop
instructions.

The trace was generated by running:
  $ bin64/drrun -t drcachesim -offline -- ./threadsig 20 200000

Counts of event types in the trace from the basic_counts [4] tool:
  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type basic_counts
  Basic counts tool results:
  Total counts:
      15885247 total (fetched) instructions
          8965 total unique (fetched) instructions
      12600066 total non-fetched instructions
             0 total prefetches
      15219806 total data loads
      17411738 total data stores
             0 total icache flushes
             0 total dcache flushes
            21 total threads
         22144 total scheduling markers
           144 total transfer markers
             0 total function id markers
             0 total function return address markers
             0 total function argument markers
             0 total function return value markers
             0 total physical address + virtual address marker pairs
             0 total physical address unavailable markers
        402439 total other markers
         15613 total encodings
  Thread 2765543 counts:
        790361 (fetched) instructions
           346 unique (fetched) instructions
  <...>

Top opcodes in the trace from the opcode_mix [5] tool:
  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type opcode_mix -module_file drmemtrace.threadsig.[0-9]*.dir/raw/modules.log
  Opcode mix tool results:
         28485313 : total executed instructions
         12800000 :  rep movs
          2400119 :   vmovdqa
          1608705 :       cmp
          1600378 :   vmovdqu
          1006405 :       add
          1000184 :     movsd
  <...>

You can run any of the commands above yourself by downloading DynamoRIO [8].

References from above:

[1] = http://dynamorio.org/page_drcachesim.html
[1] = http://dynamorio.org
[3] = https://dynamorio.org/sec_drcachesim_format.html
(source: https://github.com/DynamoRIO/dynamorio/blob/master/clients/drcachesim/common/memref.h)
[4] = https://github.com/DynamoRIO/dynamorio/blob/master/clients/drcachesim/tools/basic_counts.cpp
[5] = https://github.com/DynamoRIO/dynamorio/blob/master/clients/drcachesim/tools/opcode_mix.cpp
[6] = https://dynamorio.org/sec_drcachesim_core.html
[7] = https://github.com/DynamoRIO/dynamorio/blob/master/suite/tests/pthreads/ptsig.c
[8] = https://github.com/DynamoRIO/dynamorio/releases/latest
