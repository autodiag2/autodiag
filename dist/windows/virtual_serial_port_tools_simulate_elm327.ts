///<reference path="hhdscriptport.d.ts" />
// This file is part of autodiag

/**<hhd-description>
[[[s:150:[[[bELM327 Simulation]]]]]]
[[[iPull Model]]]

This script is a basic implementation of an ELM327 device.

</hhd-description>*/
class Utils {
	static stringToUIntArray(s) {
       	var length = s.length;
       	var index = -1;
       	var hex;
		var arr = [];
       	while (++index < length) {
        	arr.push(s.charCodeAt(index));
       	}
       	return Uint8Array.from(arr);
    }
}

enum ELM327_PROTO {
	ELM327_PROTO_NONE, ELM327_PROTO_SAE_J1850_1,
	ELM327_PROTO_SAE_J1850_2, ELM327_PROTO_ISO_9141_2,
	ELM327_PROTO_ISO_14230_4_KWP2000_1, ELM327_PROTO_ISO_14230_4_KWP2000_2,
	ELM327_PROTO_ISO_15765_4_CAN_1,ELM327_PROTO_ISO_15765_4_CAN_2,
	ELM327_PROTO_ISO_15765_4_CAN_3,ELM327_PROTO_ISO_15765_4_CAN_4,
	ELM327_PROTO_SAEJ1939,ELM327_PROTO_USER1_CAN,
	ELM327_PROTO_USER2_CAN
}
const SIM_ELM327_DEFAULT_PROTO = ELM327_PROTO.ELM327_PROTO_ISO_15765_4_CAN_1;

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

class ELM327Sim implements Port.IScriptDevice {

	public elm327;
	private strWorked;

	constructor() {
        this.run();
		this.elm327 = this.getStateDefault();
		this.strWorked = "";
    }

	getStateDefault() {
		const voltageFactory = Math.random() * 20;
		return {
			echo: true,	// PP
			eol: "\r\n", // PP
			baud_rate: 38400,
			protocol_is_auto_running: true, // TODO nvm
			protocolRunning: SIM_ELM327_DEFAULT_PROTO, // TODO nvm
    		voltage: voltageFactory,
			voltageFactory: voltageFactory,
			receive_addresse: null,
			printing_of_spaces: true, // PP
			printing_of_headers: false, // TODO PP GET
			dev_description: "dev description",
			dev_identifier: "dev identifier",
			can: {
				auto_format: true, // TODO PP GET
			}
		}
	}
	at_match(cmd) {
		if ( typeof(cmd) === typeof('') ) {
			return this.strWorked == ("at" + cmd + this.elm327.eol);
		} else {
			return cmd.exec(this.strWorked) != null;
		}
	}
	at_re_string(re) {
		return re.exec(this.strWorked)[1];
	}
	at_re_int(re) {
		return parseInt(re.exec(this.strWorked)[1]);
	}
	at_re_bool(re) {
		return Boolean(this.at_re_int(re));
	}
    async run() {
		const SERIAL_OK = "OK";
		const SERIAL_RESPONSE_PROMPT = ">";
		// Standard RS232 response
		const ELM_RESPONSE_UNKNOWN = "?";
		
		const commands = {
			echo: /ate([\d])/,
			linefeeds: /atl([\d])/,
			calibrate_voltage: /atcv ([\d]+)/,
			printing_of_spaces: /ats([\d])/,
			printing_of_headers: /ath([\d])/,
			dev_identifier: /at@3 ([\w\d]+)/,
			can: {
				auto_format: /atcaf([\d])/,
			}
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
				const elm327_protocol_as_letter = this.elm327.protocolRunning.toString(16);
				response = `${elm327_protocol_is_auto_letter}${elm327_protocol_as_letter}`;
			} else if ( this.at_match("dp") ) {
				const elm327_protocol_is_auto_str = this.elm327.protocol_is_auto_running ? "Auto, " : "";
				const elm327_protocol_as_str = elm327_protocol_to_string(this.elm327.protocolRunning);
				response = `${elm327_protocol_is_auto_str}${elm327_protocol_as_str}`;		
			} else if ( this.at_match("d") ) {
				this.elm327 = this.getStateDefault();
				response = SERIAL_OK;
			} else if ( this.at_match("rv") ) {
				response = `${this.elm327.voltage.toFixed(2)}`;
			} else if ( this.at_match("rtr") ) {
				response = SERIAL_OK;
			// rd
			// r
			} else if ( this.at_match("cv 0000") ) {
				this.elm327.voltage = this.elm327.voltageFactory;
				response = SERIAL_OK;
			// amt
			// amc
			} else if ( this.at_match(commands.calibrate_voltage) ) {
				this.elm327.voltage = this.at_re_int(commands.calibrate_voltage) / 100;
				response = SERIAL_OK;
			// pps
			// pp
			// bd
			// brd
			} else if ( this.at_match("bi") ) {
				response = SERIAL_OK;
			// brt
			} else if ( this.at_match("ar") ) {
				this.elm327.receive_addresse = null;
				response = SERIAL_OK;
			// cp
			// st
			// at
			} else if ( this.at_match("ss") ) {
				response = SERIAL_OK;
			} else if ( this.at_match("pc") ) {
				response = SERIAL_OK;
			// sh
			// sr
			// sd
			// m
			// sp A
			// ta
			// tp A
			// sp
			// tp
			} else if ( this.at_match(commands.printing_of_spaces) ) {
				this.elm327.printing_of_spaces = this.at_re_int(commands.printing_of_spaces);
				response = SERIAL_OK;
			} else if ( this.at_match(commands.printing_of_headers) ) {
				this.elm327.printing_of_headers = this.at_re_int(commands.printing_of_headers);
				response = SERIAL_OK;
			} else if ( this.at_match("@1") ) {
				response = this.elm327.dev_description;
			} else if ( this.at_match("@2") ) {
				response = this.elm327.dev_identifier;
			} else if ( this.at_match(commands.dev_identifier) ) {
				this.elm327.dev_identifier = this.at_re_string(commands.dev_identifier);
				response = SERIAL_OK;
			// ws
			// lp
			} else if ( this.at_match(commands.can.auto_format) ) {
				this.elm327.can.auto_format = this.at_re_bool(commands.can.auto_format);
				response = SERIAL_OK;
			// kw
			// cra
			// cm
			} else if ( this.at_match("csm") ) {
				response = SERIAL_OK;
			} else if ( this.at_match("cfc") ) {
				response = SERIAL_OK;
			// cf
			} else if ( this.at_match("cs") ) {
				response = "T:00 R:00 ";
			} else {
				// sim bus
				response = ELM_RESPONSE_UNKNOWN;
			}
			if ( this.elm327.echo ) {
				response = receivedString + response;
			}
			response += this.elm327.eol + SERIAL_RESPONSE_PROMPT;
            port.provideReceivedData(Utils.stringToUIntArray(response));
        }
    }
}

function createDevice(): Port.IScriptDevice
{
    return new ELM327Sim;
}
