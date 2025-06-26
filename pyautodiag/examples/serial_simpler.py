from autodiag.sim.elm327.elm327 import SimELM327
from serial import Serial, SerialException

emulation = SimELM327()
emulation.loop(daemon=True)

try:
    ser = Serial("ok", 9600, timeout=1)
except SerialException as exception:
    print(exception)

try:
    ser = Serial("/dev/ttys000", 9600, timeout=1)
except SerialException as exception:
    print(exception)

ser = Serial(emulation.device_location.decode(), 9600, timeout=1)

ser.write(b'ATI\r')
data = ser.read_until(b'>')
print(data)
ser.close()
