# BattMITM

this is version 2 (or technically version 3) of a project that all started because I was too lazy to buy a level shifter.
![a pi pico on a piece of protoboard with a small oled, voltage regulator, and molex connector, mounted to the custom-built battery on the back of my ThinkPad T420](https://ben.wiegand.pw/img/battmitm-2-pipico-thumb.webp)

this project uses a Raspberry Pi Pico RP2040 microcontroller (seen above). past hardware revisions include an off-brand Arduino Uno R3 (no picture) and a Genuino Arduino Leonardo ([picture](https://ben.wiegand.pw/img/smbus-mitm-arduino-thumb.webp)).

the Arduino version had major technical limitations that made me not want to release it. also I blew up the arduino.

## liability disclaimer

> **IMPORTANT: currently, SBS alarms may not work depending on your laptop. see project status.**

**if you use BattMITM, you are doing so at your own risk.** while battery protection technology tends to be pretty safe these days, I'm not responsible for any adverse side-effects of using BattMITM.

Additionally, BattMITM allows you to instruct your equipment to exceed safe operating limits if used improperly. it is your responsibility to understand and identify these limits to safely operate your equipment.

please refer to the MIT license text for more information on this matter.

## project status
- passthrough partially works (no host commands yet). my laptop charges and discharges as normal through it.
- read cmd reply overrides work (these encompass 99% of useful overrides). they are defined in `config_override.h`.
- a basic version of the GUI is working. it requires an SSD1331 96x64 16-bit color OLED display over SPI. the driver is built-in and made by yours truly. there are no other drivers. 
- uart control is todo

NOTE: as mentioned, laptop -> battery commands work but battery -> laptop commands don't. this means SBS alarms won't notify the laptop. 
some laptops poll the battery for alarms regardless, so this may not be a huge issue for you.

## what does it do exactly and how

### what it does
the basic job of BattMITM is to intercept SMBus commands between the laptop and the BMS. when put in this position, it can do a few useful things.

specifically:
- act as a pass-through to forward SMBus commands between the laptop and BMS and return their responses.
- intercept specified SMBus commands to return a custom response instead of forwarding them.
- allow sending SMBus commands to the BMS via a USB UART connection and getting the response back, without involving the laptop.
- query the BMS over SMBus independently to display relevant battery health metrics on an attached display so they can be viewed.


### diagram
the most frequently asked question I get about this project is about where the microcontroller sits and a clarification on what it does. I think a diagram could help explain this better:

#### glossary
- battery pack: a group of battery cells wired in a combination of parallel and series configurations
- BMS: "Battery Management System" - protects and tends to the health of your battery pack. it is supposed to know how to maintain safe and smooth operation for the specific construction and chemistry of your battery pack.
- SMBus: "System Management Bus" - a protocol based on i2c which is commonly used throughout computer hardware for communication.
- balance: in the context of the connection shown below it refers to a group of wires, one for each parallel cell group plus a GND. it's used for individually addressing each cell group to monitor it and keep the pack "balanced".

#### normal setup
this is how your battery communicates with your computer normally (without BattMITM):

```
 ________________           _____           __________________
|                |         |     |  power  |                  |
|                |  power  |     |=========|                  |
|  battery pack  |=========| BMS |  SMBus  |     laptop       |
|                | balance |     |=========|                  |
|                |=========|     |  detect |                  |
|________________|         |_____|=========|__________________|
                                            \                  \
                                             \                  \
                                              \                  \
                                               \__________________\
```

#### BattMITM
this is how BattMITM fits in to this:

```
 ________________           ______           __________________
|                |         |      |  power  |                  |
|                |  power  |      |=========|                  |
|  battery pack  |=========| BMS  |         |     laptop       |
|                | balance |      |         |                  |
|                |=========|      |  detect |                  |
|________________|         |______|=========|__________________|
                             ||||            \                  \
 ____________________        ||||             \                  \
|                    | power ||||              \                  \
|                    |======// ||               \__________________\
|  microcontroller   | SMBus   ||                  ||
| (running BattMITM) |========//                   ||
|                    | SMBus                       ||
|____________________|============================//

```
instead of your laptop and BMS talking directly, they now have to go through the microcontroller, allowing it to monitor, modify, and inject communications. if you haven't already guessed, at this point it should be clear that the "MITM" in "BattMITM" stands for "man-in-the-middle".


## why

why not?

seriously though, usually the EC (embedded controller) in your laptop is the thing that interfaces with your BMS. it's up to your laptop manufacturer as to what gets exposed to the operating system, and up to the operating system what gets exposed to you. BattMITM allows you to view any battery health metrics exposed by your BMS, and do so even while it's running. 

such metrics can include (but are not limited to):
- individual cell group voltages
- temperature
- cycle count
- instantaneous current/voltage
- maximum charge capacity
- available capacity
- design capacity
- charging current
- decimal precision battery percentage

additionally, BattMITM allows you to modify the information exposed to your laptop if you've re-celled your battery pack, letting you correct any information which has changed as opposed to when it was stock.

nothing would stop you from even trying to enter the boot rom of your BMS and reprogramming it through this (I'm not going to make any promises about success there though).

*also to be completely real it's really nice to have a battery indicator without having to wake the laptop from sleep.*


## is this safe?

> **IMPORTANT: currently, SBS alarms may not work depending on your laptop. see project status.**

to an extent.

here is my full honest answer:

the BMS is generally responsible for protecting the battery pack and ensuring it doesn't get damaged or pushed beyond its design. the BMS accomplishes this by advising the laptop in what to do in order to maintain safe operation. the BMS is also capable of limiting and cutting power to and from the battery pack to protect it if the laptop is unresponsive, though this is usually a last resort.

as BattMITM only intercepts communications between the laptop and the BMS, it in theory doesn't put the battery pack itself in too much danger. that being said, introducing new components to a system that was never designed for them always presents a risk that something could go wrong.

in an ideal world, the BMS *should* cut power to the laptop if it senses something wrong, and the laptop *should* terminate discharge if the BMS isn't behaving. both ends of this communication are *supposed to be* designed to appropriately handle a situation in which communications are degraded. in a case where the firmware on either the BMS or the laptop isn't properly designed, it could cause certain safety measures to not behave as intended if the communications are interrupted or modified.


if you use BattMITM, you are doing so at your own risk. it is not my responsibility to check if your equipment properly implements relevant safety features if something were to go wrong.

additionally, BattMITM allows you to instruct your equipment to exceed safe operating limits if used improperly. it is your responsibility to understand and identify these limits to safely operate your equipment.

please refer to the MIT license text for more information on this matter.
