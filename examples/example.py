import example_pb2
import Arcus

socket = Arcus.Socket()

socket.registerMessageType(2, example_pb2.ObjectList)
socket.registerMessageType(5, example_pb2.ProgressUpdate)
socket.registerMessageType(6, example_pb2.SlicedObjectList)

def onStateChanged():
    if socket.getState() == Arcus.Socket.ConnectedState:
        print('sending objects')
        msg = example_pb2.ObjectList()
        for i in range(10):
            obj = msg.objects.add()
            obj.id = i
            obj.vertices = b'abcdefghijklmnopqrstuvwxyz'
            obj.normals = b'abcdefghijklmnopqrstuvwxyz'
            obj.indices = b'abcdefghijklmnopqrstuvwxyz'
        socket.sendMessage(msg)

def onMessageAvailable():
    message = socket.takeNextMessage()
    if type(message) is example_pb2.ProgressUpdate:
        print('Progress:', message.amount)

    if type(message) is example_pb2.SlicedObjectList:
        print('Sliced Objects:')
        for obj in message.objects:
            print('  Object(', obj.id, '):')
            for poly in obj.polygons:
                print('    Polygon(', poly.points, ')')

socket.setStateChangedCallback(onStateChanged)
socket.setMessageAvailableCallback(onMessageAvailable)
socket.listen('127.0.0.1', 56789)
