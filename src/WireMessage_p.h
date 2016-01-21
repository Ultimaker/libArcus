/*
 * This file is part of libArcus
 *
 * Copyright (C) 2016 Ultimaker b.v. <a.hiemstra@ultimaker.com>
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

#ifndef ARCUS_WIRE_MESSAGE_P_H
#define ARCUS_WIRE_MESSAGE_P_H

#include "Types.h"

namespace Arcus
{
    namespace Private
    {
        /**
         * Private class that encapsulates a message being sent over the wire.
         */
        class WireMessage
        {
        public:
            /**
             * Current state of the message.
             */
            enum class MessageState
            {
                Header, ///< Check for the header.
                Size, ///< Check for the message size.
                Type, ///< Check for the message type.
                Data, ///< Get the message data.
                Dispatch ///< Process the message and parse it into a protobuf message.
            };

            inline ~WireMessage()
            {
                if(size > 0 && data)
                {
                    delete[] data;
                }
            }

            // Current message state.
            MessageState state = MessageState::Header;
            // Size of the message.
            uint size = 0;
            // Amount of bytes received so far.
            uint received_size = 0;
            // Is this a potentially valid message?
            bool valid = true;
            // The type of message.
            uint type = 0;
            // The data of the message.
            char* data = nullptr;

            // Return how many bytes are remaining for this message to be complete.
            inline int getRemainingSize() const
            {
                return size - received_size;
            }

            // Allocate data for this message based on size.
            inline void allocateData()
            {
                data = new char[size];
            }

            // Check if the message can be considered complete.
            inline bool isComplete() const
            {
                return received_size >= size;
            }
        };
    }
}

#endif //ARCUS_WIRE_MESSAGE_P_H
