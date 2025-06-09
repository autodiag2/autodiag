from pyautodiag.pyautodiag.libautodiag.sim.elm327.sim import ECUEmulation

ecuEmulation = ECUEmulation(0x70)
print("ECUEmulation created with address:", hex(ecuEmulation.address))

ecuEmulation = ECUEmulation(0x71)
print("ECUEmulation created with address:", hex(ecuEmulation.address))