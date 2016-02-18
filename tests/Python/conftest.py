# Copyright (c) 2016 Ultimaker B.V.
# libArcus is released under the terms of the AGPLv3 or higher.

import pytest
import subprocess

class TestServerRunner:
    def __init__(self):
        self._address = "127.0.0.1"
        self._port = 56789

        self._process = None

    def getAddress(self):
        return self._address

    def getPort(self):
        return self._port

    def start(self):
        self._process = subprocess.Popen(["./TestServer", self._address, str(self._port)])

    def stop(self):
        if self._process:
            self._process.terminate()

@pytest.fixture()
def server(request):
    server = TestServerRunner()
    server.start()

    request.addfinalizer(lambda: server.stop())

    return server
