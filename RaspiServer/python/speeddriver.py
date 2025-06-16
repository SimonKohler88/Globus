import RPi.GPIO as GPIO

import smbus
import time


class SpeedDriver:
    NUCLEO_I2C_ADDRESS = 0x12  # Angenommene Adresse des Nucleo
    REGISTER_ADDRESS = 0x01  # ersetzten durch Registeradresse von Dutycycle der Motors

    def __init__(self):
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(22, GPIO.OUT)
        

        self._bus = smbus.SMBus(1)
        

    def read_register(self, register):
        """Lese einen Wert aus einem Register des Nucleo und gib ihn aus"""
        try:
            # Lese ein Byte von der angegebenen Registeradresse
            value = self._bus.read_word_data(SpeedDriver.NUCLEO_I2C_ADDRESS, register)
            print(f"Wert {value} aus Register {register} gelesen.")
            return value
        except Exception as e:
            print(f"Fehler beim Lesen von Register {register}: {e}")
            return None

    def write_register(self, register, value):
        """Schreibt einen Wert in ein Register des Nucleo"""
        try:
            # Schreibe den Wert an das angegebene Register
            # bus.pec = 1 # enables Packet Error Checking (PEC) Keine Ahnung was das ist aber klingt gut
            self._bus.write_word_data(SpeedDriver.NUCLEO_I2C_ADDRESS, register, value)
            print(f"Erfolgreich Wert {value} an Register {register} geschrieben.")
        except Exception as e:
            print(f"Fehler beim Schreiben an Register {register}: {e}")

    def up(self):
        """liest, addiert 10 und schreibt auf Register 1 in Motor_Ctrl in Nucleo"""
        current_value = self.read_register(SpeedDriver.REGISTER_ADDRESS)
        new_value = current_value + 10
        self.write_register(SpeedDriver.REGISTER_ADDRESS, new_value)

    def down(self):
        """liest, subtrahiert 10 und schreibt auf Register 1 in Motor_Ctrl in Nucleo"""
        current_value = self.read_register(SpeedDriver.REGISTER_ADDRESS)
        if current_value >= 0 and current_value < 10:
            self.write_register(SpeedDriver.REGISTER_ADDRESS, 0)
        else:
            new_value = current_value - 10
            self.write_register(SpeedDriver.REGISTER_ADDRESS, new_value)

    def enable(self):
        GPIO.output(22, GPIO.HIGH)
        pass

    def disable(self):
        self.write_register(SpeedDriver.REGISTER_ADDRESS, 0)
        GPIO.output(22, GPIO.LOW)


    def __del__(self):
        GPIO.cleanup()
        pass
