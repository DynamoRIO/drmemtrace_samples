# drmemtrace_samples

Memory trace samples from DynamoRIO's drmemtrace tracer for its [drcachesim analyzer](http://dynamorio.org/dynamorio_docs/page_drcachesim.html).

## Trace format

The memory address tracer we use is part of the [drcachesim open-source
tool](http://dynamorio.org/dynamorio_docs/page_drcachesim.html), which is
part of the [DynamoRIO dynamic binary instrumentation
framework](http://dynamorio.org).  Here we summarize the tracing format.
See the [drcachesim documentation](http://dynamorio.org/dynamorio_docs/page_drcachesim.html)
for further information.

A trace contains a sequence of user-mode instruction and memory fetches for
each thread in a target application.  Each 32KB block of thread data has a
timestamp and records which cpu it executon on, allowing reconstructing the
thread interleaving at that granularity.

The trace format used by trace analysis tools is the [memref_t
structure](http://dynamorio.org/dynamorio_docs/union__memref__t.html)
([source
file](https://github.com/DynamoRIO/dynamorio/blob/master/clients/drcachesim/common/memref.h)).
A simple analysis tool to look at is
[basic_counts.cpp](https://github.com/DynamoRIO/dynamorio/blob/master/clients/drcachesim/tools/basic_counts.cpp).

A low-level textual view of a sample trace:

```
$ bin64/drrun -t drcachesim -indir drmemtrace*.dir -verbose 3 2>&1 | less
::142994.142994:: marker type 2 value 13214166611094895
::142994.142994:: marker type 3 value 0
new thread 142994 => core 0 (count=0)
::142994.142994::  @0x7fa4ca039f30 instr x3
::142994.142994::  @0x7fa4ca039f33 instr x5
::142994.142994::  @0x7fa4ca039f33 write 0x7fffb0db9b28 x8
::142994.142994::  @0x7fa4ca03ad30 instr x1
::142994.142994::  @0x7fa4ca03ad30 write 0x7fffb0db9b20 x8
::142994.142994::  @0x7fa4ca03ad31 instr x3
...
::142994.142994::  @0x7fa4c8afcddf instr x2
::142994.142995:: marker type 2 value 13214166611281895
::142994.142995:: marker type 3 value 3
new thread 142995 => core 1 (count=0)
::142994.142995::  @0x7fa4c8afcde1 instr x3
::142994.142995::  @0x7fa4c8afcde4 instr x2
::142994.142995:: marker type 0 value 0
::142994.142995:: marker type 2 value 13214166611282372
::142994.142995:: marker type 3 value 3
::142994.142995::  @0x7fa4c8afcde6 instr x2
::142994.142995::  @0x7fa4c605edba instr x1
::142994.142995::  @0x7fa4c605edba write 0x7fa4c89fd8e0 x8
...
```

It is a series of instruction fetch, data fetch, and metadata entries.  The
fetches contain addresses and sizes ("x3" is a size in the verbose
listing).  The addresses are all virtual (it is possible to [gather
physical addresses in some
circumstances](http://dynamorio.org/dynamorio_docs/page_drcachesim.html#sec_drcachesim_phys)).
The metadata "markers" indicate things like which core a thread executed
on, timestamps, an arriving signal causing a PC discontinuity, etc.

## Using a trace for core simulation

For using a trace in a core simulator, you will want to obtain the opcodes.
These are not part of the base trace.  They are obtained by decoding the
instruction fetch addresses from the binaries.  Library support makes this
straightforward.  A sample tool that does this is
[opcode_mix.cpp](https://github.com/DynamoRIO/dynamorio/blob/master/clients/drcachesim/tools/opcode_mix.cpp).
It uses library routines to read the "modules.log" file, which contains the
mappings of the binary and libraries from the traced execution, and map
those binaries into the address space, allowing examining the instruction
bytes.  The modules.log file and all of the binaries used by the
application are required, in addition to the trace itself.  For the vdso
the raw bytes are embedded in the modules file.

Other aspects of the trace which help core simulation are [discussed in our
documentation](http://dynamorio.org/dynamorio_docs/page_drcachesim.html#sec_drcachesim_core).
