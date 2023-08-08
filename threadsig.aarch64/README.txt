Sample aarch64 trace directory contents:
+ drmemtrace.threadsig.2444839.4049.dir/trace/* == the trace itself, with a
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
Output format:
<--record#-> <--instr#->: <---tid---> <record details>
------------------------------------------------------------
           1           0:     2444839 <marker: version 5>
           2           0:     2444839 <marker: filetype 0xe08>
           3           0:     2444839 <marker: cache line size 64>
           4           0:     2444839 <marker: chunk instruction count 10000000>
           5           0:     2444839 <marker: page size 4096>
<...>
       10660        8011:     2444840 ifetch       4 byte(s) @ 0x0000000000401d68 91038040   add    %x2 $0x00e0 lsl $0x00 -> %x0
       10661        8012:     2444840 ifetch       4 byte(s) @ 0x0000000000401d6c d4000001   svc    $0x0000
       10662        8012:     2444840 <marker: system call 99>
------------------------------------------------------------
       10663        8012:     2444839 <marker: timestamp 13334813807921548>
       10664        8012:     2444839 <marker: tid 2444839 on core 15>
       10665        8013:     2444839 ifetch       4 byte(s) @ 0x00000000004008d8 7100681f   subs   %w0 $0x001a lsl $0x00 -> %wzr
       10666        8014:     2444839 ifetch       4 byte(s) @ 0x00000000004008dc 540000c1   b.ne   $0x00000000004008f4 (untaken)
       10667        8015:     2444839 ifetch       4 byte(s) @ 0x00000000004008e0 f0000520   adrp   <rel> 0x00000000004a7000 -> %x0
<...>
       10705        8037:     2444839 ifetch       4 byte(s) @ 0x000000000040241c a8cf7bfd   ldp    (%sp)[16byte] %sp $0x00000000000000f0 -> %x29 %x30 %sp
       10706        8037:     2444839 read        16 byte(s) @ 0x0000ffffe7ac6020 by PC 0x000000000040241c
       10707        8038:     2444839 ifetch       4 byte(s) @ 0x0000000000402420 d65f03c0   ret    %x30 (target 0x400510)
       10708        8038:     2444839 <marker: kernel xfer from 0x400510 to handler>
       10709        8038:     2444839 <marker: timestamp 13334813807925181>
       10710        8038:     2444839 <marker: tid 2444839 on core 15>
       10711        8039:     2444839 ifetch       4 byte(s) @ 0x00000000004008d8 7100681f   subs   %w0 $0x001a lsl $0x00 -> %wzr
       10712        8040:     2444839 ifetch       4 byte(s) @ 0x00000000004008dc 540000c1   b.ne   $0x00000000004008f4 (untaken)
       10713        8041:     2444839 ifetch       4 byte(s) @ 0x00000000004008e0 f0000520   adrp   <rel> 0x00000000004a7000 -> %x0
       10714        8042:     2444839 ifetch       4 byte(s) @ 0x00000000004008e4 9129c000   add    %x0 $0x0a70 lsl $0x00 -> %x0
       10715        8043:     2444839 ifetch       4 byte(s) @ 0x00000000004008e8 b9404801   ldr    +0x48(%x0)[4byte] -> %w1
       10716        8043:     2444839 read         4 byte(s) @ 0x00000000004a7ab8 by PC 0x00000000004008e8
       10717        8044:     2444839 ifetch       4 byte(s) @ 0x00000000004008ec 11000421   add    %w1 $0x0001 lsl $0x00 -> %w1
       10718        8045:     2444839 ifetch       4 byte(s) @ 0x00000000004008f0 b9004801   str    %w1 -> +0x48(%x0)[4byte]
       10719        8045:     2444839 write        4 byte(s) @ 0x00000000004a7ab8 by PC 0x00000000004008f0
       10720        8046:     2444839 ifetch       4 byte(s) @ 0x00000000004008f4 d65f03c0   ret    %x30 (target 0x7142ff14)
       10721        8047:     2444839 ifetch       4 byte(s) @ 0x000000007142ff14 52801168   movz   $0x008b lsl $0x00 -> %w8
       10722        8048:     2444839 ifetch       4 byte(s) @ 0x000000007142ff18 d4000001   svc    $0x0000
       10723        8048:     2444839 <marker: system call 139>
       10724        8048:     2444839 <marker: timestamp 13334813807925213>
       10725        8048:     2444839 <marker: tid 2444839 on core 15>
       10726        8048:     2444839 <marker: syscall xfer from 0x7142ff1c>
       10727        8048:     2444839 <marker: timestamp 13334813807925239>
       10728        8048:     2444839 <marker: tid 2444839 on core 15>
       10729        8049:     2444839 ifetch       4 byte(s) @ 0x0000000000400510 340001a0   cbz    $0x0000000000400544 %w0 (taken)
       10730        8050:     2444839 ifetch       4 byte(s) @ 0x0000000000400544 a94687a0   ldp    +0x68(%x29)[16byte] -> %x0 %x1
       10731        8050:     2444839 read        16 byte(s) @ 0x0000ffffe7ac6178 by PC 0x0000000000400544
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
interleavings and signals.  The trace is from
the "threadsig" application whose source code is included.  It is an
adaptation of [7] with extra threads, signals, and string loop
instructions.

The trace was generated by running:
  $ bin64/drrun -t drcachesim -offline -- ./threadsig 20 200000

Counts of event types in the trace from the basic_counts [4] tool:
  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type basic_counts
  Basic counts tool results:
  Total counts:
      12050661 total (fetched) instructions
          6425 total unique (fetched) instructions
             0 total non-fetched instructions
            39 total prefetches
        210261 total data loads
       2607283 total data stores
             0 total icache flushes
             0 total dcache flushes
            21 total threads
          2888 total scheduling markers
           140 total transfer markers
             0 total function id markers
             0 total function return address markers
             0 total function argument markers
             0 total function return value markers
             0 total physical address + virtual address marker pairs
             0 total physical address unavailable markers
           331 total other markers
         13601 total encodings
  Thread 2444840 counts:
        600364 (fetched) instructions
           374 unique (fetched) instructions
  <...>

Top opcodes in the trace from the opcode_mix [5] tool:
  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type opcode_mix -module_file drmemtrace.threadsig.[0-9]*.dir/raw/modules.log
  Opcode mix tool results:
         12050661 : total executed instructions
          1604515 :     bcond
          1405794 :       add
          1403734 :      subs
          1403183 :       stp
           800682 :       and
           800039 :    dc_zva
  <...>

You can run any of the commands above yourself by downloading DynamoRIO [9].

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
