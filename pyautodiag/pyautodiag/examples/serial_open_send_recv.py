from pyautodiag.libautodiag.com.serial import Serial

serial = Serial()
serial.name = "/dev/ttys030".encode('utf-8')
serial.dump()
serial.open()
serial.send("ATZ")
serial.recv()
from pyautodiag.libautodiag.log import log_set_level, LOG_DEBUG, LOG_NONE
log_set_level(LOG_DEBUG)
serial.recv_buffer.contents.dump()
log_set_level(LOG_NONE)
serial.close()