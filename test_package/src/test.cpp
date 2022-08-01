#include <Arcus/Error.h>
#include <Arcus/Socket.h>
#include <Arcus/SocketListener.h>
#include <chrono>
#include <iostream>
#include <thread>

#include "test.h"

constexpr int sleep_msec{ 500 };
constexpr uint16_t port{ 44444 };
const std::string ip{ "127.0.0.1" };

int num_messages_received = 0;

class Listener : public Arcus::SocketListener
{
public:
    Listener() : Arcus::SocketListener(), id(++PREV_ID)
    {
    }

    void stateChanged(Arcus::SocketState state) override
    {
        std::cerr << id << " SocketState: " << static_cast<int>(state) << std::endl;
    }

    void messageReceived() override
    {
        ++num_messages_received;
        std::cerr << id << " MesageReceived" << std::endl;
    }

    void error(const Arcus::Error& error) override
    {
        std::cerr << id << " " << ((error.getErrorCode() == Arcus::ErrorCode::Debug) ? "Debug: " : "Error: ") << error.getErrorMessage().c_str() << std::endl;
    }

private:
    int id;
    inline static int PREV_ID = -1;
};

Arcus::Socket* newSocket()
{
    auto socket = new Arcus::Socket;
    socket->registerMessageType(&test::proto::Progress::default_instance());
#ifdef ARCUS_DEBUG
    socket->dumpMessageTypes();
#endif // ARCUS_DEBUG
    socket->addListener(new Listener);
    return socket;
}

Arcus::Socket* connectSend()
{
    auto socket = newSocket();
    socket->connect(ip, port);

    Arcus::SocketState socket_state;
    do
    {
        std::cerr << ".";
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_msec));

        socket_state = socket->getState();
    } while (socket_state != Arcus::SocketState::Connected && socket_state != Arcus::SocketState::Error);

    if (socket_state != Arcus::SocketState::Connected)
    {
        // socket->close();
        // delete socket;  // --> FIXME?: Something goes wrong here, even when closed.
        return nullptr;
    }

    return socket;
}

void receive()
{
    std::cerr << "Start reviever." << std::endl;
    auto socket_a = newSocket();
    socket_a->listen(ip, port);
}

int main(int argc, char** argv)
{
#ifdef NDEBUG
    std::cerr << "Tests For Arcus -- release\n";
#else
    std::cerr << "Tests For Arcus -- debug\n";
#endif

    // Start thread to receive.
    receive();

    // Send messages to other thread via protobuf.
    auto socket = connectSend();
    if (socket == nullptr)
    {
        std::cerr << "Failed to connect." << std::endl;
        return 2;
    }
    std::cerr << "Connected." << std::endl;

    // Send a number of messages.
    constexpr int total = 10;
    constexpr int inc = 2;
    constexpr int should_receive = total / inc;
    for (int progress = 1; progress <= total; progress += inc)
    {
        auto message = std::make_shared<test::proto::Progress>();
        message->set_amount(progress);
        socket->sendMessage(message);

        std::cerr << ".";
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_msec));
    }

    // Check result.
    std::cout << num_messages_received << std::endl;
    std::cout << should_receive << std::endl;
    return num_messages_received == should_receive ? 0 : 1;
}
