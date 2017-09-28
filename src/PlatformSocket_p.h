/*
 * This file is part of libArcus
 *
 * Copyright (C) 2016 Ultimaker b.v. <a.hiemstra@ultimaker.com>
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

#ifndef ARCUS_PLATFORM_SOCKET_P_H
#define ARCUS_PLATFORM_SOCKET_P_H

#include <memory>
#include <string>

namespace Arcus
{
    namespace Private
    {
        #ifdef _WIN32
            typedef int socket_size;
        #else
            typedef ssize_t socket_size;
        #endif

        /**
         * Private class that wraps the platform C API for dealing with Sockets.
         */
        class PlatformSocket
        {
        public:
            /**
             * Which connection direction should be shutdown?
             */
            enum class ShutdownDirection
            {
                ShutdownRead, ///< Shutdown reads from the connection.
                ShutdownWrite, ///< Shutdown writes to the connection.
                ShutdownBoth, ///< Shutdown the connection both ways.
            };

            PlatformSocket();
            ~PlatformSocket();

            /**
             * Create the socket.
             *
             * \return true if socket creation was successful, false if not.
             */
            bool create();
            /**
             * Connect to an IP address and port.
             *
             * \param address The IP address to connect to.
             * \param port The port to bind to.
             *
             * \return true if the connection was successful, false if not.
             */
            bool connect(const std::string& address, int port);
            /**
             * Bind the socket to an address and port.
             *
             * \param address The IP address to bind to.
             * \param port The port to bind to.
             *
             * \return true if successful, false if not.
             */
            bool bind(const std::string& address, int port);
            /**
             * Mark the socket as listening for new connections.
             *
             * \param backlog The amount of queued connections to accept.
             *
             * \return true if successful, false if not.
             */
            bool listen(int backlog);
            /**
             * Accept the waiting incoming connection and use it as connected socket.
             *
             * \return true if successful, false if not.
             *
             * \note This call will block until there is a connection waiting to be accepted.
             */
            bool accept();
            /**
             * Close the socket.
             *
             * \return true if successful, false if not.
             */
            bool close();
            /**
             * Shutdown the socket.
             *
             * \param direction The direction to shutdown.
             *
             * \return true if successful, false if not.
             */
            bool shutdown(ShutdownDirection direction);

            /**
             * Flush all waiting data and discard it.
             *
             * This is mostly intended as an error recovery mechanism, if we detect a failure
             * in the messages sent over the wire, we can no longer be sure about the rest of
             * the data on the wire. So rather than trying to figure out if there is an actual
             * message hidden somewhere, just flush all data so we start with a clean slate.
             */
            void flush();

            /**
             * Write an unsigned 32-bit integer to the socket.
             *
             * \param data The integer to write. Will be converted from local endianness to network endianness.
             *
             * \return The amount of bytes written (4) or -1 if an error occurred.
             */
            socket_size writeUInt32(uint32_t data);
            /**
             * Write data to the the socket.
             *
             * \param size The amount of data to write.
             * \param data A pointer to the data to send.
             *
             * \return The amount of bytes written, or -1 if an error occurred.
             */
            socket_size writeBytes(std::size_t size, const char* data);
            /**
             * Read an unsigned 32-bit integer from the socket.
             *
             * \param output A pointer to an integer that will be written to.
             *
             * \return The amount of bytes read (4) or -1 if an error occurred.
             *
             * \note This call will block if the amount of data waiting to be read is less than 4.
             */
            socket_size readUInt32(uint32_t* output);
            /**
             * Read an amount of bytes from the socket.
             *
             * \param size The amount of bytes to read.
             * \param output A pointer to a block of data that can be written to.
             *
             * \return The amount of bytes read or -1 if an error occurred.
             *
             * \note This call will block if the amount of data waiting to be read is less than size.
             */
            socket_size readBytes(std::size_t size, char* output);

            /**
             * Set the timeout for the read-related methods.
             *
             * The readInt32 and readBytes methods will block for a certain amount of time when
             * there is not enough data available. This call will set the maximum amount of time these
             * calls will block.
             *
             * \param timeout The amount of time in milliseconds to wait for data.
             */
            bool setReceiveTimeout(int timeout);
            /**
             * Return the last error code as reported by the underlying platform.
             */
            int getNativeErrorCode();

        private:
            int _socket_id;
        };
    }
}

#endif //ARCUS_PLATFORM_SOCKET_P_H
