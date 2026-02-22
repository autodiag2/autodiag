from doipclient import DoIPClient
client = DoIPClient("127.0.0.1", 13400, 0x0E00)
client.request_activation()
resp = client.send_diagnostic(b"\x10\x00")
print(resp)
