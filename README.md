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
    <td><img src="https://raw.githubusercontent.com/autodiag2/media/master/sensors-graph.png"></td>
  </tr>
</table>

# Why
Repairing modern car is not always easy due to lake of free tools. The need for expensive hardware block us from repairing cars and even small buisness that do not have money to afford tools and dive into the automotive world. So there is a need for tools that are not as complete as manufacturer official ones but enough powerfull to fix small issues with car onboard computer.<br />
This is a light approach to fix car computer because it reduce the amount of hardware required to work on it, only have to buy a small elm adaptater.
Some scantools are already avaliable on the market:
  - [ScanTool.net](https://github.com/RJSzynal/ScanTool.net) : reliable but require recompilation to add new manufacturer DTC list. Also the gui is not ruly smooth.

Autodiag will be a simple scantool for desktop computer that allow to:
  - read DTCs: with both manufacturer specific definition and generic one
  - clear MIL: this clear the malfunction lamp on the car
  - read sensors data: real time data retrieving and graphing

It is mostly inspired from scantool.net but with a revisited GUI written in GTK, possibility to add DTC manufacturer specific after compilation.

# How 
With low level c language everything is possible<br />
With unit test, the software will be reliable<br />
With open source everyone will benefit from the effort<br />
In first time autodiag will focus on UNIX platform and ELM based devices.<br />
GPL licence will allow both buisnesses and individual to use the software<br />

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

## Installation errors
See [here](/documentation/install-errors.md)

# Compatibility
OSs:<br />
 - debian based<br />
 - windows<br />
 - macOS<br />
<br />
Devices:    ELM320 ELM322 ELM323 ELM327 ELM329  
Standards:  ISO3779 SAEJ1979/ISO15031-5 ISO15031-6 ISO3780 ISO3166-1 ISO15765  
            Hayes command set 

# Developer documentation
See [here](/documentation/DEV.md)
