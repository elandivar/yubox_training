import paho.mqtt.client as mqtt 
from random import randrange, uniform
import time

mqttBroker ="zensor.ec" 
topico = "000001" 

client = mqtt.Client("Temperature_Inside")
client.connect(mqttBroker) 

while True:
    randNumber = uniform(20.0, 21.0)
    client.publish("/" + topico + "/temperatura", randNumber)
    print("Se publicaron datos al topico tempratura: " + str(randNumber))
    time.sleep(1)
