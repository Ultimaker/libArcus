import Arcus

import time
import os.path

class Listener(Arcus.SocketListener):
    def stateChanged(self, state):
        print("Socket state changed:", state)

    def messageReceived(self):
        message = self.getSocket().takeNextMessage()

        if message.getTypeName() == "Example.ProgressUpdate":
            print("Progress:", message.amount)

        if message.getTypeName() == "Example.SlicedObjectList":
            print("Sliced Objects:", message.repeatedMessageCount("objects"))

            for i in range(message.repeatedMessageCount("objects")):
                obj = message.getRepeatedMessage("objects", i)
                print("  Object ID:", obj.id)
                print("  Polygon Count:", obj.repeatedMessageCount("polygons"))

    def error(self, error):
        print(error)

print("Creating socket")

socket = Arcus.Socket()

print("Registering message types")

path = os.path.dirname(os.path.abspath(__file__)).replace("\\", "/")
if  not socket.registerAllMessageTypes(path + "/example.proto"):
    print("Failed to register messages:", socket.getLastError())


print("Creating listener")

listener = Listener()
socket.addListener(listener)

print("Listening for connections on 127.0.0.1:56789")

socket.listen('127.0.0.1', 56789)

while(socket.getState() != Arcus.SocketState.Connected):
    time.sleep(0.1)

#time.sleep(5) #Sleep for a bit so the other side can connect

if(socket.getState() == Arcus.SocketState.Connected):
    print("Connection established")
else:
    print(socket.getState())
    print("Could not establish a connection:", socket.getLastError())
    exit(1)

for i in range(10):
    msg = socket.createMessage("Example.ObjectList")

    for i in range(100):
        obj = msg.addRepeatedMessage("objects")
        obj.id = i
        obj.vertices = b'abcdefghijklmnopqrstuvwxyz' * 10
        obj.normals = b'abcdefghijklmnopqrstuvwxyz' * 10
        obj.indices = b'abcdefghijklmnopqrstuvwxyz' * 10

    print("Sending message containing", msg.repeatedMessageCount("objects"), "objects")
    socket.sendMessage(msg)

    time.sleep(1)

    if socket.getState() != Arcus.SocketState.Connected:
        print("Socket lost connection, aborting!")
        break

time.sleep(5) #Sleep for a bit more so we can receive replies to what we just sent.

print("Closing connection")

socket.close()
