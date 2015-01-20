import threading
import struct
import socket
import time

##  Raised when an unknown message type is received.
class UnknownMessageError(Exception):
    pass

##  Threaded socket communication class.
#
#
class Socket(threading.Thread):
    InitialState = 1
    ConnectingState = 2
    ConnectedState = 3
    OpeningState = 4
    ListeningState = 5
    ClosingState = 6
    ClosedState = 7

    def __init__(self):
        super().__init__()

        self._state = self.InitialState
        self._next_state = self.InitialState

        self._server_socket = None
        self._data_socket = None

        self._message_type = -1
        self._message_size = 0
        self._partial_message = None
        self._amount_received = 0

        self._send_queue = []
        self._send_queue_lock = threading.Lock()

        self._received_queue = []
        self._received_queue_lock = threading.Lock()

        self._message_types = {}
        self._message_type_mapping = {}

        self._stateChangedCallback = None
        self._messageAvailableCallback = None
        self._errorCallback = None

    ##  Get the current state of the socket.
    def getState(self):
        return self._state

    def setStateChangedCallback(self, func):
        self._stateChangedCallback = func

    def setMessageAvailableCallback(self, func):
        self._messageAvailableCallback = func

    def setErrorCallback(self, func):
        self._errorCallback = func

    ##  Register a message type to handle.
    def registerMessageType(self, id, type):
        self._message_types[id] = type
        self._message_type_mapping[type] = id;

    ##  Listen for connections on a specified address and port.
    def listen(self, address, port):
        self._address = (address, port)
        self._next_state = self.OpeningState
        self.start()

    ##  Connect to an address of a specified address and port.
    def connect(self, address, port):
        self._address = (address, port)
        self._next_state = self.ConnectingState
        self.start()

    ##  Close the connection and stop the thread.
    def close(self):
        self._next_state = self.ClosingState

    ##  Queue a message to be sent to the other side.
    def sendMessage(self, message):
        with self._send_queue_lock:
            self._send_queue.append(message)

    ##  Take the next available message from the received message queue.
    #
    #   \return The next available message or False if the queue is empty.
    def takeNextMessage(self):
        with self._received_queue_lock:
            if not self._received_queue:
                return False
            return self._received_queue.pop(0)

    ##  Reimplemented from threading.Thread.run()
    def run(self):
        while True:
            if self._state == self.InitialState:
                time.sleep(0.25) #Prevent uninitialized thread from overloading CPU
            elif self._state == self.ConnectingState:
                self._data_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self._data_socket.connect(self._address)
                self._data_socket.settimeout(1.0)

                self._next_state = self.ConnectedState
            elif self._state == self.OpeningState:
                self._server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self._server_socket.bind(self._address)

                self._next_state = self.ListeningState
            elif self._state == self.ListeningState:
                print("listening on", self._address)
                self._server_socket.listen(1)
                self._data_socket, address = self._server_socket.accept()
                print("connected on", address)
                self._data_socket.settimeout(1.0)

                self._next_state = self.ConnectedState
            elif self._state == self.ClosingState:
                self._server_socket.close()
                self._data_socket.close()
                self._state = self.ClosedState
                break; # Exit the infinite loop.
            elif self._state == self.ConnectedState:
                self._send_queue_lock.acquire()
                messages_to_send = []
                for message in self._send_queue:
                    messages_to_send.append(self._send_queue.pop(0))
                self._send_queue_lock.release()

                try:
                    for message in messages_to_send:
                        self._sendMessage(message)

                    self._receiveNextMessage()
                except OSError as e:
                    print(e)

            if self._next_state != self._state:
                self._state = self._next_state

                if self._stateChangedCallback:
                    self._stateChangedCallback(self._state)

    ## private:

    #
    def _sendMessage(self, message):
        self._sendBytes(struct.pack('!i', self._message_type_mapping[type(message)]))
        self._sendBytes(struct.pack('!i', message.ByteSize()))
        self._sendBytes(message.SerializeToString())

    # Send a byte array across the socket.
    def _sendBytes(self, data):
        amount_to_send = len(data)
        while amount_to_send > 0:
            try:
                n = self._data_socket.send(data)
                amount_to_send -= n
            except socket.timeout:
                continue

    # Try and receive the next message.
    def _receiveNextMessage(self):
        # Handle continuation of message receive
        if self._partial_message:
            data = self._receiveBytes(self._message_size - self._amount_received)
            self._partial_message += data
            self._amount_received += len(data)
            if self._amount_received >= self._message_size:
                self._handleMessage(self._partial_message)
                self._message_size = 0
                self._partial_message = None
            else:
                return

        self._message_type = self._receiveUInt32()
        if self._message_type == -1:
            return

        self._message_size = self._receiveUInt32()
        if self._message_size == -1:
            return

        data = self._receiveBytes(self._message_size)
        if len(data) != self._message_size:
            self._partial_message = data
            self._amount_received = len(data)
            return

        self._handleMessage(data)

    # Parse message from a bytearray and put it onto the received messages queue.
    def _handleMessage(self, data):
        if not self._message_type in self._message_types:
            raise UnknownMessageError("Unknown message type {0}".format(self._message_type))

        message = self._message_types[self._message_type]()
        message.ParseFromString(bytes(data))

        with self._received_queue_lock:
            self._received_queue.append(message)

        if self._messageAvailableCallback:
            self._messageAvailableCallback()

    # Receive an integer from the socket
    def _receiveUInt32(self):
        try:
            data = self._data_socket.recv(4)
            if data:
                return struct.unpack('@i', data)[0]
        except socket.timeout:
            pass

        return -1

    # Receive an amount of bytes from the socket and write it into dest.
    def _receiveBytes(self, maxlen = 0):
        try:
            return self._data_socket.recv(maxlen)
        except socket.timeout:
            pass
