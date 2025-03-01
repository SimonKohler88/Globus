@echo off
REM Build script for a vhdl module using ghdl
REM set MODULE with your module name

ghdl -v
echo ---------------------------------------------

Set MODULE=integration_tb

REM Don't touch following:
Set FILES=ram_master_temp\ram_master.vhd  qspi_temp\qspi_interface.vhd qspi_temp\qspi_simulate.vhd integration_verify_ram_qspi_led_interface.vhdl integration_complete_tb.vhdl ram_master_temp\ram_emulator.vhdl encoder\new_encoder.vhdl led_interface_temp\led_interface.vhd
ghdl -a  --std=08 %FILES% 

if %ERRORLEVEL%==1 (
	PAUSE
	goto end
)
ghdl -r --coverage --std=08 --time-resolution=ns integration_complete_tb --vcd=func2_vcd.vcd --stop-time=400us
REM ghdl -r --std=08 integration_complete_tb   --stop-time=120us --wave=func2.ghw

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
	gtkwave func2_vcd.vcd wave_save_2_vcd.gtkw ../gtkrcfile.gtkwaverc
	REM gtkwave func2_vcd.vcd wave_save.gtkw
)
PAUSE
:end


