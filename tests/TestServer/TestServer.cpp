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

#include "TestServer.h"

#include <thread>
#include <iostream>

#include <google/protobuf/message.h>

#include "Socket.h"
#include "SocketListener.h"

class TestServer::Private
{
public:
    void handleMessage(const Arcus::MessagePtr& message);

    std::shared_ptr<Arcus::Socket> socket;

    std::string address;
    int port;
};

TestServer::TestServer(const std::string& address, int port) : d(new Private)
{
    d->address = address;
    d->port = port;
}

TestServer::~TestServer()
{
}

bool TestServer::run()
{
    d->socket = std::make_shared<Arcus::Socket>();
    d->socket->registerAllMessageTypes("Test.proto");

    std::cout << d->socket->getLastError();

    d->socket->listen(d->address, d->port);

    while(d->socket->getState() != Arcus::SocketState::Connected && d->socket->getState() != Arcus::SocketState::Error)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    while(d->socket->getState() == Arcus::SocketState::Connected)
    {
        auto message = d->socket->takeNextMessage();
        if(message)
        {
            d->handleMessage(message);
        }
    }

    std::cout << d->socket->getLastError();

    return d->socket->getState() == Arcus::SocketState::Closed;
}

void TestServer::Private::handleMessage(const Arcus::MessagePtr& message)
{
    auto new_message = socket->createMessage(message->GetTypeName());
    new_message->CopyFrom(*message);
    socket->sendMessage(new_message);
}
