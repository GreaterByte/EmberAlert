import requests
import json

url = "http://127.0.0.1:5050/receive"

humidity = input("Enter humidity: ")
temperature_c = input("Enter temperature_c: ")
temperature_f = input("Enter temperature_f: ")
ir_raw = input("Enter ir_raw: ")
ir_measured = input("Enter ir_measured: ")
ir_estimated = input("Enter ir_estimated: ")

data = {
    "humidity": humidity,
    "temperature_c": temperature_c,
    "temperature_f": temperature_f,
    "ir_raw": ir_raw,
    "ir_measured": ir_measured,
    "ir_estimated": ir_estimated
}

response = requests.post(url, json = data)

print("Status Code:", response.status_code)
print("Server Response:", response.json())