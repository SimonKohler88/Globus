@echo off
REM Build script for a vhdl module using ghdl
REM set MODULE with your module name
Set MODULE=integration_tb

REM Don't touch following:
Set FILES=ram_master_temp\ram_master.vhd  qspi_temp\qspi_interface.vhd qspi_temp\qspi_simulate.vhd integration_verify_ram_qspi.vhdl integration_tb.vhdl ram_master_temp\ram_emulator.vhdl encoder\new_encoder.vhdl
ghdl -a  --std=08 %FILES% 

if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)
REM ghdl -r --std=08 --time-resolution=ns integration_tb --vcd=func.vcd --stop-time=120us
ghdl -r --std=08 integration_tb   --stop-time=120us --wave=func.ghw

if %ERRORLEVEL%==1 (
	PAUSE
) else (
	gtkwave func.ghw wave_save.gtkw
	REM gtkwave func.vcd wave_save.gtkw
)
:end


