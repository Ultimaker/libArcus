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

%Module(name = Arcus, call_super_init = True)

%Include Types.sip
%Include SocketListener.sip
%Include PythonMessage.sip
%Include Error.sip

%ModuleHeaderCode
using namespace Arcus;
%End

class Socket
{
    %TypeHeaderCode
    #include "Socket.h"
    %End

public:
    Socket();
    virtual ~Socket();

    SocketState::SocketState getState() const;

    Error getLastError() const;
    void clearError();

    void addListener(SocketListener* listener /TransferThis/);
    void removeListener(SocketListener* listener);

    void connect(const std::string& address, int port);
    void listen(const std::string& address, int port);
    void close() /ReleaseGIL/;
    void reset() /ReleaseGIL/;

    void sendMessage(MessagePtr message);
    MessagePtr takeNextMessage();
    MessagePtr createMessage(const std::string& type_name);

    bool registerAllMessageTypes(const std::string& file_name);

private:
    Socket(const Socket&);
    Socket& operator=(const Socket&);
};

