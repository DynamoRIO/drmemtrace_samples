Sample aarch64 trace directory contents:
+ drmemtrace.threadsig.1773966.7161.dir/trace/* == the trace itself, with a
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
<record#>: T<tid> <record details>
------------------------------------------------------------
        1: T1773976 <marker: version 4>
        2: T1773976 <marker: filetype 0x208>
        3: T1773976 <marker: cache line size 64>
        4: T1773976 <marker: chunk instruction count 10000000>
        5: T1773976 <marker: page size 4096>
<...>
    10562: T1773966 ifetch       4 byte(s) @ 0x000000000042e268 d2801b88   movz   $0x00dc lsl $0x00 -> %x8
    10563: T1773966 ifetch       4 byte(s) @ 0x000000000042e26c d4000001   svc    $0x0000
------------------------------------------------------------
    10564: T1773967 <marker: timestamp 13309327193840464>
    10565: T1773967 <marker: tid 1773967 on core 52>
    10566: T1773967 ifetch       4 byte(s) @ 0x000000000042e270 f100001f   subs   %x0 $0x0000 lsl $0x00 -> %xzr
    10567: T1773967 ifetch       4 byte(s) @ 0x000000000042e274 540000e0   b.eq   $0x000000000042e290
    10568: T1773967 ifetch       4 byte(s) @ 0x000000000042e290 d280001d   movz   $0x0000 lsl $0x00 -> %x29
<...>
   108834: T1773966 ifetch       4 byte(s) @ 0x0000000000428540 17fff04c   b      $0x0000000000424670
   108835: T1773966 <marker: kernel xfer from 0x424670 to handler>
------------------------------------------------------------
<...>
------------------------------------------------------------
   122607: T1773966 <marker: timestamp 13309327193842204>
   122608: T1773966 <marker: tid 1773966 on core 51>
   122609: T1773966 ifetch       4 byte(s) @ 0x00000000004008d8 7100681f   subs   %w0 $0x001a lsl $0x00 -> %wzr
   122610: T1773966 ifetch       4 byte(s) @ 0x00000000004008dc 540000c1   b.ne   $0x00000000004008f4
   122611: T1773966 ifetch       4 byte(s) @ 0x00000000004008e0 f0000520   adrp   <rel> 0x00000000004a7000 -> %x0
   122612: T1773966 ifetch       4 byte(s) @ 0x00000000004008e4 9129c000   add    %x0 $0x0a70 lsl $0x00 -> %x0
   122613: T1773966 ifetch       4 byte(s) @ 0x00000000004008e8 b9404801   ldr    +0x48(%x0)[4byte] -> %w1
   122614: T1773966 read         4 byte(s) @ 0x00000000004a7ab8 by PC 0x00000000004008e8
   122615: T1773966 ifetch       4 byte(s) @ 0x00000000004008ec 11000421   add    %w1 $0x0001 lsl $0x00 -> %w1
   122616: T1773966 ifetch       4 byte(s) @ 0x00000000004008f0 b9004801   str    %w1 -> +0x48(%x0)[4byte]
   122617: T1773966 write        4 byte(s) @ 0x00000000004a7ab8 by PC 0x00000000004008f0
   122618: T1773966 ifetch       4 byte(s) @ 0x00000000004008f4 d65f03c0   ret    %x30
   122619: T1773966 ifetch       4 byte(s) @ 0x0000000071170610 52801168   movz   $0x008b lsl $0x00 -> %w8
   122620: T1773966 ifetch       4 byte(s) @ 0x0000000071170614 d4000001   svc    $0x0000
   122621: T1773966 <marker: timestamp 13309327193842230>
   122622: T1773966 <marker: tid 1773966 on core 51>
   122623: T1773966 <marker: syscall xfer from 0x71170618>
------------------------------------------------------------
<...>
------------------------------------------------------------
   384400: T1773966 <marker: timestamp 13309327193843065>
   384401: T1773966 <marker: tid 1773966 on core 51>
   384402: T1773966 ifetch       4 byte(s) @ 0x0000000000424670 a9b77bfd   stp    %x29 %x30 %sp $0xffffffffffffff70 -> -0x90(%sp)[16byte] %sp
   384403: T1773966 write       16 byte(s) @ 0x0000ffffe33a8960 by PC 0x0000000000424670
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
      12050265 total (fetched) instructions
          6447 total unique (fetched) instructions
             0 total non-fetched instructions
            39 total prefetches
        210230 total data loads
       2607246 total data stores
             0 total icache flushes
             0 total dcache flushes
            21 total threads
          2550 total scheduling markers
            22 total transfer markers
             0 total function id markers
             0 total function return address markers
             0 total function argument markers
             0 total function return value markers
             0 total physical address + virtual address marker pairs
             0 total physical address unavailable markers
           105 total other markers
         13584 total encodings
  Thread 1773982 counts:
        600366 (fetched) instructions
           399 unique (fetched) instructions
             0 non-fetched instructions
  <...>

Top opcodes in the trace from the opcode_mix [5] tool:
  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type opcode_mix -module_file drmemtrace.threadsig.[0-9]*.dir/raw/modules.log
  Opcode mix tool results:
         12050265 : total executed instructions
          1604469 :     bcond
          1405678 :       add
          1403688 :      subs
          1403183 :       stp
           800687 :       and
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
