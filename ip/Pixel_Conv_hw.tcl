# (C) 2001-2015 Altera Corporation. All rights reserved.
# Your use of Altera Corporation's design tools, logic functions and other 
# software and tools, and its AMPP partner logic functions, and any output 
# files any of the foregoing (including device programming or simulation 
# files), and any associated documentation or information are expressly subject 
# to the terms and conditions of the Altera Program License Subscription 
# Agreement, Altera MegaCore Function License Agreement, or other applicable 
# license agreement, including, without limitation, that your use is for the 
# sole purpose of programming logic devices manufactured by Altera and sold by 
# Altera or its authorized distributors.  Please refer to the applicable 
# agreement for further details.

# ******** THIS is a Special RGB Version of the std Pixel Converter by wwkong<<<---------{{{{{

# Do NOT use this file in any product.
# ***********************************
# This file is NOT Suitable for use in any product.
# No part of this file is Suitable for use in any product.

# This file is only provided for you to read because there is some possibility that 
# it may improve your understanding of the Cyclone 5 starter kit.

# This file is provided on CONDITION that the authors shall NOT be liable for the 
# consequences of your use of any part of it.
# Do NOT use this file in any product.
# If you do not agree then delete all copies of this file in your possession.
# ALL copies or Modified copies of this file MUST contain these warnings & conditions.


# Do NOT use this file in any product.
# ***********************************


# Module Pixel_Conv
set_module_property "TOP_LEVEL_HDL_FILE" "Pixel_Conv.v"
set_module_property "TOP_LEVEL_HDL_MODULE" "Pixel_Conv"
set_module_property className "Pixel_Conv"
set_module_property displayName "Pixel Converter"
set_module_property "DESCRIPTION" "CbYCr0 to CrYCb AvalonST Pixel Conv"
set_module_property instantiateInSystemModule "true"
set_module_property version "1.0"
set_module_property editable "true"
set_module_property simulationModelInVHDL "true"
set_module_property simulationModelInVerilog "true"
set_module_property simulationFiles [ list "Pixel_Conv.v" ]
set_module_property synthesisFiles [ list "Pixel_Conv.v" ]
set_module_property HIDE_FROM_QUARTUS true

set parameter_name "SOURCE_SYMBOLS_PER_BEAT"
add_parameter $parameter_name "integer" "3" "Source symbols per beat"
set_parameter_property $parameter_name displayName "Source symbols per beat"
set_parameter_property $parameter_name allowedRanges [ list 1 3 ]

set_module_property previewElaborationCallback "elaborate"

proc elaborate {} {
	# AvalonST clock interface
	add_interface "clk" "clock" "end"
	add_interface_port "clk" "clk" "clk" "input" "1"
	add_interface_port "clk" "reset_n" "reset_n" "input" "1"


	


	# AvalonST sink interface
	add_interface "in" "avalon_streaming" "sink" "clk"
	set_interface_property "in" "symbolsPerBeat" "4"
	set_interface_property "in" "dataBitsPerSymbol" "8"
	set_interface_property "in" "readyLatency" "0"
	set_interface_property "in" "maxChannel" "0"
	
	add_interface_port "in" "ready_out" "ready"        "output" 1
	add_interface_port "in" "valid_in" "valid"         "input"  1
	add_interface_port "in" "data_in" "data"           "input"  32
	add_interface_port "in" "eop_in" "endofpacket"     "input"  1
	add_interface_port "in" "sop_in" "startofpacket"   "input"  1
	add_interface_port "in" "empty_in" "empty"         "input"  2




	
	# AvalonST source interface
	set source_symbols_per_beat [ get_parameter_value "SOURCE_SYMBOLS_PER_BEAT" ]
	set source_bits_per_symbol [ expr { 24 / $source_symbols_per_beat } ]

	if { [ expr $source_symbols_per_beat == 1] } {
		set source_empty_width 1
	} else {
		set source_empty_width 2
	}

	add_interface "out" "avalon_streaming" "source" "clk"
	set_interface_property "out" "symbolsPerBeat" $source_symbols_per_beat
	set_interface_property "out" "dataBitsPerSymbol" $source_bits_per_symbol
	set_interface_property "out" "readyLatency" "0"
	set_interface_property "out" "maxChannel" "0"
	
	add_interface_port "out" "ready_in" "ready"           "input" 1
	add_interface_port "out" "valid_out" "valid"          "output" 1
	add_interface_port "out" "data_out" "data"            "output" 24
	add_interface_port "out" "eop_out" "endofpacket"      "output" 1
	add_interface_port "out" "sop_out" "startofpacket"    "output" 1
	add_interface_port "out" "empty_out" "empty"          "output" $source_empty_width


}
