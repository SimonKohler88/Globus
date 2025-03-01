@echo off
REM Build script for a vhdl module using ghdl
ghdl -v
echo ---------------------------------------------

REM set MODULE with your module name
Set MODULE=integration_tb

REM Don't touch following:
Set FILES=ram_master_temp\ram_master.vhd  qspi_temp\qspi_interface.vhd qspi_temp\qspi_simulate.vhd integration_verify_ram_qspi_led_interface.vhdl integration_complete_tb.vhdl ram_master_temp\ram_emulator.vhdl encoder\new_encoder.vhdl led_interface_temp\led_interface.vhd
ghdl -a  --std=08 %FILES% 

if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)
REM ghdl -r --std=08 --time-resolution=ns integration_tb --vcd=func.vcd --stop-time=120us
ghdl -r --coverage --std=08 integration_complete_tb   --stop-time=120us --wave=func2.ghw

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
	gtkwave func2.ghw wave_save_2.gtkw ../gtkrcfile.gtkwaverc
	REM gtkwave func.vcd wave_save.gtkw
)
:end


