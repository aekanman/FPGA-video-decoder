# FPGA MJPEG Video Decoder

Design and implementation of a video decoder on an Altera Cyclone V FPGA board.

## At a Glance

Design and processing elements:

- 3 cores
- 4 mailboxes
- 1 read/write DMA path
- 1 two-dimensional IDCT hardware accelerator
- 1 periodic timer

Methods and tasks:

- Lossless decoding
- Parallelizing the cores
- Hardware accelerators
- System-level hardware/software co-design
- FPGA resource utilization
- Memory utilization
- Inverse discrete cosine transform (IDCT)
- Scheduling and synchronization
- Cache coherency

This repository contains a hardware/software video decoder built for an Intel/Altera FPGA system with Nios II soft processors. The design decodes an MJPEG-style stream from SD storage, runs lossless coefficient decoding and color conversion across worker CPUs, uses a hardware IDCT accelerator through msgDMA, and writes frames to the video display pipeline.

This project demonstrates:

- Multi-core embedded scheduling with mailbox command passing.
- Hardware/software co-design around an IDCT accelerator.
- DMA-based streaming between CPU memory and FPGA hardware.
- Block-based video decode stages: entropy decode, dequantization, IDCT, color conversion, and display buffering.
- Practical embedded tradeoffs, including cache coherency, polling loops, and generated toolchain artifacts.

## Architecture

- `Software/HW_00/main_decoder_app.c` is the main application. It initializes SD card access, video output, mailboxes, timers, and push-button controls, then orchestrates frame decode and display.
- `Software/HW_01/worker_cpu_1.c` is worker CPU 1. It receives mailbox commands, decodes the Y lossless stream, and handles the middle color-conversion slice.
- `Software/HW_02/worker_cpu_2.c` is worker CPU 2. It receives mailbox commands, decodes the chroma lossless stream, and handles the upper color-conversion slice.
- `IDCT/` contains the custom two-dimensional IDCT hardware block.
- `ip/` contains custom and third-party FPGA IP, including pixel conversion and SD controller logic.
- `HW_QSYS/` and `HW_config/` contain the generated platform system, top-level hardware configuration, reports, and bitstream outputs.

See `docs/architecture.md` for a diagram of the runtime pipeline and CPU/FPGA responsibilities.

At runtime, the decode path is:

1. The main CPU reads the video file header, frame headers, and compressed payload from SD storage.
2. Y, Cb, and Cr bitstreams are split from each frame payload.
3. Worker CPUs receive mailbox messages for lossless coefficient decoding.
4. The main CPU streams decoded DCT blocks through the hardware IDCT accelerator using msgDMA.
5. Color conversion is split across CPUs by row range.
6. The main CPU registers completed frame buffers with the video display driver.

The interesting engineering work is in the boundaries: deciding which stages run on worker CPUs, keeping frame buffers coherent across processors and DMA, preserving enough pipeline overlap for playback, and making the display path reliable despite polling, cache flushes, and generated toolchain artifacts.

## Repository Map

```text
.
|-- Software/
|   |-- HW_00/        Main decoder application and SD/video support
|   |-- HW_01/        Worker CPU 1 application
|   |-- HW_02/        Worker CPU 2 application
|   |-- *_bsp/        Generated Nios II BSP projects
|-- IDCT/             Custom hardware IDCT source
|-- ip/               Custom/third-party FPGA IP blocks
|-- HW_QSYS/          Generated Qsys platform files
|-- HW_config/        Quartus project, reports, and hardware outputs
|-- db/               Quartus compilation database
|-- incremental_db/   Incremental compilation cache
```

## Notable Implementation Details

- The software decoder uses 8x8 YCbCr and DCT block types defined in `Software/HW_00/common/mjpeg423_types.h`.
- Quantization and zig-zag tables live in each app's `common/tables.c`.
- `lossless_decode.c` handles JPEG-style run-length and variable-length integer decoding.
- `idct.c` provides a software IDCT implementation for the main app, while the deployed path also uses the hardware IDCT block and msgDMA.
- `ycbcr_to_rgb.c` currently has `NULL_COLORCONV` enabled, so the active path packs component channels directly for the display pipeline instead of running the full fixed-point RGB conversion.

## Known Tradeoffs

The current implementation keeps several embedded-system tradeoffs visible:

- Generated artifacts are retained with the project so the Quartus/Nios II system state is inspectable, even though that makes the repository larger than a source-only layout.
- Decoder support files are duplicated across the three Nios II apps because each app is generated and built as a separate software project. A shared-source layout would be cleaner, but would require toolchain-aware project changes.
- The main decoder function keeps file parsing, worker scheduling, DMA setup, frame buffering, and display registration close together. That makes the real-time pipeline easy to follow in one place, at the cost of a larger orchestration function.
- Cache handling uses broad `alt_dcache_flush_all()` calls. More targeted cache maintenance could reduce overhead, but would need board-level validation.
- DMA and mailbox synchronization use polling and busy-wait loops. Interrupt-driven or timeout-aware waits would improve robustness, but the current form keeps synchronization explicit for lab/debug visibility.
- The startup path searches for a hard-coded `V1_72.MPG` file before entering the browsing/playback loop. That simplifies demo startup and can be generalized later.
