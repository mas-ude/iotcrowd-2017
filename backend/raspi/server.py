from flask import Flask, render_template
from flask_sockets import Sockets
from flask_mqtt import Mqtt
import json
import time

app = Flask(__name__, template_folder='.')
app.config['SECRET_KEY'] = 'super-secret'
app.config['MQTT_BROKER_URL'] = '10.130.1.100'
app.config['MQTT_BROKER_PORT'] = 1883
#app.config['MQTT_USERNAME'] = 'user'
#app.config['MQTT_PASSWORD'] = 'secret'
app.config['MQTT_REFRESH_TIME'] = 0.2  # refresh time in seconds
sockets = Sockets(app)
mqtt = Mqtt(app)

socket_list = []

@sockets.route('/socket')
def handle_message(ws):
	socket_list.append(ws)

	while not ws.closed:
		message = ws.receive()

	socket_list.remove(ws)

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
	json_data = message.payload.decode()
	data = json.loads(json_data)
	data["time"] = int(time.time())
	new_json = json.dumps(data)

	for ws in socket_list:
		ws.send(new_json)

@app.route("/")
def main_page():
    return render_template("template.html")

if __name__ == '__main__':
    mqtt.subscribe('dragino')

    from gevent import pywsgi
    from geventwebsocket.handler import WebSocketHandler
    server = pywsgi.WSGIServer(('', 5000), app, handler_class=WebSocketHandler)
    server.serve_forever()
