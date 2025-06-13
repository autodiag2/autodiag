from autodiag.sim.elm327.elm327 import ELM327emulation
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE

emulation = ELM327emulation()
emulation.loop(daemon=True)

print(emulation.eol.decode() if emulation.eol else None)
print(emulation.echo)
print(emulation.protocolRunning)
print(emulation.protocol_is_auto_running)
print(emulation.printing_of_spaces)
print(emulation.printing_of_headers)
print(emulation.dev_description) # None instead of dev description
print(emulation.dev_identifier)
print(emulation.port_name)
print(emulation.voltage)
print(emulation.voltageFactory)
print(emulation.baud_rate)
print(emulation.baud_rate_timeout_msec)
print(emulation.activity_monitor_count)
print(emulation.activity_monitor_timeout)
print(emulation.vehicle_response_timeout)
print(emulation.vehicle_response_timeout_adaptive)
print(emulation.responses)
print(emulation.isMemoryEnabled)
print(emulation.testerAddress)

if emulation.receive_address:
    print(emulation.receive_address)
else:
    print(None)

# Optional checks for pointer members
print(bool(emulation.custom_header))
print(bool(emulation.obd_buffer))
print(bool(emulation.programmable_parameters_defaults))
print(bool(emulation.programmable_parameters_pending_load_type))
print(bool(emulation.ecus))

# CANSettings
print(emulation.can.auto_format)
print(bool(emulation.can.mask))
print(bool(emulation.can.filter))
print(emulation.can.priority_29bits)
print(emulation.can.extended_addressing)
print(emulation.can.extended_addressing_target_address)
print(emulation.can.timeout_multiplier)
print(emulation.can.display_dlc)

# NVM
print(emulation.nvm.user_memory)
print(emulation.nvm.protocol)
print(emulation.nvm.protocol_is_auto)
print(bool(emulation.nvm.programmable_parameters))
print(bool(emulation.nvm.programmable_parameters_pending))
print(bool(emulation.nvm.programmable_parameters_states))

log_set_level(LOG_DEBUG)
serial = Serial()
serial.set_port_location(emulation.port_name)
iface = OBDIFace.open_from_device(serial)
iface.clear_data()
iface.send("0101")
iface.recv()