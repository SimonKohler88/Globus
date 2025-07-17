# Raspberri Pi Code Ordner

Source Code Repository für P3/P4 an der FHNW Studiengang EIT Herbst/Frühlingssemester 2024/25

Beinhaltet:

* in ./python: Code Ordner vom Raspberry Pi
  
  Wichtige Files für Server, derzeit verwendet:
  * app.py: Flask instanziierung, von Apache2 ausgeführt.
  * GlobusServer.py: Anbindung an GIF's und ImageProcessing funktionen zur Bilddarstellung
  * Ordner globus_video_utils: Klasse(n) für GlobusServer.py 
  * speeddriver.py: Funktionen für die Motorsteuerung
  * Ordner temp_upload: Verwendet vom Server für Fileupload als destination-directory
  * Ordner templates: HTML-Files für Server
  * Ordner static: JavaScript für HTML-Sites
  * Ordner gifs: Ablage für beide GIF's, welche von GlobusServer.py verwendet werden

  Anbindungen an andere Sensoren:
  * Ordner tof_sensor: TOF-Kamera
    * camera.py
    * takeimage.py
    * commandList.py
    * crc.py
    * communicationType.py
    
  * light.py: Helligkeitssensor
  * recordandplay.py: Microfon/Speaker Beispiel

  Andere Files:
  * raspi_i2c_readtest.py: Testfile für Kommunikation zw. Raspi und Nucleo
  * raspi_i2c_writetest.py: Testfile für Kommunikation zw. Raspi und Nucleo
  * serialInterface.py: UART Communication for serial Output



Access

Auf dem Raspberry pi ist ein Samba Server installiert, welcher auf den Python-Ordner des Server zeigt.

Username: pi
PW: pi

SSH
über putty:
username: pi
PW: raspberry


Apache2

Logs: var/log/apache2
tail -n 100 /var/log/apache2/error.log
Common Apache commands:

sudo service apache2 restart

Networkmanager
sudo nmcli