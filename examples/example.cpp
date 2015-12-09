#include <vector>
#include <iostream>
#include <thread>

#include "../src/Socket.h"

#include "example.pb.h"

struct Object
{
public:
    int id;
    std::string vertices;
    std::string normals;
    std::string indices;
};

std::vector<Object> objects;

void handleMessage(Arcus::Socket& socket, Arcus::MessagePtr message);

int main(int argc, char** argv)
{
    Arcus::Socket socket;

    socket.registerMessageType(2, &Example::ObjectList::default_instance());
    socket.registerMessageType(5, &Example::ProgressUpdate::default_instance());
    socket.registerMessageType(6, &Example::SlicedObjectList::default_instance());

    std::cout << "Connecting to server\n";
    socket.connect("127.0.0.1", 56789);

    while (true)
    {
        if (socket.state() == Arcus::SocketState::Connected)
        {
            auto message = socket.takeNextMessage();
            if (message)
            {
                handleMessage(socket, message);
            }
            std::this_thread::sleep_for (std::chrono::milliseconds(250));
        }
        else if (socket.state() == Arcus::SocketState::Closed || socket.state() == Arcus::SocketState::Error)
        {
            if (!socket.errorString().empty())
            {
                std::cout << "An error occurred: " << socket.errorString() << std::endl;
            }
            break;
        }
    }

    return 0;
}

void handleMessage(Arcus::Socket& socket, Arcus::MessagePtr message)
{
    // (Dynamicly) cast the message to one of our types. If this works (does not return a nullptr), we've found the right type.
    auto objectList = dynamic_cast<Example::ObjectList*>(message.get()); 
    if (objectList)
    {
        objects.clear();

        for (auto objectDesc : objectList->objects())
        {
            Object obj;
            obj.id = objectDesc.id();
            obj.vertices = objectDesc.vertices();
            obj.normals = objectDesc.normals();
            obj.indices = objectDesc.indices();
            objects.push_back(obj);
        }

        auto msg = std::make_shared<Example::SlicedObjectList>();
        int progress = 0;
        for (auto object : objects)
        {
            auto slicedObject = msg->add_objects();
            slicedObject->set_id(object.id);

            for (int i = 0; i < 100; ++i)
            {
                auto polygon = slicedObject->add_polygons();
                polygon->set_points("abcdefghijklmnopqrstuvwxyz");
            }

            auto update = std::make_shared<Example::ProgressUpdate>();
            update->set_objectid(object.id);
            update->set_amount((float(++progress) / float(objects.size())) * 100.f);
            socket.sendMessage(update);
        }
        socket.sendMessage(msg);

        return;
    }
}
