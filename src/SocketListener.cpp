#include "SocketListener.h"

#include "Socket.h"

using namespace Arcus;

SocketListener::SocketListener()
    : m_socket(nullptr)
{

}

SocketListener::~SocketListener()
{

}

Socket* SocketListener::socket() const
{
    return m_socket;
}

void SocketListener::setSocket(Socket* socket)
{
    m_socket = socket;
}

void SocketListener::stateChanged()
{
    if(m_socket)
    {
        stateChangedImpl(m_socket->state());
    }
}

void SocketListener::messageReceived()
{
    if(m_socket)
    {
        messageReceivedImpl(m_socket->takeNextMessage());
    }
}

void SocketListener::error()
{
    if(m_socket)
    {
        errorImpl(m_socket->errorString());
    }
}
