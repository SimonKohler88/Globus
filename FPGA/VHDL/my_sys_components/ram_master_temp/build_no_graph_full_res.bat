@echo off
REM Build script for a vhdl module using ghdl
REM set MODULE with your module name
Set MODULE=ram_master

ghdl -v
echo ---------------------------------------------


REM Don't touch following:
Set FILES=%MODULE%_full_res_tb.vhd  %MODULE%.vhd  %MODULE%_verify_full_res.vhd ram_emulator.vhdl
ghdl -a  --std=08 %FILES% 
if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)

echo ---------------------------------------------
ghdl -e --std=08 ram_master_full_res_tb
if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)

echo ---------------------------------------------
ghdl -r --std=08 --coverage --time-resolution=ns %MODULE%_full_res_tb --stop-time=5ms --psl-report=PSL_REP.json --psl-report-uncovered
if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)

echo:
echo --------------------COVERAGE-------------------------
del coverage.json
ren coverage-* coverage.json
ghdl coverage  coverage.json
echo ------------------PYTHON CHECK DATA OUTPUT--------------------

python \stream_120x256\check_data.py
PAUSE
:end


