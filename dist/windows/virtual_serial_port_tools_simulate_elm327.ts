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
    async run()
    {
	const SERIAL_OK = "OK";
	var eol =  "\r\n";
	log("Started emulator");
        while (true)
        {
            	var sentData = await port.getSentData();	
		const receivedString = String.fromCharCode(...sentData).toLowerCase();
		log(receivedString);

	    	var response = receivedString;
		if ( receivedString == ("ate0" + eol) ) {
			response = SERIAL_OK + eol;
		} else if ( receivedString == ("ati" + eol) ) {
			response = "ELM327 v1.5" + eol;
		} else if ( receivedString == ("atl0" + eol) ) {
			response = SERIAL_OK + eol;
			eol = "\r";
		}
            	port.provideReceivedData(this.stringToUIntArray(response));
        }
    }
}

function createDevice(): Port.IScriptDevice
{
    return new LoopbackSerialDevicePull;
}
