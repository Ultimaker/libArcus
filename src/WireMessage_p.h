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

#ifndef ARCUS_WIRE_MESSAGE_H
#define ARCUS_WIRE_MESSAGE_H

#include "Types.h"

namespace Arcus
{
    class WireMessage
    {
    public:
        enum MessageState
        {
            MessageStateHeader,
            MessageStateSize,
            MessageStateType,
            MessageStateData,
            MessageStateDispatch
        };

        inline ~WireMessage()
        {
            if(_size > 0 && _data)
            {
                delete[] _data;
            }
        }

        inline MessageState getState() const
        {
            return _state;
        }

        inline void setState(MessageState state)
        {
            _state = state;
        }

        inline uint getSize() const
        {
            return _size;
        }

        inline void setSize(uint size)
        {
            _size = size;
        }

        inline bool isValid() const
        {
            return _valid;
        }

        inline void setValid(bool valid)
        {
            _valid = valid;
        }

        inline uint getType() const
        {
            return _type;
        }

        inline void setType(uint type)
        {
            _type = type;
        }

        inline int getRemainingSize() const
        {
            return _size - _size_received;
        }

        inline void allocateData()
        {
            _data = new char[_size];
        }

        inline char* getData() const
        {
            return _data;
        }

        inline uint getSizeReceived() const
        {
            return _size_received;
        }

        inline void setSizeReceived(uint size)
        {
            _size_received = size;
        }

        inline bool isComplete() const
        {
            return _size_received >= _size;
        }

    private:
        MessageState _state = MessageStateHeader;
        uint _type = 0;
        uint _size = 0;
        uint _size_received = 0;
        bool _valid = true;
        char* _data = nullptr;
    };
}

#endif //ARCUS_WIRE_MESSAGE_H
