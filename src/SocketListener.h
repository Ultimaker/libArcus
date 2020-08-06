/*
 * This file is part of libArcus
 *
 * Copyright (C) 2015 Ultimaker b.v. <a.hiemstra@ultimaker.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License v3.0 as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License v3.0 for more details.
 * You should have received a copy of the GNU Lesser General Public License v3.0
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARCUS_SOCKETLISTENER_H
#define ARCUS_SOCKETLISTENER_H

#include "Types.h"

#include "ArcusExport.h"

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
    class ARCUS_EXPORT SocketListener
    {
    public:
        SocketListener() : _socket(nullptr) { }
        virtual ~SocketListener() { }

        /**
         * \return The socket this listener is listening to.
         */
        Socket* getSocket() const;

        /**
         * Called whenever the socket's state changes.
         *
         * \param newState The new state of the socket.
         */
        virtual void stateChanged(SocketState::SocketState newState) = 0;
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
}

#endif // ARCUS_SOCKETLISTENER_H
