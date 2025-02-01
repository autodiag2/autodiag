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
</table>


# Quick guide to read codes
1. Buy [ELM327 adaptater](https://www.amazon.fr/elm327-usb/s?k=elm327+usb)
2. Download autodiag from [releases](https://github.com/autodiag2/autodiag/releases)
3. Start the car and plug adaptater
4. Launch autodiag and select /dev/ttyUSB0 device
5. Read codes

# Develop
## Prerequisite
### Windows
 - Install [MSYS2](https://www.msys2.org/)
 - Install Dependencies
 ```bash
 pacman -S mingw-w64-ucrt-x86_64-gtk3 git mingw-w64-ucrt-x86_64-toolchain base-devel gcc pkg-config mingw-w64-x86_64-gtk3 && \
  echo "export LIBRARY_PATH=/mingw64/lib:$LIBRARY_PATH" >> ~/.bashrc && \
  source ~/.bashrc
 ```
### MacOS
```bash
brew install gtk+3
```
### Debian based
```bash
sudo apt-get install libgtk-3-0
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
autodiag
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
<br />
Devices: ELM320 ELM322 ELM323 ELM327 ELM329


