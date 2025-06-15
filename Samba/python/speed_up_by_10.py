import RPi.GPIO as GPIO
import smbus
import time

# I2C-Adresse des Nucleo
NUCLEO_I2C_ADDRESS = 0x12  # Angenommene Adresse des Nucleo
# Initialisiere den I2C-Bus
bus = smbus.SMBus(1)  # Verwende I2C-Bus 1

register_address = 0x01  # ersetzten durch Registeradresse von Dutycycle der Motors

def read_register(register):
    """Lese einen Wert aus einem Register des Nucleo und gib ihn aus"""
    try:
        # Lese ein Byte von der angegebenen Registeradresse
        value = bus.read_word_data(NUCLEO_I2C_ADDRESS, register)
        print(f"Wert {value} aus Register {register} gelesen.")
        return value
    except Exception as e:
        print(f"Fehler beim Lesen von Register {register}: {e}")
        return None

def write_register(register, value):
    """Schreibt einen Wert in ein Register des Nucleo"""
    try:
        # Schreibe den Wert an das angegebene Register
        # bus.pec = 1 # enables Packet Error Checking (PEC) Keine Ahnung was das ist aber klingt gut
        bus.write_word_data(NUCLEO_I2C_ADDRESS, register, value)
        print(f"Erfolgreich Wert {value} an Register {register} geschrieben.")
    except Exception as e:
        print(f"Fehler beim Schreiben an Register {register}: {e}")

    """liest, addiert 10 und schreibt auf Register 1 in Motor_Ctrl in Nucleo"""

current_value = read_register(register_address)
new_value = current_value + 10
write_register(register_address, new_value)

