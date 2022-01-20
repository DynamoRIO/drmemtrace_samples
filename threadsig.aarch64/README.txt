Sample aarch64 trace directory contents:
+ drmemtrace.threadsig.1287316.5396.dir/trace/* == the trace itself, with a
  separate file per application thread.
+ drmemtrace.threadsig.1287316.5396.dir/raw/modules.log == the list of libraries
  mapped into the application address space.
+ binaries/ == binaries for the librares listed in the modules.log file.
+ threadsig.cpp == the source code of the traced application.
+ README.txt == this file.

The memory address tracer we use is part of the "drcachesim" open-source
tool [1], which is part of the DynamoRIO dynamic binary instrumentation
framework [2].

The trace format used by analysis tools is described in online docs [3].  A
simple analysis tool to look at is basic_counts.cpp [4].

A human-readable view of the sample trace highlighting thread switches and
a signal handler:
--------------------------------------------------
  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type view 2>&1 | less
  <...>
  T1287316   0x000000007132b464  52801168   movz   $0x008b lsl $0x00 -> %w8
  T1287316   0x000000007132b468  d4000001   svc    $0x0000
  T1287316 <marker: timestamp 13270240897157787>
  T1287316 <marker: tid 1287316 on core 29>
  T1287316 <marker: syscall xfer from 0x7132b46c>
  ------------------------------------------------------------
  T1287317 <marker: timestamp 13270240897161378>
  T1287317 <marker: tid 1287317 on core 30>
  T1287317   0x000000000042e270  f100001f   subs   %x0 $0x0000 lsl $0x00 -> %xzr
  T1287317   0x000000000042e274  540000e0   b.eq   $0x000000000042e290
  T1287317   0x000000000042e290  d280001d   movz   $0x0000 lsl $0x00 -> %x29
  T1287317   0x000000000042e294  aa0c03e0   orr    %xzr %x12 lsl $0x00 -> %x0
  T1287317   0x000000000042e298  d63f0140   blr    %x10 -> %x30
  T1287317   0x0000000000401d10  a9ae7bfd   stp    %x29 %x30 %sp $0xfffffffffffffee0 -> -0x0120(%sp)[16byte] %sp
  T1287317     write 16 byte(s) @ 0xfffda57f6880
  <...>
  T1287316   0x0000000000400b28  a9425bf5   ldp    +0x20(%sp)[16byte] -> %x21 %x22
  T1287316     read  16 byte(s) @ 0xffffd79a6f50
  T1287316   0x0000000000400b2c  a8c57bfd   ldp    (%sp)[16byte] %sp $0x0000000000000050 -> %x29 %x30 %sp
  T1287316     read  16 byte(s) @ 0xffffd79a6f30
  T1287316   0x0000000000400b30  d65f03c0   ret    %x30
  T1287316 <marker: kernel xfer from 0x4023dc to handler>
  T1287316 <marker: timestamp 13270240897161609>
  T1287316 <marker: tid 1287316 on core 29>
  T1287316   0x00000000004008d8  7100681f   subs   %w0 $0x001a lsl $0x00 -> %wzr
  T1287316   0x00000000004008dc  540000c1   b.ne   $0x00000000004008f4
  T1287316   0x00000000004008e0  f0000520   adrp   <rel> 0x00000000004a7000 -> %x0
  T1287316   0x00000000004008e4  9129c000   add    %x0 $0x0a70 lsl $0x00 -> %x0
  T1287316   0x00000000004008e8  b9404801   ldr    +0x48(%x0)[4byte] -> %w1
  T1287316     read  4 byte(s) @ 0x4a7ab8
  T1287316   0x00000000004008ec  11000421   add    %w1 $0x0001 lsl $0x00 -> %w1
  T1287316   0x00000000004008f0  b9004801   str    %w1 -> +0x48(%x0)[4byte]
  T1287316     write 4 byte(s) @ 0x4a7ab8
  T1287316   0x00000000004008f4  d65f03c0   ret    %x30
  T1287316   0x000000007132b464  52801168   movz   $0x008b lsl $0x00 -> %w8
  T1287316   0x000000007132b468  d4000001   svc    $0x0000
  T1287316 <marker: timestamp 13270240897161623>
  T1287316 <marker: tid 1287316 on core 29>
  T1287316 <marker: syscall xfer from 0x7132b46c>
  T1287316 <marker: timestamp 13270240897165865>
  T1287316 <marker: tid 1287316 on core 29>
  T1287316   0x00000000004023dc  2a0003e4   orr    %wzr %w0 lsl $0x00 -> %w4
  T1287316   0x00000000004023e0  35003280   cbnz   $0x0000000000402a30 %w0
  T1287316   0x00000000004023e4  39504f80   ldrb   +0x0413(%x28)[1byte] -> %w0
  T1287316     read  1 byte(s) @ 0xfffda5c9a413
  T1287316   0x00000000004023e8  35000e40   cbnz   $0x00000000004025b0 %w0
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
      12050396 total (fetched) instructions
          6401 total unique (fetched) instructions
             0 total non-fetched instructions
            39 total prefetches
        210224 total data loads
       2607266 total data stores
             0 total icache flushes
             0 total dcache flushes
            21 total threads
          2738 total scheduling markers
            90 total transfer markers
             0 total function id markers
             0 total function return address markers
             0 total function argument markers
             0 total function return value markers
            63 total other markers
  Thread 1287317 counts:
        600344 (fetched) instructions
           374 unique (fetched) instructions
  <...>

Top opcodes in the trace:
  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type opcode_mix -module_file drmemtrace.threadsig.[0-9]*.dir/raw/modules.log
  Opcode mix tool results:
         12050396 : total executed instructions
          1604491 :     bcond
          1405742 :       add
          1403710 :      subs
          1403183 :       stp
           800681 :       and
           800039 :       sys
           403334 :       str
           401790 :      movz
  <...>

(If the opcode "xx" is displayed, it is part of a set of opcodes not yet
added to the decoder [8].)

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
[8] = https://github.com/DynamoRIO/dynamorio/issues/2626
[9] = https://github.com/DynamoRIO/dynamorio/releases/latest
