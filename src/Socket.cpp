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

#include "Socket.h"
#include "Socket_p.h"

#include <algorithm>

using namespace Arcus;

Socket::Socket() : d(new SocketPrivate)
{
}

Socket::~Socket()
{
    if(d->thread)
    {
        if(d->state != SocketState::Closed || d->state != SocketState::Error)
        {
            d->nextState = SocketState::Closing;
            if(d->thread)
            {
                d->thread->join();
                d->thread = nullptr;
            }
        }
        delete d->thread;
    }
}

SocketState::State Socket::state() const
{
    return d->state;
}

std::string Socket::errorString() const
{
    return d->errorString;
}

void Socket::clearError()
{
    d->errorString.clear();
}

bool Socket::registerMessageType(const google::protobuf::Message* message_type)
{
    if(d->state != SocketState::Initial)
    {
        return false;
    }

    return d->message_types.registerMessageType(message_type);
}

bool Socket::registerAllMessageTypes(std::string file_name)
{
    if(file_name.empty())
    {
        return false;
    }

    if(d->state != SocketState::Initial)
    {
        return false;
    }

    return d->message_types.registerAllMessageTypes(file_name);
}

void Socket::addListener(SocketListener* listener)
{
    if(d->state != SocketState::Initial)
    {
        return;
    }

    listener->setSocket(this);
    d->listeners.push_back(listener);
}

void Socket::removeListener(SocketListener* listener)
{
    if(d->state != SocketState::Initial)
    {
        return;
    }

    auto itr = std::find(d->listeners.begin(), d->listeners.end(), listener);
    d->listeners.erase(itr);
}

void Socket::connect(const std::string& address, int port)
{
    if(d->state != SocketState::Initial || d->thread != nullptr)
    {
        return;
    }

    d->address = address;
    d->port = port;
    d->nextState = SocketState::Connecting;
    d->thread = new std::thread([&]() { d->run(); });
}

void Socket::reset()
{
    if (d->state != SocketState::Closed &&
        d->state != SocketState::Error)
        return;

    if(d->thread)
    {
        d->thread->join();
        d->thread = nullptr;
    }

    d->state = SocketState::Initial;
    d->nextState = SocketState::Initial;
    d->errorString = "";
}

void Socket::listen(const std::string& address, int port)
{
    d->address = address;
    d->port = port;
    d->nextState = SocketState::Opening;
    d->thread = new std::thread([&]() { d->run(); });
}

void Socket::close()
{
    d->nextState = SocketState::Closing;
    if(d->thread)
    {
        d->thread->join();
        d->thread = nullptr;
    }
}

void Socket::sendMessage(MessagePtr message)
{
    if(!message)
        return;
    
    std::lock_guard<std::mutex> lock(d->sendQueueMutex);
    d->sendQueue.push_back(message);
}

MessagePtr Socket::takeNextMessage()
{
    std::lock_guard<std::mutex> lock(d->receiveQueueMutex);
    if(d->receiveQueue.size() > 0)
    {
        MessagePtr next = d->receiveQueue.front();
        d->receiveQueue.pop_front();
        return next;
    }
    else
    {
        return nullptr;
    }
}

void Arcus::Socket::dumpMessageTypes()
{
    d->message_types.dumpMessageTypes();
}

MessagePtr Arcus::Socket::createMessage(uint type_id)
{
    return d->message_types.createMessage(type_id);
}

MessagePtr Arcus::Socket::createMessage(const std::string& type)
{
    return d->message_types.createMessage(type);
}
