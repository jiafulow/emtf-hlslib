# To run: vivado_hls -f run_hls.tcl

set PROJ "myproject_prj"
set SOLN "solution1"
set XPART xc7vx690tffg1927-2
#set XPART xcvu9p-flga2104-2l-e
#set XPART xcvu13p-fsga2577-1-e
#set XPART xcku15p-ffva1760-2-e
set CLKP 160MHz
#set CLKP 240MHz
set CFLAGS "-std=c++11"
set CSIMFLAGS "-Wall -Wextra -Wno-unused-label -Wno-unused-parameter"
set CSIM 1
set CSYNTH 1
set COSIM 0
set VIVADO_SYN 0
set VIVADO_IMPL 0


# ########################################################
# Create a project
open_project -reset $PROJ
# Add design files
add_files firmware/myproject.cpp -cflags $CFLAGS -csimflags $CSIMFLAGS
add_files -tb myproject_test.cpp -cflags $CFLAGS -csimflags $CSIMFLAGS
add_files -tb tb_data
# Set the top-level function
set_top myproject

# ########################################################
# Create a solution
open_solution -reset $SOLN
# Define technology and clock rate
set_part $XPART
create_clock -period $CLKP -name default
set_clock_uncertainty 20.0%

# Set any optimization directives
#config_array_partition -maximum_size 2048
config_array_partition -auto_partition_threshold 64
config_array_partition -auto_promotion_threshold 64
config_array_partition -include_extern_globals
config_array_partition -throughput_driven
config_compile -pipeline_loops 64
config_compile -name_max_length 60
config_interface -trim_dangling_port
config_schedule -relax_ii_for_timing
# End of directives

# ########################################################
# Execute

# Run C Simulation
if {$CSIM == 1} {
  csim_design
}

# Run C Synthesis
if {$CSYNTH == 1} {
  csynth_design
}

# Run C/RTL Co-simulation
if {$COSIM == 1} {
  cosim_design -trace_level all
}

# Run RTL Synthesis
if {$VIVADO_SYN == 1} {
  export_design -flow syn -rtl verilog
}

# Run RTL Synthesis and Implementation
if {$VIVADO_IMPL == 1} {
  export_design -flow impl -rtl verilog
}

exit
