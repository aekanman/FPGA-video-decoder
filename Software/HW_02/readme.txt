Worker CPU 2 Application

DESCRIPTION:
This Nios II worker receives mailbox commands from the main decoder. It handles
chroma lossless coefficient decoding and the top slice of frame color
conversion.

SOFTWARE SOURCE FILES:
- worker_cpu_2.c: mailbox loop, chroma lossless decode command, top color slice
- decoder/lossless_decode.c: entropy/run-length coefficient decoding
- decoder/ycbcr_to_rgb.c: color block conversion/display packing
- common/: shared block types, quantization tables, and debug utilities

NOTES:
This worker mirrors Worker CPU 1 by design so the multi-core partitioning is
easy to inspect and compare.
