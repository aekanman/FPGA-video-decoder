set_global_assignment -entity "ECE423_QSYS" -library "lib_ECE423_QSYS" -name IP_TOOL_NAME "Qsys"
set_global_assignment -entity "ECE423_QSYS" -library "lib_ECE423_QSYS" -name IP_TOOL_VERSION "15.1"
set_global_assignment -entity "ECE423_QSYS" -library "lib_ECE423_QSYS" -name IP_TOOL_ENV "Qsys"
set_global_assignment -library "lib_ECE423_QSYS" -name SPD_FILE [file join $::quartus(sip_path) "../ECE423_QSYS.spd"]
set_global_assignment -library "lib_ECE423_QSYS" -name MISC_FILE [file join $::quartus(sip_path) "../../ECE423_QSYS.qsys"]

set_global_assignment -library "lib_ECE423_QSYS" -name MISC_FILE [file join $::quartus(sip_path) "ECE423_QSYS.v"]
set_global_assignment -library "lib_ECE423_QSYS" -name MISC_FILE [file join $::quartus(sip_path) "submodules/ECE423_QSYS_QSYS_0.v"]
