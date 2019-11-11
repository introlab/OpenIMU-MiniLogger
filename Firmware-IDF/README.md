# User Guide
The OpenIMU-MiniLogger was programmed to be a user friendly device that let you collect data from GPS, IMU, Barometer and more using the grove expansion.

After opening the device using the power switch on the side, the starting menu will appear with different widgets available. You can navigate through them using the "**Next**" and "**Previous**" button and perform their action by **selecting** one. Each widgets possess their own message telling information about the device. The "**Change Menu**" button let you move from the **starting menu** to the **configuration menu**.

The date and time will be updated once the GPS **is fixed**. You can see this information in the GPS widget in the **starting menu**. This only has to be done once or as long as the board is not disconnected from the battery.

## Start Acquiring Data
The OpenIMU-MiniLogger offers data acquisation of multiple sensors for a long period of time. To log information from the GPS, make sure it **is fixed** first. 

### Logging

1. Make sure your device is **open** and there's an **SD card** in the slots. If no SD card is present, you won't be able to log or to connect your device to external (Red icon).
2. **Select** the **folder widget** to start logging. You should see the logging time at the top of the screen appear and the icon turning blue meaning you are logging.
3. To stop logging, simply **select** the same widget again. The icon will get back to its original state and your file will be saved on the SD Card.

### Transfer File

To process your data using the OpenIMU [software](https://github.com/introlab/OpenIMU):
1. **Plug** your device on your computer with a micro USB cable 
2. **Select** the SD card icon to connect to USB allowing your computer to read the card.
3. Follow the steps in the [OpenIMU project](https://github.com/introlab/OpenIMU) to download and process your data.

### Configuration

The OpenIMU-MiniLogger allows changeable configuration for your data acquisition. These parameter can be changed using the [ConfigAPP](../ConfigAPP/README.md) created for this purpose or inside the device itself with the configuration menu:

1. From the home screen, press the "**Change Menu**" button to go to the configuration menu.
2. Different widget are available to change parameters of the IMU:
    1. Sample rate (10,50,100,200 Hz)
    2. Gyroscope range (250,500,1000,2000 dps)
    3. Accelerometer range (2,4,8,16 g).
3. **Select** the widget to switch between the possibilities.

The fourth widget tells you the space left on the SD card.
