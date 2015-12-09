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
#include <typeinfo>

using namespace Arcus;

Socket::Socket() : d(new SocketPrivate)
{
}

Socket::~Socket()
{
    if (d->thread)
    {
        if (d->state != SocketState::Closed || d->state != SocketState::Error)
        {
            d->next_state = SocketState::Closing;
            if (d->thread)
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
    return d->error_string;
}

void Socket::clearError()
{
    d->error_string.clear();
}

void Socket::registerMessageType(int type, const google::protobuf::Message* message_type)
{
    if (d->state != SocketState::Initial)
    {
        return;
    }

    if (type <= 0)
        throw new std::bad_typeid();

    d->message_types[type] = message_type;
    d->message_type_mapping[message_type->GetDescriptor()] = type;
}

void Socket::addListener(SocketListener* listener)
{
    if (d->state != SocketState::Initial)
    {
        return;
    }

    listener->setSocket(this);
    d->listeners.push_back(listener);
}

void Socket::removeListener(SocketListener* listener)
{
    if (d->state != SocketState::Initial)
    {
        return;
    }

    auto itr = std::find(d->listeners.begin(), d->listeners.end(), listener);
    d->listeners.erase(itr);
}

void Socket::connect(const std::string& address, int port)
{
    if (d->state != SocketState::Initial || d->thread != nullptr)
    {
        return;
    }

    d->address = address;
    d->port = port;
    d->next_state = SocketState::Connecting;
    d->thread = new std::thread([&]() { d->run(); });
}

void Socket::reset()
{
    if (d->state != SocketState::Closed &&
        d->state != SocketState::Error)
        return;

    if (d->thread)
    {
        d->thread->join();
        d->thread = nullptr;
    }

    d->state = SocketState::Initial;
    d->next_state = SocketState::Initial;
    d->error_string = "";
}

void Socket::listen(const std::string& address, int port)
{
    d->address = address;
    d->port = port;
    d->next_state = SocketState::Opening;
    d->thread = new std::thread([&]() { d->run(); });
}

void Socket::close()
{
    d->next_state = SocketState::Closing;
    if (d->thread)
    {
        d->thread->join();
        d->thread = nullptr;
    }
}

void Socket::sendMessage(MessagePtr message)
{
    if (!message)
        return;
    
    std::lock_guard<std::mutex> lock(d->send_queue_mutex);
    d->send_queue.push_back(message);
}

MessagePtr Socket::takeNextMessage()
{
    std::lock_guard<std::mutex> lock(d->receive_queue_mutex);
    if (d->receive_queue.size() > 0)
    {
        MessagePtr next = d->receive_queue.front();
        d->receive_queue.pop_front();
        return next;
    }
    else
    {
        return nullptr;
    }
}
