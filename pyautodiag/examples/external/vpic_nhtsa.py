from vpic import Client
import sys
import json

vins = sys.argv[1:]
if len(vins) == 0:
    vins = ["5UXKR6C57E0J71584"]

client = Client()

for vin in vins:
    print(f"VIN {vin} decoding:")
    result = client.decode_vin(vin, model_year=2014)
    print(json.dumps(result, indent=4, ensure_ascii=False))
