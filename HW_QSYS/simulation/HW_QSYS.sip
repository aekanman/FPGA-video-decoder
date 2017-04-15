set_global_assignment -entity "HW_QSYS" -library "lib_HW_QSYS" -name IP_TOOL_NAME "Qsys"
set_global_assignment -entity "HW_QSYS" -library "lib_HW_QSYS" -name IP_TOOL_VERSION "15.1"
set_global_assignment -entity "HW_QSYS" -library "lib_HW_QSYS" -name IP_TOOL_ENV "Qsys"
set_global_assignment -library "lib_HW_QSYS" -name SPD_FILE [file join $::quartus(sip_path) "../HW_QSYS.spd"]
set_global_assignment -library "lib_HW_QSYS" -name MISC_FILE [file join $::quartus(sip_path) "../../HW_QSYS.qsys"]

set_global_assignment -library "lib_HW_QSYS" -name MISC_FILE [file join $::quartus(sip_path) "HW_QSYS.v"]
set_global_assignment -library "lib_HW_QSYS" -name MISC_FILE [file join $::quartus(sip_path) "submodules/HW_QSYS_QSYS_0.v"]
