@echo off
REM Build script for a vhdl module using ghdl
REM set MODULE with your module name
Set MODULE=led_interface
REM Don't touch following:
Set FILES=%MODULE%.vhd %MODULE%_tb.vhd %MODULE%_verify.vhd
ghdl -a  --std=08 %FILES% 

if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)
REM ghdl -r --std=08 --time-resolution=ns integration_tb --vcd=func.vcd --stop-time=120us
ghdl -r --std=08 %MODULE%_tb   --stop-time=120us --wave=func.ghw

if %ERRORLEVEL%==1 (
	PAUSE
) else (
	gtkwave func.ghw wave_save.gtkw
	REM gtkwave func.vcd wave_save.gtkw
)

:end


