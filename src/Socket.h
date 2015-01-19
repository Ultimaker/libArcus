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

#ifndef ARCUS_SOCKET_H
#define ARCUS_SOCKET_H

#include <memory>

#include "Types.h"

namespace Arcus
{
    class SocketListener;
    class SocketPrivate;

    /**
    * \brief Threaded socket class.
    *
    *
    */
    class Socket
    {
    public:
        Socket();
        virtual ~Socket();

        // Copy and assignment is not supported.
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket& other) = delete;

        /**
        * Get the socket state.
        *
        * \return The current socket state.
        */
        SocketState::State state() const;

        /**
         * Get the last error string.
         *
         * \return 
         */
        std::string errorString() const;

        /**
        * Register a new type of Message to handle.
        *
        * \param type An integer ID to use to identify the message.
        * \param messageType An instance of the Message that will be used as factory object.
        *
        * \note The `type` parameter should be the same both on the sender and receiver side.
        * It is used to identify the messages when sent across the wire.
        */
        void registerMessageType(int type, ConstMessagePtr messageType);

        /**
        * Add a listener object that will be notified of socket events.
        *
        * \param listener The listener to add.
        */
        void addListener(SocketListener* listener);
        /**
        * Remove a listener from the list of listeners.
        *
        * \param listener The listener to remove.
        */
        void removeListener(SocketListener* listener);

        /**
        * Connect to an address and port.
        *
        * \param address The IP address to connect to.
        * \param port The port to connect to.
        */
        void connect(const std::string& address, int port);
        /**
        * Listen for connections on an address and port.
        *
        * \param address The IP address to listen on.
        * \param port The port to listen on.
        */
        void listen(const std::string& address, int port);
        /**
        * Close the connection and stop handling any messages.
        */
        void close();

        /**
        * Send a message across the socket.
        */
        void sendMessage(MessagePtr message);

        /**
        * Remove the next pending message from the queue.
        */
        MessagePtr takeNextMessage();

    private:
        const std::unique_ptr<SocketPrivate> d;
    };
}

#endif // ARCUS_SOCKET_H
