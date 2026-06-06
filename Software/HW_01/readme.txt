Worker CPU 1 Application

DESCRIPTION:
This Nios II worker receives mailbox commands from the main decoder. It handles
Y-channel lossless coefficient decoding and the middle slice of frame color
conversion.

SOFTWARE SOURCE FILES:
- worker_cpu_1.c: mailbox loop, Y lossless decode command, middle color slice
- decoder/lossless_decode.c: entropy/run-length coefficient decoding
- decoder/ycbcr_to_rgb.c: color block conversion/display packing
- common/: shared block types, quantization tables, and debug utilities

NOTES:
This worker mirrors Worker CPU 2 by design so the multi-core partitioning is
easy to inspect and compare.
