# Ascom Alpaca Switch, Dome, CoverCalibrator Board ESP32

Do you have a remote observatory? This board is made for you!
The Ascom-compatible WiFi board using the Alpaca protocol is here!

Main Feature:

- Dome controller (Only R.O.R)
- Switch Controller
- CoverCalibration (flat frame)
- lidar securtity scan

Each device is recognized by Ascom via the Alpaca protocol, with no need for USB connection or drivers, just WiFi!

A USB cable is only required for the first software upload; after that, a WiFi Manager will help you connect the board to your WiFi network (this board cannot be used as an access point).

- Automatic WiFi reconnection in case of disconnection (e.g., after a router reboot).
- OTA updates allow firmware updates without cables.

## Dome
Dome support only R.O.R. type.

You can use a normal gate board or two realy to provide open and close functions.

If you have a **gate board**, you don't have to do nothing. Except is your board is an Aleko, in that case uncomment the ALEKO define
If you have **two realy to command a motor directly**, you need to do some steps:
- Open main.cpp
- comment the line `#define GATE_BOARD` adding two / before;

Open the directory

The board provide the signals:
- Move (open/close)
- Halt
And need back the shutter state:
- Open
- Close

Features:
- Since gate boards do not have directional feedback, if the roof moves in the wrong direction, the board will detect it and send a correction command.
- You can enable an automatic closing system, which will close the roof if there is no communication between the board and Ascom or the PC (you can set the timeout duration).
- Automatic fault detection if the movement doesn’t complete within a set time (you can configure the timeout in seconds).

## Switch

You can configure up to 16 Switch which can be:
-Gauges
-Switch
-PWM signal
-Analog Input

A configuration page with validation is provided, so you don’t need to manually modify the software! :D  

> [!IMPORTANT]
> You can't use a Switch Output for more device, ex pwm output as switch can't be the same for CoverCalibrator

> [!IMPORTANT]
> At the state of art, this board work only with PNP relay boards or any things you want, the reason is to be sure that command will be executed when you decide, using NPN board will bring you in strange problem (ex: with NPN realy board, if ESP32 is borken, your ralay will be activated)

## CoverCalibration (aka Automatic Flat Panel)
You can have a PWM signal to handle a mosfet to encrease or decrease the flat panel brightness, for a bettere experience (avoid led flickering) you can command a led power supply with the PWM or a mosfet
The CoverCalibrator is a must have to have perfect flat frame!

## Lidar based security scan
By using a low cost LD06 Lidar you can check if something is in the way of your roof and prevent it from operating. Connect the data pin to pin GPIO6 (Serial1 Rx) and place the lidar in the plane you want to check (like 5cm under the roof line). Configure the mindist and scan angle (start and end) and if something is inside of that region the lidar will flag as UNSAFE.

## Tested with
- Voyager
- N.I.N.A
- Sequence Generator Pro

#### Changelog

💥 **3.0.0** - BREACKING NEWS! Now switch, calibrator and dome are modules, if you don't plan to use it, you can disable it!, the web page and the discovery will hide it for you. Also pin validation during the setup is better now
If you don't use a module you can comment the respective row at the beginning of the main.cpp:

`#define DOME`

`#define SWITCH`

`#define COVERC`

`#define LIDAR`

You don't use the dome?
`\\#define DOME`

`#define SWITCH`

`#define COVERC`


2.0.0 - Now all the board can be setupped by the browser. Dome I/O and Switch setting is now editable from the browser and you can save the confi files.
Before update write in a memo all your setting.
Web pages run now alpine.js and all the js under the hood is rewrited in a modern way.
Now Dome and switch are in the same page, olso settings now are in the same page!
New dome function: Auto Close if anyone is talking with the board (browser or ascom alpacha driver) [IS UNDER DEVELOPMENT], you can enable or disable this featur by browser setup page.


### ToDo List:
- Make Web Pages Translatable


# How to use

## First time board installation

First of all we need to install Visual Studio Code and add Platformio extension.


Download Visual Studio Code from: https://code.visualstudio.com/

Install it.

One installation is finish, Go to Extension and search Platformio, then install it.

![platformio](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/d98877fb-aad2-4a95-8ed2-27ca3c7f2d11)

And wait until restart vs code is needed:

![restartvscode](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/cba04a24-7c34-466e-bf7c-61187920322d)

At this point, download the software from this page, go up click on Code and click on Download Zip

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/3221a252-7aa6-4844-860c-6ff7f2ea973d)

Extact the folder where you want, BE CAREFUL, don't extract the files inside the folder! extract directly the folder!

Go to VsCode and click on the Platformio Icon, click on Pick a Folder and select the extracted folder

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/c3af02f5-2c4d-4720-97f1-36f7b6ec2528)

At this point wait platformio will download all the necessary things needed to make the board work:

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/eaaf9c67-e40a-4d33-ac0d-ffbdfacbf22f)

Once is finish on the right you will have this menu:

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/9bd040e0-7b05-466a-8a48-5ec8335a7ceb)

Click build and wait until you will not have this message:

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/5eb184d7-bb6e-44c9-9231-a6a07f06b311)

Now we are ready to upload the firmware!

If it is your first time connecting an esp32 to the pc you need to install serial driver depending on your bard serial chip:

CH340 driver: https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all (download at the buttom of the page)
CP210x driver: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads

(check the video on the CH340 page to see wich COM port is used by your device)

Connect your esp32 board to a usb port 
On the menu we seen before we need to click:

Upload Filesystem Image

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/58f7db3d-f677-45a6-93cd-fa3bf2e5c8f1)

After some while the board will rebot.

Now we need to click on Upload

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/8e8e496e-f380-4362-937a-61dd0c350b40)

