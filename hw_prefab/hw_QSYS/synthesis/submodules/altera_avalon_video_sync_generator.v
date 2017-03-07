// (C) 2001-2015 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


`timescale 100 ps / 100 ps


////////////////////////////////////////////////////////////////
//
// Video Sync Generator
//     (altera_avalon_video_sync_generator.v)
//
// Author: chang
//
// OVERVIEW
//    This IP core is the output stage of a video-pipeline. The outputs
//    from this core can be directly-connected to an LCD display panel
//    (e.g. Toppoly TD043MTEA1).    
// 
//     This core adds H-sync and V-sync signals to an incoming stream of
//     B,G,R pixel-data.   Also, it "meters" the delivery of 
//     the B,G,R stream so bytes and sync-signals are produced on the
//     output with correct timing for the LCD display.
//
// DEPLOYMENT
//     This core is implemented as an SOPC Builder component with a
//     single Avalon-ST data-sink (input) and LCD-driving export
//     (output) signals. An associated TCL Metadata file declares this
//     core and  its interfaces for use in an SOPC Builder system.
//
//     One noteworthy & unusual feature is that this core sports no
//     Avalon-MM slave whatsoever.  You couldn't talk to it with your
//     CPU even if you wanted to.  It's purpose in life is to condition a
//     stream of pixel-data and stream it to an off-chip LCD display
//     with correct timing.
//         
// PLACEMENT IN A SYSTEM
//     The outputs from this core can directly drive an LCD display
//     module.  The input is a sequential stream of BGR pixel-data in
//     row-major order.  An Avalon-ST SOP (Start-Of-Packet) pulse
//     marks the first pixel in each frame.
//
//     This core is completely agnostic about the pixel-data
//     source. In a typical application, an Avalon-ST FIFO would drive BGR
//     data to this component.  The ultimate data-source might be a
//     frame-buffer in memory (via a DMA), or some external video
//     source.  This component doesn't care.  It just takes a stream
//     of incoming pixel-data and delivers it to an LCD display with
//     correct timing, accompanied by synchronization signals.
//
// PARAMETERS
//     The low-level timing of the H-sync and V-sync waveforms (and
//     pixel-delivery) are determined by the various timing
//     parameters.  Different displays and different resolutions may
//     require different timing parameters.  These parameters have
//     been named to agree with terminology used in the Toppoly
//     TD043MTEA1 datasheet.  These are fairly-typical names for these
//     kinds of timing parameters.
//
//     The default values of these parameters are suitable for
//     connection to the TD043MTEA1 panel.
//
// ERROR CONDITIONS
//     Under normal operation, the incoming pixel-data stream will
//     never "run dry" (in other words: the Avalon-ST source will
//     always have a "valid" datum whenever this module's Avalon-ST
//     sink is "ready.")  If a condition arises wherein the source is
//     not signalling "valid" when this core is signalling "ready,"
//     then the behavior of the outputs for the remainder of that
//     frame-time (packet) is undefined.
//
//     It is the responsibility of the system-designer to make sure
//     the data-source for this core never "runs dry."  If it does,
//     the result will (temporarily) be garbage on the LCD-display.
//
// PACKETS = FRAMES
//     The incoming data is assumed to contain the correct number of
//     pixels (LCD_NUM_ROWS * LCD_NUM_COLUMNS) in a packet.  Packets
//     with fewer or more pixels will result in undefined behavior on
//     the LCD screen.  Every first beat of every packet (=frame)
//     must be accompanied by a SOP (start-of-packet) pulse on the
//     Avalon-ST interface.
//
// HOW SYNCHRONIZATION WORKS.
//     This core is the synchronization source.  It is a free-running
//     sync-generator.  There's nothing you can do to stop it (well,
//     OK.  You can assert reset_n).  Other than that: It comes out of
//     reset and runs forever, frame-sequence after frame-sequence,
//     nonstop.  Whether there's incoming data or not.
//
//     That's a key point, which is why I'm emphasizing it.  This is a
//     free-running generator of timing waveforms, which runs forever
//     and always does the same thing.  This has some cardinal
//     benefits:
//           * It's behavior is easy to explain...
//           * And easy to implement...
//           * And easy to document...
//           * And easy to test.
//
//     So.  If this thing runs all the time, how can we be sure that
//     the first pixel in a frame (=packet) is presented to the LCD
//     screen at the right point in the H/Vsync sequence? (In other
//     words, how does this core synchronize the video datastream with
//     the video H/V timing waveforms?)
//
//     Easy.  It just holds-off the incoming datastream (by setting
//     ready=0 on its Avalon-ST sink).  The datasource (whatever it
//     is) tells us that some particular pixel is the first one in a
//     frame by asserting SOP for that pixel. If this core sees "SOP"
//     on its input, then it will HOLD the incoming datastream *until
//     the appropriate time in the sync sequence*.  The appropriate
//     time to present the first pixel happens to be: the first pixel
//     when DEN is true after the next Vsync (VD) signal.  
//     So: This core responds to a SOP by "backpressuring" the
//     incoming data stream until the next new frame rolls-around.
//
//     And, as we said before: The frame-sequence is always rolling,
//     and never stops.
//
// B,G,R TIME-MULTIPLEXING
//     The initial deployment of this core is in the Nios Embedded
//     Software Evaluation Kit (EEK).  this core drives signals to an
//     LCD-display daughtercard assembly, attached through an HSMC
//     connector.  In this particular hardware configuration, this
//     core's outputs are not connected directly to the Toppoly LCD
//     display module.  Instead, they are connected to an Altera MAX
//     chip (on the daughtercard).  This MAX chip allows the B,G,and R
//     datastreams to be time-multiplexed over the same 8 wires.
//     Thus: (BGR) data is presented as "triplets" of three clocks
//     each on the same 8 wires running to the daughtercard.
//
//
//         +--------+                   +------+        +---------------+
//         |        |        [H]        |      |---/--->| R             |
//   B,G,R |  This  | B,G,R  [S] B,G,R  |      |   8    |               |
//   --/-->|  Core  |----/-->[M]---/--->| MAX  |---/--->| G    LCD      |
//     8   |        |    8   [C]   8    |      |   8    |     Display   |
//         +--------+                   |      |---/--->| B             |
//                                      +------+   8    +---------------+
//
//               Configuration for EEK daughtercard
//                  (clocks and sync-signals omitted for clarity)
//
// 
//    In this configuration, each pixel in the datastream is
//    represented by three consecutive "beats" of data on the
//    Avalon-ST input.
//
//    That's great for the EEK, but this core would be much more
//    useful if it could also drive LCD screens which didn't happen to
//    reside on a daughtercard.  And, of course, it can.
//
//    Because this core doesn't really *do anything* with the
//    pixel-data, it doesn't really care about how wide it is, or what
//    it means.  The only "operational" part of the incoming Avalon-ST
//    input interface is the SOP pulse--the rest of the data is just
//    payload which we ignore.
//
//    Consequently: This core can be parameterized to accept a pixel
//    datastream of any bit-width (DATA_STREAM_BIT_WIDTH), and one
//    pixel can be divided into any number of "beats" on the Avalon-ST
//    interface (BEATS_PER_PIXEL). These two parameters allow this
//    core to be used directly with an LCD display module, or with the
//    module attached through some sort of EEK-daughtercard-like
//    multiplexing scheme.
//
// CLOCKING
//     The "clk" input to this core serves as the timebase.  The core
//     delivers one "beat" per clock.  Note that this is NOT the same
//     as one pixel per clock, because pixels might (or might not) be
//     broken-up into multiple beats.  In other words, you can
//     configure this core to pass-through a 24-bit (R,G,B) stream, or
//     you can configure it to pass-through an 8-bit stream which
//     transferrs B, G, and R, pixel-data bytewise-sequentially, three
//     clocks per pixel.  This is determined by the BEATS_PER_PIXEL parameter.
//
// REGISTERED OUTPUTS
//     The several synchronization pulses are
//     computed as decodes from counters.  We need to run these
//     through simple synchronization delay-registers, otherwise they
//     *will* glitch, and the display will do who-knows-what.
//
//     Because we register all the sync-outputs, we also have to
//     register the pixel-data...or else it will be out-of-sync with
//     the sync pulses--and you wouldn't want that.
//
//     And, it would appear: the MAX chip expects our data to be produced
//     on the *rising* edge of the clock. So this core is written to (optionally)
//     produce its RGB-data and sync-signals with posedge output-timing.
//
// HARD SYNCHRONIZATION (Just-barely-unimplemented feature)
//     As-designed, this core comes out of reset generating
//     frame-sequences, and keeps generating them forever.  It's only
//     time-reference is the reset_n signal (and the clock, of
//     course).  And that's the way it is.
//
//     But I can imagine the day when someone might want to *force*
//     the LCD-display frame-update to be synchronized with some other
//     source.  So: Built-into all the counters is a "sync_reset"
//     signal. This signal is tied to zero (1'b0) at the top-level of
//     this module.  But if, one day, you want to expose this external
//     signal: just run "sync_reset" to the top and drive it any way
//     you want.
//
////////////////////////////////////////////////////////////////
 


module altera_avalon_video_sync_generator
  (
   // Global Signals
   clk,
   reset_n,
   // sync_reset    // Uncomment to get a synchronization-input.

   // Avalon ST sink
   ready,
   valid,
   data,   // Passed straight-through (with registers) to "RGB_OUT" output.
   sop,    
   eop,    // Required by Avaon-ST spec.  Unused in this core.
   empty,  // Required by Avaon-ST spec.  Unused in this core.
   
   // LCD Panel Signals
   //     "clk" to the MAX chip is just "clk." (I assume the MAX
   //     chip somehow makes the signal "NCLK" (once-per-pixel) which
   //     the LCD-display module expects.)
   RGB_OUT,     
   HD,
   VD,
   DEN
   );

   
   //////////////// 
   // PARAMETERS
   //
   //   These parameters allow this module to drive different displays
   //   at different resolutions. Their default-values are set to
   //   drive a Toppoly TD043MTEA1 through data-multiplexing logic,
   //   as-deployed in the Cyclone II Embedded Evaluation Kit
   //   with HSMC LCD Daughtercard.
   //
   parameter DATA_STREAM_BIT_WIDTH = 8;
   parameter BEATS_PER_PIXEL       = 3;
   parameter OUTPUT_CLOCK_EDGE     = 1;  // 0 = negedge, 1 = posedge

   parameter NUM_ROWS              = 480;
   parameter NUM_COLUMNS           = 800;
   parameter H_BLANK_PIXELS        = 216;
   parameter H_FRONT_PORCH_PIXELS  = 40;
   parameter H_SYNC_PULSE_PIXELS   = 1;
   parameter H_SYNC_PULSE_POLARITY = 0;
   parameter V_BLANK_LINES         = 35;
   parameter V_FRONT_PORCH_LINES   = 10;
   parameter V_SYNC_PULSE_LINES    = 1;
   parameter V_SYNC_PULSE_POLARITY = 0;

     // Should be Cols + Hbp + Hfp, but.. why risk it?
   parameter TOTAL_HSCAN_PIXELS    = 1056;
     // Should be Rows + Vbp + Vfp, but.. why risk it?
   parameter TOTAL_VSCAN_LINES     = 525;   

   // Derived parameters:
   //     (TOTAL_...  might go in here some day?)
   localparam COLUMN_COUNTER_WIDTH       = (log2(TOTAL_HSCAN_PIXELS ));
   localparam ROW_COUNTER_WIDTH          = (log2(TOTAL_VSCAN_LINES  ));
    // Make this wider to avoid zero-condition.
   localparam PIXEL_PART_COUNTER_WIDTH   = (log2(BEATS_PER_PIXEL+1  )); 

   // Global Signals
   input     reset_n;
   input     clk;
   // input     sync_reset;  // Un-comment to get sync-reset feature
   wire      sync_reset = 1'b0;   // Comment-out to get sync-reset feature.
   

   // LCD Panel Signals
   output [(DATA_STREAM_BIT_WIDTH - 1):0] RGB_OUT;
   output 				  HD;
   output 				  VD;
   output 				  DEN;
   
   // Avalon ST sink
   output 				  ready;
   input 				  valid;
   input [(DATA_STREAM_BIT_WIDTH - 1):0]  data;
   input 				  sop;
   input 				  eop;

   input 				  empty;  

   // The function for logarithms calculation.
   function integer log2;
      input [31:0] 	  value;
      for (log2=0; value>0; log2=log2+1)
	value = value>>1;
   endfunction // log2

   ////////////////
   // Straight-through connection of pixel-data
   //
   //    We just ignore the pixel-data and pass it on through
   //    without alteration.
   //
   wire [(DATA_STREAM_BIT_WIDTH - 1):0] pRGB_OUT = data;
    
   ////////////////
   // Pixel-rate divider.
   //
   // If BEATS_PER_PIXEL = 1, this is just a do-nothing counter which
   // doesn't count, the output of which is not used, and which gets
   // "eaten" by Quartus.
   //
   // If BEATS_PER_PIXEL is greater than one, then this produces a
   // synchronous signal that will go true for one clock-cycle every N
   // clocks (where N = BEATS_PER_PIXEL).
   //
   // (Note that the pixel_part_counter signal might be one or two bits
   // wider than we might really like.  Since this is either going to
   // be 2 bits (for BEATS_PER_PIXEL = 3) or get totally-eaten by 
   // the synthesis step (for BEATS_PER_PIXEL = 1), it doesn't really
   // matter if we build a 4-bit counter when we might have gotten
   // away with only 2. After all: Quartus will eat the extra two
   // bits anyhow.
   //
   reg  [PIXEL_PART_COUNTER_WIDTH : 0] pixel_part_counter;
   
     // Compute the next counter-value with a continuous
     // assignment to simplify the always-block below and make it more
     // readable.
   wire [PIXEL_PART_COUNTER_WIDTH : 0] next_pixel_part_counter;
   assign next_pixel_part_counter = (pixel_part_counter == (BEATS_PER_PIXEL - 1)) ?
			             0                                            :
			             pixel_part_counter + 1                       ;
   

   always @(posedge clk or negedge reset_n) begin
      if (reset_n == 0)             // Async reset term
	pixel_part_counter <= 0;
      else
	pixel_part_counter <= next_pixel_part_counter;
   end


   // If there's one-clock-per-pixel, then this whole mess devolves to
   // a constant and Quartus sweeps it away. 
   //
   wire  pixel_count_enable;
   assign pixel_count_enable = (BEATS_PER_PIXEL == 1)    ? 
			       1'b1                      : 
			       (pixel_part_counter == 0) ;


   ////////////////
   // Column counter
   //
   // This is just a fixed period free running pixel-counter
   // with (of course) an async-reset.
   //
   reg  [COLUMN_COUNTER_WIDTH-1:0] column_counter;   

     // Compute the next counter-value with a continuous
     // assignment to simplify the always-block below and make it more
     // readable.
   wire [COLUMN_COUNTER_WIDTH-1:0] next_column_counter;
   assign next_column_counter = (column_counter == (TOTAL_HSCAN_PIXELS - 1)) ? 
            	                0                                            :
	                        column_counter + 1   		             ;
   
     // Just to re-name the enable-signal.
   wire   column_count_enable = pixel_count_enable;
   
   
   always @(posedge clk or negedge reset_n) begin
      if (reset_n == 0)               // Async-clear term.
	column_counter <= 0;
      else begin
	if (sync_reset)               // Synchronous-clear term.
	  column_counter <= 0;
	else 
	  if (column_count_enable)    // Synchronous count-enable term. 
	    column_counter <= next_column_counter;
      end
   end // always @ (posedge clk or negedge reset_n)


   ////////////////
   // Row counter
   //
   // Same principle as the column-counter, except that it's only
   // enabled once per line.
   //
   reg  [ROW_COUNTER_WIDTH-1 : 0] row_counter;
   
     // Compute the next counter-value with a continuous
     // assignment to simplify the always-block below and make it more
     // readable.
   wire [ROW_COUNTER_WIDTH-1 : 0] next_row_counter;
   assign next_row_counter = (row_counter == (TOTAL_VSCAN_LINES - 1)) ? 
                              0                                       : 
	                      row_counter + 1                         ;

     // Enable row-counter when column-counter is at its max-value.
     // It is only enabled once per pixel.
   wire row_counter_enable = column_count_enable                          && 
	                         (column_counter == (TOTAL_HSCAN_PIXELS - 1))  ; 
   
   always @(posedge clk or negedge reset_n) begin
      if (reset_n == 0)                 // Async-clear term
	row_counter <= 0;
      else begin
	 if (sync_reset)                // Sync-clear term
	   row_counter <= 0;
	 else
	   if (row_counter_enable)      // Sync count-enable term
	     row_counter <= next_row_counter;
      end
   end // always @ (posedge clk or negedge reset_n)
   

   ////////////////
   // H/V Sync pulses
   //
   // The HD sync-pulse is just a single pixel-period long, asserted 
   // at the beginning of the H-sequence (column_counter == 0).
   //
   // The VD sync-pulse is just a single line long, asserted at the
   // beginning of the V-sequence (row_counter == 0).  
   //
   // These go straight to the display.  They're both active-low.
   //
   // The "pX" is used as the "pre-registered" version of a signal,
   // "X", which comes out of a simple 1-clock-delay flip-flop.
   //
   wire pHD = ((column_counter >= 0) && (column_counter <= (H_SYNC_PULSE_PIXELS - 1)));
   wire pVD = ((   row_counter >= 0) && (   row_counter <= (V_SYNC_PULSE_LINES - 1 )));

   //////////////////
   // Polarize HD/VD according to polarity.
   //
   wire ppHD = (H_SYNC_PULSE_POLARITY == 1) ? pHD : ~pHD;
   wire ppVD = (V_SYNC_PULSE_POLARITY == 1) ? pVD : ~pVD;

   ////////////////
   // DEN (data qualification) pulse
   //
   // It's easiest to compute separate H/V qualification pulses,
   // then AND them together.
   // 
   // Blanking is false (thus: pixel-data is "qualified") after the
   // blanking-interval, but before the so-called "front porch."
   //
   wire h_active = (column_counter >= H_BLANK_PIXELS              ) &&
                   (column_counter <  H_BLANK_PIXELS + NUM_COLUMNS)  ;
   
   wire v_active = (   row_counter >= V_BLANK_LINES               ) &&
                   (   row_counter <  V_BLANK_LINES  + NUM_ROWS   )  ;

   wire pDEN     = h_active & v_active;

   ////////////////
   // Register the LCD outputs.
   //
   //  Any time we decode anything from a counter, it can glitch.
   //  So, we have to add a resynchronization-register to the output.
   //
   //  Now we want to implement a circuit which can produce negative or
   //  positive-edge timing for the oputput signals depending on the
   //  value of a parameter. We'll define a signal "output_clk"
   //  which is either (a) just plain-old clk, or (b) ~clk.
   // 
   reg 	HD;
   reg 	VD;
   reg 	DEN;
   reg 	[(DATA_STREAM_BIT_WIDTH - 1):0] RGB_OUT;

   wire output_clk = (OUTPUT_CLOCK_EDGE == 0) ? ~clk : clk;
   
   
   always @(posedge output_clk or negedge reset_n) begin
      if (reset_n == 0) begin
	 HD      <= 1;
	 VD      <= 1;
	 DEN     <= 0;
	 RGB_OUT <= 0;
      end
      else begin
	 HD      <= ppHD;
	 VD      <= ppVD;
	 DEN     <= pDEN;
	 RGB_OUT <= pRGB_OUT;
      end
   end


   ////////////////
   // First-pixel signal (internal)
   //
   // This is true only on the (first beat of the) first pixel of the
   // first line of a frame.
   //
   // We need this signal because we need to know how to respond to a
   // SOP on the Avalon-ST input.
   //
   // This is implemented with a brute-force hard-decode for a
   // certain count-value on the row-counters and column-counters.
   //
   wire first_pixel = (column_counter == H_BLANK_PIXELS) &&
	              (row_counter    == V_BLANK_LINES ) &&
	              ((pixel_part_counter == 1) || (BEATS_PER_PIXEL == 1)) ;

   ////////////////
   // ready-signal computation
   //
   // We actually synchronize the incoming datastream by PAUSING the
   // dataflow to line-up the SOP-marked data with the "first pixel" in a
   // frame.
   //
   // We only want to STOP the incoming data if:
   //     A) "SOP" is received but we're not at the first pixel yet, or...
   //     B) There's blanking.  We don't accept new data pixels during
   //        blanked (pDENB = 0) parts of the frame-sequence
   //
   wire stop_dataflow = (pDEN == 0)           ||
	                    (sop && ~first_pixel)  ;

   assign ready = ~stop_dataflow;
endmodule // altera_avalon_video_sync_generator

