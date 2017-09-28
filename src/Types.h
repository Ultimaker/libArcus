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

#ifndef ARCUS_TYPES_H
#define ARCUS_TYPES_H

#include <string>
#include <memory>

namespace google
{
    namespace protobuf
    {
        class Message;
    }
}

namespace Arcus
{
    // Convenience typedef so uint can be used.
    typedef uint32_t uint;
    // Convenience typedef for standard message argument.
    typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

    /**
     * Socket state.
     */
    namespace SocketState
    {
        // Note: Not using enum class due to incompatibility with SIP.
        enum SocketState
        {
            Initial, ///< Created, not running.
            Connecting, ///< Connecting to an address and port.
            Connected, ///< Connected and capable of sending and receiving messages.
            Opening, ///< Opening for incoming connections.
            Listening, ///< Listening for incoming connections.
            Closing, ///< Closing down.
            Closed, ///< Closed, not running.
            Error ///< A fatal error happened that blocks the socket from operating.
        };
    }
}

#endif //ARCUS_TYPES_H
