import Arcus

import time
import os.path

class Listener(Arcus.SocketListener):
    def stateChanged(self, state):
        print("Socket state changed:", state)

    def messageReceived(self):
        message = self.socket().takeNextMessage()

        if message.getTypeName() == "Example.ProgressUpdate":
            print("Progress:", message.amount)

        if message.getTypeName() == "Example.SlicedObjectList":
            print("Sliced Objects:", message.repeatedFieldCount("objects"))

    def error(self, error):
        print(error)

print("Creating socket")

socket = Arcus.Socket()

print("Registering message types")

socket.registerAllMessageTypes(os.path.dirname(os.path.abspath(__file__)) + "/example.proto")

print("Creating listener")

listener = Listener()
socket.addListener(listener)

print("Listening for connections on 127.0.0.1:56789")

socket.listen('127.0.0.1', 56789)

while(socket.state() != Arcus.SocketState.Connected):
    time.sleep(0.1)

#time.sleep(5) #Sleep for a bit so the other side can connect

if(socket.state() == Arcus.SocketState.Connected):
    print("Connection established")
else:
    print(socket.state())
    print("Could not establish a connection:", socket.errorString())
    exit(1)

for i in range(10):
    msg = socket.createMessage("Example.ObjectList")

    for i in range(10):
        obj = msg.addRepeatedField("objects")
        obj.id = i
        obj.vertices = b'abcdefghijklmnopqrstuvwxyz'
        obj.normals = b'abcdefghijklmnopqrstuvwxyz'
        obj.indices = b'abcdefghijklmnopqrstuvwxyz'

    socket.sendMessage(msg)

    time.sleep(1)

    if socket.state() != Arcus.SocketState.Connected:
        break

time.sleep(5) #Sleep for a bit more so we can receive replies to what we just sent.

print("Closing connection")

socket.close()
