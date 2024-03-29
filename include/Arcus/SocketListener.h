// Copyright (c) 2022 Ultimaker B.V.
// libArcus is released under the terms of the LGPLv3 or higher.

#ifndef ARCUS_SOCKETLISTENER_H
#define ARCUS_SOCKETLISTENER_H

#include "Arcus/Types.h"

namespace Arcus
{
class Socket;
class Error;

/**
 * Interface for socket event listeners.
 *
 * This interface should be implemented to receive notifications when
 * certain events occur on the socket. The methods of this interface
 * are called from the Socket's worker thread and thus with that thread
 * as current thread. This interface is thus primarily intended as an
 * abstraction to implement your own thread synchronisation.
 *
 * For example, when using the Qt event loop, you could emit a queued
 * signal from a subclass of this class, to make sure the actual event
 * is handled on the main thread.
 */
class SocketListener
{
public:
    SocketListener() : _socket(nullptr)
    {
    }
    virtual ~SocketListener()
    {
    }

    /**
     * \return The socket this listener is listening to.
     */
    Socket* getSocket() const;

    /**
     * Called whenever the socket's state changes.
     *
     * \param newState The new state of the socket.
     */
    virtual void stateChanged(SocketState newState) = 0;
    /**
     * Called whenever a new message has been received and
     * correctly parsed.
     *
     * \note This is explicitly not passed the received message. Instead, it is put
     * on a receive queue so other threads can take care of it.
     */
    virtual void messageReceived() = 0;
    /**
     * Called whenever an error occurs on the socket.
     *
     * \param errorMessage The error message.
     */
    virtual void error(const Error& error) = 0;

private:
    // So we can call setSocket from Socket without making it public interface.
    friend class Socket;

    // Set the socket this listener is listening to.
    // This is automatically called by the socket when Socket::addListener() is called.
    void setSocket(Socket* socket);

    Socket* _socket;
};
} // namespace Arcus

#endif // ARCUS_SOCKETLISTENER_H
