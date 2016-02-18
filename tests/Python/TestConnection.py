
import pytest
import time

import Arcus

class TestConnection:
    def test_basic_connection(self, server):
        socket = Arcus.Socket()
        socket.connect(server.getAddress(), server.getPort())

        time.sleep(1)

        state = socket.getState()
        assert state == Arcus.SocketState.Connected
        assert not socket.getLastError().isValid()

        socket.close()

        time.sleep(1)

        state = socket.getState()
        assert state == Arcus.SocketState.Closed
        assert not socket.getLastError().isValid()
