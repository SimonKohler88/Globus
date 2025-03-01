@echo off
REM Build script for a vhdl module using ghdl
REM set MODULE with your module name
ghdl -v
echo ---------------------------------------------

Set MODULE=qspi_interface

REM Don't touch following:
Set FILES=%MODULE%_tb.vhd  %MODULE%.vhd %MODULE%_verify.vhd

ghdl -a -v --std=08 -fpsl  %FILES%
echo ---------------------------------------------

::ghdl -e --std=08 qspi_interface

::ghdl --synth --std=08 %FILES% -e qspi_interface_tb
::ghdl --synth --std=08 %FILES%

::echo ---------------------------------------------
if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)

::ghdl -r --std=08 --time-resolution=ns %MODULE%_tb --vcd=func.vcd --stop-time=600us
ghdl -r --coverage --std=08 --time-resolution=ns %MODULE%_tb --wave=func.ghw --stop-time=600us --psl-report=PSL_REP.json --psl-report-uncovered

if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)
echo:
echo --------------------COVERAGE-------------------------
del coverage.json
ren coverage-* coverage.json
ghdl coverage  coverage.json
echo ---------------------------------------------
if %ERRORLEVEL%==1 (
	PAUSE
) else (
	::gtkwave func.vcd wave_save.gtkw
	gtkwave func.ghw wave_save.gtkw ../../gtkrcfile.gtkwaverc
)
:end


