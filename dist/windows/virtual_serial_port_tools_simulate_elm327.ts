///<reference path="hhdscriptport.d.ts" />
// This file is part of autodiag

/**<hhd-description>
[[[s:150:[[[bELM327 Simulation]]]]]]
[[[iPull Model]]]

This script is a basic implementation of an ELM327 device.

</hhd-description>*/

enum ELM327_PROTO {
	ELM327_PROTO_NONE, ELM327_PROTO_SAE_J1850_1,
	ELM327_PROTO_SAE_J1850_2, ELM327_PROTO_ISO_9141_2,
	ELM327_PROTO_ISO_14230_4_KWP2000_1, ELM327_PROTO_ISO_14230_4_KWP2000_2,
	ELM327_PROTO_ISO_15765_4_CAN_1,ELM327_PROTO_ISO_15765_4_CAN_2,
	ELM327_PROTO_ISO_15765_4_CAN_3,ELM327_PROTO_ISO_15765_4_CAN_4,
	ELM327_PROTO_SAEJ1939,ELM327_PROTO_USER1_CAN,
	ELM327_PROTO_USER2_CAN
}
const ELM327_SIM_DEFAULT_PROTO = ELM327_PROTO.ELM327_PROTO_ISO_15765_4_CAN_1;

function elm327_protocol_to_string(proto) {
    switch(proto) {
        case ELM327_PROTO.ELM327_PROTO_NONE:                    return "N/A";
        case ELM327_PROTO.ELM327_PROTO_SAE_J1850_1:             return "SAE J1850 PWM (41.6 kBit/s)";
        case ELM327_PROTO.ELM327_PROTO_SAE_J1850_2:             return "SAE J1850 VPW (10.4 kBit/s)";
        case ELM327_PROTO.ELM327_PROTO_ISO_9141_2:              return "ISO 9141-2";
        case ELM327_PROTO.ELM327_PROTO_ISO_14230_4_KWP2000_1:   return "ISO 14230-4 KWP2000 (5-baud init)";
        case ELM327_PROTO.ELM327_PROTO_ISO_14230_4_KWP2000_2:   return "ISO 14230-4 KWP2000 (fast init)";
        case ELM327_PROTO.ELM327_PROTO_ISO_15765_4_CAN_1:       return "ISO 15765-4 (CAN 11-bit ID, 500 kBit/s)";
        case ELM327_PROTO.ELM327_PROTO_ISO_15765_4_CAN_2:       return "ISO 15765-4 (CAN 29-bit ID, 500 kBit/s)";
        case ELM327_PROTO.ELM327_PROTO_ISO_15765_4_CAN_3:       return "ISO 15765-4 (CAN 11-bit ID, 250 kBit/s)";
        case ELM327_PROTO.ELM327_PROTO_ISO_15765_4_CAN_4:       return "ISO 15765-4 (CAN 29-bit ID, 250 kBit/s)";
        case ELM327_PROTO.ELM327_PROTO_SAEJ1939:                return "SAE J1939 (CAN 29-bit ID, 250 kBit/s)";
        case ELM327_PROTO.ELM327_PROTO_USER1_CAN:               return "USER1 CAN";
        case ELM327_PROTO.ELM327_PROTO_USER2_CAN:               return "USER2 CAN";
    }
    return null;
}

class LoopbackSerialDevicePull implements Port.IScriptDevice {

	public elm327;
	private strWorked;

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
			protocol_is_auto_running: true,
			protocolRunning: ELM327_SIM_DEFAULT_PROTO
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
			} else if ( this.at_match(commands.linefeeds) ) {
				if ( this.at_re_bool(commands.linefeeds) ) {
					this.elm327.eol = "\r\n";
				} else {
					this.elm327.eol = "\r";
				}
				response = SERIAL_OK;
			} else if ( this.at_match("z") ) {
				response = "ELM327 v2.1";
			} else if ( this.at_match("ign") ) {
				response = Math.random() < 0.5 ? "ON" : "OFF";
			} else if ( this.at_match("ib 10") ) {
				this.elm327.baud_rate = 10400;
				response = SERIAL_OK;
			} else if ( this.at_match("fe") ) {
				response = SERIAL_OK;
			} else if ( this.at_match("ib 48") ) {
				this.elm327.baud_rate = 4800;
				response = SERIAL_OK;
			} else if ( this.at_match("ib 96") ) {
				this.elm327.baud_rate = 9600;
				response = SERIAL_OK;
			} else if ( this.at_match("i") ) {
				response = "ELM327 v2.1";
			} else if ( this.at_match("dpn") ) {
				const elm327_protocol_is_auto_letter = this.elm327.protocol_is_auto_running ? "A" : "";
				const elm327_protocol_as_int = this.elm327.protocolRunning;
				response = `${elm327_protocol_is_auto_letter}${elm327_protocol_as_int}`;
			} else if ( this.at_match("dp") ) {
				const elm327_protocol_is_auto_str = this.elm327.protocol_is_auto_running ? "Auto, " : "";
				const elm327_protocol_as_str = elm327_protocol_to_string(this.elm327.protocolRunning);
				response = `${elm327_protocol_is_auto_str}${elm327_protocol_as_str}`;		
			} else if ( this.at_match("d") ) {
				this.elm327 = this.getStateDefault();
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