Wait for success printed out.

### Connect the board to your own wifi

If is your first time setup, you need to connct the board to your wifi, don't check into the code, the board will manage everything for you!
Open your WiFi and you will have TeslaBoard wifi, connect to him

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/f0096d68-8f93-4f54-b429-013b7b53d3f6)

Open a brwoser and type http://192.168.4.1, once the portal is ready click on Configure Wifi
![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/203cae3e-81f9-41d2-933e-44e7b4567d60)

Click on your WiFi, type the password and click on save
![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/df91e6dc-8caf-4447-b116-007b4646f0c4)

In few seconds the board will be up and running, go to your route setting and check the board IP.
From this moment, the board will try yo connect everytime to your wifi without asking nothing.





# Using the board with Ascom driver.

First of all we need to install the last Ascom platform (version 6 or higher) from here: https://ascom-standards.org/Downloads/Index.htm

Not every software as ex. NINA, open the Ascom Device Selector, and the board cannot be founded, in this case we need to crate a new ascom dynamic driver:

Open ASCOM Diagnostic:
![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/efe40cad-6272-41a1-a1b3-73b77fd58456)

Click on Choose Device>Choose device to connect:
![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/2a0de3af-c08c-4fa6-8143-3d43818e0cda)

Select the device type:
![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/c156eceb-0ac8-461d-87d3-be287b078cc8)

This operation is needed for each device Dome,Switch,Cover Calibrator (you can skip the device you are not planning to use.

Click on Alpaca and then Enable Discovery

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/356bfcbb-e3b6-4f8d-93a5-c5efbb138800)

Once is green, open the list and you will see New Device:

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/64e1040b-d89b-4d52-a70d-ce0e698ef7a2)

 Open the setting menu, and check the Manage conncetion as well:

 ![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/96ba0fca-7145-4d60-b5c8-2a5ea826e505)

 Click OK and then OK again.

 The board is ready to use! Go to your software (in my case N.I.N.A) and the device will appear:

 ![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/086f4dca-93f2-4ef5-b666-e2169b2c0f78)


# Board web server

To acces to the Board page, type on your browser the IP of the board.

## Home Page

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/a20259d2-73a0-4fa2-9018-c8f9a32c4833)

### Dome Section

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/7a9d54db-68db-4c6c-baf0-fb93c91187a9)

From here we can see the state of our Dome cover and you can send Open Close or Halt commands.

### Switch Section

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/e859d039-7b2a-40b7-af95-f8974efb7ab6)

From here you can see the state of the switches and command it, for Input switch you cannot send any commands.
For Output you can set ON/OFF state
For PWM Output you can slide to the desired value.

## Setup Page

### Dome Section

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/c9d7de4f-d401-4f45-b726-0b0ab57f544b)

Pin Start is the pin used to give the start to the motor
Pin Halt is used to stop the motor

Pin Shutter Open should be ON when the shutter is Open
Pin Shutter Close  should be ON when the shutter is Closed

The board does not allow having both inputs active.

~~Enable closing in case of missing connection - If selected the shutter will be closed in ascom don't talk with the board within xxx minutes (computer crash safety)~~ not developed yet

Save you store the configuration on the board
Download Configurazione - you can download the Dome configuration for backup.


### Switch Section

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/7e7d6ab4-7ad5-46d8-8c18-36f12af5473c)

Here you can setup all the Pin you want to use (MAX 16)

You can give a name and a description (max lenght 32)

You can choose the type of the switch (Input, Output, PWM or analog input)

According to the esp32 datashet be sure of wich pin you want to use or you can you can run into problems, **any valditation rules are developed for the moment!**
Read this article: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
If you need a PCB you can use a mine one: https://github.com/StefanoTesla/esp32-kraken as you can see the pin signed with S are Safe to use, and with the I the Imput only pin.
From pin 16 to 33 are safe to use, pin 34 35 36 and 39 are input only.
**Don't set pin from 6 to 11, you will brick the board!**
All the PWN singals are automatically setted up with a range from 0 to 8192.

Save to store modification.
**If you change the pin number or type, it will not work until a board reboot.++

Save config file to have a configuration backup

# Update the firmware in the future

Only if you have download the firmware from the usb port, you are able to upload the firmware via wifi

To do this from your browser you can go on http://your.ip.add.ress/update

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/2e3f86bd-afa3-4a6b-b529-7c55002b9298)

You can choose if you want to upload a new firmware or filesystem, the difference is: firmware change only the software, filesystem write webpages and configuration files.

## Use a backup of your configuration files

From setup page download each configuration files.
Go to the project folder and open the folder data

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/566f4e4f-8df5-49f0-9cfd-fab0ae6f1ccb)

Paste here your configuration files and override the existing one.

## Upload the firmware via OTA

Open the project with platformio and click in the right menu on build

Now open your browser and go to http://your.ip.add.ress/update choose firmware
![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/f7ca0531-fa78-43d3-8daa-7e9c1b0040d6)


And select the file firmware.bin that you can find in yourProjectFolder/.pio/build/esp32doit-devkit-v1
![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/10e58df4-045f-4d91-87dd-a524a817fadd)

## Upload the filesystem via OTA

**Read "Use of a backup of your configuration files** 

Open the project with platformio and click in the right menu on Build Filesystem Image

Now open your browser and go to http://your.ip.add.ress/update choose filesystem

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/3700331a-a55b-4b55-8622-d515e6c34758)

And select the file spiffs.bin file that you can find in yourProjectFolder/.pio/build/esp32doit-devkit-v1

![image](https://github.com/StefanoTesla/Ascom-Alpacha-ES32-Switch-Borad-Dome-Board/assets/92517402/10e58df4-045f-4d91-87dd-a524a817fadd)

