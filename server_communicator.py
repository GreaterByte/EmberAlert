import requests
import json

url = "http://127.0.0.1:5050/receive"

data = {
    "humidity": 67,
    "temperature_c": 41,
    "temperature_f": 105.8,
    "ir_raw": 67.67,
    "ir_measured": 41,
    "ir_estimated": 44
}

response = requests.post(url, json = data)

print("Status Code:", response.status_code)
print("Server Response:", response.json())