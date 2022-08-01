// Copyright (c) 2022 Ultimaker B.V.
// libArcus is released under the terms of the LGPLv3 or higher.

#include "Arcus/SocketListener.h"

#include "Arcus/Socket.h"

using namespace Arcus;

Socket* SocketListener::getSocket() const
{
    return _socket;
}

void SocketListener::setSocket(Socket* socket)
{
    _socket = socket;
}
