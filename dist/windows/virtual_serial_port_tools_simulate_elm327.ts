///<reference path="hhdscriptport.d.ts" />
// This file is part of autodiag

/**<hhd-description>
[[[s:150:[[[bELM327 Simulation]]]]]]
[[[iPull Model]]]

This script is a basic implementation of an ELM327 device.

</hhd-description>*/

class LoopbackSerialDevicePull implements Port.IScriptDevice {

	elm327;
	strWorked;

    constructor() {
        this.run();
		this.elm327 = this.getStateDefault();
		this.strWorked = "";
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
	at_match(cmd) {
		if ( typeof(cmd) === typeof('') ) {
			return this.strWorked == ("at" + cmd + this.elm327.eol);
		} else {
			return cmd.exec(this.strWorked) != null;
		}
	}
	at_re_bool(re) {
		return Boolean(parseInt(re.exec(this.strWorked)[1]));
	}
    async run() {
		const SERIAL_OK = "OK";
		
		const commands = {
			echo: /ate([\d])/,
			linefeeds: /atl([\d])/,
		};
		log("Started emulator");
        while (true) {
            var sentData = await port.getSentData();	
			const receivedString = String.fromCharCode(...sentData).toLowerCase();
			this.strWorked = receivedString;

	    	let response;
			if ( this.at_match(commands.echo) ) {
				this.elm327.echo = this.at_re_bool(commands.echo);
				response = SERIAL_OK;
			} else if ( this.at_match("ib 10") ) {
				this.elm327.baud_rate = 10400;
				response = SERIAL_OK;
			} else if ( this.at_match("i") ) {
				response = "ELM327 v2.1";
			} else if ( this.at_match("ign") ) {
				response = Math.random() < 0.5 ? "ON" : "OFF";
			} else if ( this.at_match("d") ) {
				this.elm327 = this.getStateDefault();
				response = SERIAL_OK;
			} else if ( this.at_match(commands.linefeeds) ) {
				if ( this.at_re_bool(commands.linefeeds) ) {
					this.elm327.eol = "\r\n";
				} else {
					this.elm327.eol = "\r";
				}
				response = SERIAL_OK;
			} else {
				response = receivedString;
			}
			if ( this.elm327.echo ) {
				response = receivedString + response;
			}
			response += this.elm327.eol;
            port.provideReceivedData(this.stringToUIntArray(response));
        }
    }
}

function createDevice(): Port.IScriptDevice
{
    return new LoopbackSerialDevicePull;
}
