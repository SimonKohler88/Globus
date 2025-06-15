import smbus

# I2C-Adresse des Nucleo-Boards
NUCLEO_I2C_ADDRESS = 0x12  # Angenommene Adresse des Nucleo

# I2C-Bus initialisieren
bus = smbus.SMBus(1)  # Verwende I2C-Bus 1

def write_register(register, value):
    """Schreibt einen Wert in ein Register des Nucleo"""
    try:
        # Schreibe den Wert an das angegebene Register
        bus.pec = 1 # enables Packet Error Checking (PEC) Keine Ahnung was das ist aber klingt gut
        bus.write_word_data(NUCLEO_I2C_ADDRESS, register, value)
        print(f"Erfolgreich Wert {value} an Register {register} geschrieben.")
    except Exception as e:
        print(f"Fehler beim Schreiben an Register {register}: {e}")

# def write_register(register, value):
#     try:
#         with bus(1) as bus:
#             bus.pec = 1  # enables Packet Error Checking (PEC) Keine Ahnung was das ist aber klingt gut
#             bus.write_word_data(NUCLEO_I2C_ADDRESS, register, value)
#     except Exception as e:
#          print(f"Fehler beim Schreiben an Register {register}: {e}")

if __name__ == "__main__":
    register_address = 0x01  # Adresse des neuen Registers
    new_value =   20   # Neuer Wert, der geschrieben werden soll
    write_register(register_address, new_value)
