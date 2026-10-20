# TeslaBoard

### Observatory control, built around the ESP32.

TeslaBoard is an ESP32-based observatory controller that brings roof automation, telescope cover control, flat panel management, auxiliary devices, and safety monitoring together in a configurable platform.

Configure and operate your observatory through a web interface, and connect it to your astrophotography software using ASCOM Alpaca over Wi-Fi. No firmware coding is required to configure the board.

[Website](https://board.stefanotesla.it/en/) · [Documentation](https://board.stefanotesla.it/en/documentation/requirements.html)

## Features

### Observatory roof control

Control the opening and closing of your observatory roof, either manually or through your acquisition software.

- Designed for sliding / roll-off roofs.
- Exposed through the ASCOM Dome profile.
- Integrates roof operation into your imaging workflow.

> The Dome module currently supports sliding roofs, not rotating domes.

### Telescope cover and flat panel

Manage your telescope cover and flat-field light source from the same controller.

- Open and close the telescope cover.
- Adjust flat panel brightness.
- Support automatic brightness adjustment through compatible acquisition software, such as N.I.N.A.

### Switches and auxiliary devices

Configure the inputs and outputs your observatory needs, including power controls, fans, dew heaters, auxiliary sensors, and additional limit switches.

Up to 15 switches can be configured on a single board and exposed through the ASCOM Switch profile.

| Switch type | Purpose |
| --- | --- |
| Digital Input | Read a contact, sensor, or limit switch. |
| Digital Output | Control a relay or transistor-driven load. |
| PWM Output | Adjust brightness, fan speed, or another modulated load. |
| Servo Output | Position a servo motor at a configurable angle. |
| Virtual Input | Receive a numeric value from an external system over HTTP. |

### Safety monitoring

Monitor configured conditions and report the observatory's safety status to your acquisition software.

The Safety module evaluates switches configured in the Switches module. These can include physical inputs or values supplied by external systems through virtual inputs.

> Safety monitoring requires at least one configured switch to monitor. Your acquisition software must be configured to respond appropriately to an unsafe condition.

### Browser-based configuration

Configure and control TeslaBoard from a browser on the same Wi-Fi network.

- Use the Home page to operate enabled modules and view their status.
- Use Setup to configure modules and I/O resources.
- Assign functions without modifying the firmware source code.

## ASCOM integration

TeslaBoard exposes observatory functions through the corresponding ASCOM device profiles.

| Function | ASCOM profile |
| --- | --- |
| Observatory roof | Dome |
| Telescope cover and flat panel | CoverCalibrator |
| Auxiliary inputs and outputs | Switch |
| Safety status | SafetyMonitor |

Communication with your acquisition computer is handled through ASCOM Alpaca over Wi-Fi.

> USB connectivity to the ASCOM platform is under development.

## How it works

TeslaBoard separates physical hardware, I/O management, and observatory functions into distinct layers.

```text
Web Interface                    Astrophotography Software
      |                                   |
      |                            ASCOM Alpaca / Wi-Fi
      |                                   |
      +------------ Observatory Modules --+
                             |
                         IO Manager
                             |
                        ESP32 GPIOs
                             |
                Sensors, relays, servos,
                and external control hardware
```

### IO Manager

The IO Manager turns ESP32 GPIOs into logical resources:

- Digital inputs.
- Digital outputs.
- PWM outputs.
- Servo outputs.
- Virtual inputs updated over HTTP.

### Observatory modules

Modules use those resources to implement higher-level functions, such as opening the roof, controlling a flat panel, switching equipment power, or evaluating safety conditions.

This separation keeps hardware configuration independent from the observatory logic and allows multiple modules to share I/O resources.

## Getting started

1. Review the [hardware and software requirements](https://board.stefanotesla.it/en/documentation/requirements.html).
2. Install the TeslaBoard firmware on a supported ESP32 board, following the documentation.
3. Connect the board to your Wi-Fi network.
4. Open the board's web interface from a device on the same network.
5. Configure your I/O resources and enable the modules you need.
6. Connect your acquisition software through ASCOM Alpaca.
7. Test each input, output, movement, and safety condition before relying on an automated sequence.

For wiring details, firmware installation, and module configuration, refer to the documentation.

## Hardware considerations

TeslaBoard provides the control logic. Your installation still needs suitable sensors, power supplies, and interface hardware for the connected equipment.

Choose wiring and control hardware appropriate to your loads, and verify roof and cover operation carefully before unattended use.

## Documentation

- [Project website](https://board.stefanotesla.it/en/)
- [Requirements](https://board.stefanotesla.it/en/documentation/requirements.html)
- [Single-board and multi-board setups](https://board.stefanotesla.it/en/documentation/usage.html)
- [Dome module](https://board.stefanotesla.it/en/documentation/modules/dome/)
- [Switches module](https://board.stefanotesla.it/en/documentation/modules/switch/)
