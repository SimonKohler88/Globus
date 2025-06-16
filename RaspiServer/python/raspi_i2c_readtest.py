import smbus
import time

# I2C-Adresse des Nucleo
NUCLEO_I2C_ADDRESS = 0x12  # Angenommene Adresse des Nucleo

# Initialisiere den I2C-Bus
bus = smbus.SMBus(1)  # Verwende I2C-Bus 1

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

if __name__ == "__main__":
    register_address = 0x01  # Beispielhafte Registeradresse
    read_register(register_address)
