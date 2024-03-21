Sample threadsig.x86_64.w_syscalls trace directory contents:
+ drmemtrace.threadsig.21646.1316.dir/trace/* == the trace itself, with a
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

A human-readable view of the sample trace highlighting thread switches,
a signal handler, and a kernel system call trace:
--------------------------------------------------
$ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type view -view_syntax intel 2>&1 | less
Output format:
<--record#-> <--instr#->: <---tid---> <record details>
------------------------------------------------------------
           1           0:       21646 <marker: version 6>
           2           0:       21646 <marker: filetype 0x8e40>
           3           0:       21646 <marker: cache line size 64>
           4           0:       21646 <marker: chunk instruction count 10000000>
           5           0:       21646 <marker: page size 4096>
<...>
       11910        9759:       21646 ifetch       1 byte(s) @ 0x00007fcae55ed1a8 c3                   ret    %rsp (%rsp)[8byte] -> %rsp (target 0x7fcae55efa9b)
       11911        9759:       21646 read         8 byte(s) @ 0x00007ffd1fded688 by PC 0x00007fcae55ed1a8
       11912        9760:       21646 ifetch       2 byte(s) @ 0x00007fcae55efa9b 31 ff                xor    %edi %edi -> %edi
       11913        9761:       21646 ifetch       5 byte(s) @ 0x00007fcae55efa9d e8 9e 14 00 00       call   $0x00007fcae55f0f40 %rsp -> %rsp 0xfffffff8(%rsp)[8byte]
       11914        9761:       21646 write        8 byte(s) @ 0x00007ffd1fded688 by PC 0x00007fcae55efa9d
       11915        9762:       21646 ifetch       4 byte(s) @ 0x00007fcae55f0f40 f3 0f 1e fa          nop    %edx
       11916        9763:       21646 ifetch       5 byte(s) @ 0x00007fcae55f0f44 b8 0c 00 00 00       mov    $0x0000000c -> %eax
       11917        9764:       21646 ifetch       2 byte(s) @ 0x00007fcae55f0f49 0f 05                syscall  -> %rcx %r11
       11918        9764:       21646 <marker: timestamp 13355492075699772>
       11919        9764:       21646 <marker: tid 21646 on core 3>
       11920        9764:       21646 <marker: system call 12>
       11921        9764:       21646 <marker: trace start for system call number 12>
       11922        9765:       21646 ifetch       2 byte(s) @ 0xffffffffb8495c36 66 90                data16 nop
       11923        9766:       21646 ifetch       1 byte(s) @ 0xffffffffb8495c38 c3                   ret    %rsp (%rsp)[8byte] -> %rsp (target 0xffffffffb841b994)
<...>
       13586       11429:       21646 ifetch       4 byte(s) @ 0xffffffffb841bf6a 48 c1 ea 20          shr    $0x0000000000000020 %rdx -> %rdx
       13587       11430:       21646 ifetch       5 byte(s) @ 0xffffffffb841bf6e e8 bd 9c 07 00       call   $0xffffffffb8495c30 %rsp -> %rsp 0xfffffff8(%rsp)[8byte]
       13588       11431:       21646 ifetch       2 byte(s) @ 0xffffffffb8495c30 89 f9                mov    %edi -> %ecx
       13589       11432:       21646 ifetch       2 byte(s) @ 0xffffffffb8495c32 89 f0                mov    %esi -> %eax
       13590       11432:       21646 <marker: trace end for system call number 12>
       13591       11432:       21646 <marker: timestamp 13355492075724084>
       13592       11432:       21646 <marker: tid 21646 on core 3>
       13593       11433:       21646 ifetch       6 byte(s) @ 0x00007fcae55f0f4b 48 3d 00 f0 ff ff    cmp    %rax $0xfffffffffffff000
<...>
     8289350     5704054:       21647 ifetch       3 byte(s) @ 0x00007fcae15fd512 f3 48 a5             rep movs %ds:(%rsi)[8byte] %rsi %rdi %rcx -> %es:(%rdi)[8byte] %rsi %rdi %rcx
     8289351     5704054:       21647 read         8 byte(s) @ 0x00007fcae47edc48 by PC 0x00007fcae15fd512
     8289352     5704054:       21647 write        8 byte(s) @ 0x00007fcae47ede48 by PC 0x00007fcae15fd512
     8289353     5704054:       21647 <marker: kernel xfer from 0x7fcae15fd512 to handler>
     8289354     5704054:       21647 <marker: timestamp 13355492075947847>
     8289355     5704054:       21647 <marker: tid 21647 on core 0>
     8289356     5704055:       21647 ifetch       4 byte(s) @ 0x00007fcae15fd3e9 f3 0f 1e fa          nop    %edx
     8289357     5704056:       21647 ifetch       1 byte(s) @ 0x00007fcae15fd3ed 55                   push   %rbp %rsp -> %rsp 0xfffffff8(%rsp)[8byte]
     8289358     5704056:       21647 write        8 byte(s) @ 0x00007fcae47ed3e0 by PC 0x00007fcae15fd3ed
     8289359     5704057:       21647 ifetch       3 byte(s) @ 0x00007fcae15fd3ee 48 89 e5             mov    %rsp -> %rbp
<...>
     8289413     5704084:       21647 ifetch       1 byte(s) @ 0x00007fcae15fd413 c3                   ret    %rsp (%rsp)[8byte] -> %rsp (target 0x7fcae50ae420)
     8289414     5704084:       21647 read         8 byte(s) @ 0x00007fcae47ed3e8 by PC 0x00007fcae15fd413
     8289415     5704085:       21647 ifetch       7 byte(s) @ 0x00007fcae50ae420 48 c7 c0 0f 00 00 00 mov    $0x000000000000000f -> %rax
     8289416     5704086:       21647 ifetch       2 byte(s) @ 0x00007fcae50ae427 0f 05                syscall  -> %rcx %r11
     8289417     5704086:       21647 <marker: timestamp 13355492075947861>
     8289418     5704086:       21647 <marker: tid 21647 on core 0>
     8289419     5704086:       21647 <marker: system call 15>
     8289420     5704086:       21647 <marker: syscall xfer from 0x7fcae50ae429>
     8289421     5704086:       21647 <marker: timestamp 13355492075947863>
     8289422     5704086:       21647 <marker: tid 21647 on core 0>
     8289423     5704086:       21647 <marker: timestamp 13355492075947865>
     8289424     5704086:       21647 <marker: tid 21647 on core 0>
     8289425     5704087:       21647 ifetch       3 byte(s) @ 0x00007fcae15fd512 f3 48 a5             rep movs %ds:(%rsi)[8byte] %rsi %rdi %rcx -> %es:(%rdi)[8byte] %rsi %rdi %rcx
     8289426     5704087:       21647 read         8 byte(s) @ 0x00007fcae47edc50 by PC 0x00007fcae15fd512
     8289427     5704087:       21647 write        8 byte(s) @ 0x00007fcae47ede50 by PC 0x00007fcae15fd512
<...>
--------------------------------------------------

It is a series of user-mode instruction fetch, data fetch, and metadata entries, and kernel
system call instruction fetch entries.  The fetches contain addresses and sizes and, for
instruction fetches, instruction encodings.  The addresses are all virtual (it is
possible to [gather physical addresses in some
circumstances](https://dynamorio.org/sec_drcachesim_phys.html)).
The metadata "markers" indicate things like which core a thread executed
on, timestamps, an arriving signal causing a PC discontinuity, etc.

The kernel execution during system calls is indicated by
TRACE_MARKER_TYPE_SYSCALL_TRACE_START and TRACE_MARKER_TYPE_SYSCALL_TRACE_END markers[3] which
have the system call number as their value.  The system call trace is collected using
drpttracer[9] which uses Intel-PT, on a system that supports the PT feature.
Currently, we have an instruction-only trace for the system call execution by the
kernel.  Data fetch entries will also be added in future. Some system calls that change the
control flow or terminate the thread/process do not have a trace available (since DynamoRIO
does not provide a post-syscall callback); this includes system calls like rt_sigreturn, exit,
execve, and exit_group.

For using a trace in a core simulator, the provided instruction encodings
can be decoded into opcodes and operands as is done with the "view" tool above.
This can be done with DynamoRIO's decoder, as in the provided sample tool
opcode_mix.cpp [5].  Other aspects of the trace which help core simulation
are discussed in our documentation [6].

The sample trace here is small but includes corner cases of thread
interleavings, signals, "non-fetched" instructions, and system calls.  The trace is from
the "threadsig" application whose source code is included.  It is an
adaptation of [7] with extra threads, signals, and string loop
instructions. Getting a user+kernel trace requires running the app using super-user
permissions.

The trace was generated by running:
  # Workaround for i#6486.
  $ sudo bash -c "echo 0 > /proc/sys/net/core/bpf_jit_harden"
  $ sudo bash -c "echo 1 > /proc/sys/net/core/bpf_jit_kallsyms"

  $ sudo bin64/drrun -t drcachesim -offline -enable_kernel_tracing -- ./threadsig 20 200000

Counts of event types in the trace from the basic_counts [4] tool are as follows. Note that the
count of userspace and kernel instructions are shown separately as well.

  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type basic_counts
  Basic counts tool results:
  Total counts:
      22650333 total (fetched) instructions
         60007 total unique (fetched) instructions
      12600382 total non-fetched instructions
      18558081 total userspace instructions
       4092252 total kernel instructions
             0 total prefetches
      15735202 total data loads
      17595815 total data stores
             0 total icache flushes
             0 total dcache flushes
            21 total threads
         44920 total scheduling markers
           108 total transfer markers
             0 total function id markers
             0 total function return address markers
             0 total function argument markers
             0 total function return value markers
             0 total physical address + virtual address marker pairs
             0 total physical address unavailable markers
           275 total system call number markers
            27 total blocking system call markers
           105 total other markers
        138769 total encodings
  Thread 21646 counts:
       6154205 (fetched) instructions
         58880 unique (fetched) instructions
           397 non-fetched instructions
       2346024 userspace instructions
       3808181 kernel instructions
  <...>

Top opcodes in the trace from the opcode_mix [5] tool are as follows. Note that this includes some
privileged opcodes too.

  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type opcode_mix -module_file drmemtrace.threadsig.[0-9]*.dir/raw/modules.log
  Opcode mix tool results:
         35250715 : total executed instructions
         12800446 :  rep movs
          2400108 :   vmovdqa
          1986601 :       cmp
          1843341 :       mov
  <...>
               28 :      iret
               28 :       cld
               28 :    xrstor
               28 :    xsavec
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
[9] = https://dynamorio.org/page_drpttracer.html
