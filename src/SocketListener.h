/*
 * This file is part of libArcus
 *
 * Copyright (C) 2015 Ultimaker b.v. <a.hiemstra@ultimaker.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARCUS_SOCKETLISTENER_H
#define ARCUS_SOCKETLISTENER_H

#include "Types.h"

namespace Arcus
{
    class Socket;

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
        SocketListener();
        virtual ~SocketListener();

        /**
         * \return The socket this listener is listening to.
         */
        Socket* socket() const;
        /**
         * Set the socket this listener is listening to.
         *
         * This is automatically called by the socket when Socket::addListener() is called.
         */
        void setSocket(Socket* socket);

        /**
         * Called whenever the socket's state changes.
         *
         * \param newState The new state of the socket.
         */
        virtual void stateChanged(SocketState::State newState) = 0;
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
        virtual void error(std::string errorMessage) = 0;

    private:
        Socket* m_socket;
    };
}

#endif // ARCUS_SOCKETLISTENER_H
