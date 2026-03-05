import socket
import time
import struct
import netifaces

DOIP_PORT = 13400

# minimal DoIP Vehicle Identification request (header 0x02, payload 0x0001)
def build_discovery_request():
    payload_type = 0x0001
    payload_length = 0
    # DoIP header: protocol version (0x02), inverse (0xFD), payload type (2 bytes), payload length (4 bytes)
    header = struct.pack(">BBHI", 0x02, 0xFD, payload_type, payload_length)
    return header

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
try:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
except AttributeError:
    pass

s.bind(("", 0))

request = build_discovery_request()

def get_default_broadcast():
    gws = netifaces.gateways()
    default = gws.get('default', {})
    iface = None
    if netifaces.AF_INET in default:
        iface = default[netifaces.AF_INET][1]  # (gateway_ip, interface)
    if iface:
        addrs = netifaces.ifaddresses(iface)
        if netifaces.AF_INET in addrs:
            for link in addrs[netifaces.AF_INET]:
                if 'broadcast' in link:
                    return link['broadcast']
    return "255.255.255.255"

BCAST_ADDR = get_default_broadcast()
print(f"Using broadcast of default route: {BCAST_ADDR}")

print(f"Sending DoIP discovery requests on UDP port {DOIP_PORT}...")

while True:
    s.sendto(request, (BCAST_ADDR, DOIP_PORT))
    s.settimeout(1.0)
    try:
        while True:
            data, addr = s.recvfrom(4096)
            print(f"Received DoIP response from {addr}: {data.hex()}")
    except socket.timeout:
        print("No response, retrying...")
    time.sleep(2)