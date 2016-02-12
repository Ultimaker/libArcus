# This file is part of libArcus
#
# Copyright (C) 2015 Ultimaker b.v. <a.hiemstra@ultimaker.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

import threading
import struct
import socket
import sys #debugging
import time

##  Raised when an unknown message type is received.
class UnknownMessageError(Exception):
    pass

##  Raised when the message header does not match the excpected header.
class UnknownProtocolError(Exception):
    pass

class InvalidMessageError(Exception):
    pass

##  Threaded socket communication class.
#
#   This class represents a socket and the logic for parsing and handling
#   protobuf messages that can be sent and received over this socket.
#
#   Please see the README in libArcus for more details.
#
#   \note This class only handles protobuf messages. This is by design, it is
#         not meant for dealing with raw data.
class Socket(threading.Thread):
    InitialState = 1
    ConnectingState = 2
    ConnectedState = 3
    OpeningState = 4
    ListeningState = 5
    ClosingState = 6
    ClosedState = 7
    ErrorState = 8

    KeepAliveRate = 0.5 #Number of seconds between keepalive packets

    ProtocolVersionMajor = 0
    ProtocolVersionMinor = 1
    ProtocolSignature = 0x2BAD

    def __init__(self):
        super().__init__()
        self.daemon = True

        self._state = self.InitialState
        self._next_state = self.InitialState

        self._socket = None

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
        self._messageReceivedCallback = None
        self._errorCallback = None

        self._lastKeepAliveSent = time.time()

    ##  Get the current state of the socket.
    def getState(self):
        return self._state

    ##  Set the function that gets called when the socket's state changes.
    #
    #   \param func The callable to call whenever the socket's state changes. It is passed the
    #               new state as first parameter.
    def setStateChangedCallback(self, func):
        self._stateChangedCallback = func

    ##  Set the callable that gets called when a message has been received.
    #
    #   \param func The callable to call whenever a new message has been received.
    #
    #   \note The callable gets no parameters, instead use takeNextMessage() to get the next
    #         message off the received message queue. This is done explicitly to facilitate
    #         separate threads handling the messages.
    def setMessageReceivedCallback(self, func):
        self._messageReceivedCallback = func

    ##  Set the callable that gets called whenever an error occurs.
    #
    #   \param func The callable to call whenever an error occurs. It is passed the error string
    #               as first parameter.
    def setErrorCallback(self, func):
        self._errorCallback = func

    ##  Register a message type to handle.
    #
    #   \note The id should be the same between server and client.
    def registerMessageType(self, id, type):
        self._message_types[id] = type
        self._message_type_mapping[type] = id;

    ##  Listen for connections on a specified address and port.
    #
    #   \param address The IP address to listen on.
    #   \param port The port to listen on.
    def listen(self, address, port):
        self._address = (address, port)
        self._next_state = self.OpeningState
        self.start()

    ##  Connect to an address of a specified address and port.
    #
    #   \param address The IP address to connect to.
    #   \param port The port to connect on.
    def connect(self, address, port):
        if self._state != self.InitialState:
            return

        self._address = (address, port)
        self._next_state = self.ConnectingState
        self.start()

    ##  Close the connection and stop the thread.
    def close(self):
        if self._socket is not None:
            self._socket.close()
        self._next_state = self.ClosingState
        self.join()

    ##  Queue a message to be sent to the other side.
    #
    #   \param message The message to send.
    #
    #   \note Messages will be put on a queue and processed at a later point in time by
    #         a separate thread. No guarantees are given about when and in which order this
    #         happens.
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

    ## private:

    # Reimplemented from threading.Thread.run(), implements the actual connection and message
    # handling logic.
    def run(self):
        while self._state != self.ClosedState and self._state != self.ErrorState:
            if self._state == self.InitialState:
                time.sleep(0.25) #Prevent uninitialized thread from overloading CPU
            elif self._state == self.ConnectingState:
                self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                try:
                    self._socket.connect(self._address)
                except OSError as e:
                    self._next_state = self.ErrorState
                    if self._errorCallback:
                        self._errorCallback(e)
                else:
                    self._socket.settimeout(0.25)
                    self._next_state = self.ConnectedState
            elif self._state == self.OpeningState:
                self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                try:
                    self._socket.bind(self._address)
                except OSError as e:
                    self._next_state = self.ErrorState
                    if self._errorCallback:
                        self._errorCallback(e)
                else:
                    self._next_state = self.ListeningState
            elif self._state == self.ListeningState:
                try:
                    self._socket.listen(1)
                    newSocket, address = self._socket.accept()
                except OSError as e:
                    self._socket.close()
                    self._next_state = self.ErrorState
                    if self._errorCallback:
                        self._errorCallback(e)
                else:
                    self._socket.close()
                    self._socket = newSocket
                    self._socket.settimeout(0.25)

                self._next_state = self.ConnectedState
            elif self._state == self.ClosingState:
                self._socket.close()
                self._next_state = self.ClosedState
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

                    self._checkConnectionState()
                except OSError as e:
                    if self._errorCallback:
                        self._errorCallback(e)
                    # Receiving an OSError here is considered a fatal error
                    self._next_state = self.ClosingState

            if self._next_state != self._state:
                self._state = self._next_state

                if self._stateChangedCallback:
                    self._stateChangedCallback(self._state)

    # Send a message to the connected peer.
    def _sendMessage(self, message):
        self._sendBytes(struct.pack('!i', (self.ProtocolSignature << 16 | self.ProtocolVersionMajor << 8 | self.ProtocolVersionMinor)))
        self._sendBytes(struct.pack('!i', message.ByteSize()))
        self._sendBytes(struct.pack('!i', self._message_type_mapping[type(message)]))
        self._sendBytes(message.SerializeToString())

    # Send a byte array across the socket.
    def _sendBytes(self, data):
        amount_to_send = len(data)
        offset = 0
        while amount_to_send > 0:
            try:
                n = self._socket.send(data[offset:])
                offset += n
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

        header = self._receiveInt32()
        if header == 0:
            return #Keepalive, ignore

        if ((header & 0xffff0000) >> 16) != self.ProtocolSignature:
            if self._errorCallback:
                self._errorCallback(UnknownProtocolError())
            self._next_state = self.ErrorState
            return

        self._message_size = self._receiveInt32()
        if self._message_size < 0:
            if self._errorCallback:
                self._errorCallback(InvalidMessageError("Invalid message size"))
            self._next_state = self.ErrorState
            return

        self._message_type = self._receiveInt32()

        data = self._receiveBytes(self._message_size)
        if type(data) == bool and not data:
            return

        if len(data) != self._message_size:
            self._partial_message = data
            self._amount_received = len(data)
            return

        self._handleMessage(data)

    # Parse message from a bytearray and put it onto the received messages queue.
    def _handleMessage(self, data):
        if not self._message_type in self._message_types:
            if self._errorCallback:
                self._errorCallback(UnknownMessageError("Unknown message type {0}".format(self._message_type)))
            return

        message = self._message_types[self._message_type]()
        if data:
            message.ParseFromString(bytes(data))

        with self._received_queue_lock:
            self._received_queue.append(message)

        if self._messageReceivedCallback:
            self._messageReceivedCallback()

    # Receive an integer from the socket
    def _receiveInt32(self):
        try:
            data = self._socket.recv(4)
            if data:
                return struct.unpack('!i', data)[0]
        except socket.timeout:
            pass

        return False

    # Receive an amount of bytes from the socket and write it into dest.
    def _receiveBytes(self, maxlen = 0):
        try:
            return self._socket.recv(maxlen)
        except socket.timeout:
            pass

        return False

    def _checkConnectionState(self):
        now = time.time()

        if now - self._lastKeepAliveSent > self.KeepAliveRate:
            self._socket.send(struct.pack('!i', 0))
            self._lastKeepAliveSent = now
