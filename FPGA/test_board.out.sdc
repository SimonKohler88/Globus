## Generated SDC file "test_board.out.sdc"

## Copyright (C) 2019  Intel Corporation. All rights reserved.
## Your use of Intel Corporation's design tools, logic functions 
## and other software and tools, and any partner logic 
## functions, and any output files from any of the foregoing 
## (including device programming or simulation files), and any 
## associated documentation or information are expressly subject 
## to the terms and conditions of the Intel Program License 
## Subscription Agreement, the Intel Quartus Prime License Agreement,
## the Intel FPGA IP License Agreement, or other applicable license
## agreement, including, without limitation, that your use is for
## the sole purpose of programming logic devices manufactured by
## Intel and sold by Intel or its authorized distributors.  Please
## refer to the applicable agreement for further details, at
## https://fpgasoftware.intel.com/eula.



#**************************************************************
# Time Information
#**************************************************************



#**************************************************************
# Create Clock
#**************************************************************

create_clock -name {altera_reserved_tck} -period 100.000 -waveform { 0.000 50.000 } [get_ports {altera_reserved_tck}]
create_clock -name {CLK12M} -period 83.333 -waveform { 0.000 41.666 } [get_ports {CLK12M}]


#**************************************************************
# Create Generated Clock
#**************************************************************
set sdram_pll "inst5|altpll_0|sd1|pll7|clk[1]"
set sys_clk   "inst5|altpll_0|sd1|pll7|clk[0]"

create_generated_clock \
	-name CLK \
	-source $sdram_pll \
	[get_ports {CLK}]

#**************************************************************
# Set Clock Latency
#**************************************************************



#**************************************************************
# Set Clock Uncertainty
#**************************************************************

derive_clock_uncertainty
derive_pll_clocks


#**************************************************************
# Set Input Delay
#**************************************************************



#**************************************************************
# Set Output Delay
#**************************************************************
#                       clock: 90MHz           Data Sheet W9825  -6, CAS=3
# CLK cycle time        t_ck = 11ns             min: 6 max:1000
# write recovery        t_wr =                   2*tck
# clk high lvl width    t_ch                      min 2 ns
# clk low lvl width     t_cl                      min 2 ns
# access time from clk  t_ac                      max 6 ns
# output data hold time t_oh                      min 3 ns
# Data in Setup time    t_ds                      min 1.5 ns
# data in hold time     t_dh  min 0.8 ns
# addr setup time       t_as  min 1.5
# addr hold time        t_ah  min 0.8
# cmd setup time        t_cms  min 1.5
# cmd hold  time        t_cmh  min 0.8
# Mode regset cycle time t_rsc  min 2 t_ck 

#data tco(min) = tOH, tco(max) = tAC
set sdram_tsu       1.5
set sdram_th        0.8
set sdram_tco_min   3
set sdram_tco_max   6

# FPGA timing constraints
set sdram_input_delay_min        $sdram_tco_min
set sdram_input_delay_max        $sdram_tco_max
set sdram_output_delay_min      -$sdram_th
set sdram_output_delay_max       $sdram_tsu

# PLL to FPGA output (clear the unconstrained path warning)
set_min_delay -from $sdram_pll -to [get_ports {CLK}] 1
set_max_delay -from $sdram_pll -to [get_ports {CLK}] 6


# FPGA Outputs
set sdram_outputs [get_ports {
	A[*]
	BA[*]
	CAS
	CKE
	CS
	DQ[*]
	DQM[*]
	RAS
	WE
}]
set_output_delay \
	-clock CLK \
	-min $sdram_output_delay_min \
	$sdram_outputs
set_output_delay \
	-clock CLK \
	-max $sdram_output_delay_max \
	$sdram_outputs

# FPGA Inputs
set sdram_inputs [get_ports {
	DQ[*]
}]
set_input_delay \
	-clock CLK \
	-min $sdram_input_delay_min \
	$sdram_inputs
set_input_delay \
	-clock CLK \
	-max $sdram_input_delay_max \
	$sdram_inputs

# SDRAM-to-FPGA multi-cycle constraint
#
# * The PLL is configured so that SDRAM clock leads the system
#   clock by 90-degrees (0.25 period or 11.1/4=2.75ns).
#
# * The PLL phase-shift of -90-degrees was selected so that
#   the timing margin for read setup/hold was fairly symmetric,
#   i.e., the sdram_dq FPGA input setup slack is ~2.3ns and
#   hold slack is ~2.9ns
#
# * The following multi-cycle constraint adds an extra clock
#   period to the read path to ensure that the latch clock that
#   occurs 1.25 periods after the launch clock is used in the
#   timing analysis.
#

set_multicycle_path -setup -end -from CLK -to $sys_clk 2

#**************************************************************
# Set Clock Groups
#**************************************************************




#**************************************************************
# Set False Path
#**************************************************************



#**************************************************************
# Set Multicycle Path
#**************************************************************



#**************************************************************
# Set Maximum Delay
#**************************************************************



#**************************************************************
# Set Minimum Delay
#**************************************************************



#**************************************************************
# Set Input Transition
#**************************************************************



#**************************************************************
# Set Net Delay
#**************************************************************

