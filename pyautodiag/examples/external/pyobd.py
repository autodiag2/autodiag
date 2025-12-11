import obd
import sys
from obd import OBDStatus

obd.logger.setLevel(obd.logging.DEBUG)

if len(sys.argv) < 2:
    print(f"usage {sys.argv[0]} <block file>")
    exit(1)

blockFile = f"{sys.argv[1]}"

connection = obd.OBD(blockFile) # auto-connects to USB or RF port
if connection.status() == OBDStatus.NOT_CONNECTED:
    print("not connected")
    exit(1)
else:
    cmd = obd.commands.SPEED # select an OBD command (sensor)

    response = connection.query(cmd) # send the command, and parse the response
    if response.value is None:
        print("None returned")
    else:
        print(response.value) # returns unit-bearing values thanks to Pint
        print(response.value.to("mph")) # user-friendly unit conversions