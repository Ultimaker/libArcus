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

#ifndef ARCUS_TYPES_H
#define ARCUS_TYPES_H

#include <string>

namespace google
{
    namespace protobuf
    {
        class Message;
    }
}

namespace Arcus
{
    typedef google::protobuf::Message* MessagePtr;
    typedef const google::protobuf::Message* ConstMessagePtr;

    namespace SocketState
    {
        /**
        * Socket state.
        */
        enum State
        {
            Initial, ///< Created, not running.
            Connecting, ///< Connecting to an address and port.
            Connected, ///< Connected and capable of sending and receiving messages.
            Opening, ///< Opening for incoming connections.
            Listening, ///< Listening for incoming connections.
            Closing, ///< Closing down.
            Closed, ///< Closed, not running.
            Error ///< An error happened.
        };
    }
}

#endif //ARCUS_TYPES_H
