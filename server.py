# Made by Michael Zhang 9/26/2025
# Test on mac (paste in non-python terminal):
# curl -X POST -H "Content-Type: application/json" -d '{"humidity":67,"temperature_c":41,"temperature_f":105.8,"ir_raw":67.67,"ir_measured":41,"ir_estimated":44}' http://127.0.0.1:5050/receive

from flask import Flask, request

app = Flask(__name__)

@app.route("/receive", methods=["POST"])
def receive_data():
    data = request.json
    print("Received sensor data:", data) # print to console
    return {"status": "ok"}  # reply/confirm received

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5050)
