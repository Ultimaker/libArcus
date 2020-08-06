#include <vector>
#include <iostream>
#include <thread>

#include "../src/Socket.h"
#include "../src/SocketListener.h"
#include "../src/Error.h"

#include "example.pb.h"

struct Object
{
public:
    int id;
    std::string vertices;
    std::string normals;
    std::string indices;
};

class Listener : public Arcus::SocketListener
{
public:
    void stateChanged(Arcus::SocketState::SocketState new_state)
    {
        std::cout << "State Changed: " << new_state << std::endl;
    }

    void messageReceived()
    {
    }

    void error(const Arcus::Error& new_error)
    {
        std::cout << new_error << std::endl;
    }
};

std::vector<Object> objects;

void handleMessage(Arcus::Socket& socket, Arcus::MessagePtr message);

int main(int argc, char** argv)
{
    Arcus::Socket socket;

    socket.registerMessageType(&Example::ObjectList::default_instance());
    socket.registerMessageType(&Example::ProgressUpdate::default_instance());
    socket.registerMessageType(&Example::SlicedObjectList::default_instance());

    socket.addListener(new Listener());

    std::cout << "Connecting to server\n";
    socket.connect("127.0.0.1", 56789);

    while(socket.getState() != Arcus::SocketState::Connected)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Connected to server\n";

    while(true)
    {
        if(socket.getState() == Arcus::SocketState::Connected)
        {
            auto message = socket.takeNextMessage();
            if(message)
            {
                handleMessage(socket, message);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        else if(socket.getState() == Arcus::SocketState::Closed || socket.getState() == Arcus::SocketState::Error)
        {
            break;
        }
    }

    socket.close();
    return 0;
}

void handleMessage(Arcus::Socket& socket, Arcus::MessagePtr message)
{
    // (Dynamicly) cast the message to one of our types. If this works (does not return a nullptr), we've found the right type.
    auto objectList = dynamic_cast<Example::ObjectList*>(message.get());
    if(objectList)
    {
        objects.clear();

        std::cout << "Received object list containing " << objectList->objects_size() << " objects" << std::endl;

        for(auto objectDesc : objectList->objects())
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
        for(auto object : objects)
        {
            auto slicedObject = msg->add_objects();
            slicedObject->set_id(object.id);

            for(int i = 0; i < 1000; ++i)
            {
                auto polygon = slicedObject->add_polygons();
                polygon->set_type(i % 2 == 0 ? Example::Polygon_Type_InnerType : Example::Polygon_Type_OuterType);
                polygon->set_points(object.vertices);
            }

            auto update = std::make_shared<Example::ProgressUpdate>();
            update->set_objectid(object.id);
            update->set_amount((float(++progress) / float(objects.size())) * 100.f);
            socket.sendMessage(update);
        }

        std::cout << "Sending SlicedObjectList" << std::endl;
        socket.sendMessage(msg);

        return;
    }
}
