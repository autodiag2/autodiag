///<reference path="hhdscriptport.d.ts" />
// This file is part of autodiag

/**<hhd-description>
[[[s:150:[[[bELM327 Simulation]]]]]]
[[[iPull Model]]]

This script is a basic implementation of an ELM327 device.

</hhd-description>*/

class LoopbackSerialDevicePull implements Port.IScriptDevice
{
    constructor()
    {
        this.run();
    }

    stringToUIntArray(s) {
    	var length = s.length;
    	var index = -1;
    	var hex;
		var arr = [];
    	while (++index < length) {
    	    arr.push(s.charCodeAt(index));
    	}
    	return Uint8Array.from(arr);
    }
	getStateDefault() {
		return {
			echo: true,
			eol: "\r\n",
			baud_rate: 38400,
		}
	}
    async run() {
		const SERIAL_OK = "OK";
		let elm327 = this.getStateDefault();
		const commands = {
			echo: /ate([\d])/,
			linefeeds: /atl([\d])/,
		};
		log("Started emulator");
        while (true) {
            var sentData = await port.getSentData();	
			const receivedString = String.fromCharCode(...sentData).toLowerCase();
			log(receivedString);

	    	let response;
			if ( commands.echo.exec(receivedString) != null ) {
				elm327.echo = Boolean(parseInt(commands.echo.exec(receivedString)[1]));
				response = SERIAL_OK;
			} else if (receivedString == ("atib 10" + elm327.eol)) {
				elm327.baud_rate = 10400;
				response = SERIAL_OK;
			} else if ( receivedString == ("ati" + elm327.eol) ) {
				response = "ELM327 v2.1";
			} else if ( receivedString == ("atign" + elm327.eol) ) {
				response = Math.random() < 0.5 ? "ON" : "OFF";
			} else if ( receivedString == ("atd" + elm327.eol) ) {
				elm327 = this.getStateDefault();
				response = SERIAL_OK;
			} else if ( commands.linefeeds.exec(receivedString) != null ) {
				if ( Boolean(parseInt(commands.linefeeds.exec(receivedString)[1])) ) {
					elm327.eol = "\r\n";
				} else {
					elm327.eol = "\r";
				}
				response = SERIAL_OK;
			} else {
				response = receivedString;
			}
			if ( elm327.echo ) {
				response = receivedString + response;
			}
			response += elm327.eol;
            port.provideReceivedData(this.stringToUIntArray(response));
        }
    }
}

function createDevice(): Port.IScriptDevice
{
    return new LoopbackSerialDevicePull;
}
