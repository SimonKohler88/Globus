@echo off
REM Build script for a vhdl module using ghdl
REM set MODULE with your module name
Set MODULE=clear_led_logic
REM Don't touch following:
Set FILES=%MODULE%.vhdl %MODULE%_tb.vhdl
ghdl -a  --std=08 %FILES% 

if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)
REM ghdl -r --std=08 --time-resolution=ns integration_tb --vcd=func.vcd --stop-time=120us
ghdl -r --std=08 --time-resolution=ns %MODULE%_tb --vcd=func.vcd --stop-time=1500us

if %ERRORLEVEL%==1 (
	PAUSE
) else (
	gtkwave func.vcd wave_save.gtkw
	REM gtkwave func.vcd wave_save.gtkw
)

:end


