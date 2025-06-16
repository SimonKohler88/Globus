import numpy as np
import matplotlib.pyplot as plt
from camera import Camera  
from serialInterface import SerialInterface  
import time
import datetime
import struct


# camera connection
com = SerialInterface('/dev/serial0')
camera = Camera(com)

# set camera settings
camera.powerOn()
camera.setIntTime_us(500)  # integration time in µSeconds

# get camera information
productionYear, productionWeek = camera.getProductionDate()
print(f'# camera production date: year {productionYear}, week {productionWeek}')

chipId, waferId = camera.getChipInfo()
print(f'# chipID: {chipId}, waferID: {waferId}')

chipType, device, version = camera.getIdentification()
print(f'# chipType: {chipType}, device: {device}, version: {version}')

fwVersionMajor, fwVersionMinor = camera.getFwRelease()
print(f'# firmware version: V{fwVersionMajor}.{fwVersionMinor}')

chipTemperature = camera.getChipTemperature()
print(f'# chip temperature: {chipTemperature}°C')

# measure
tof_distance = np.array(camera.getDistance())
print('TOF distance image:')
print(np.around(tof_distance, decimals=1))

# plot
plt.imshow(tof_distance)
plt.savefig("tof_distance_image.png")
plt.title('TOF distance image', fontsize=12)
plt.colorbar()
plt.show()