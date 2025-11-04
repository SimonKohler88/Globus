import math
import traceback

LOCAL = True
if not LOCAL:
    import RPi.GPIO as GPIO

    import smbus
import time

MAX_SPEED = 3000  # TODO: Whats top speed?


class SpeedDriver:
    NUCLEO_I2C_ADDRESS = 0x12  # Adresse des Nucleo

    I2C_ADDR_LED_BLINK = 0x00
    I2C_ADDR_MOT_DUTY_CYCLE_SET = 0x01
    I2C_ADDR_MOT_DUTY_CYCLE_IS = 0x02
    I2C_ADDR_MOT_DUTY_CYCLE_SLOPE_PER_S = 0x03
    I2C_ADDR_MOT_DUTY_RESET = 0x04

    def __init__(self):

        self.mot_enabled = 1
        if not LOCAL:
            GPIO.setmode(GPIO.BCM)
            GPIO.setup(22, GPIO.OUT)
            self._bus = smbus.SMBus(1)

            # safety: turn off mot
            self.set_speed(0)

        else:
            self.speed_t = 0
            pass

        self.enable()

    def read_register(self, register):
        """Lese einen Wert aus einem Register des Nucleo und gib ihn aus"""
        try:
            if not LOCAL:
                # Lese ein Byte von der angegebenen Registeradresse
                value = self._bus.read_word_data(SpeedDriver.NUCLEO_I2C_ADDRESS, register)
            else:
                value = self.speed_t
            print(f"Wert {value} aus Register {register} gelesen.")
            return value
        except Exception as e:
            print(f"Fehler beim Lesen von Register {register}: {e}")
            return None

    def write_register(self, register, value):
        """Schreibt einen Wert in ein Register des Nucleo"""
        try:
            if not LOCAL:
                # Schreibe den Wert an das angegebene Register
                # bus.pec = 1 # enables Packet Error Checking (PEC) Keine Ahnung was das ist aber klingt gut
                self._bus.write_word_data(SpeedDriver.NUCLEO_I2C_ADDRESS, register, value)
            else:
                if register == SpeedDriver.I2C_ADDR_MOT_DUTY_RESET:
                    self.speed_t = 0
                else:
                    self.speed_t = value

            print(f"Erfolgreich Wert {value} an Register {register} geschrieben.")
        except Exception as e:
            print(f"Fehler beim Schreiben an Register {register}: {e}")

    def up(self):
        """liest, addiert 10 und schreibt auf Register 1 in Motor_Ctrl in Nucleo"""
        if not self.mot_enabled:
            print("Fail: Motor not enabled")
            return
        current_value = self.read_register(SpeedDriver.I2C_ADDR_MOT_DUTY_CYCLE_SET)
        new_value = current_value + 10
        self.write_register(SpeedDriver.I2C_ADDR_MOT_DUTY_CYCLE_SET, new_value)

    def down(self):
        """liest, subtrahiert 10 und schreibt auf Register 1 in Motor_Ctrl in Nucleo"""
        if not self.mot_enabled:
            print("Fail: Motor not enabled")
            return
        current_value = self.read_register(SpeedDriver.I2C_ADDR_MOT_DUTY_CYCLE_SET)
        if current_value >= 0 and current_value < 10:
            self.write_register(SpeedDriver.I2C_ADDR_MOT_DUTY_CYCLE_SET, 0)
        else:
            new_value = current_value - 10
            self.write_register(SpeedDriver.I2C_ADDR_MOT_DUTY_CYCLE_SET, new_value)

    def read_target_speed(self):
        perc = self.read_register(SpeedDriver.I2C_ADDR_MOT_DUTY_CYCLE_SET)
        return int(perc / 100 * MAX_SPEED)

    def read_speed(self):
        perc = self.read_register(SpeedDriver.I2C_ADDR_MOT_DUTY_CYCLE_IS)
        return int(perc / 100 * MAX_SPEED)

    def set_speed(self, u_per_min):
        if not self.mot_enabled:
            print("Fail: Motor not enabled")
            return
        # convert from u/min to percent
        if u_per_min < 0:
            u_per_min = 0

        perc = min(int(u_per_min / MAX_SPEED * 100), 100)

        if u_per_min == 0:
            self.write_register(SpeedDriver.I2C_ADDR_MOT_DUTY_CYCLE_SET, 0)
        else:
            self.write_register(SpeedDriver.I2C_ADDR_MOT_DUTY_CYCLE_SET, perc)

        print(f"Speed {perc}%")
        pass

    def enable(self):
        self.mot_enabled = 1
        if not LOCAL:
            GPIO.output(22, GPIO.HIGH)

    def disable(self):
        self.write_register(SpeedDriver.I2C_ADDR_MOT_DUTY_RESET, 1)
        self.mot_enabled = 0

        if not LOCAL:
            GPIO.output(22, GPIO.LOW)

    def is_motor_enabled(self):
        return self.mot_enabled

    def __del__(self):
        if not LOCAL:
            GPIO.cleanup()
