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
