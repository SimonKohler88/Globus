@echo off
REM Build script for a vhdl module using ghdl
REM set MODULE with your module name
Set MODULE=new_encoder

REM Don't touch following:
Set FILES=%MODULE%.vhdl %MODULE%_tb.vhdl
ghdl -a  --std=08 %FILES% 
ghdl -r --std=08 --time-resolution=ns %MODULE%_tb --vcd=func.vcd --stop-time=600us

if %ERRORLEVEL%==1 (
	PAUSE
) else (
	gtkwave func.vcd wave_save.gtkw
)



