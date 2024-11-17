@echo off
REM Build script for a vhdl module using ghdl
REM set MODULE with your module name
Set MODULE=qspi_interface

REM Don't touch following:
Set FILES=%MODULE%_tb.vhd  %MODULE%.vhd %MODULE%_verify.vhd
ghdl -a  --std=08 %FILES% 

if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)
ghdl -r --std=08 --time-resolution=ns %MODULE%_tb --vcd=func.vcd --stop-time=600us

if %ERRORLEVEL%==1 (
	PAUSE
) else (
	gtkwave func.vcd wave_save.gtkw
)
:end


