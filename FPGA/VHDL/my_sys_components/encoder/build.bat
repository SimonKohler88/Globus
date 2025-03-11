@echo off
REM Build script for a vhdl module using ghdl
REM set MODULE with your module name
Set MODULE=new_encoder

ghdl -v
echo ---------------------------------------------

REM Don't touch following:
Set FILES=%MODULE%.vhdl %MODULE%_tb.vhdl
ghdl -a  --std=08 %FILES%

if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)

echo ---------------------------------------------

ghdl -r --std=08 --coverage --time-resolution=ns %MODULE%_tb --wave=func.ghw --stop-time=600us --psl-report=PSL_REP.json --psl-report-uncovered


if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)

echo:
echo --------------------COVERAGE-------------------------
del coverage.json
ren coverage-* coverage.json
ghdl coverage  coverage.json
echo ------------------

if %ERRORLEVEL%==1 (
	PAUSE
) else (
	gtkwave func.ghw wave_save.gtkw ../../gtkrcfile.gtkwaverc
)

:end

