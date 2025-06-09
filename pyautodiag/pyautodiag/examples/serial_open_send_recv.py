from pyautodiag.libautodiag.com.serial import Serial

from pyautodiag.libautodiag.log import log_set_level, LOG_DEBUG
log_set_level(LOG_DEBUG)

serial = Serial()
serial.name = "/dev/ttys030".encode('utf-8')
serial.dump()
serial.open()
serial.send("ATZ")
serial.recv()
serial.recv_buffer.contents.dump()
serial.close()