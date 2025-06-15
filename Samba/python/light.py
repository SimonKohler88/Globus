import smbus
import socket

# I2C channel 1 is connected to the GPIO pins
channel = 1

# Adresse des Lichtsensors
address = 0b0100011
#Power_ON
#Power_On = 0x01

# Befehl um eine Messung mit 1lux Auflösung zu erhalten
ONE_TIME_HIGH_RES_MODE_1 = 0x20

# Initialize I2C (SMBus)
bus = smbus.SMBus(channel)
# Lesen des Lichtsensorwertes

def bright():
    # Lese Daten vom Sensor
    data = bus.read_word_data(address, ONE_TIME_HIGH_RES_MODE_1)
    
    # Berechnung des Helligkeitswertes
    value = int(1.7 * data / 1000) + 30
    
    # Ausgabe des Werts auf der Konsole
    print(f"Gemessener Lux-Wert: {value} lx")
    

# Hauptprogramm
try:
    while True:
        bright()  # Helligkeit messen und ausgeben
        time.sleep(1)  # Aktualisierung alle 1 Sekunde
except KeyboardInterrupt:
    print("\nMessung beendet.")