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

%Module Arcus

%Include Types.sip
%Include SocketListener.sip

class Socket
{
    %TypeHeaderCode
    #include "Socket.h"
    %End

public:
    Socket();
    virtual ~Socket();

    SocketState::State state() const;

    std::string errorString() const;

    void clearError();

    void addListener(SocketListener* listener /TransferThis/);
    void removeListener(SocketListener* listener);

    void connect(const std::string& address, int port);
    void listen(const std::string& address, int port);
    void close();

    void sendMessage(MessagePtr message);

    MessagePtr takeNextMessage();

    void reset();
};
