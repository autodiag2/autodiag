# autodiag
Automotive diagnostic software with many protocols

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
Repairing modern vehicle is not always easy due to lake of free tools. The need for expensive hardware block us from repairing vehicles and even small buisness that do not have money to afford tools and dive into the automotive world. So there is a need for tools that are not as complete as manufacturer official ones but enough powerfull to repair and diagnose your vehicle.  
This is a light approach to fix vehicle because it reduces the amount of hardware required to work on it, only have to buy a small elm adaptater.
Some scantools are already avaliable on the market:
  - [ScanTool.net](https://github.com/RJSzynal/ScanTool.net) : reliable but require recompilation to add new manufacturer DTC list. Also the gui is not ruly smooth.

Autodiag will be a simple scantool for desktop computer that allow to:
  - read DTCs: with both manufacturer specific definition and generic one
  - clear MIL: this clear the malfunction lamp on the car
  - read sensors data: real time data retrieving and graphing

It is mostly inspired from scantool.net but with a revisited GUI written in GTK, possibility to add DTC manufacturer specific after compilation.

# How 
With low level c language everything is possible.  
With unit test, the software will be reliable.  
With open source everyone will benefit from the effort.  
In first time autodiag will focus on ELM based devices.   
This software rely on OBD2 and UDS to diagnose your vehicle, from the settings you can configure which data protocol to use.   
GPL licence will allow both buisnesses and individual to use the software.  

# Quick guide to read codes with ELM327
1. Buy ELM327 adaptater[USB](https://www.amazon.fr/elm327-usb/s?k=elm327+usb) or [Bluetooth](https://www.amazon.fr/s?k=elm327+bluetooth)
2. Download autodiag from [releases](https://github.com/autodiag2/autodiag/releases)
3. Start the car and plug adaptater
4. Launch autodiag
#### Debian based
5. Select /dev/ttyUSB0 device or /dev/rfcomm0 (for bluetooth)
#### Windows
5. If ELM327 USB: Download the driver example from [ftdichip](https://ftdichip.com/drivers/)
6. Select the right COM port eg. COM5
<br /><br />

7. Read codes

## Installation errors
See [here](/documentation/install-errors.md)

# Compatibility
| Component      | Debian-based Linux | Windows | macOS  |
|----------------|--------------------|---------|--------|
| Software       | ✔️                 | ✔️       | ✔️     |
| ELM USB        | ✔️                 | ✔️       | ❌     |
| ELM Bluetooth  | ✔️                 | ✔️       | ❌     |

Devices:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; ELM320 ELM322 ELM323 ELM327 ELM329<br />
Standards:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; ISO3779 SAEJ1979/ISO15031-5 ISO15031-6 ISO3780 ISO3166-1 ISO15765 ISO14229 OBD2 UDS CAN CAN-TP KWP2000

# Developer documentation
See [here](/documentation/DEV.md)
