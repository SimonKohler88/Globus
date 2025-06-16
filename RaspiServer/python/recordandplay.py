import os
import time
import subprocess

def main():

    #eine Sekunde warten
    time.sleep(1)
    
    # Stimme spricht: "Aufnahme startet, bitte sprechen sie ins Mikrofon"
    subprocess.run(['aplay', '/home/pi/sounds/aufnahmestartet.wav'])
    
    # 5 Sekunden aufnehmen
    subprocess.run(['arecord', '-D', 'plughw:3,0', '-f', 'cd', '-d', '5', '/home/pi/sounds/aufnahme.wav'])
    
    # Stimme spricht: "Aufnahme wird abgespielt"
    subprocess.run(['aplay', '/home/pi/sounds/wirdabgespielt.wav'])

    # eine Sekunde wwarten
    time.sleep(1)
    
    # Aufnahme wird abgespielt
    subprocess.run(['aplay', '/home/pi/sounds/aufnahme.wav'])

if __name__ == "__main__":
    main()
