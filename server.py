from flask import Flask
from flask import request
import datetime

app = Flask(__name__)

@app.route("/")
def hello():
    # get the temperature and humidity values from the request
    temp = request.args.get("temp")
    humid = request.args.get("humid")
    
    # get current timestamp
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    # print to the console (this is what fulfills the "prints on the cloud side" requirement)
    print(f"[{timestamp}] Temperature: {temp}°C, Humidity: {humid}%")
    
    # send response back to ESP32
    return f"Received: Temperature={temp}°C, Humidity={humid}%"
