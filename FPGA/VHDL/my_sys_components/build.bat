@echo off
REM Build script for a vhdl module using ghdl
REM set MODULE with your module name
Set MODULE=integration_tb

REM Don't touch following:
Set FILES=ram_master_temp\ram_master.vhd  qspi_temp\qspi_interface.vhd qspi_temp\qspi_simulate.vhd integration_verify_ram_qspi.vhdl integration_tb.vhdl ram_master_temp\ram_emulator.vhdl
ghdl -a  --std=08 %FILES% 

if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)
ghdl -r --std=08 --time-resolution=ns integration_tb --vcd=func.vcd --stop-time=10us

if %ERRORLEVEL%==1 (
	PAUSE
) else (
	gtkwave func.vcd wave_save.gtkw
)
:end


