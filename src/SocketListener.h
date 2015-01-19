#ifndef ARCUS_SOCKETLISTENER_H
#define ARCUS_SOCKETLISTENER_H

#include "Types.h"

namespace Arcus
{
    class Socket;

    class SocketListener
    {
    public:
        SocketListener();
        virtual ~SocketListener();

        Socket* socket() const;
        void setSocket(Socket* socket);

        void stateChanged();
        void messageReceived();
        void error();

    protected:
        virtual void stateChangedImpl(SocketState::State newState) = 0;
        virtual void messageReceivedImpl(MessagePtr message) = 0;
        virtual void errorImpl(std::string error) = 0;

    private:
        Socket* m_socket;
    };
}

#endif // ARCUS_SOCKETLISTENER_H
