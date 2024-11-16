@echo off
REM Build script for a vhdl module using ghdl
REM set MODULE with your module name
Set MODULE=ram_master

REM Don't touch following:
Set FILES=%MODULE%.vhd %MODULE%_tb.vhd
ghdl -a %FILES%
ghdl -r %MODULE%_tb --vcd=func.vcd


PAUSE
