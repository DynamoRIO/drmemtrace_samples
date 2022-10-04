Sample x86_64 trace directory contents:
+ drmemtrace.threadsig.1774668.2588.dir/trace/* == the trace itself, with a
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
<record#>: T<tid> <record details>
------------------------------------------------------------
        1: T1774673 <marker: version 4>
        2: T1774673 <marker: filetype 0x240>
        3: T1774673 <marker: cache line size 64>
        4: T1774673 <marker: chunk instruction count 10000000>
        5: T1774673 <marker: page size 4096>
<...>
   173369: T1774669 ifetch       3 byte(s) @ 0x0000000000402680 f3 48 a5             rep movsq
   173370: T1774669 read         8 byte(s) @ 0x00007f8881e37c70 by PC 0x0000000000402680
   173371: T1774669 write        8 byte(s) @ 0x00007f8881e37a70 by PC 0x0000000000402680
------------------------------------------------------------
   173372: T1774668 <marker: timestamp 13309323446399692>
   173373: T1774668 <marker: tid 1774668 on core 3>
   173374: T1774668 ifetch       4 byte(s) @ 0x000000000040361a 48 8b 45 f8          mov    -0x08(%rbp), %rax
   173375: T1774668 read         8 byte(s) @ 0x00007ffdf9d85088 by PC 0x000000000040361a
   173376: T1774668 ifetch       1 byte(s) @ 0x000000000040361e 5d                   pop    %rbp
   173377: T1774668 read         8 byte(s) @ 0x00007ffdf9d85090 by PC 0x000000000040361e
   173378: T1774668 ifetch       1 byte(s) @ 0x000000000040361f c3                   ret
   173379: T1774668 read         8 byte(s) @ 0x00007ffdf9d85098 by PC 0x000000000040361f
<...>
    47292: T1774668 ifetch       6 byte(s) @ 0x00000000004053bd 0f 85 e5 01 00 00    jnz    $0x00000000004055a8
    47293: T1774668 <marker: kernel xfer from 0x4053c3 to handler>
    47294: T1774668 <marker: timestamp 13309323446391801>
    47295: T1774668 <marker: tid 1774668 on core 3>
    47296: T1774668 ifetch       1 byte(s) @ 0x000000000040257d 55                   push   %rbp
    47297: T1774668 write        8 byte(s) @ 0x00007ffdf9d843f0 by PC 0x000000000040257d
    47298: T1774668 ifetch       3 byte(s) @ 0x000000000040257e 48 89 e5             mov    %rsp, %rbp
    47299: T1774668 ifetch       3 byte(s) @ 0x0000000000402581 89 7d fc             mov    %edi, -0x04(%rbp)
    47300: T1774668 write        4 byte(s) @ 0x00007ffdf9d843ec by PC 0x0000000000402581
    47301: T1774668 ifetch       4 byte(s) @ 0x0000000000402584 48 89 75 f0          mov    %rsi, -0x10(%rbp)
    47302: T1774668 write        8 byte(s) @ 0x00007ffdf9d843e0 by PC 0x0000000000402584
    47303: T1774668 ifetch       4 byte(s) @ 0x0000000000402588 48 89 55 e8          mov    %rdx, -0x18(%rbp)
    47304: T1774668 write        8 byte(s) @ 0x00007ffdf9d843d8 by PC 0x0000000000402588
    47305: T1774668 ifetch       4 byte(s) @ 0x000000000040258c 83 7d fc 1a          cmp    -0x04(%rbp), $0x1a
    47306: T1774668 read         4 byte(s) @ 0x00007ffdf9d843ec by PC 0x000000000040258c
    47307: T1774668 ifetch       2 byte(s) @ 0x0000000000402590 75 0f                jnz    $0x00000000004025a1
    47308: T1774668 ifetch       6 byte(s) @ 0x0000000000402592 8b 05 5c 0f 0e 00    mov    <rel> 0x00000000004e34f4, %eax
    47309: T1774668 read         4 byte(s) @ 0x00000000004e34f4 by PC 0x0000000000402592
    47310: T1774668 ifetch       3 byte(s) @ 0x0000000000402598 83 c0 01             add    $0x01, %eax
    47311: T1774668 ifetch       6 byte(s) @ 0x000000000040259b 89 05 53 0f 0e 00    mov    %eax, <rel> 0x00000000004e34f4
    47312: T1774668 write        4 byte(s) @ 0x00000000004e34f4 by PC 0x000000000040259b
    47313: T1774668 ifetch       1 byte(s) @ 0x00000000004025a1 90                   nop
    47314: T1774668 ifetch       1 byte(s) @ 0x00000000004025a2 5d                   pop    %rbp
    47315: T1774668 read         8 byte(s) @ 0x00007ffdf9d843f0 by PC 0x00000000004025a2
    47316: T1774668 ifetch       1 byte(s) @ 0x00000000004025a3 c3                   ret
    47317: T1774668 read         8 byte(s) @ 0x00007ffdf9d843f8 by PC 0x00000000004025a3
    47318: T1774668 ifetch       7 byte(s) @ 0x0000000000407bb0 48 c7 c0 0f 00 00 00 mov    $0x0000000f, %rax
    47319: T1774668 ifetch       2 byte(s) @ 0x0000000000407bb7 0f 05                syscall
    47320: T1774668 <marker: timestamp 13309323446391808>
    47321: T1774668 <marker: tid 1774668 on core 3>
    47322: T1774668 <marker: syscall xfer from 0x407bb9>
------------------------------------------------------------
    47323: T1774669 <marker: timestamp 13309323446393338>
    47324: T1774669 <marker: tid 1774669 on core 11>
    47325: T1774669 ifetch       3 byte(s) @ 0x0000000000467c51 48 85 c0             test   %rax, %rax
<...>
------------------------------------------------------------
    47603: T1774668 <marker: timestamp 13309323446395891>
    47604: T1774668 <marker: tid 1774668 on core 3>
    47605: T1774668 ifetch       4 byte(s) @ 0x00000000004053c3 48 8b 45 c8          mov    -0x38(%rbp), %rax
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
      15884769 total (fetched) instructions 
          9016 total unique (fetched) instructions
      12600068 total non-fetched instructions                                           
             0 total prefetches         
      15219672 total data loads                                                         
      17411556 total data stores                                                        
             0 total icache flushes                                                     
             0 total dcache flushes                                                     
            21 total threads                                                            
         21914 total scheduling markers                                                 
            64 total transfer markers                                                   
             0 total function id markers                                                
             0 total function return address markers                                    
             0 total function argument markers                
             0 total function return value markers 
             0 total physical address + virtual address marker pairs
             0 total physical address unavailable markers
           105 total other markers          
         15649 total encodings         
  Thread 1774670 counts:                    
        790316 (fetched) instructions  
           346 unique (fetched) instructions                                            
        629999 non-fetched instructions 
  <...>

Top opcodes in the trace from the opcode_mix [5] tool:
  $ bin64/drrun -t drcachesim -indir drmemtrace.threadsig.[0-9]*.dir -simulator_type opcode_mix -module_file drmemtrace.threadsig.[0-9]*.dir/raw/modules.log
  Opcode mix tool results:
         28484837 : total executed instructions
         12800000 :  rep movs
          2400121 :   vmovdqa
          1608677 :       cmp
          1600378 :   vmovdqu
          1006366 :       add
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
