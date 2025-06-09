# autodiag
Scantool to read diagnostic codes with OBD2

<table>
  <tr>
    <td><img src="https://raw.githubusercontent.com/autodiag2/media/master/main.png"></td>
    <td><img src="https://raw.githubusercontent.com/autodiag2/media/master/preferences.png"></td>
  </tr>
  <tr>
    <td><img src="https://raw.githubusercontent.com/autodiag2/media/master/terminal.png"></td>
    <td><img src="https://raw.githubusercontent.com/autodiag2/media/master/read_dtc.png"></td>
  </tr>
  <tr>
    <td><img src="https://raw.githubusercontent.com/autodiag2/media/master/sensors.png"></td>
  </tr>
</table>


# Quick guide to read codes
- Buy [ELM327 adaptater](https://www.amazon.fr/elm327-usb/s?k=elm327+usb)
- Download autodiag from [releases](https://github.com/autodiag2/autodiag/releases)
- Start the car and plug adaptater
- Launch autodiag
#### Debian based
- Select /dev/ttyUSB0 device
#### Windows
- Download the driver example from [ftdichip](https://ftdichip.com/drivers/)
- Select the right COM port
<br /><br />

- Read codes

# Develop
## Prerequisite
#### Windows
 - Install [MSYS2](https://www.msys2.org/)
 - Open MSYS2 MINGW64 shell
 - Install Dependencies
 ```bash
 pacman -S mingw-w64-ucrt-x86_64-gtk3 git mingw-w64-ucrt-x86_64-toolchain base-devel gcc pkg-config mingw-w64-x86_64-gtk3 && \
  echo "export LIBRARY_PATH=/mingw64/lib:$LIBRARY_PATH" >> ~/.bashrc && \
  echo "export PKG_CONFIG_PATH=/mingw64/libautodiag/pkgconfig:/mingw64/share/pkgconfig" >> ~/.bashrc && \
  source ~/.bashrc
 ```
#### MacOS
```bash
brew install gtk+3
```
#### Debian based
```bash
sudo apt-get install make gcc build-essential debhelper dh-make devscripts libgtk-3-0 libgtk-3-dev git
```
## Build
```bash
git clone https://github.com/autodiag2/autodiag && \
 cd autodiag && \
 git submodule update --init --recursive && \
 make && \
 make install
```
## Run
```bash
./bin/autodiag
./bin/elm327sim
```
#### Simulation for windows
- Download [Virtual Serial Port Tools](https://freevirtualserialports.com/)
- Setup a script port
- Add the script [virtual_serial_port_tools_simulate_elm327.ts](https://raw.githubusercontent.com/autodiag2/autodiag/refs/heads/main/dist/windows/virtual_serial_port_tools_simulate_elm327.ts)
#### MacOS troubleshoot
```
(autodiag:5985): GLib-GIO-ERROR **: 23:33:55.223: No GSettings schemas are installed on the system
```
Specify the correct location of gsettings schemas
```bash
echo "export GSETTINGS_SCHEMA_DIR=/opt/homebrew/share/glib-2.0/schemas/" >> ~/.bashrc && \
 source ~/.bashrc
```
#### Alternative simulator
##### ELM327 simulator
```bash
python3 -m pip install ELM327-emulator
python3 -m elm
```

# Why
Repairing modern car is not always easy due to lake of free tools. The need for expensive hardware block us from repairing cars and even small buisness that do not have money to afford tools and dive into the automotive world. So there is a need for tools that are not as complete as manufacturer official ones but enough powerfull to fix small issues with car onboard computer.<br />
This is a light approach to fix car computer because it reduce the amount of hardware required to work on it, only have to buy a small elm adaptater.
Some scantools are already avaliable on the market:
  - [ScanTool.net](https://github.com/RJSzynal/ScanTool.net) : reliable but require recompilation to add new manufacturer DTC list. Also the gui is not ruly smooth.

Autodiag will be a simple scantool for desktop computer that allow to:
  - read DTCs: with both manufacturer specific definition and generic one
  - clear MIL: this clear the malfunction lamp on the car
  - read sensors data: real time data retrieving and maybe graphing

It is mostly inspired from scantool.net but with a revisited GUI written in GTK, possibility to add DTC manufacturer specific after compilation.

# How 
With low level c language everything is possible<br />
With unit test, the software will be reliable<br />
With open source everyone will benefit from the effort<br />
In first time autodiag will focus on UNIX platform and ELM based devices.<br />
GPL licence will allow both buisnesses and individual to use the software<br />

# Compatibility
OSs:<br />
 - debian based 64 bits <br />
 - windows 64 bits<br />
 - macOS<br />
<br />
Devices: ELM320 ELM322 ELM323 ELM327 ELM329


