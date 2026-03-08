from udsonip import UdsOnIpClient
from udsoncan import services
from udsoncan import Response

# Simple single-ECU client
client = UdsOnIpClient('127.0.0.1', client_logical_address=0x00E0, ecu_address=0xFFFF)
res = client.change_session(services.DiagnosticSessionControl.Session.defaultSession)
assert res.code == Response.Code.PositiveResponse
print("session changed successfully")
client.close()