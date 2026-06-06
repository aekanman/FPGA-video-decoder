Main Decoder Application

DESCRIPTION:
This Nios II application coordinates the FPGA MJPEG video decoder. It owns SD
card input, playback controls, mailbox scheduling for worker CPUs, hardware
IDCT DMA transfers, and display buffer registration.

SOFTWARE SOURCE FILES:
- main_decoder_app.c: application entry point and playback/control loop
- decoder/mjpeg423_decoder.c: frame parsing, worker coordination, DMA-backed
  IDCT scheduling, and display-buffer handoff
- decoder/idct.c: software IDCT reference implementation
- decoder/lossless_decode.c: entropy/run-length coefficient decoding
- decoder/ycbcr_to_rgb.c: color block conversion/display packing
- common/: shared block types, quantization tables, and debug utilities

NOTES:
Generated BSP and build-output files are retained with the project for hardware
context. See the top-level README for the full architecture overview.
